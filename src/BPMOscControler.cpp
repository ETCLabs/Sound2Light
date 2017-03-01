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

#include "BPMOscControler.h"

BPMOscControler::BPMOscControler(OSCNetworkManager &osc) :
    m_osc(osc)
  , m_oscCommands()
{
}

// Restore the commands from e.g. a preset or whatever else
void BPMOscControler::restore(QSettings& settings)
{
    int count = settings.value("bpm/osc/count").toInt();

    m_oscCommands.clear();
    for (int index = 0; index < count; ++index) {
        QString command = settings.value("bpm/osc/" + QString::number(index)).toString();
        m_oscCommands.append(command);
    }
}

// Save the commands for e.g. a preset
void BPMOscControler::save(QSettings& settings) {
    // Save the number of commands
    settings.setValue("bpm/osc/count", m_oscCommands.size());

    // Store each command under the key "bpm/osc/*index*"
    for (int index = 0; index < m_oscCommands.size(); ++index) {
        settings.setValue("bpm/osc/" + QString::number(index), m_oscCommands[index]);
    }
}


inline int round(float value) { return (fmod(value,1.0) < 0.5) ? value : value + 1; }

// Called by the bpm detector to make the controller send the new bpm to the clients
void BPMOscControler::transmitBPM(float bpm)
{
    // Send user specified commands

    for (QString& command : m_oscCommands) {
        //Continue if the command is invalid, e.g. because it doesn't have a BPM
        if (command.indexOf("<BPM") == -1) {
            continue;
        }
        QString message(command);

        //Every message is prefixed by a Zero because the EOS will interpret a single digit BPM incorrectly, e.g. "3" as "30"
        //Sending "03" is correctly interpreted as "3"
        message.replace("<BPM>", "0" + QString::number(round(bpm)));
        message.replace("<BPM1>", "0" + QString::number(round(bpm)));
        message.replace("<BPM1-2>", "0" + QString::number(round(bpm*0.5)));
        message.replace("<BPM1-4>", "0" + QString::number(round(bpm*0.25)));
        message.replace("<BPM1-8>", "0" + QString::number(round(bpm*0.125)));
        message.replace("<BPM1-16>", "0" + QString::number(round(bpm*0.0625)));
        message.replace("<BPM1-32>", "0" + QString::number(round(bpm*0.03125)));
        message.replace("<BPM2>", "0" + QString::number(round(bpm*2)));
        message.replace("<BPM4>", "0" + QString::number(round(bpm*4)));
        message.replace("<BPM8>", "0" + QString::number(round(bpm*8)));
        message.replace("<BPM16>", "0" + QString::number(round(bpm*16)));
        message.replace("<BPM32>", "0" + QString::number(round(bpm*32)));
        m_osc.sendMessage(message);
    }

    // Send information command
    m_osc.sendMessage("/s2l/out/bpm=" + QString::number(round(bpm)), true);
}
