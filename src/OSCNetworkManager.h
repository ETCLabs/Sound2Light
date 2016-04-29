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

#ifndef OSCWRAPPER_H
#define OSCWRAPPER_H

#include "OSCParser.h"
#include "OSCMessage.h"
#include "utils.h"

#include <QObject>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimer>


// time to try to connect again after an error in ms
static const int TRY_CONNECT_AGAIN_TIME = 3000;  // ms

// default TCP port
static const quint16 DEFAULT_TCP_PORT = 3032;

// default UDP Tx port
static const quint16 DEFAULT_UDP_TX_PORT = 8001;

// default UDP Rx port
static const quint16 DEFAULT_UDP_RX_PORT = 8000;

// maximum entries in OSC log
static const int MAX_LOG_LENGTH = 1000;


// A class that manages OSC data exchange.
// It can send and receive OSC messages via UDP and TCP
// and supports OSC 1.0 and 1.1 packet-framing.
class OSCNetworkManager : public QObject
{
	Q_OBJECT

public:
	OSCNetworkManager();

	// ------------------- Getter / Setter --------------------

	// returns the IP address used for UDP and TCP
	QHostAddress getIpAddress() const { return m_ipAddress; }
	// sets the IP address used for UDP and TCP
	void setIpAddress(const QHostAddress& value);

	// returns the port to send UDP messages to
	quint16 getUdpTxPort() const { return m_udpTxPort; }
	// sets the port to send UDP messages to
	void setUdpTxPort(const quint16& value) { m_udpTxPort = limit(0, value, 65535); updateUdpBinding(); emit addressChanged(); }

	// returns the port to receive UDP messages from
	quint16 getUdpRxPort() const { return m_udpRxPort; }
	// sets the port to receive UDP messages from
	void setUdpRxPort(const quint16& value) { m_udpRxPort = limit(0, value, 65535); updateUdpBinding(); emit addressChanged(); }

	// returns the port to send and receive TCP messages
	quint16 getTcpPort() const { return m_tcpPort; }
	// sets the port to send and receive TCP messages
	void setTcpPort(const quint16& value);

	// returns if OSC output is enabled
	bool getEnabled() const { return m_isEnabled; }
	// sets if OSC output is enabled
	void setEnabled(bool value) { m_isEnabled = value; }

	// returns the user used for Eos messages
	QString getEosUser() const { return m_eosUser; }

	// sets the user used for Eos messages (default is 0 -> Background User)
	void setEosUser(QString value) { m_eosUser = value; }

	// returns if TCP is used to send OSC messages
	bool getUseTcp() const { return m_useTcp; }
	// sets if TCP is used to send OSC messages
	void setUseTcp(bool value);

	// returns if OSC 1.1 is used (SLIP-Framing)
	bool getUseOsc_1_1() const;
	// sets if OSC 1.1 is used (SLIP-Framing) (default is false)
	void setUseOsc_1_1(bool value);

	// returns if the TCP socket is connected, returns true if UDP is used
	bool isConnected() const;

	// ------------------- Logging --------------------

	// returns the log as a QStringList to be displayed in UI
	const QStringList& getLog() const { return m_log; }

	// enables logging separatly for incoming and outgoing messages
	void enableLogging(bool incoming, bool outgoing) { m_logIncomingMsg = incoming; m_logOutgoingMsg = outgoing;}

	// returns if logging of incoming messages is enabled
	bool getLogIncomingIsEnabled() const { return m_logIncomingMsg; }

	// returns if logging of outgoing messages is enabled
	bool getLogOutgoingIsEnabled() const { return m_logOutgoingMsg; }

	// clears the log
	void clearLog() const { m_log.clear(); emit logChanged(); }

public slots:

	// ------------------- Send Message --------------------

	// Sends an OSC packet with a message in the following format:
	// /x/y/z=1.0,2.0
	void sendMessage(QString messageString, bool forced = false);

	// Sends an OSC message with a string as the only argument
	void sendMessage(QString path, QString argument, bool forced = false);

signals:

	// ------------------- Receive Message --------------------

	// emitted when an OSC messages has been received
	void messageReceived(OSCMessage msg);

	// ------------------- Status --------------------

	// emitted when a packet is successfully sent
	void packetSent();

	// emitted when the useTcp parameter changed
	void useTcpChanged();

	// emitted when the connection state changed
	void isConnectedChanged();

	// emitted when the IP address or a port has changed
	void addressChanged();

	// emitted when the log has changed
	void logChanged() const;



	// ------------------- Private / Internal --------------------

protected:
	// reconnects to host if TCP is used
	void reconnect();

	// binds the UDP socket to the correct port or disables the binding if TCP is used
	void updateUdpBinding();

	// sends raw OSC message data
	void sendMessageData(char* packet, size_t outSize);

	// returns and removes the raw OSC message data from a framed packet in a TCP stream
	// or returns nothing if the OSC message is not yet complete
	QByteArray popPacketFromStreamData(QByteArray& data) const;
	QByteArray popPacketLengthFramedPacketFromStreamData(QByteArray& data) const;
	QByteArray popSlipFramedPacketFromStreamData(QByteArray& data) const;

	// adds a text to the log
	void addToLog(QString text) const;

private slots:

	// tries to establish a connection via TCP
	void tryToConnectTCP();

	// called when the TCP socket is connected, only for logging
	void onConnected();

	// called when an error occurs while connected via TCP
	// starts a timer to retry the connection
	void onError();

	// processes incoming UDP datagrams
	void readIncomingUdpDatagrams();

	// processes incoming TCP stream
	void readIncomingTcpStream();

	// processes incoming raw data and checks if it is an OSC bundle
	void processIncomingRawData(QByteArray msgData);

	// processes incoming single raw OSC messages
	void processIncomingRawMessage(QByteArray msgData);

private:
	QUdpSocket				m_udpSocket;  // UDP socket
	QTcpSocket				m_tcpSocket;  // TCP socket
	QHostAddress			m_ipAddress;  // IP address for UDP and TCP
	quint16					m_udpTxPort;  // UDP Tx Port
	quint16					m_udpRxPort;  // UDP Rx Port
	quint16					m_tcpPort;  // TCP Port (Rx and Tx)
	bool					m_isEnabled;  // output enabled (can be overwriten by "forced" argument)
	bool					m_useTcp;  // true if TCP should be used instead of UDP
	QTimer					m_tryConnectAgainTimer;  // Timer to connect again after error
	OSCStream::EnumFrameMode m_tcpFrameMode;  // TCP frame mode (OSC 1.0 or 1.1 SLIP)
	mutable QStringList		m_log;  // log of incoming and / or outgoing messages
	bool					m_logIncomingMsg;  // true if incoming messages should be logged
	bool					m_logOutgoingMsg;  // true if outgoing messages should be logged
	QString					m_eosUser;  // number of the Eos User (default = 0 -> Background User)
	QByteArray				m_incompleteStreamData;  // may contain the begin of an incomplete OSC packet (from TCP stream)
};

#endif // OSCWRAPPER_H
