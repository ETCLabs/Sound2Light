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

#include "BPMDetector.h"

#include "FFTRealWrapper.h"
#include "QCircularBuffer.h"

#include <QTime>
#include <QColor>

/* The BPM Detector is responsible for detecting the musical tempo of the input Signal 
 * in Beats Per Minute. This inforamtion is then sent via osc to set Tempo of an effect
 * or BeatBoss (on cobalt).
 *
 *
 * Interfacing
 * ===========
 *
 * A BPM Detector is initialized with a MonoAudioBuffer and a BPMOscController. The 
 * MonoAudioBuffer will be polled for new samples of audio data, and the 
 * BPMOscController will be provided with every newly detected tempo.
 *
 * It is the owners responsiblility to regularly call the detectBPM() function, which
 * will trigger the detection process from polling new data to pushing new value out to 
 * the BPMOscController. The owner should also call resetCache() before calling 
 * detectBPM() again after any break longer than the AudioBuffer.He can then get the 
 * bpm value and a boolean value indicating if the value is older than five seconds, 
 * which indicates that the signal did not contain sufficient rhythmic information in 
 * the last seconds.
 *
 * The owner can also set a range in which the detecteed bpm shall reside. Any
 * values outside the range are multiplied or divided by two until they are within
 * the range. This is important, as different people can percieve the tempo of
 * a song different by factors of two, and ultimately there is no difference between
 * e.g. quater notes at 120 or eight notes at 60. To fix this issue, a minBPM can be
 * set that to a value x, which implies a range between x and 2x-1. Setting the
 * min BPM to 0 implies a very broad rang of 50-299, which only keeps completely
 * unrealistic values out.
 *
 * A waveform representation of last five seconds of audio data can be accessed
 * in three buffers: a float value for the amplitude, boolean that indicates wether
 * an onset was detected at the position and a QColor that can be used to color
 * the waveform based on the spectrum (red = low frequencies, green = mid frequencies
 * blue = high frequencies, yellow = red & green etc.)
 *
 *
 * Detection
 * =========
 *
 * The bpm detection works in four distinct steps, and is executed every time
 * detectBPM() is called.
 * - First, the lates audio data is polled from the m_inputBuffer, reduced to its
 * spectral flux value and then appended to a buffer of spectral flux value that
 * holds the last five seconds of values.
 * - Second, these five seconds of values are analyzed to identify onsets in the
 * signal by performing peak detection on the spectral flux history
 * - Third, these five seconds worth of onsets are analyzed to find strings of
 * evenly spaced onsets that could be the tempo
 * - Fourth, a tempo is chosen from these strings by picking the strongest one, and
 * smoothing it in relation the previous values.
 * The stages are described in detail below
 *
 * 1. Spectral Flux `updateSpectralFluxes()`
 * -----------------------------------------
 * To detect onsets even in very dense music material, merely analyzing the amplitude
 * has proven inefficient. This is why this uses the spectral flux, which is calculated
 * by regularly taking an fft of the audio data and summing the increases per band.
 * We analyze the latest 2048 samples of audio data every time 256 new samples come in.
 * This creates an overlap of 87%, which is fine because it ensures a high resolution
 * (172 fps at 44100 kHz) while still including frequencies even under 50Hz. The 256
 * Samples were chosen because we wanted at least 100 fps of prescision, and the 2048
 * were then determined by experiment.
 *
 * 2. Onset Detection `updateOnsets()`
 * -----------------------------------
 * Onset detection is performed every time new samples come in, because the peak 
 * detection algorithm takes an input of values normalized to an average of 0 and
 * a standard deviation of 1. This ensures that the volume of the signal has no
 * effect on the detection, but requires repeating the whole peak detection to ensure
 * that onsets are detected even if they are only considered onsets once they are two
 * seconds old.
 *
 * _The spectral flux and onset detection approach are taken from the magnificent paper
 * "Evaluation of the Audio Beat Tracking System BeatRoot" by Simon Dixon_
 *
 * 3. Beat Strings `updateStrings()`
 * ---------------------------------
 * To determine the beat of a song, simple approchaes like evaluating all distances
 * between two onsets have prooven inefficient, as they have too often caught onto 
 * triplets, syncopes and other rhythmic phenomena in most types of music more complex
 * than a metronome. Therefore, the algorithm tries to identify what I like to call
 * "BeatStrings". A Beat String is a subset of the detected onsets that are evenly
 * spaced within a certain tolerance. They are identified by taking every combination
 * of onsets and adding onsets one by one that are at the right distance. If there is
 * no onset that has a valid distance from the last one, a "ghost onset" is infered and
 * the search continues. If this happens again, the search terminates. Strings with
 * less than 4 onsets (excluding ghost onsets of course) are discarded.
 *
 * 4. Evaluation and Smoothing `evaluateStrings()`
 * -----------------------------------------------
 * As a first step, the string that has the highest score (sum of the collected 
 * amplitudes) is identified, and its average interval extracted. This could allready
 * be interpreted as the distance between two beats, converted to BPM and outputed
 * as the BPM. This has a very inconsistent output however, with sudden jumps to a
 * a factor of the tempo or a wrong value, only showing the right tempo most of the 
 * time. Smoothing by mere averaging would obviously not solve these problems either.
 * This is why there are multiple smoothing stages:
 *
 * - Compare the highest scoring interval to the last interval output by the whole
 *   process (not only this stage). If it deviates by one of a few selected factors or 
 *   fractions, and there is another string with a sufficiently high score that has 
 *   approximately the same interval, replace the highest scoring interval with its 
 *   interval
 * - Put the new value into a buffer that stores the last 16 Values. Perform clustering
 *   to group similar intervals, and only output a value if one of the clusters
 *   contains at least 75% of the values.
 *
 * If an interval has been identified, it is first remembered as the last interval for
 * the comparison by factors. It is then converted to a BPM, adapted to the range set
 * by the user and then set as m_bpm, from where it can be retrieved by calling
 * getBPM(). The OscController is also notified.
 * If no interval could be identified, a counter is increased to eventually return
 * true from bpmIsOld()
 */

// --------------------------------------- Constants for BPM Detection ------------------------------

// the number of samples that the bpm detection moves forward as the exponent of 2
static const int NUM_BPM_SAMPLES_EXPONENT = 8;

// the real number of samples calculated from NUM_BPM_SAMPLES_EXPONENT
static const int NUM_BPM_SAMPLES = qPow(2, NUM_BPM_SAMPLES_EXPONENT);

// the number of samples fft-ed for each sample (more to allow overlap) as an exponent of 2
static const int NUM_BPM_FFT_SAMPLES_EXPONENT = 11;

// the number of samples fft-ed for each sample (more to allow overlap
static const int NUM_BPM_FFT_SAMPLES = qPow(2, NUM_BPM_FFT_SAMPLES_EXPONENT);

// Sampling Rate
static const int SAMPLE_RATE = 44100;

// the number of seconds to be cached for detection
static const int SECONDS_TO_CACHE = 5;

// the number of frames to be cached, based on the seconds
static const int FRAMES_TO_CACHE = (SAMPLE_RATE / NUM_BPM_SAMPLES) * SECONDS_TO_CACHE;

// the number of refresh calls to wait before calculating the bpm
static const int CALLS_TO_WAIT = 5;

// the seconds of bpms to store to allow smoothing of the output
static const int SECONDS_OF_INTERVALS_TO_STORE = 4;

// the actual number of bpms to store to allow smoothing of the
static const int INTERVALS_TO_STORE = SECONDS_OF_INTERVALS_TO_STORE * BPM_UPDATE_RATE / CALLS_TO_WAIT;



// ------------------------------- Utility Functions for BPM Detection -------------------------------
inline float bpmToMs(const float bpm) {
    return 60000.0 / bpm;
}

inline float msToBPM(const float ms) {
    return 60000.0 / ms;
}

inline int frequencyToIndex(const int frequency) {
    return NUM_BPM_FFT_SAMPLES*frequency / SAMPLE_RATE;
}

inline int framesToMs(const int frames) {
    return frames * NUM_BPM_SAMPLES * 1000 / SAMPLE_RATE;
}

inline int msToFrames(const int ms) {
    return ms * SAMPLE_RATE / NUM_BPM_SAMPLES / 1000;
}

constexpr int GLOBAL_MIN_BPM = 50;
constexpr int GLOBAL_MAX_BPM = 300;

inline float bpmInRange(float bpm, const int minBPM) {
    if (minBPM > 0) {
        while (bpm < minBPM && bpm != 0) {
            bpm *= 2;
        }
        while (bpm >= minBPM*2) {
            bpm /= 2;
        }
    }

    while (bpm < GLOBAL_MIN_BPM && bpm != 0) {
        bpm *= 2;
    }
    while (bpm >= GLOBAL_MAX_BPM) {
        bpm /= 2;
    }

    return bpm;
}


// ---------------------------------- Initialization and Interfacting ---------------------------------


BPMDetector::BPMDetector(const MonoAudioBuffer &buffer, BPMOscControler *osc) :
    m_inputBuffer(buffer)
  , m_lastInputBufferNumSamples(0)
  , m_refreshesSinceCalculation(0)
  , m_bpm(0)
  , m_framesSinceLastBPMDetection(0)
  , m_minBPM(75)
  , m_fft()
  , m_window(NUM_BPM_FFT_SAMPLES)
  , m_onsetBuffer(FRAMES_TO_CACHE)
  , m_spectralFluxBuffer(FRAMES_TO_CACHE)
  , m_spectralFluxNormalized(FRAMES_TO_CACHE)
  , m_waveColors(FRAMES_TO_CACHE)
  , m_buffer(NUM_BPM_FFT_SAMPLES)
  , m_fftOutput(NUM_BPM_FFT_SAMPLES)
  , m_currentSpectrum(NUM_BPM_FFT_SAMPLES)
  , m_lastSpectrum(NUM_BPM_FFT_SAMPLES)
  , m_beatStrings()
  , m_lastIntervals(INTERVALS_TO_STORE)
  , m_transmitBpm(false)
  , m_oscController(osc)
{
    m_fft = static_cast<BasicFFTInterface*>(new FFTRealWrapper<NUM_BPM_FFT_SAMPLES_EXPONENT>());
    calculateWindow();
}

BPMDetector::~BPMDetector()
{
    delete m_fft;
}

void BPMDetector::resetCache()
{
    m_bpm = 0.0;
    m_onsetBuffer.fill(false);
    m_spectralFluxBuffer.clear();
    m_waveColors.clear();
    m_lastInputBufferNumSamples = m_inputBuffer.getNumPutSamples();
}

void BPMDetector::calculateWindow()
{
    // Hann Window function
    // used to prepare the PCM data for FFT
    for (int i=0; i<NUM_BPM_FFT_SAMPLES; ++i) {
        m_window[i] = 0.5f * (1 - qCos((2 * M_PI * i) / (NUM_BPM_FFT_SAMPLES - 1)));
    }
}

// Sets the minimum bpm of the range, and rounds it to one of the allowed values
void BPMDetector::setMinBPM(int value) {
    if (value == 0) {
        m_minBPM = 0;
    } else if (value < 63) {
        m_minBPM = 50;
    } else if (value < 88) {
        m_minBPM = 75;
    } else if (value < 125) {
        m_minBPM = 100;
    } else {
        m_minBPM = 150;
    }
    m_bpm = bpmInRange(m_bpm, m_minBPM);
}


// --------------------------------------------------- Detection ------------------------------------------

// "Master" Function that calls all subroutines of the BPM Detection Procedure.
// Detection works in a few discrete steps:
// 0. get as many new samples from the buffer as available
// 1. identify onsets ("hits") in the audio signal
// 2. evaluate the positions of the onsets into strings
// 3. evaluate these and smooth the output
void BPMDetector::detectBPM()
{
    // add as many new samples to the spectral flux history as available
    int64_t currentNumPutSamples = m_inputBuffer.getNumPutSamples();
    while (currentNumPutSamples - m_lastInputBufferNumSamples > NUM_BPM_FFT_SAMPLES) {
        updateSpectralFluxes(m_inputBuffer.getCapacity() - (currentNumPutSamples - m_lastInputBufferNumSamples));
        m_lastInputBufferNumSamples += NUM_BPM_SAMPLES;
    }

    // if the buffer isn't full yet, don't continue
    if (m_spectralFluxBuffer.count() < m_spectralFluxBuffer.capacity()) {
        return;
    }

    // Analyze the spectral flux to retrieve onsets information
    updateOnsets();

    // Use a counter to only perform the tempo detection calculations every n times, because they are expensive
    m_refreshesSinceCalculation++;
    if (m_refreshesSinceCalculation >= CALLS_TO_WAIT) {
        m_refreshesSinceCalculation = 0;
    } else {
        return;
    }

    // Retrieve a set of strings of evenly spaced onsets
    updateStrings();

    // Find the highest scored string, and perform smoothing to get a consisten value
    evaluateStrings();
}


// Calculates the spectral flux for the samples from the given index
// Spectral flux is the sum of only the *increases* in frequency.
// See "Evaluation of the Audio Beat Tracking System BeatRoot" by Simon Dixon
// (in Journal of New Music Research, 36, 2007/8) for further detail
void BPMDetector::updateSpectralFluxes(const int fromIndex)
{
    // Stop if the index to go forward from is out of the buffers bound
    if (fromIndex+NUM_BPM_FFT_SAMPLES >= m_inputBuffer.getCapacity() || fromIndex < 0) {
        return;
    }

    // apply hann window to new data to prepare it for the FFT
    for (int i=0; i < NUM_BPM_FFT_SAMPLES; ++i) {
        m_buffer[i] = m_inputBuffer.at(fromIndex+i) * m_window[i];
    }

    // apply FFT:
    m_fft->doFft(m_fftOutput.data(), m_buffer.constData());

    // calculate spectral flux by adding all increases in energy in each band
    float flux = 0.0;

    for (int i = 0; i < NUM_BPM_FFT_SAMPLES / 2; ++i) {
        if (m_fftOutput[i] > m_lastSpectrum[i]) {
            flux += (m_fftOutput[i] - m_lastSpectrum[i]);
        }
        if (m_fftOutput[i + NUM_BPM_FFT_SAMPLES / 2] > m_lastSpectrum[i + NUM_BPM_FFT_SAMPLES / 2]) {
            flux += (m_fftOutput[i + NUM_BPM_FFT_SAMPLES / 2] - m_lastSpectrum[i + NUM_BPM_FFT_SAMPLES / 2]);
        }
    }

    // Store the new spectral flux value
    m_spectralFluxBuffer.push_back(flux);

    // Store the spectrum for comparison in the next iteration
    m_lastSpectrum = m_fftOutput;


    // Calculate a color for the gui that represents the spectral content of this sample
    // The amount of red, green and blue is proportional to the amount of low, mid and high
    // in the signal, and the color
    int col[] = {0,0,0}; // r,g and b values in 0..255

    // Sum up low, mid an high frequencies
    for (int i = 0; i < frequencyToIndex(200); i++) {
        col[0] += qAbs(m_fftOutput[i])*1000;
    }

    for (int i = frequencyToIndex(200); i < frequencyToIndex(2000); i+=10) {
        col[1] += qAbs(m_fftOutput[i])*5000;
    }

    for (int i = frequencyToIndex(2000); i < NUM_BPM_FFT_SAMPLES / 2; i+=20) {
        col[2] += qAbs(m_fftOutput[i])*10000;
    }

    // Normalize so that at least one value is 255
    int max = col[0];
    if (col[1] > max) {
        max = col[1];
    }
    if (col[2] > max) {
        max = col[2];
    }

    if (max > 0 && !m_waveColors.isEmpty()) {
        col[0] = col[0] * 128 / max + m_waveColors.last().red() / 2;
        col[1] = col[1] * 128 / max + m_waveColors.last().green() / 2;
        col[2] = col[2] * 128 / max + m_waveColors.last().blue() / 2;
    }

    // Store the value
    m_waveColors.push_back(QColor(col[0],col[1],col[2]));
}

// finds onsets in the audio material
// Algorithm is again from "Evaluation of the Audio Beat Tracking System BeatRoot"
// by Simon Dixon (in Journal of New Music Research, 36, 2007/8)
void BPMDetector::updateOnsets()
{
    // clear the onset history
    for (int i=0; i < FRAMES_TO_CACHE; ++i) {
        m_onsetBuffer[i] = false;
    }
    // normalize the spectral flux to an average of 0 and a standard deviation of 1,
    // by determining the current average and standard deviation and then subtracting
    // the average from each value and dividing it by the standard Deviation
    float average = 0.0;
    for (int i=0; i < FRAMES_TO_CACHE; ++i) {
        average += m_spectralFluxBuffer[i];
    }
    average = average / FRAMES_TO_CACHE;

    float variance = 0.0;
    for (int i = 0; i < FRAMES_TO_CACHE; ++i) {
        variance += m_spectralFluxBuffer[i]*m_spectralFluxBuffer[i];
    }
    float stdDev = qSqrt(variance);

    // Cap the standard Deviation to prevent onset detection in ADC Noise Floor, which has a
    // stdDev of under 20 (music is usually over 100, to about 30000)
    stdDev = qMax(stdDev, 20.0f);

    for (int i = 0; i < FRAMES_TO_CACHE; ++i) {
        m_spectralFluxNormalized[i] = (m_spectralFluxBuffer[i] - average) / stdDev;
    }

    const char w = 5; //window for local maximum detection
    const char m = 3; // multiplier to increase range before onset
    const float pastThresholdWeight = 0.84;
    const float averageThresholdDelta = 0.008;

    // detect onsets by iterating over spectral flux and checking for the three criteria
    // that a sample must fullfill to be considered an onset
    // 1. Past Threshold: have a greater value than the (g in the paper)
    // 2. Local Maximum: have value greater than the neighboring samples within a window of +- w samples
    // 3. Average Threshold: have a greater value than the average of the surrounding samples (-m*w to +w) with an added Delta
    // The order is changed from the paper, to ensure calculation of the recursive function, while keeping
    // the code easy to read

    float pastThreshold = m_spectralFluxNormalized[m*w-1];

    // Iterate over all samples except the edges where there are not enough surrounding samples
    for (int n=m*w; n < FRAMES_TO_CACHE - w; ++n) {

        // ------------------------------- 1. Past Threshold -----------------------------
        // Calculate the past threshold recursively, as the maximum between a weighted average between
        // the last threshold and the last sample, and the last sample itself
        float pastThresholdNew = qMax(m_spectralFluxNormalized[n-1], pastThresholdWeight*pastThreshold + (1-pastThresholdWeight)*m_spectralFluxNormalized[n-1]);
        pastThreshold = pastThresholdNew;

        // Continue if the sample does not meet the past threshold
        if (m_spectralFluxNormalized[n] < pastThreshold) {
            continue;
        }

        // -------------------------------- 2. Local Maximum -----------------------------
        // Compare to the releavant surronding samples
        bool localMaximum = true;
        for (int k = n-w; k <= n+w; ++k) {
            if (m_spectralFluxNormalized[n] < m_spectralFluxNormalized[k]) {
                localMaximum = false;
                break;
            }
        }

        // Continue of the sample does no meet the local neighbour criterium
        if (!localMaximum) {
            continue;
        }

        // ---------------------------- 3. Average Threshold -----------------------------
        // Sum the surounding samples, then divide by their number and add the delta
        float averageThreshold = 0.0;
        for (int k = n-w*m; k < n+w; ++k) {
            averageThreshold += m_spectralFluxNormalized[k];
        }
        averageThreshold /= (m*w + w +1);
        averageThreshold += averageThresholdDelta;

        // Continue if the sample does not meet the average threshold
        if (m_spectralFluxNormalized[n] < averageThreshold) {
            continue;
        }

        // Set the sample to be an onset if it has met all the criteria
        m_onsetBuffer[n] = true;
    }
}


// constants that store the cluster width (allowed deviation for two intervals to be considered
// related) and the maximum interval considered sensible to analyze
const static int CLUSTER_WIDTH = 30; // ms
static int CLUSTER_WIDTH_IN_FRAMES = msToFrames(CLUSTER_WIDTH); // frames
const static int MAX_INTERVAL = 2000; // ms

// A class that models a Sequence of Beats, by only storing the average interval and the
// summed up score, and also the number of contained intervals to be able to update the
// average accordingly.
class BeatString
{
public:
    BeatString(int interval, float score) :
        m_averageInterval(interval)
      , m_size(1)
      , m_nativeScore(score)
    {}

    float getSize() { return m_size; }
    float getScore() { return m_nativeScore; }
    float getAverageInterval() { return m_averageInterval; }

    void addInterval(float interval, float score) {
        m_averageInterval = (m_size * m_averageInterval + interval) / (m_size + 1);
        m_nativeScore += score;
        ++m_size;
    }

    bool operator==(const BeatString& other) {
        return m_averageInterval == other.m_averageInterval
                && m_size == other.m_size
                && m_nativeScore == other.m_nativeScore;
    }

protected:
    float   m_averageInterval;
    int     m_size;
    float   m_nativeScore;
};

// A class that models a Cluster of Intervals, by only storing the average interval and the
// the number of contained intervals to be able to update the average accordingly. Similar
// to the beat string without the scoring
class IntervalCluster
{
public:
    IntervalCluster(int interval) :
        m_averageInterval(interval)
      , m_size(1)
    {}

    float getScore() { return m_size; }
    float getAverageInterval() { return m_averageInterval; }

    void addInterval(float interval) {
        m_averageInterval = (m_size * m_averageInterval + interval) / (m_size + 1);
        ++m_size;
    }

    bool operator==(const IntervalCluster& other) {
        return m_averageInterval == other.m_averageInterval
                && m_size == other.m_size;
    }

protected:
    float   m_averageInterval;
    int     m_size;
};

static const int MIN_BEATS_IN_STRING = 4; // the minimum number of beats a string needs to contain

// Identifies BeatStrings (strings of Onsets with roughly consistent Intervals
// from the detected onsets. Very loosely Based on "Automatic Extraction of Tempo
// and beat from Expressive Performances" by Simon Dixon (2001)
void BPMDetector::updateStrings()
{
    // Delete all existing strings
    m_beatStrings.clear();

    // Iterate of all pairs of indices i and j, where there is an onset at
    // both indices and i < j. Then try to find as many onsets as possible
    // with the same equal interval:
    //
    // In this illustration, the i/j/x are the onsets in the signal, and the
    // stars mark all the consecutive onsets with the same interval that
    // could be found in the signal.
    //
    // *    *    *    *    *    *    *
    // i----j-x--x----x-x--x----x--x-------x--x--x-x----x
    //
    //
    for (int i = 0; i < FRAMES_TO_CACHE; ++i) {
        if (m_onsetBuffer[i]) {
            for (int j = i+1; j < FRAMES_TO_CACHE; ++j) {
                if (m_onsetBuffer[j]) {

                    // Detect the interval and score, and continue right away if the interval is to short or to long
                    float interval = framesToMs(j-i);
                    if (!(CLUSTER_WIDTH < interval && interval < MAX_INTERVAL)) {
                        continue;
                    }
                    // Take the minimum of the two onsets fluxes as the score, to weigh intervals between strong
                    // onsets more
                    float score = qMin(m_spectralFluxNormalized[i], m_spectralFluxNormalized[j]);

                    // Initialize a new Beat String with tha interval and score
                    BeatString string(interval, score);

                    // Store the index of the last onset (start with j)
                    int lastOnsetIndex = j;
                    // Get the bounds in which the next interval needs to sit
                    float minInterval = string.getAverageInterval() - CLUSTER_WIDTH;
                    float maxInterval = string.getAverageInterval() + CLUSTER_WIDTH;
                    // Allow to skip one beat
                    bool skipedBeat = false;

                    // Iterate over all future indices
                    for (int k = j+msToFrames(minInterval); k < FRAMES_TO_CACHE; ++k) {
                        // Calculate the interval from the last onset
                        float interval = framesToMs(k-lastOnsetIndex);

                        // If the interval became to long, simulate a beat to allow one missing one
                        // or break if this has already been the done
                        if (interval > maxInterval) {
                            if (skipedBeat) {
                                break;
                            } else {
                                lastOnsetIndex += msToFrames(string.getAverageInterval());
                                // Skip ahead the minimal distance two onsets may be apart
                                skipedBeat = true;
                                k+= qMax(msToFrames(minInterval - CLUSTER_WIDTH) - 1, 0);
                                continue;
                            }
                        }
                        // If an onset was found, update the string and set it as the last onset
                        if (m_onsetBuffer[k]) {
                            // The score is the minimum of the two onsets spectral fluxes
                            float score = qMin(m_spectralFluxNormalized[lastOnsetIndex], m_spectralFluxNormalized[k]);
                            string.addInterval(interval, score);
                            lastOnsetIndex = k;

                            // Recalculate the margin of tolerance from the new interval
                            minInterval = string.getAverageInterval() - CLUSTER_WIDTH;
                            maxInterval = string.getAverageInterval() + CLUSTER_WIDTH;

                            // Skip ahead the minimal distance two onsets may be apart
                            k += qMax(msToFrames(minInterval) - 1, 0);
                        }
                    }


                    // Discard the string if he doesn't have at least 4 beats === 4 - 1 intervals
                    if (string.getSize() < MIN_BEATS_IN_STRING - 1) {
                        continue;
                    }

                    bool discardString = false;
                    // See if there is another string that is within cluster width. If it is scored higher, delete it. Else, mark the new one to be deleted
                    for (auto k = m_beatStrings.begin(); k != m_beatStrings.end(); ++k) {
                        if (qAbs(k->getAverageInterval() - string.getAverageInterval()) < CLUSTER_WIDTH) {
                            if (k->getScore() > string.getScore()) {
                                discardString = true;
                                break;
                            } else {
                                // Save a reference to j to remove it from the list
                                BeatString& stringToDie = *k;
                                // Decrease the iterator in order not to point to an item that is will be gone
                                --k;
                                // Remove j from the list
                                m_beatStrings.removeOne(stringToDie);

                            }
                            break;
                        }
                    }

                    if (!discardString) {
                        m_beatStrings.append(string);
                    }
                }
            }
        }
    }
}

// Checks a given interval for sufficent support in the strings,
// to evaluate if after a drastic tempo change the old tempo is still plausible
// This is the case if there is an string within CLUSTER_WIDTH of the interval
// that has at least 40% the score of the other interval
BeatString* BPMDetector::plausibleStringForInterval(float interval, float maxScore)
{
    for (BeatString& string : m_beatStrings) {
        if (qAbs(string.getAverageInterval() - interval) < CLUSTER_WIDTH) {
            if (string.getScore() >= 0.40 * maxScore) {
                return &string;
            }
        }
    }

    return 0;
}

// The
static const float fractionsToCheck[] = {
    2.0,
    0.5,
    0.25,
    4.0,
    4.0/3.0,
    2.0/3.0,
    3.0};




// evaluates the strings by using the highest scored strings interval to calculate the bpm
void BPMDetector::evaluateStrings()
{
    BeatString* maxString = 0;
    for (BeatString& cluster : m_beatStrings) {
        if (!maxString || cluster.getScore() > maxString->getScore()) {
            maxString = &cluster;
        }
    }

    // Only change the value if a string has been identified as the "winner" (i.e. there were strings)
    if (maxString) {
        // Itendify the interval
        float newInterval = maxString->getAverageInterval();

        // If the new tempo is substantially different from the old one, check common fractions by
        // which the tempo is likely to deviate from what is probably the actual tempo (represented
        // by the last recognized tempo, which is our best guess to this point)
        if ( m_lastIntervals.size() && qAbs(newInterval - m_lastWinningInterval) > CLUSTER_WIDTH)
        {
            for ( const float& fraction : fractionsToCheck )
            {
                // Check if the difference is small enough
                if ( (qAbs(fraction*newInterval - m_lastWinningInterval) < 2*CLUSTER_WIDTH) )
                {
                    // Check if there is another string that suggest strong enough evidence that the multiplied tempo could be the actual tempo
                    BeatString* plausibleString = plausibleStringForInterval(m_lastWinningInterval, maxString->getScore() / fraction);
                    if (plausibleString) {
                        newInterval = plausibleString->getAverageInterval();
                        break;
                    }
                }
            }
        }
        m_lastIntervals.append(newInterval);

        // Perform clustering on the last bpms to smooth out any spikes
        QLinkedList<IntervalCluster> finalIntervalClusters;
        for (float& interval : m_lastIntervals) {
            // Identify the cluster that most closely matches the interval (up to CLUSTER_WIDTH deviation is allowd)
            IntervalCluster* closestCluster = nullptr;
            int closestDistance = INT_MAX;
            for (IntervalCluster& cluster : finalIntervalClusters) {
                int distance = qAbs(cluster.getAverageInterval() - interval);
                if (distance < CLUSTER_WIDTH && distance < closestDistance) {
                    closestDistance = distance;
                    closestCluster = &cluster;
                }
            }

            //If a matching cluster was found, add the interval to it. If not, create a new cluster
            if (closestCluster) {
                closestCluster->addInterval(interval);
            } else {
                finalIntervalClusters.append(IntervalCluster(interval));
            }
        }

        // Identify the winning cluster of tempos
        IntervalCluster* maxFinalCluster = nullptr;
        for (IntervalCluster& cluster : finalIntervalClusters) {
            if (!maxFinalCluster || cluster.getScore() > maxFinalCluster->getScore()) {
                maxFinalCluster = &cluster;
            }
        }

        // Only call the cluster winning if it contains at least 75% of the intervals. else keep the old tempo
        if (maxFinalCluster && maxFinalCluster->getScore()*4 > 3*INTERVALS_TO_STORE) {
            m_lastWinningInterval = maxFinalCluster->getAverageInterval();
            float newBPM = bpmInRange(msToBPM(maxFinalCluster->getAverageInterval()), m_minBPM);
            m_bpm = newBPM;
            if (m_transmitBpm) m_oscController->transmitBPM(m_bpm);
            m_framesSinceLastBPMDetection = 0;
            return;
        }
    }
    m_framesSinceLastBPMDetection += CALLS_TO_WAIT;
}

