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

#ifndef BPMDETECTOR_H
#define BPMDETECTOR_H

#include "BasicFFTInterface.h"
#include "ScaledSpectrum.h"
#include "MonoAudioBuffer.h"
#include "BPMOscControler.h"

#include "QCircularBuffer.h"
#include <QtMath>
#include <QVector>
#include <QLinkedList>
#include <QColor>

// Rate to calculate the BPM (significantly lower than the sampling period,
// but still only quater the buffer length, so this should be fine)
static const int BPM_UPDATE_RATE = 20; // Hz

// A class to modell a cluster of Inter Offset Intervalls (IOIs).
class BeatString;


// A class to process the contents of an audio buffer to detect its BPM
class BPMDetector
{
public:
    explicit BPMDetector(const MonoAudioBuffer& buffer, BPMOscControler* osc);
    ~BPMDetector();

    void resetCache(); // to be called when the bpm detection is restarted after a pause, to remove old data from the buffer

    void detectBPM(); // recalculates the BPM considering the newest data in the buffer

    float getBPM() { return m_bpm; } // returns the detected BPM

    bool bpmIsOld() { return m_framesSinceLastBPMDetection / BPM_UPDATE_RATE > 5; } // returns wether the last time a value detected was longer than five seconds ago

    void setMinBPM(int value); // Sets the minimum bpm of the range

    int getMinBPM() { return m_minBPM; } // Returns the minium bpm of the range

    void setTransmitBpm(bool value) { m_transmitBpm = value; }

    // Helper functions to display a nice GUI
    const QVector<bool>& getOnsets() { return m_onsetBuffer; }
    const Qt3DCore::QCircularBuffer<float>& getWaveDisplay() { return m_spectralFluxBuffer; }
    const Qt3DCore::QCircularBuffer<QColor>& getWaveColors() { return m_waveColors; }

protected:
    // calculates a Hann Window for FFT and saves it to m_window
    void calculateWindow();

    // updates the arrays of spectral flux values
    void updateSpectralFluxes(int from);

    // performs onset recognition
    void updateOnsets();

    // categorize intervalls between the onsets into clusters
    void updateStrings();

    // helper function for the evaluation
    BeatString* plausibleStringForInterval(float interval, float maxScore);

    // find the cluster with the highest
    void evaluateStrings();

    const MonoAudioBuffer&              m_inputBuffer; // buffer that stores the audio samples
    int64_t                             m_lastInputBufferNumSamples; // the number of samples ever put into the buffer when last getting data from there
    int                                 m_refreshesSinceCalculation; // used to calculate the bpm every n-th call
    float                               m_bpm; // the detected bpm
    int                                 m_framesSinceLastBPMDetection; // time since the bpm has last changed in frames
    int                                 m_minBPM; // the minimum bpm that sets the range of possible bpms as min to 2*min. That solves the 60 vs 120 BPM debate
    BasicFFTInterface*                  m_fft; // FFT implementation
    QVector<float>                      m_window; // array with window data
    QVector<bool>                       m_onsetBuffer; // a boolen buffer indicating wether there was a onset i frames ago
    Qt3DCore::QCircularBuffer<float>    m_spectralFluxBuffer; // a float buffer caching the spectral flux of the bands of the last frames
    QVector<float>                      m_spectralFluxNormalized; // a vector to copy the normalized spectral flux data into
    Qt3DCore::QCircularBuffer<QColor>   m_waveColors; // the color for each sample to give spectral information in the GUI
    QVector<float>                      m_buffer;  // buffer for prepared data (intermediate result)
    QVector<float>                      m_fftOutput; // buffer for FFT Data
    QVector<float>                      m_currentSpectrum; // the spectrum currently being calculated
    QVector<float>                      m_lastSpectrum; // the spectrum calculated in the last frame for calculating the spectral flux, which is a difference
    QLinkedList<BeatString>             m_beatStrings; // the IOI Clusters identified from the intervalls
    Qt3DCore::QCircularBuffer<float>    m_lastIntervals; // the last bpm values stored as their interval, to achieve smoothing
    float                               m_lastWinningInterval; // the last outputed bpm as an interval before doubling/halfing
    bool                                m_transmitBpm;  // true if the BPM should be transmitted via OSC

    BPMOscControler*                    m_oscController; // the object respoinsible for handling osc output
};

#endif // BPMDETECTOR_H
