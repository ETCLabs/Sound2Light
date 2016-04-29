// Copyright (c) 2016 Electronic Theatre Controls, Inc., http://www.etcconnect.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include "ScaledSpectrum.h"

#include <QtMath>
#include <QDebug>

#include "FFTAnalyzer.h"

ScaledSpectrum::ScaledSpectrum(const int &baseFreq, const int &scaledLength)
    : m_baseFreq(baseFreq)
    , m_scaledLength(scaledLength)
    , m_freqScaleFactor(0)
	, m_gain(1)
	, m_compression(1)
	, m_convertToDecibel(false)
    , m_normSpectrum(scaledLength)
	, m_agcEnabled(true)
	, m_lastMaxValues(AGC_AVERAGING_LENGTH)
{
    // freqScaleFactor is a constant that is used in for-loop in updateWithLinearSpectrum
    // to calculate the next frequency in logarithmic scale:
    m_freqScaleFactor = qPow(22050.0 / baseFreq, 1./scaledLength);

    // logOfFreqScaleFactor is a constant used in getIndexForFreq
    // to convert a frequency back to the index in the logarithmic array:
	m_logOfFreqScaleFactor = qLn(22050. / baseFreq) / scaledLength;

	// initialize lastMaxValues with 0:
	for (int i=0; i<m_lastMaxValues.size(); ++i) {
		m_lastMaxValues.push_back(0.0f);
	}
}

void ScaledSpectrum::updateWithLinearSpectrum(const QVector<float> linearSpectrum)
{
    const int linearLength = linearSpectrum.size();
	double freq = m_baseFreq;
	float maxValue = 0;


    // This for-loop generates frequencies so that there are equally many steps between every octave of frequencies:
    // (There are as many steps between 100Hz and 200Hz as between 400Hz and 800Hz.)
    for (int i = 0; i<m_scaledLength; ++i) {
        // calculate begin and end frequency of this step:
        double nextFreq = m_baseFreq * qPow(m_freqScaleFactor, i+1);
        const std::size_t startIndex = freq / 22050 * (linearLength);
        const std::size_t endIndex = nextFreq / 22050 * (linearLength);
        const std::size_t valuesTillNext = endIndex - startIndex;
        freq = nextFreq;

        // Sum up the energies of all FFT elements betwen the two frequencies:
        float energy = linearSpectrum[startIndex];
        for (std::size_t j=1; j < valuesTillNext; ++j) {
            energy += linearSpectrum[startIndex+j];
        }

        // Maximum of FFT is sqrt(NUM_SAMPLES)
        // in this case: sqrt(2048) = 45.2548339959

		//const float maxPossibleEnergy = (MAX_FFT_VALUE*qMax(std::size_t(1), valuesTillNext));
		const float maxPossibleEnergy = MAX_FFT_VALUE;

        if (m_convertToDecibel) {
            // Convert energy to dB:
			float dB = 20 * qLn(energy / maxPossibleEnergy) / qLn(10);
			float valueBeforeGain = (dB + 60) / 60;
			maxValue = qMax(maxValue, valueBeforeGain);

            // Scale the value with factor and exponent:
			m_normSpectrum[i] = qPow(qMax(0.0f, qMin(valueBeforeGain * m_gain, 1.0f)), (1 / m_compression));
        } else {
            // Apply reasonable scale:
			energy /= maxPossibleEnergy;
			maxValue = qMax(maxValue, energy);
			energy *= m_gain;

            // Scale the value with factor and exponent:
			m_normSpectrum[i] = qPow(qMax(0.0f, qMin(energy, 1.0f)), (1 / m_compression));
		}
    }
	// add maximum value to circular buffer:
	m_lastMaxValues.push_back(maxValue);
	updateAGC();
}

int ScaledSpectrum::getIndexForFreq(const int &freq) const
{
    // convert a frequency back to the index in the logarithmic array:
	return qMax(0.0, qMin(qLn(qreal(freq) / m_baseFreq) / m_logOfFreqScaleFactor, m_scaledLength - 1.0));
}

double ScaledSpectrum::getFreqAtPosition(const double &value) const
{
	double freq = m_baseFreq * qPow(m_freqScaleFactor, value * m_scaledLength);
	return freq;
}

float ScaledSpectrum::getMaxLevel(const int &midFreq, const qreal &width) const
{
	int midIndex = getIndexForFreq(midFreq);
	int startIndex = qMax(0, qMin(int(midIndex - m_scaledLength*width/2), m_scaledLength - 1));
	int endIndex = qMax(0, qMin(int(midIndex + m_scaledLength*width/2), m_scaledLength - 1));
	if (endIndex == startIndex) ++endIndex;
    // get max level between both indexes:
    float max = 0.0;
    for (int i=startIndex; i<=endIndex; ++i) {
        max = qMax(m_normSpectrum[i], max);
    }
    return max;
}

float ScaledSpectrum::getMaxLevel() const
{
    float max = 0.0;
    for (int i=0; i<m_scaledLength; ++i) {
        max = qMax(m_normSpectrum[i], max);
    }
	return max;
}

void ScaledSpectrum::updateAGC()
{
	if (!m_agcEnabled) return;

	// get max of last values:
	float maxValue = 0.0;
	for (int i=0; i<m_lastMaxValues.size(); ++i) {
		maxValue = qMax(maxValue, m_lastMaxValues[i]);
	}

	// check if maxValue is below noise threshold:
	if (maxValue < AGC_NOISE_THRESHOLD || maxValue <= 0) {
		// do not change current gain
		return;
	}

	// calculate required gain:
	float requiredGain = (1 - AGC_HEADROOM) / maxValue;

	// adjust gain in small steps:
	if (requiredGain < m_gain) {
		m_gain = qMax(AGC_MIN_GAIN, qMax(requiredGain, m_gain - AGC_DECREMENT_STEPSIZE));
	} else {
		m_gain = qMin(AGC_MAX_GAIN, qMin(requiredGain, m_gain + AGC_INCREMENT_STEPSIZE));
	}
}
