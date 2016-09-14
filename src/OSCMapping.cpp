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

#include "OSCMapping.h"

#include "MainController.h"
#include "TriggerGuiController.h"

OSCMapping::OSCMapping(MainController *controller, QObject *parent)
	: QObject(parent)
	, m_controller(controller)
	, m_inputIsEnabled(true)
{

}

void OSCMapping::handleMessage(OSCMessage msg)
{
	// do nothing if input is not enabled:
	if (!m_inputIsEnabled) return;

	// check what to to with the incoming message based on the start of the path:
	if (msg.pathStartsWith("/s2l/enabled/toggle")) {
		// toggle OSC trigger output state:
		if (msg.isTrue()) {
			m_controller->setOscEnabled(!m_controller->getOscEnabled());
		}
	} else if (msg.pathStartsWith("/s2l/enabled")) {
		// set OSC trigger output state:
		m_controller->setOscEnabled(msg.isTrue());
	} else if (msg.pathStartsWith("/s2l/level_feedback/toggle")) {
		// toggle OSC level feedback state:
		if (msg.isTrue()) {
			m_controller->enableOscLevelFeedback(!m_controller->oscLevelFeedbackIsEnabled());
		}
	} else if (msg.pathStartsWith("/s2l/level_feedback")) {
		// set OSC level feedback state:
		m_controller->enableOscLevelFeedback(msg.isTrue());
	} else if (msg.pathStartsWith("/s2l/preset")) {
		// load preset if first argument is a string:
		if (msg.arguments().size() == 1) {
			QVariant arg = msg.arguments().first();
			if (arg.type() == QVariant::String) {
				QString presetDir = m_controller->getPresetDirectory() + "/";
				m_controller->loadPreset(presetDir + arg.toString() + ".s2l");
			}
		}
    } else if (msg.pathStartsWith("/s2l/bpm/enabled/toggle")) {
        // togle bpm recognition
        if (msg.isTrue()) {
            m_controller->setBPMActive(!m_controller->getBPMActive());
        }
    } else if (msg.pathStartsWith("/s2l/bpm/enabled")) {
        // set bpm recognition on/of
        m_controller->setBPMActive(msg.isTrue());
    } else if (msg.pathStartsWith("/s2l/bpm/range")) {
        // Sets the range for the bpm detection by the minimum (max is always 2* min)
        if (msg.arguments().size() == 1) {
            int minBPM = msg.arguments().at(0).toInt();
            // Round to the values presented in the gui to avoid confusion
            m_controller->setMinBPM(minBPM);
        }
    } else if (msg.pathStartsWith("/s2l/bpm/tap")) {
        // Taps the bpm if there is no argument, or an enable argument
        if (msg.arguments().size() == 0 || msg.arguments().at(0).toBool()) {
            m_controller->triggerBeat();
        }
    }
}

void OSCMapping::sendLevelFeedback()
{
    // send the levels of the bandpasses and the bpm via OSC as feedback:

	qreal bassValue = m_controller->m_bassController->getCurrentLevel();
	m_controller->sendOscMessage(QString("/s2l/out/bass=").append(QString::number(bassValue, 'f', 3)), true);

	qreal loMidValue = m_controller->m_loMidController->getCurrentLevel();
	m_controller->sendOscMessage(QString("/s2l/out/lo_mid=").append(QString::number(loMidValue, 'f', 3)), true);

	qreal hiMidValue = m_controller->m_hiMidController->getCurrentLevel();
	m_controller->sendOscMessage(QString("/s2l/out/hi_mid=").append(QString::number(hiMidValue, 'f', 3)), true);

	qreal highValue = m_controller->m_highController->getCurrentLevel();
	m_controller->sendOscMessage(QString("/s2l/out/high=").append(QString::number(highValue, 'f', 3)), true);

	qreal levelValue = m_controller->m_envelopeController->getCurrentLevel();
	m_controller->sendOscMessage(QString("/s2l/out/level=").append(QString::number(levelValue, 'f', 3)), true);

	qreal silenceValue = m_controller->m_silenceController->getCurrentLevel();
	m_controller->sendOscMessage(QString("/s2l/out/silence=").append(QString::number(silenceValue, 'f', 3)), true);

}

void OSCMapping::sendCurrentState()
{
	// OSC Level Feedback enabled state:
	bool enabled = m_controller->getOscEnabled();
	m_controller->sendOscMessage(QString("/s2l/out/enabled=").append(enabled ? "1" : "0"), true);

	// Active Preset Name:
	m_controller->sendOscMessage("/s2l/out/active_preset", m_controller->getPresetName(), true);

    // BPM Enabled
    bool bpmEnabled = m_controller->getBPMActive();
    m_controller->sendOscMessage("/s2l/out/bpm/enabled", (bpmEnabled ? "1" : "0"), true);

    // BPM Range
    m_controller->sendOscMessage("/s2l/out/bpm/range", QString::number(m_controller->getMinBPM()), true);
}
