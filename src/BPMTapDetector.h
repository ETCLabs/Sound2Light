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

// Copied largely from Luminosus. Edited to work with sound2light by Hendrik Noeller
// (edits mainly involved removing luminosus block and gui functions and adding
// s2l gui functions)

#ifndef BPMTAPDETECTOR_H
#define BPMTAPDETECTOR_H

#include "utils.h"

#include "QCircularBuffer.h"

#include "BPMOscControler.h"

/**
 * @brief The BpmConstants namespace contains all constants used in BpmTapDetection
 */
namespace BpmConstants {
	/**
	 * @brief HISTORY_LENGTH is the length of the beat duration history to evaluate
	 */
    static const int HISTORY_LENGTH = 3;

    static const int MIN_BPM = 30; // for deciding when a beat is too old

    static const int GLOBAL_MIN_BPM = 50;
    static const int GLOBAL_MAX_BPM = 300;
}


class BPMTapDetector
{

public:

    explicit BPMTapDetector(BPMOscControler* osc);

    void triggerBeat(); // "Tap" impulse

    void reset(); // Reset all information

    bool hasBpm() { return m_bpm != 0; } // Wether there is a value

    float getBpm() const { return m_bpm; } // Get the current bpm
    void setBpm(float value) { m_bpm = value; m_oscController->transmitBPM(m_bpm);} // Set the bpm (if a user enters a value by keypad)

    void setMinBPM(int value); // Sets the minimum bpm of the range

    int getMinBPM() { return m_minBPM; } // Returns the minium bpm of the range

protected:
	/**
	 * @brief m_bpm current detected BPM value
	 */
    float               m_bpm;
    /**
     * @brief m_startTime stores the time when this object was created or rest
     */
    HighResTime::time_point_t   m_startTime;
	/**
	 * @brief m_lastBeats stores the times of the last beats in seconds since start
	 */
    Qt3DCore::QCircularBuffer<double> m_lastBeats;
	/**
	 * @brief m_lastValue stores the last input value
	 */
    double              m_lastValue;

    int                 m_minBPM;
    BPMOscControler*    m_oscController; // the object responsible for handling osc output
};

#endif // BPMTAPDETECTOR_H
