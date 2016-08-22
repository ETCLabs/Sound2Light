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

#ifndef FFTWRAPPER_H
#define FFTWRAPPER_H

#include "BasicFFTInterface.h"
#include "ScaledSpectrum.h"
#include "TriggerGeneratorInterface.h"
#include "MonoAudioBuffer.h"

#include <QObject>
#include <QtMath>
#include <QVector>
#include <QDebug>

// the number of samples used for FFT expressed as an exponent of 2
static const int NUM_SAMPLES_EXPONENT = 12;

// the real number of samples calculated from NUM_SAMPLES_EXPONENT
static const int NUM_SAMPLES = qPow(2, NUM_SAMPLES_EXPONENT);

// maximum absolute value in the result of the FFT
// estimated from previous tests (exponent/samples: max value):
// 11/2048: 51, 12/4096: 96, 13/8192: 195, 14/16384: 350
static const int MAX_FFT_VALUE = 96;

// number of frequency bins in the resulting ScaledSpectrum
static const int SCALED_SPECTRUM_LENGTH = 200;

// base frequency of the ScaledSpectrum in Hz
static const int SCALED_SPECTRUM_BASE_FREQ = 20;  // ms

// A class to prepare the content of an audio buffer for FFT,
// calculate the FFT and create a ScaledSpectrum of the results.
// Calls checkForTrigger() of a TriggerGeneratorContainer object when a new FFT is done.
class FFTAnalyzer
{

public:
	explicit FFTAnalyzer(const MonoAudioBuffer& buffer, QVector<TriggerGeneratorInterface*>& m_triggerContainer);
	~FFTAnalyzer();

	// calculates the FFT based on the data in the inputBuffer and updates the ScaledSpectrum
	// - usually called periodically 44 times per second
    void calculateFFT(bool lowSoloMode);

	// returns the normalized spectrum of the ScaledSpectrum
	const QVector<float>& getNormalizedSpectrum() const { return m_scaledSpectrum.getNormalizedSpectrum(); }

	// returns a const ScaledSpectrum reference to read the last FFT results
	const ScaledSpectrum& getScaledSpectrum() const { return m_scaledSpectrum; }

	// returns a modifiable ScaledSpectrum reference to change its parameters
	ScaledSpectrum& getScaledSpectrum() { return m_scaledSpectrum; }

protected:
	// calculates a Hann Window for FFT and saves it to m_window
	void calculateWindow();

	const MonoAudioBuffer&	m_inputBuffer;  // buffer that stores the audio samples
	QVector<TriggerGeneratorInterface*>& m_triggerContainer;  // list of all controlled triggerGenerators
	BasicFFTInterface*		m_fft;  // FFT implementation
	QVector<float>			m_buffer;  // buffer for prepared data (intermediate result)
	QVector<float>			m_window;  // array with window data
	QVector<float>			m_fftOutput;  // buffer containing the FFT output (intermediate result)
	QVector<float>			m_linearSpectrum;  // buffer containing the non-scaled spectrum data (intermediate result)
	ScaledSpectrum			m_scaledSpectrum;  // stores the scaled data of the spectrum
};

#endif // FFTWRAPPER_H
