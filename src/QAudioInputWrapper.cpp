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

#include "QAudioInputWrapper.h"

#include "utils.h"

#include <QList>
#include <QVector>
#include <QByteArray>
#include <QDebug>

#include <iostream>

QAudioInputWrapper::QAudioInputWrapper(MonoAudioBuffer *buffer)
    : AudioInputInterface(buffer)
    , m_audioInput(0)
	, m_audioIODevice(0)
{
    // Set up the desired format:
    // If the input device doesn't support this the nearest format will be used.
    m_desiredAudioFormat.setSampleRate(44100);
    m_desiredAudioFormat.setChannelCount(2);
    m_desiredAudioFormat.setSampleSize(16);
    m_desiredAudioFormat.setCodec("audio/pcm");
    m_desiredAudioFormat.setByteOrder(QAudioFormat::LittleEndian);
    m_desiredAudioFormat.setSampleType(QAudioFormat::SignedInt);
}

QAudioInputWrapper::~QAudioInputWrapper()
{
	// Close and delete previous input device:
	if (m_audioInput) {
		m_audioInput->stop();
	}
	if (m_audioIODevice && m_audioIODevice->isOpen()) {
		m_audioIODevice->close();
	}
	delete m_audioInput;
}

QStringList QAudioInputWrapper::getAvailableInputs() const
{
    // get List of input device names from QList<QAudioDeviceInfo>:
    QStringList deviceList;
    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    foreach(QAudioDeviceInfo device, devices) {
        deviceList.append(device.deviceName());
    }
	return deviceList;
}

QString QAudioInputWrapper::getDefaultInputName() const
{
	QStringList devices = getAvailableInputs();
	// if there are no devices, return empty string:
	if (devices.size() <= 0) return "";
	QString defaultInputName = QAudioDeviceInfo::defaultInputDevice().deviceName();
	return defaultInputName;
}

void QAudioInputWrapper::setInputByName(const QString &inputName)
{
    // Close and delete previous input device:
    if (m_audioInput) {
        m_audioInput->stop();
    }
    if (m_audioIODevice && m_audioIODevice->isOpen()) {
        disconnect(m_audioIODevice, SIGNAL(readyRead()), this, SLOT(audioDataReady()));
        m_audioIODevice->close();
    }
    delete m_audioInput;
    m_audioInput = 0;

    // Get device info of new input:
    QList<QAudioDeviceInfo> devices = QAudioDeviceInfo::availableDevices(QAudio::AudioInput);
    QAudioDeviceInfo info = *devices.begin();
    foreach(QAudioDeviceInfo device, devices) {
        if (device.deviceName() == inputName) {
            info = device;
        }
    }

    // check if desired format is supported:
    if (!info.isFormatSupported(m_desiredAudioFormat)) {
        qWarning() << "Default audio format not supported, trying to use the nearest.";
        m_actualAudioFormat = info.nearestFormat(m_desiredAudioFormat);
    } else {
        m_actualAudioFormat = m_desiredAudioFormat;
    }

    // create new input:
    m_activeInputName = inputName;
    m_audioInput = new QAudioInput(info, m_actualAudioFormat, this);
	m_audioInput->setVolume(1.0);
    m_audioIODevice = m_audioInput->start();
    connect(m_audioIODevice, SIGNAL(readyRead()), this, SLOT(audioDataReady()));
}

qreal QAudioInputWrapper::getVolume() const
{
	if (!m_audioInput) return 0.0;
	return m_audioInput->volume();
}

void QAudioInputWrapper::setVolume(const qreal &value)
{
	if (!m_audioInput) return;
	m_audioInput->setVolume(limit(0, value, 1));
}

void QAudioInputWrapper::audioDataReady()
{
	// read data from input as QByteArray:
	QByteArray data = m_audioIODevice->readAll();
    const int bytesPerSample = m_actualAudioFormat.sampleSize() / 8;
    const std::size_t numSamples = data.size() / bytesPerSample;
    QVector<qreal> realData(numSamples);
    const char *ptr = data.constData();

    for (std::size_t i=0; i<numSamples; ++i) {
        // interpret byte in QByteArray as int16 (because 16bit is desired sample format):
        const qint16 pcmSample = *reinterpret_cast<const qint16*>(ptr);
        // convert to real and scale down to range [-1.0, 1.0]:
        const qreal scaled = qreal(pcmSample) / 32768;
        realData[i] = scaled;
        ptr += bytesPerSample;
    }

    // Call MonoAudioBuffer as next element in processing chain:
	m_buffer->putSamples(realData, m_actualAudioFormat.channelCount());
}
