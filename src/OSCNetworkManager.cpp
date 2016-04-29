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

#include "OSCNetworkManager.h"

#include <QTime>

// http://www.rfc-editor.org/rfc/rfc1055.txt
#define SLIP_END		0xc0    /* indicates end of packet */
#define SLIP_ESC		0xdb    /* indicates byte stuffing */
#define SLIP_ESC_END	0xdc    /* ESC ESC_END means END data byte */
#define SLIP_ESC_ESC	0xdd    /* ESC ESC_ESC means ESC data byte */

#define SLIP_CHAR(x)	static_cast<char>(static_cast<unsigned char>(x))

OSCNetworkManager::OSCNetworkManager()
	: m_ipAddress(QHostAddress::LocalHost)
	, m_udpTxPort(DEFAULT_UDP_TX_PORT)
	, m_udpRxPort(DEFAULT_UDP_RX_PORT)
	, m_tcpPort(DEFAULT_TCP_PORT)
	, m_isEnabled(true)
	, m_useTcp(true)
	, m_tcpFrameMode(OSCStream::FRAME_MODE_1_0)
	, m_log()
	, m_logIncomingMsg(true)
	, m_logOutgoingMsg(true)
	, m_eosUser("0")
	, m_incompleteStreamData()
{
	// prepare timer that is used to try to connect again to TCP target:
	m_tryConnectAgainTimer.setSingleShot(true);
	connect(&m_tryConnectAgainTimer, SIGNAL(timeout()), this, SLOT(tryToConnectTCP()));

	// connect TCP socket with onConnected and onError slots:
	connect(&m_tcpSocket, SIGNAL(connected()), this, SLOT(onConnected()));
	connect(&m_tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(onError()));
	connect(&m_udpSocket, SIGNAL(readyRead()), this, SLOT(readIncomingUdpDatagrams()));
	connect(&m_tcpSocket, SIGNAL(readyRead()), this, SLOT(readIncomingTcpStream()));

	// try to connect:
	reconnect();
	updateUdpBinding();
}

void OSCNetworkManager::setIpAddress(const QHostAddress &value)
{
	if (value == m_ipAddress) return;
	// set new IP address:
	m_ipAddress = value;
	reconnect();
	updateUdpBinding();
	emit addressChanged();
}

void OSCNetworkManager::setTcpPort(const quint16 &value)
{
	if (value == m_tcpPort) return;
	m_tcpPort = limit(0, value, 65535);
	reconnect();
	emit addressChanged();
}

void OSCNetworkManager::sendMessage(QString messageString, bool forced)
{
	if (!m_isEnabled && !forced) return;

	// replace <USER> with chosen user number, used for Eos messages
	messageString.replace("<USER>", m_eosUser);

	size_t outSize;
	char* packet = OSCPacketWriter::CreateForString(messageString.toLatin1().data(), outSize);
	sendMessageData(packet, outSize);

	// Log if logging of outgoing messages is enabled:
	if (m_logOutgoingMsg) {
		addToLog("[Out] " + messageString);
	}
}

void OSCNetworkManager::sendMessage(QString path, QString argument, bool forced)
{
	if (!m_isEnabled && !forced) return;

	// replace <USER> with chosen user number, used for Eos messages
	path.replace("<USER>", m_eosUser);

	size_t outSize;
	OSCPacketWriter packetWriter(path.toStdString());
	packetWriter.AddString(argument.toStdString());
	char* packet = packetWriter.Create(outSize);
	sendMessageData(packet, outSize);

	// Log if logging of outgoing messages is enabled:
	if (m_logOutgoingMsg) {
		addToLog("[Out] " + path + "=" + argument);
	}
}

void OSCNetworkManager::sendMessageData(char* packet, size_t outSize)
{
	// send packet either with UDP or TCP:
	if (m_useTcp) {
		// check if TCP socket is connected:
		if (m_tcpSocket.state() == QAbstractSocket::ConnectedState) {
			// socket is connected
			// for TCP transmission the packet has to be framed:
			char* framedPacket = OSCStream::CreateFrame(m_tcpFrameMode, packet, outSize);
			m_tcpSocket.write(framedPacket, outSize);
			delete[] framedPacket;
		}
	} else {
		// use UDP:
		m_udpSocket.writeDatagram(packet, outSize, m_ipAddress, m_udpTxPort);
	}

	delete[] packet;
	emit packetSent();
}

void OSCNetworkManager::setUseTcp(bool value)
{
	m_useTcp = value;

	// try to connect:
	m_tryConnectAgainTimer.stop();
	if (m_useTcp) {
		m_tryConnectAgainTimer.start(20);
	}
	updateUdpBinding();
	emit useTcpChanged();
	emit isConnectedChanged();
}

bool OSCNetworkManager::getUseOsc_1_1() const
{
	return m_tcpFrameMode == OSCStream::FRAME_MODE_1_1;
}

void OSCNetworkManager::setUseOsc_1_1(bool value)
{
	m_tcpFrameMode = value ? OSCStream::FRAME_MODE_1_1 : OSCStream::FRAME_MODE_1_0;
}

void OSCNetworkManager::reconnect()
{
	// disconnect from old IP address if connected:
	if (m_tcpSocket.state() != QAbstractSocket::UnconnectedState) {
		m_tcpSocket.abort();
	}
	// connect to new IP if useTcp:
	if (m_useTcp) {
		m_tryConnectAgainTimer.start(20);
	}
}

void OSCNetworkManager::updateUdpBinding()
{
	if (m_useTcp) {
		// unbind UDP port:
		m_udpSocket.close();
	} else {
		// bind UDP port to new port:
		m_udpSocket.close();
		m_udpSocket.bind(m_udpRxPort);
	}
}

QByteArray OSCNetworkManager::popPacketFromStreamData(QByteArray& data) const
{
	if (m_tcpFrameMode == OSCStream::FRAME_MODE_1_0) {
		return popPacketLengthFramedPacketFromStreamData(data);
	} else {
		return popSlipFramedPacketFromStreamData(data);
	}
}

QByteArray OSCNetworkManager::popPacketLengthFramedPacketFromStreamData(QByteArray& tcpData) const
{
	// the first 4 bytes are the length of the OSC message following as an int32:
	int32_t messageLength = reinterpret_cast<int32_t*>(tcpData.data())[0];
	// adjust the network byte order (?):
	OSCArgument::Swap32(&messageLength);

	// check if the messageLength is in the range of a valid message:
	if (messageLength <= 0 || messageLength > 512) {
		// this is not a valid OSC message
		// received data will be discarded:
		addToLog("[In] Invalid data received (message length in TCP packet is out of range). Check Protocol Settings.");
		tcpData.resize(0);
		return QByteArray();
	}

	// check if the message is completely received:
	if (messageLength > tcpData.size()) {
		// the message length is greater than the size of the data received
		// so there is nothing to pop / return yet:
		return QByteArray();
	}

	// create a new QByteArray containing only the OSC message data:
	QByteArray messageData = QByteArray(tcpData.data() + sizeof(messageLength), messageLength);
	// remove the messageLength and the message data from the received data:
	tcpData.remove(0, messageLength + sizeof(messageLength));

	return messageData;
}

QByteArray OSCNetworkManager::popSlipFramedPacketFromStreamData(QByteArray& tcpData) const
{
	// A SLIP framed packet begins and ends with a SLIP END character.

	// find first SLIP END character:
	// most probably it is the first character:
	int firstSlipEndPosition = 0;
	if (tcpData[0] != SLIP_CHAR(SLIP_END)) {
		for (int i=1; i<tcpData.size(); ++i) {
			if (tcpData[i] == SLIP_CHAR(SLIP_END)) {
				firstSlipEndPosition = i;
				break;
			}
		}
		if (firstSlipEndPosition == 0) {
			// if first slip end postion is still 0,
			// this means there is no SLIP END character in the data
			// -> discard the data and return nothing:
			addToLog("[In] Invalid data received (missing SLIP END character). Check Protocol Settings.");
			tcpData.resize(0);
			return QByteArray();
		}
	}

	// find second SLIP END character:
	int secondSlipEndPosition = -1;
	for (int i=firstSlipEndPosition+1; i<tcpData.size(); ++i) {
		if (tcpData[i] == SLIP_CHAR(SLIP_END)) {
			secondSlipEndPosition = i;
			break;
		}
	}
	if (secondSlipEndPosition < 0) {
		// there is no second SLIP END character
		// -> message is not yet complete:
		return QByteArray();
	}

	// at this point we should have found two SLIP END characters
	Q_ASSERT(secondSlipEndPosition > firstSlipEndPosition);

	int messageLength = secondSlipEndPosition - firstSlipEndPosition - 1;
	// create a new QByteArray containing only the OSC message data:
	QByteArray messageData = QByteArray(tcpData.data() + firstSlipEndPosition + 1, messageLength);
	// remove everything before the second SLIP END from the received data:
	tcpData.remove(0, secondSlipEndPosition + 1);

	// replace characters that have been escaped because of SLIP framing:
	messageData.replace(SLIP_CHAR(SLIP_ESC) + SLIP_CHAR(SLIP_ESC_END), SLIP_CHAR(SLIP_END));
	messageData.replace(SLIP_CHAR(SLIP_ESC) + SLIP_CHAR(SLIP_ESC_ESC), SLIP_CHAR(SLIP_ESC));

	return messageData;
}

void OSCNetworkManager::addToLog(QString text) const
{
	QString time = "[" + QTime::currentTime().toString() + "] ";
	m_log.prepend(time + text);
	if (m_log.size() > MAX_LOG_LENGTH) m_log.removeLast();
	emit logChanged();
}

void OSCNetworkManager::tryToConnectTCP()
{
	if (!m_useTcp) return;
	// do nothing if socket is not in unconnected state:
	if (m_tcpSocket.state() != QAbstractSocket::UnconnectedState) return;
	// try to connect:
	m_tcpSocket.connectToHost(m_ipAddress, m_tcpPort);
}

void OSCNetworkManager::onConnected()
{
	emit isConnectedChanged();
}

void OSCNetworkManager::onError()
{
	emit isConnectedChanged();
	qDebug() << "TCP Error:" << m_tcpSocket.errorString();

	// try again if user still wants to use TCP:
	if (m_useTcp) {
		m_tryConnectAgainTimer.start(TRY_CONNECT_AGAIN_TIME);
	}
}

bool OSCNetworkManager::isConnected() const
{
	if (m_useTcp) {
		return m_tcpSocket.state() == QAbstractSocket::ConnectedState;
	} else {
		// if UDP is used, the connection can not be "not connected"
		return true;
	}
}

void OSCNetworkManager::readIncomingUdpDatagrams()
{
	while (m_udpSocket.hasPendingDatagrams()) {
		// prepare empty variables to be written in:
		QByteArray datagram;
		datagram.resize(m_udpSocket.pendingDatagramSize());
		QHostAddress sender;
		quint16 senderPort;

		// get data from socket:
		m_udpSocket.readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

		// process data:
		processIncomingRawData(datagram);
	}
}

void OSCNetworkManager::readIncomingTcpStream()
{
	QByteArray streamData = m_tcpSocket.readAll();

	// check if there is incomplete stream data left from last call:
	if (!m_incompleteStreamData.isEmpty()) {
		streamData = m_incompleteStreamData + streamData;
	}

	// as long as there is stream data left:
	while (streamData.size()) {
		// try to get a complete packet from the stream data:
		QByteArray packet = popPacketFromStreamData(streamData);

		if (packet.isEmpty()) {
			// there is no more complete packet
			// but maybe there is incomplete packet data left:
			m_incompleteStreamData = streamData;
			return;
		}

		processIncomingRawData(packet);
	}
}

void OSCNetworkManager::processIncomingRawData(QByteArray msgData)
{
	// check if the data is a single message or a bundle of messages:
	if (msgData[0] == '/') {
		// it starts with a "/" -> it is a single message:
		processIncomingRawMessage(msgData);
	} else if (msgData.startsWith("#bundle")) {
		// it starts with "#bundle" -> it is a bundle
		// remove "#bundle" string (8 bytes) and unused timetag (8 bytes):
		msgData.remove(0, 16);
		// try to get all messages in the bundle:
		while (msgData.size()) {
			// each message starts with the length of the message as int32
			// this is the same as a TCP 1.0 packet, so the function for that can be used:
			QByteArray message = popPacketLengthFramedPacketFromStreamData(msgData);
			// process the message:
			processIncomingRawMessage(message);
		}
	} else {
		// invalid data
		addToLog("[In] [Invalid] Raw: " + QString::fromLatin1(msgData.data(), msgData.size()));
	}
}

void OSCNetworkManager::processIncomingRawMessage(QByteArray msgData)
{
	// build an OSC message from the data:
	OSCMessage msg(msgData);

	// Log if logging of incoming messages is enabled:
	if (m_logIncomingMsg) {
		if (msg.isValid()) {
			addToLog("[In] " + msg.pathString() + msg.getArgumentsAsDebugString());
		} else {
			addToLog("[In] [Invalid] Raw: " + QString::fromLatin1(msgData.data(), msgData.size()));
		}
	}

	// emit message received signal:
	if (msg.isValid()) {
		emit messageReceived(msg);
	}
}
