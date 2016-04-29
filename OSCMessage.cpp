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

#include "OSCMessage.h"

#include "OSCParser.h"

#include <QDebug>

OSCMessage::OSCMessage()
	: m_pathString()
	, m_path()
	, m_arguments(0)
	, m_isValid(false)
{

}

OSCMessage::OSCMessage(const QByteArray &data, bool convertNumberStrings)
	: m_pathString()
	, m_path()
	, m_arguments(0)
	, m_isValid(false)
{
	setData(data, convertNumberStrings);
}

void OSCMessage::setData(const QByteArray &data, bool convertNumberStrings)
{
	// set path:
	m_pathString = getPathFromMessage(data);
	m_path = m_pathString.split("/");
	// since the string always starts with "/" the first element of path is empty:
	m_path.removeFirst();

	// message is valid when the path is not empty:
	if (!m_path.isEmpty()) {
		m_isValid = true;
	}

	// set arguments:
	size_t argumentCount = 0xffffffff;  // this is also maximum arguments
	OSCArgument* args = OSCArgument::GetArgs(const_cast<char*>(data.data()), data.size(), argumentCount);
	if( args ) {
		// iterate over arguments and try to convert them to corresponding QVariants:
		for(size_t i=0; i<argumentCount; i++) {
			OSCArgument &arg = args[i];
			int intValue = 0;
			double doubleValue = 0;
			std::string stringValue = "";

			switch (arg.GetType()) {
			case OSCArgument::OSC_TYPE_FALSE:
				m_arguments.append(QVariant(false));
				break;
			case OSCArgument::OSC_TYPE_TRUE:
				m_arguments.append(QVariant(true));
				break;
			case OSCArgument::OSC_TYPE_INT32:  // intended fallthrough
			case OSCArgument::OSC_TYPE_INT64:
				arg.GetInt(intValue);
				m_arguments.append(QVariant(intValue));
				break;
			case OSCArgument::OSC_TYPE_FLOAT32:  // intended fallthrough
			case OSCArgument::OSC_TYPE_FLOAT64:
				arg.GetDouble(doubleValue);
				m_arguments.append(QVariant(doubleValue));
				break;
			case OSCArgument::OSC_TYPE_STRING:
				if (convertNumberStrings && OSCArgument::IsIntString(arg.GetRaw())) {
					arg.GetInt(intValue);
					m_arguments.append(QVariant(intValue));
				} else if (convertNumberStrings && OSCArgument::IsFloatString(arg.GetRaw())) {
					arg.GetDouble(doubleValue);
					m_arguments.append(QVariant(doubleValue));
				} else {
					arg.GetString(stringValue);
					m_arguments.append(QVariant(QString::fromStdString(stringValue)));
				}
				break;
			default:
				qDebug() << "OSC Argument Type not supported. Type: " << OSCArgument::GetCharFromArgumentType(arg.GetType());
				m_arguments.append(QVariant());
			}  // end switch
		}  // end for (arguments)

		delete[] args;
	}  // end if (args)
}

const QString &OSCMessage::pathPart(int index) const
{
	Q_ASSERT(index > m_path.size());
	return m_path[index];
}

bool OSCMessage::pathStartsWith(QString value) const
{
	return m_pathString.startsWith(value);
}

bool OSCMessage::isTrue()
{
	if (m_arguments.isEmpty()) return true;
	if (m_arguments.first().type() == QVariant::Double && m_arguments.first().toDouble() > 0.99) return true;
	if (m_arguments.first().type() == QVariant::Int && m_arguments.first().toInt() == 1) return true;
	if (m_arguments.first().type() == QVariant::Bool && m_arguments.first().toBool()) return true;
	return false;
}

qreal OSCMessage::value()
{
	if (m_arguments.isEmpty()) return 0.0;
	if (m_arguments.first().type() == QVariant::Double) return m_arguments.first().toDouble();
	if (m_arguments.first().type() == QVariant::Int) return qreal(m_arguments.first().toInt());
	return 0.0;
}

QString OSCMessage::getArgumentsAsDebugString()
{
	QString result = "";
	for (int i=0; i<m_arguments.size(); ++i) {
		const QVariant& arg = m_arguments[i];
		if (arg.type() == QVariant::Int) {
			result += ", Int: " + QString::number(arg.toInt());
		} else if (arg.type() == QVariant::Double) {
			result += ", Double: " + QString::number(arg.toDouble());
		} else if (arg.type() == QVariant::String) {
			result += ", String: " + arg.toString();
		} else if (arg.type() == QVariant::Bool) {
			result += ", Bool: " + (arg.toBool() ? QString("true") : QString("false"));
		} else {
			result += ", Unknown Argument Type: " + QString::fromStdString(arg.typeName());
		}
	}
	return result;
}

void OSCMessage::printToQDebug() const
{
	qDebug() << "---------------------------------";
	qDebug() << "Path: " << m_path;
	for (int i=0; i<m_arguments.size(); ++i) {
		const QVariant& arg = m_arguments[i];
		if (arg.type() == QVariant::Int) {
			qDebug() << i << " Int Argument: " << arg.toInt();
		} else if (arg.type() == QVariant::Double) {
			qDebug() << i << " Double Argument: " << arg.toDouble();
		} else if (arg.type() == QVariant::String) {
			qDebug() << i << " String Argument: " << arg.toString();
		} else if (arg.type() == QVariant::Bool) {
			qDebug() << i << " Bool Argument: " << arg.toBool();
		} else {
			qDebug() << i << " Unknown Argument Type: " << arg.typeName();
		}
	}
}

QString OSCMessage::getPathFromMessage(const QByteArray &data)
{
	// The path of an OSC message is a string from the beginning to the first null character.

	// iterate over each character:
	for (int i=0; i<data.size(); ++i) {
		// check if character is null:
		if (data[i] == 0) {
			// the character is null -> end of path
			QString path = QString::fromLatin1(data.data(), i);
			return path;
		}
	}
	// there is no null-termination in the data
	// return an empty string:
	return QString();
}
