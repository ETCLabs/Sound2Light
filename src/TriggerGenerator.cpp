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

#include "TriggerGenerator.h"

#include "OSCNetworkManager.h"

#include <QSettings>
#include <QtMath>

TriggerGenerator::TriggerGenerator(QString name, OSCNetworkManager* osc, bool isBandpass, bool invert, int midFreq)
    : TriggerGeneratorInterface(isBandpass)
	, m_name(name)
    , m_osc(osc)
	, m_invert(invert)
	, m_midFreq(midFreq)
	, m_defaultMidFreq(midFreq)
	, m_width(0.1)
	, m_threshold(0.5)
	, m_isActive(false)
	, m_oscParameters()
	, m_filter(osc, m_oscParameters)
{
	resetParameters();
}

bool TriggerGenerator::checkForTrigger(ScaledSpectrum &spectrum, bool forceRelease)
{
	qreal value;
	if (m_isBandpass) {
		value = spectrum.getMaxLevel(m_midFreq, m_width);
	} else {
		value = spectrum.getMaxLevel();
	}
	if (m_invert) value = 1 - value;

	// check for trigger:
    if ((!m_isActive && value >= m_threshold) && !forceRelease) {
		// activate trigger:
		m_isActive = true;
		m_filter.triggerOn();
    } else if ((m_isActive && value < m_threshold) || forceRelease) {
		// release trigger:
		m_isActive = false;
		m_filter.triggerOff();
	}

	// send level if levelMessage is set:
	// and if difference to last value is greater than 0.001:
	qreal diff = qAbs(m_lastValue - value);
	if (diff > 0.001 && !m_oscParameters.getLevelMessage().isEmpty() && m_threshold > 0) {
		qreal valueUnderThreshold = limit(0, (value / m_threshold), 1);
		qreal minValue = m_oscParameters.getMinLevelValue();
		qreal maxValue = m_oscParameters.getMaxLevelValue();
		qreal scaledValue = minValue + valueUnderThreshold * (maxValue - minValue);
		QString oscMessage = m_oscParameters.getLevelMessage() + QString::number(scaledValue, 'f', 3);
		m_osc->sendMessage(oscMessage);
	}

	m_lastValue = value;
    return m_isActive;
}

void TriggerGenerator::save(QSettings& settings) const
{
	settings.setValue(m_name + "/threshold", m_threshold);
	settings.setValue(m_name + "/midFreq", m_midFreq);
	settings.setValue(m_name + "/width", m_width);
	m_filter.save(m_name, settings);
	m_oscParameters.save(m_name, settings);
}

void TriggerGenerator::restore(QSettings& settings)
{
	setThreshold(settings.value(m_name + "/threshold").toReal());
	setMidFreq(settings.value(m_name + "/midFreq").toReal());
	setWidth(settings.value(m_name + "/width").toReal());
	m_filter.restore(m_name, settings);
	m_oscParameters.restore(m_name, settings);
}

void TriggerGenerator::resetParameters()
{
	setMidFreq(m_defaultMidFreq);
	setWidth(0.1);
	if (m_isBandpass) {
		setThreshold(0.5);
		// default Bandpass settings:
		m_filter.setOnDelay(0.0);
		m_filter.setOffDelay(0.0);
		m_filter.setMaxHold(0.0);
	} else if (!m_invert) {
		// default Level settings:
		setThreshold(0.1);
		m_filter.setOnDelay(0.5);
		m_filter.setOffDelay(2.0);
		m_filter.setMaxHold(0.0);
	} else {
		// default Silence settings:
		setThreshold(0.9);
		m_filter.setOnDelay(2.5);
		m_filter.setOffDelay(1.0);
		m_filter.setMaxHold(0.0);
	}
	m_oscParameters.resetParameters();
}
