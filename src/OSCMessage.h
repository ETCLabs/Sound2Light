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

#ifndef OSCMESSAGE_H
#define OSCMESSAGE_H

#include <QtGlobal>
#include <QVector>
#include <QVariant>
#include <QByteArray>

// This class represents an OSC Message.

// currently supported argument types:
// - bool (FALSE, TRUE)
// - int (INT32, INT64, optional from STRING)
// - double (FLOAT32, FLOAT64, optional from STRING)
// - string (STRING)

class OSCMessage
{

public:
	// creates an empty message
	OSCMessage();

	// creates a message from the raw OSC packet data (without frame)
	OSCMessage(const QByteArray& data, bool convertNumberStrings = false);

	// sets the data from the raw OSC packet data (without frame)
	void setData(const QByteArray& data, bool convertNumberStrings = false);

	// returns if the message is valid and not emtpy
	bool isValid() const { return m_isValid; }

	// returns the path as a list of strings
	const QStringList& path() const { return m_path; }

	// returns a specific part of the path
	const QString& pathPart(int index) const;

	// returns if the path starts with a given string
	bool pathStartsWith(QString value) const;

	// returns the path as a single string
	const QString& pathString() const { return m_pathString; }

	// returns a the arguments as a vector of QVariants
	const QVector<QVariant>& arguments() const { return m_arguments; }

	// returns true if there are no arguments or if the first argument is true or 1
	bool isTrue();

	// returns the value of the first argument if it exists and is a number, otherwise return 0.0
	qreal value();

	// ------------- Debug ----------------------

	// returns the arguments as a human readable string for debugging
	QString getArgumentsAsDebugString();

	// prints the message in human readable form to QDebug
	// for debugging only
	void printToQDebug() const;

private:
	// extracts the path string from the raw OSC packet data (without frame)
	static QString getPathFromMessage(const QByteArray& data);

protected:
	QString				m_pathString;  // stores the path as a string
	QStringList			m_path;  // stores the path as a list of strings (separated at slashes)
	QVector<QVariant>	m_arguments;  // list of arguments as QVariants
	bool				m_isValid;  // true, if the message is valid (a path exists)
};

#endif // OSCMESSAGE_H
