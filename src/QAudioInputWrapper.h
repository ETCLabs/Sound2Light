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

#ifndef QAUDIOINPUTWRAPPER_H
#define QAUDIOINPUTWRAPPER_H

#include "AudioInputInterface.h"
#include "MonoAudioBuffer.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QtMultimedia/QAudio>
#include <QtMultimedia/QAudioInput>
#include <QtMultimedia/QAudioFormat>


// An AudioInputInterface implementation with QAudioInput
// see AudioInputInterface.h for documentation of overridden functions
class QAudioInputWrapper : public QObject, public AudioInputInterface
{
	Q_OBJECT

public:
	explicit QAudioInputWrapper(MonoAudioBuffer* m_buffer);
	~QAudioInputWrapper() override;

	QStringList getAvailableInputs() const override;

	QString getDefaultInputName() const override;

	QString getActiveInputName() const override { return m_activeInputName; }
	void setInputByName(const QString& name) override;

	qreal getVolume() const override;
	void setVolume(const qreal& value) override;


private slots:
	// Converts the incoming QByteArray to QVector<qreal>
	// and scales down the values to [-1...1]
	// - called by Qt singal when audio data is ready
	void audioDataReady();

protected:
	QAudioFormat	m_desiredAudioFormat;  // the desired audio format, may not be available
	QAudioFormat	m_actualAudioFormat;  // the actual audio format used for recording
	QAudioInput*	m_audioInput;  // a pointer to the used audio input object
	QIODevice*		m_audioIODevice;  // a pointer to the stream like "device" used while recording
	QString			m_activeInputName;  // the name of the active audio input
};

#endif // QAUDIOINPUTWRAPPER_H
