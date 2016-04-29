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

#include "MonoAudioBuffer.h"

#include "FFTAnalyzer.h"

MonoAudioBuffer::MonoAudioBuffer(int capacity)
	: m_capacity(capacity)
	, m_buffer(capacity)
{
	for (int i=0; i < m_buffer.capacity(); ++i) {
		m_buffer.push_back(0.0);
	}
}

void MonoAudioBuffer::putSamples(QVector<qreal>& data, const int& channelCount)
{
	// convert incoming mulitchannel audio to mono:
	convertToMonoInplace(data, channelCount);

	for (int i=0; i<data.size(); ++i) {
		m_buffer.push_back(data[i]);
	}
}

void MonoAudioBuffer::convertToMonoInplace(QVector<qreal>& data, const int& channelCount) const {
	// - assumes that data for two channels A and B looks like ABABABABAB...
	// - channels are average to get mono signal
	// - result will be written inplace and data object will be decreased in size
	switch (channelCount) {
	case 1:
		break;
	case 2:
		for (int i=0; i<data.size(); i+=2) {
			// calculate average of both channels:
			data[i/2] = (data[i] + data[i+1]) / 2.0;
		}
		data.resize(data.size() / 2);
		break;
	default:
		for (int i=0; i<data.size(); i+=channelCount) {
			qreal mono = 0.0;
			for (int ch=0; ch<channelCount; ++ch) {
				// sum up all channels:
				mono += data[i+ch];
			}
			// calculate average of all channels:
			mono /= channelCount;
			data[i/channelCount] = mono;
		}
		data.resize(data.size() / channelCount);
		break;
	}

}
