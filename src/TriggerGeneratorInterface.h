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

#ifndef TRIGGERGENERATORINTERFACE_H
#define TRIGGERGENERATORINTERFACE_H

#include "ScaledSpectrum.h"
#include "TriggerFilter.h"

#include <QObject>


// Forward declaration to reduce dependencies:
class OSCNetworkManager;


// An interface that describes an object that can generate trigger signals.
class TriggerGeneratorInterface
{

public:
    explicit TriggerGeneratorInterface(bool isBandpass)
        : m_isBandpass(isBandpass) {}
	virtual ~TriggerGeneratorInterface() {}

	// checks if a signal should be triggered by analyzing the given spectrum
    // forceRelease is true when low solo mode is active and a lower trigger was activated
    virtual bool checkForTrigger(ScaledSpectrum& spectrum, bool forceRelease) = 0;

	// returns a reference to the internal TriggerFilter
	virtual TriggerFilter& getTriggerFilter() = 0;

	// saves parameters in QSettings
	virtual void save(QSettings& settings) const = 0;

	// restores parameters from QSettings
	virtual void restore(QSettings& settings) = 0;

    // returns if this is a Bandpass trigger generator
    bool isBandpass() const { return m_isBandpass; }

protected:
    const bool m_isBandpass;  // true if this is a bandpass (with frequency and width parameter)
};


#endif // TRIGGERGENERATORINTERFACE_H
