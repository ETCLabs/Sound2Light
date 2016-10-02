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

#include "TriggerOscParameters.h"

TriggerOscParameters::TriggerOscParameters()
	: m_onMessage()
	, m_offMessage()
	, m_levelMessage()
	, m_minLevelValue(0)
	, m_maxLevelValue(1)
	, m_labelText()
{

}

void TriggerOscParameters::save(const QString name, QSettings &settings) const
{
	settings.setValue(name + "/osc/onMessage", m_onMessage);
	settings.setValue(name + "/osc/offMessage", m_offMessage);
	settings.setValue(name + "/osc/levelMessage", m_levelMessage);
    settings.setValue(name + "/osc/rangeMessage", m_rangeMessage);
	settings.setValue(name + "/osc/minLevelValue", m_minLevelValue);
	settings.setValue(name + "/osc/maxLevelValue", m_maxLevelValue);
	settings.setValue(name + "/osc/labelText", m_labelText);
}

void TriggerOscParameters::restore(const QString name, QSettings &settings)
{
	setOnMessage(settings.value(name + "/osc/onMessage").toString());
	setOffMessage(settings.value(name + "/osc/offMessage").toString());
	setLevelMessage(settings.value(name + "/osc/levelMessage").toString());
//    setRangeMessage(settings.value(name + "/osc/rangeMessage").toStringList());
	setMinLevelValue(settings.value(name + "/osc/minLevelValue").toReal());
	setMaxLevelValue(settings.value(name + "/osc/maxLevelValue").toReal());
	setLabelText(settings.value(name + "/osc/labelText").toString());
}

void TriggerOscParameters::resetParameters()
{
	setOnMessage("");
	setOffMessage("");
	setLevelMessage("");
	setMinLevelValue(0.0);
	setMaxLevelValue(1.0);
	setLabelText("");
}
