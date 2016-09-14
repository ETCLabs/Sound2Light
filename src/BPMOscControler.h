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

#ifndef BPMOSCCONTROLER_H
#define BPMOSCCONTROLER_H

#include "OSCNetworkManager.h"
#include <QSettings>

class BPMOscControler
{
public:
    BPMOscControler(OSCNetworkManager& osc);

    // Called by the bpm detector to make the controller send the new bpm to the clients
    void transmitBPM(float bpm);

    // Restores the state from e.g. a preset
    void restore(QSettings& settings);

    // Save the commands for e.g. a preset
    void save(QSettings& settings);

    // --------------------------------------- GUI Functions -----------------------------------------------
    // Returns the commands
    QStringList getCommands() { return m_oscCommands; }

    // Sets the command at the given index
    void setCommands(QStringList commands) {
        m_oscCommands = QStringList(commands);
    }


protected:
    OSCNetworkManager&  m_osc; // The network manager to send network signals thorugh
    QStringList         m_oscCommands; // The osc messages to be sent on a tempo changed. Delivered as finished strings with the <BPM> (<BPM1-2>, <BPM4> etc. for fractions from 1/4 to 4) qualifier to be changed. The message is generated in the qml because thats the way tim did it with the other osc messages
};

#endif // BPMOSCCONTROLER_H
