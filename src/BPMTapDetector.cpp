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

#include "BPMTapDetector.h"

BPMTapDetector::BPMTapDetector(BPMOscControler *osc) :
      m_bpm(0)
	, m_lastBeats(BpmConstants::HISTORY_LENGTH)
    , m_lastValue(0)
    , m_minBPM(75)
    , m_oscController(osc)
{
    m_startTime = HighResTime::now();
}

inline float bpmInRange(float bpm, const int minBPM) {
    if (minBPM > 0) {
        while (bpm < minBPM && bpm != 0) {
            bpm *= 2;
        }
        while (bpm >= minBPM*2) {
            bpm /= 2;
        }
    }

    while (bpm < BpmConstants::GLOBAL_MIN_BPM && bpm != 0) {
        bpm *= 2;
    }
    while (bpm >= BpmConstants::GLOBAL_MAX_BPM) {
        bpm /= 2;
    }

    return bpm;
}

// Sets the minimum bpm of the range, and rounds it to one of the allowed values
void BPMTapDetector::setMinBPM(int value) {
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

void BPMTapDetector::triggerBeat() {
    double beatTime = HighResTime::elapsedSecSince(m_startTime);
    // check if history should be discarded:
    if (!m_lastBeats.isEmpty()) {
        double secSinceLast = beatTime - m_lastBeats.last();
        if (secSinceLast > 60. / BpmConstants::MIN_BPM) {
            // yes last beat is too old -> discard history:
            m_lastBeats.clear();
        }
    }
    m_lastBeats.append(beatTime);

    // check if there are at least 2 values:
    if (m_lastBeats.size() < 2) return;

    double sumBeatDuration = 0;  // in sec
    for (int i=1; i<m_lastBeats.size(); ++i) {
        sumBeatDuration += m_lastBeats[i] - m_lastBeats[i-1];
    }
    // averageBeatDuration is in sec
    float averageBeatDuration = sumBeatDuration / (m_lastBeats.size() - 1);
    m_bpm = bpmInRange((1. / averageBeatDuration) * 60., m_minBPM);
    m_oscController->transmitBPM(m_bpm);
}

void BPMTapDetector::reset() {
    m_startTime = HighResTime::now();
    m_lastValue = 0;
    m_lastBeats.clear();
    m_bpm = 0;
}
