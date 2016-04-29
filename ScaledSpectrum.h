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

#ifndef SPECTRUM_H
#define SPECTRUM_H

#include "utils.h"
#include "QCircularBuffer.h"

#include <QVector>

// ----------------- AGC Constants -----------------

// AGC = Automatic Gain Control

// number of samples to average for the AGC calculation
static const int AGC_AVERAGING_LENGTH = 44*2;  // samples

// headroom to leave when using AGC [0...1]
static const float AGC_HEADROOM = 0.1;  // 10%

// min value for AGC to be active == max value of noise [0...1]
static const float AGC_NOISE_THRESHOLD = 0.1; // 10%

// amount to increase the gain per frame when gain is too low
// i.e. 1 / <frames needed to increase the gain from 0 to 1>
static const float AGC_INCREMENT_STEPSIZE = 1.0 / (3*44);  // 3s * 44fps

// amount to decrease the gain per frame when gain is too high
// i.e. 1 / <frames needed to decrease the gain from 1 to 0>
static const float AGC_DECREMENT_STEPSIZE = 1.0 / (1*44);  // 1s * 44fps

// minimum gain of AGC
static const float AGC_MIN_GAIN = 0.5;

// maximum gain of AGC
static const float AGC_MAX_GAIN = 5.0;


// This class represents a spectrum that is scaled
// in frequency scale and energy scale to logarithmic values.
// It also implements Gain and Automatic Gain Control,
// when it is updated with new values by updateWithLinearSpectrum().
class ScaledSpectrum
{

public:
	explicit ScaledSpectrum(const int& m_baseFreq, const int& m_scaledLength);

	// returns the factor used to multiply the results of the FFT with
	float getGain() const { return m_gain; }
	// sets the factor used to multiply the scaled energies with
	// - reasonable values are 0.5...5
	void setGain(const float& value) { m_gain = limit(0.01F, value, 100.0F); }

	// returns the compression factor to be used to scale the output of the FFT
	float getCompression() const { return m_compression; }
	// sets the compression factor to be used for an additional scale of the energies
	// - reasonable values are 0.3...5
	void setCompression(const float& value) { m_compression = limit(0.01F, value, 10.0F); }

	// returns if energy values will be converted to dB values
	bool getDecibelConversion() const { return m_convertToDecibel; }
	// set if energy values should be converted to dB values
	void setDecibelConversion(bool value) { m_convertToDecibel = value; }

	// returns if the AGC is enabled
	bool getAgcEnabled() const { return m_agcEnabled; }
	// sets if the AGC is enabled
	void setAgcEnabled(bool value) { m_agcEnabled = value; }

	// Scales the incoming linear spectrum to a logarithmic spectrum.
	// Results will be written in dbSpectrum and normSpectrum.
	void updateWithLinearSpectrum(const QVector<float> linearSpectrum);

	// returns a normalized spectrum (energy value from 0 to 1)
	// This spectrum is scaled by both factor and exponent.
	const QVector<float>& getNormalizedSpectrum() const { return m_normSpectrum; }

	// returns the index in the scaled spectrum for a certain frequency
	int getIndexForFreq(const int& freq) const;

	// returns the frequency at a position on the scaled freq-axis in range 0...1
	double getFreqAtPosition(const double& value) const;

	// returns the energy level of a certain frequency
	float getLevelAtFreq(const int& freq) const { return m_normSpectrum[getIndexForFreq(freq)]; }

	// returns the max level within a frequency band
	float getMaxLevel(const int& midFreq, const qreal& width) const;

	// returns the overall max level
	float getMaxLevel() const;

private:
	// calculates the required gain and changes the actual gain in small steps
	// based on the last maximum values of the FFT
	void updateAGC();

protected:
	const int		m_baseFreq;  // lowest frequency of input data
	const int		m_scaledLength;  // resulting number of frequency bins after scaling
	qreal			m_freqScaleFactor;  // internal factor used to calculate the scaled frequencies
	qreal			m_logOfFreqScaleFactor;  // log of m_freqScaleFactor (often used in calculations)
	float			m_gain;  // Gain factor
	float			m_compression;  // Compression factor (the higher it is the more the energy values get compressed)
	bool			m_convertToDecibel;  // true if the energy values should be converted to dB
	QVector<float>	m_normSpectrum;  // stores the spectrum with energy values between 0 and 1
	bool			m_agcEnabled;  // true if AGC is enabled
	Qt3DCore::QCircularBuffer<float> m_lastMaxValues;  // list of last maximum energy values used for AGC
};

#endif // SPECTRUM_H
