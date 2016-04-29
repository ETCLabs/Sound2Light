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

#ifndef BANDPASSTRIGGERGUICONTROLLER_H
#define BANDPASSTRIGGERGUICONTROLLER_H

#include "TriggerGenerator.h"

#include <QObject>


// This controller manages the connection of a BandpassSettings GUI component
// to a TriggerGenerator object.
class TriggerGuiController : public QObject
{
	Q_OBJECT

	// these properties are used by GUI:
	Q_PROPERTY(bool active READ getActive NOTIFY activeChanged)
	Q_PROPERTY(qreal midFreq READ getMidFreq WRITE setMidFreq NOTIFY parameterChanged)
	Q_PROPERTY(qreal width READ getWidth WRITE setWidth NOTIFY parameterChanged)
	Q_PROPERTY(qreal threshold READ getThreshold WRITE setThreshold NOTIFY parameterChanged)
	Q_PROPERTY(qreal onDelay READ getOnDelay NOTIFY parameterChanged)
	Q_PROPERTY(qreal offDelay READ getOffDelay NOTIFY parameterChanged)
	Q_PROPERTY(qreal maxHold READ getMaxHold NOTIFY parameterChanged)
	Q_PROPERTY(QString oscLabelText READ getLabelText NOTIFY oscLabelTextChanged)

public:
	explicit TriggerGuiController(TriggerGenerator* m_trigger, QObject *parent = 0);

signals:
	// emitted when the filtered trigger is activated
	void triggerOn();
	// emitted when the filtered trigger is released
	void triggerOff();
	// emitted when the state of the filtered trigger changed
	void activeChanged();
	// emitted when one of the parameters midFreq, width or threshold or delays changed
	void parameterChanged();
	// emitted when the OSC labelText changed
	void oscLabelTextChanged();
	// emitted when a value of the preset changed
	void presetChanged();

public slots:

	// forward calls to TriggerGenerator
	// see TriggerGenerator.h for documentation

	int getMidFreq() const { return m_trigger->getMidFreq(); }
	void setMidFreq(const int& value) { m_trigger->setMidFreq(value); emit parameterChanged(); emit presetChanged(); }

	qreal getWidth() const { return m_trigger->getWidth(); }
	void setWidth(const qreal& value) { m_trigger->setWidth(value); emit parameterChanged(); emit presetChanged(); }

	qreal getThreshold() const { return m_trigger->getThreshold(); }
	void setThreshold(const qreal& value) { m_trigger->setThreshold(value); emit parameterChanged(); emit presetChanged(); }

	qreal getCurrentLevel() const { return m_trigger->getCurrentLevel(); }


	// forward calls to TriggerFilter
	// see TriggerFilter.h for documentation

	qreal getOnDelay() const { return m_trigger->getTriggerFilter().getOnDelay(); }
	void setOnDelay(const qreal& value) { m_trigger->getTriggerFilter().setOnDelay(value); emit parameterChanged(); emit presetChanged(); }

	qreal getOffDelay() const { return m_trigger->getTriggerFilter().getOffDelay(); }
	void setOffDelay(const qreal& value) { m_trigger->getTriggerFilter().setOffDelay(value); emit parameterChanged(); emit presetChanged(); }

	qreal getMaxHold() const { return m_trigger->getTriggerFilter().getMaxHold(); }
	void setMaxHold(const qreal& value) { m_trigger->getTriggerFilter().setMaxHold(value); emit parameterChanged(); emit presetChanged(); }

	// forward calls to TriggerOscParameters
	// see TriggerOscSettings.h for documentation

	QString getOnMessage() const { return m_trigger->getOscParameters().getOnMessage(); }
	void setOnMessage(const QString& value) { m_trigger->getOscParameters().setOnMessage(value); emit presetChanged(); }

	QString getOffMessage() const { return m_trigger->getOscParameters().getOffMessage(); }
	void setOffMessage(const QString& value) { m_trigger->getOscParameters().setOffMessage(value); emit presetChanged(); }

	QString getLevelMessage() const { return m_trigger->getOscParameters().getLevelMessage(); }
	void setLevelMessage(const QString& value) {m_trigger->getOscParameters().setLevelMessage(value); emit presetChanged(); }

	qreal getMinLevelValue() const { return m_trigger->getOscParameters().getMinLevelValue(); }
	void setMinLevelValue(const qreal& value) { m_trigger->getOscParameters().setMinLevelValue(value); emit presetChanged(); }

	qreal getMaxLevelValue() const { return m_trigger->getOscParameters().getMaxLevelValue(); }
	void setMaxLevelValue(const qreal& value) { m_trigger->getOscParameters().setMaxLevelValue(value); emit presetChanged(); }

	QString getLabelText() const { return m_trigger->getOscParameters().getLabelText(); }
	void setLabelText(const QString& value) { m_trigger->getOscParameters().setLabelText(value); emit oscLabelTextChanged(); emit presetChanged(); }

	// --- shortcut to set all OSC parameters:
	void setOscMessages(QString on, QString off, QString level, qreal minLevel, qreal maxLevel, QString labelText)
	{
		m_trigger->getOscParameters().setOnMessage(on);
		m_trigger->getOscParameters().setOffMessage(off);
		m_trigger->getOscParameters().setLevelMessage(level);
		m_trigger->getOscParameters().setMinLevelValue(minLevel);
		m_trigger->getOscParameters().setMaxLevelValue(maxLevel);
		m_trigger->getOscParameters().setLabelText(labelText);
		emit oscLabelTextChanged();
		emit presetChanged();
	}

	// resets all parameters to default values
	void resetParameters();

	// Following is used by GUI components like BandpassPreviewInSpectrum:

	// returns the mid frequency in the range of 0 to 1
	qreal getMidFreqNormalized() const;

	// sets the mid frequency by a normalized value between 0 and 1
	void setMidFreqNormalized(const qreal& value);

	// returns if the trigger is activated
	bool getActive() const { return m_trigger->getTriggerFilter().getOutputIsActive(); }

protected:
	TriggerGenerator* m_trigger;  // pointer to TriggerGenerator object that this Controller refers to
};

#endif // BANDPASSTRIGGERGUICONTROLLER_H
