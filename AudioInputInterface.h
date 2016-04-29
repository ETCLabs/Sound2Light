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

#ifndef AUDIOINPUTINTERFACE_H
#define AUDIOINPUTINTERFACE_H

#include "MonoAudioBuffer.h"

#include <QString>
#include <QStringList>

// An interface for an audio input.
// The input device can be selected
// and the volume can be changed.
// Calls putSamples() of MonoAudioBuffer when new data is available.

class AudioInputInterface
{

public:
	explicit AudioInputInterface(MonoAudioBuffer* buffer) : m_buffer(buffer) {}
	virtual ~AudioInputInterface() {}

	// returns a list of the names of all available input devices
	virtual QStringList getAvailableInputs() const = 0;

	// returns the name of the default input device
	// returns an empty string if there are no input devices
	virtual QString getDefaultInputName() const = 0;

	// returns the name of the currently used input device
	virtual QString getActiveInputName() const = 0;
	// changes the used input device by its name
	virtual void setInputByName(const QString& name) = 0;

	// returns the volume of the input device [0...1]
	virtual qreal getVolume() const = 0;
	// sets the volume of the input device [0...1]
	virtual void setVolume(const qreal& value) = 0;

protected:
	MonoAudioBuffer* m_buffer;  // a buffer storing the last audio samples
};


#endif // AUDIOINPUTINTERFACE_H
