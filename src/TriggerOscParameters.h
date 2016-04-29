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

#ifndef TRIGGEROSCSETTINGS_H
#define TRIGGEROSCSETTINGS_H

#include <QtGlobal>
#include <QString>
#include <QSettings>


// A class to store OSC parameters (messages and min and max values).
class TriggerOscParameters
{
public:
	TriggerOscParameters();

	// returns the OSC message to be sent when the trigger is activated
	QString getOnMessage() const { return m_onMessage; }
	// sets the OSC message to be sent when the trigger is activated
	void setOnMessage(const QString& value) { m_onMessage = value; }

	// returns the OSC message to be sent when the trigger is released
	QString getOffMessage() const { return m_offMessage; }
	// sets the OSC message to be sent when the trigger is released
	void setOffMessage(const QString& value) { m_offMessage = value; }

	// returns the OSC path where the level value should be sent to
	QString getLevelMessage() const { return m_levelMessage; }
	// sets the OSC path where the level value should be sent to
	void setLevelMessage(const QString& value) { m_levelMessage = value; }

	// returns the value to send when the trigger level is zero
	qreal getMinLevelValue() const { return m_minLevelValue; }
	// sets the value to send when the trigger level is zero
	void setMinLevelValue(const qreal& value) { m_minLevelValue = value; }

	// returns the value to send when the trigger level is at its maximum
	qreal getMaxLevelValue() const { return m_maxLevelValue; }
	// sets the value to send when the trigger level is at its maximum
	void setMaxLevelValue(const qreal& value) { m_maxLevelValue = value; }

	// returns a short label that describes the OSC target
	QString getLabelText() const { return m_labelText; }
	// sets a short label that describes the OSC target
	void setLabelText(const QString& value) { m_labelText = value; }

	// --------------------- Save / Restore ---------------------

	// saves the parameters to QSettings with the given name
	void save(const QString name, QSettings& settings) const;
	// restores the parameters from QSettings with the given name
	void restore(const QString name, QSettings& settings);

	// resets all parameters to default values
	void resetParameters();

protected:
	QString		m_onMessage;  // On message ("/path/value=argument")
	QString		m_offMessage;  // Off message ("/path/value=argument")
	QString		m_levelMessage;  // Level message ("/path/value=")
	qreal		m_minLevelValue;  // min value to be used for Level message
	qreal		m_maxLevelValue;  // max value to be used for Level message
	QString		m_labelText;  // Short description text of parameters to be displayed in UI

};

#endif // TRIGGEROSCSETTINGS_H
