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

#ifndef BANDPASSTRIGGERGENERATOR_H
#define BANDPASSTRIGGERGENERATOR_H

#include "TriggerGeneratorInterface.h"
#include "ScaledSpectrum.h"
#include "TriggerOscParameters.h"
#include "utils.h"

#include <QObject>
#include <QSettings>
#include <QDebug>


// Forward declaration to reduce dependencies:
class OSCNetworkManager;


// A trigger generator that is activated when the max level
// either within a band of frequencies or in the total spectrum
// is over a certain threshold.
class TriggerGenerator : public TriggerGeneratorInterface
{

public:
	// Creates a new TriggerGenerator object with the name name and an OSCWrapper instance osc.
	// This will be a bandpass trigger generator if isBandpass is true. If not it is a "level" / "envelope" trigger generator.
	// If invert is true the max level will be inverted.
	explicit TriggerGenerator(QString name, OSCNetworkManager* osc, bool isBandpass = true, bool invert = false,
									  int midFreq = 1000);

	// ---------------- Parameters -------------

    // returns wether the frequency band is muted
    bool getMute() const { return m_mute; }

    // toggles mute on and off
    void toggleMute();

    // toggles mute on and off
    void setMute(bool mute);


	// returns the middle frequency of the frequency band [20...22050]
	int getMidFreq() const { return m_midFreq; }

	// sets the middle frequency of the frequency band [20...22050]
	void setMidFreq(const int& value) { m_midFreq = limit(10, value, 22050); }


	// returns the width of the frequency band [0...1]
	qreal getWidth() const { return m_width; }

	// sets the width of the frequency band ]0...1]
	void setWidth(const qreal& value) { m_width = limit(0.00001, value, 1); }


	// returns the threshold that is used to generate the trigger [0...1]
	qreal getThreshold() const { return m_threshold; }

	// sets the threshold that is used to generate the trigger [0...1]
	void setThreshold(const qreal& value) { m_threshold = limit(0, value, 1); }

	// returns a reference to the internal TriggerFilter
	TriggerFilter& getTriggerFilter() override { return m_filter; }

	// returns a reference to the OSC message settings of this trigger
	TriggerOscParameters& getOscParameters() { return m_oscParameters; }


	// --------------- calculate Level and Trigger -----------

	// returns the last maximum value within the frequency band [0...1]
	qreal getCurrentLevel() const { return m_lastValue; }

	// checks if the max level within the frequency band is greater than the threshold
    bool checkForTrigger(ScaledSpectrum& spectrum, bool forceRelease) override;

	// ---------------- Save and Restore ---------------

	// saves parameters in QSettings
	void save(QSettings& settings) const override;

	// restores parameters from QSettings
	void restore(QSettings& settings) override;

	// resets all parameters to default values
	void resetParameters();

protected:
    const QString	m_name;  // name of the Trigger (used for save, restore and UI)
    OSCNetworkManager*	m_osc;  // pointer to OSCNetworkManager instance (i.e. of MainController)
	const bool		m_invert;  // true if signal values should be inverted (i.e. for "silence" trigger)
    bool            m_mute; // true if the band is muted, which will supress OSC Output
	int				m_midFreq;  // middle frequency of bandpass in Hz
	const int		m_defaultMidFreq;  // default midFreq in Hz, used for reset
	qreal			m_width;  // width of bandpass [0...1]
	qreal			m_threshold;  // threshold for Trigger generation [0...1]
	bool			m_isActive;  // true if value is above threshold
	qreal			m_lastValue;  // last value (used to check if new level message should be sent)
	TriggerOscParameters m_oscParameters;  // OSC parameter object (stores OSC messages)
	TriggerFilter m_filter;  // TriggerFilter instance (for "filtering" in time domain: delays and decay)

};

#endif // BANDPASSTRIGGERGENERATOR_H
