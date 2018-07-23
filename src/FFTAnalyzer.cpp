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

#include "FFTAnalyzer.h"

#include "FFTRealWrapper.h"

FFTAnalyzer::FFTAnalyzer(const MonoAudioBuffer& buffer,QVector<TriggerGeneratorInterface*>& triggerContainer)
	: m_inputBuffer(buffer)
	, m_triggerContainer(triggerContainer)
	, m_fft(0)
	, m_buffer(NUM_SAMPLES)
	, m_window(NUM_SAMPLES)
	, m_fftOutput(NUM_SAMPLES)
	, m_linearSpectrum(NUM_SAMPLES / 2)
	, m_scaledSpectrum(SCALED_SPECTRUM_BASE_FREQ, SCALED_SPECTRUM_LENGTH)
{
	m_fft = (BasicFFTInterface*) new FFTRealWrapper<NUM_SAMPLES_EXPONENT>();
	calculateWindow();
}

FFTAnalyzer::~FFTAnalyzer()
{
	delete m_fft;
}

void FFTAnalyzer::calculateWindow()
{
	// Hann Window function
	// used to prepare the PCM data for FFT
	for (int i=0; i<NUM_SAMPLES; ++i) {
		m_window[i] = 0.5f * (1 - qCos((2 * M_PI * i) / (NUM_SAMPLES - 1)));
	}
}

void FFTAnalyzer::calculateFFT(bool lowSoloMode)
{
	// apply window:
	int bufferOffset = m_inputBuffer.getCapacity() - NUM_SAMPLES;
	for (int i=0; i < NUM_SAMPLES; ++i) {
		m_buffer[i] = m_inputBuffer.at(i + bufferOffset) * m_window[i];
	}

	// apply FFT:
	m_fft->doFft(m_fftOutput.data(), m_buffer.constData());

	// convert complex output of FFT to real numbers:
	for (int i=0; i < NUM_SAMPLES / 2; ++i) {
		const float real = m_fftOutput[i];
		const float img = m_fftOutput[NUM_SAMPLES / 2 + i];
		const float energy = qSqrt(real*real + img*img) / 10;
		m_linearSpectrum[i] = energy;
	}
	// first value is 0Hz / DC value and is not usefull:
	m_linearSpectrum[0] = 0.0;

	// give linear spectrum to ScaledSpectrum object to be scalled:
	m_scaledSpectrum.updateWithLinearSpectrum(m_linearSpectrum);

    // next element in processing chain: TriggerGenerators
    bool triggered = false;
    for (int i=0; i<m_triggerContainer.size(); ++i) {
        TriggerGeneratorInterface* trigger = m_triggerContainer[i];
        if (trigger->isBandpass()) {
            bool active = trigger->checkForTrigger(m_scaledSpectrum, lowSoloMode && triggered);
            triggered = triggered || active;
        } else {
            trigger->checkForTrigger(m_scaledSpectrum, false);
        }
    }
}
