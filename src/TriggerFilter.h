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

#ifndef TRIGGERFILTER_H
#define TRIGGERFILTER_H

#include "TriggerOscParameters.h"

#include <QObject>
#include <QString>
#include <QTimer>
#include <QSettings>
#include <QtMath>


// Forward declaration to reduce dependencies:
class OSCNetworkManager;


// This class is used to receive trigger signals from a TriggerGenerator
// and filter them in time domain accordingly to some parameters.
class TriggerFilter : public QObject
{
	Q_OBJECT

public:
    explicit TriggerFilter(OSCNetworkManager* osc, TriggerOscParameters& oscParameters, bool mute);

    void setMute(bool mute) { m_mute = mute; }

	// returns the on delay time in seconds
	qreal getOnDelay() const { return m_onDelay; }

	// sets the on delay time in seconds
	void setOnDelay(const qreal& value) { m_onDelay = qMax(0.0, value); }


	// returns the off delay time in seconds
	qreal getOffDelay() const { return m_offDelay; }

	// sets the off delay time in seconds
	void setOffDelay(const qreal& value) { m_offDelay = qMax(0.0, value); }


	// returns the max hold time in seconds
	qreal getMaxHold() const { return m_maxHold; }

	// sets the max hold time in seconds
	void setMaxHold(const qreal& value) { m_maxHold = qMax(0.0, value); }


	// to be called when the trigger from the raw signal is activated
	// - starts onDelayTimer
	void triggerOn();
	// to be called when the trigger from the raw signal is released
	// - starts offDelayTimer
	void triggerOff();

	// to be called when the filtered on signal should be sent
	void sendOnSignal();
	// to be called when the filtered off signal should be sent
	void sendOffSignal();

	// returns if trigger output is active
	bool getOutputIsActive() const { return m_outputIsActive; }

	// saves parameters in QSettings
	void save(const QString name, QSettings& settings) const;

	// restores parameters from QSettings
	void restore(const QString name, QSettings& settings);

signals:
	// will be emitted when filtered on signal is sent
	void onSignalSent();
	// will be emitted when filtered off signal is sent
	void offSignalSent();

public slots:
	// will be called after on delay time if no triggerOff happend
	// - calls sendOnSignal() and starts holdMaxTimer
	void onOnDelayEnd();

	// will be called after off delay time if no triggerOn happend
	// - calls sendOffSignal()
	void onOffDelayEnd();

	// will be called after max hold time if no triggerOff happend
	// - calls sendOffSignal()
	void onMaxHoldEnd();

protected:
    bool        m_mute; // Wether the associated band is muted
	qreal		m_onDelay;  // On delay in seconds
	qreal		m_offDelay;  // Off delay in seconds
	qreal		m_maxHold;  // max hold time (decay) in seconds
	bool		m_outputIsActive;  // true if trigger is activated and not yet released

	QTimer		m_onDelayTimer;  // Timer object for On delay
	QTimer		m_maxHoldTimer;  // Timer object for max hold (decay)
	QTimer		m_offDelayTimer;  // Timer object for Off delay

	OSCNetworkManager* m_osc;  // pointer to OSCNetworkManager instance (i.e. of MainController)
	TriggerOscParameters& m_oscParameters;  // Reference to OSC parameters (message strings) to use

};

#endif // TRIGGERFILTER_H
