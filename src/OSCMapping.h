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

#ifndef OSCMAPPING_H
#define OSCMAPPING_H

#include "OSCMessage.h"

#include <QObject>


// Forward declaration
class MainController;


// This class maps application specific OSC messages to function calls
// and returns some information as feedback.
class OSCMapping : public QObject
{
	Q_OBJECT
public:
	explicit OSCMapping(MainController* controller, QObject *parent = 0);

public slots:

	// maps incoming messages to function calls
	void handleMessage(OSCMessage msg);

	// sends the current audio levels as OSC messages
	void sendLevelFeedback();

	// sends the current state (Preset Name + Trigger Output) as OSC messages
	void sendCurrentState();

	// returns if OSC input is enabled and if incoming messages will be handled
	bool getInputEnabled() const { return m_inputIsEnabled; }

	// enables or disables OSC input
	void setInputEnabled(bool value) { m_inputIsEnabled = value; }

protected:
	MainController* m_controller;  // pointer to MainController instance
	bool			m_inputIsEnabled;  // true if input is enabled and incoming messages will be handled
};

#endif // OSCMAPPING_H
