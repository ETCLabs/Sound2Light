// Copyright (c) 2015 Electronic Theatre Controls, Inc., http://www.etcconnect.com
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

// Changed by Tim Henning: Don't treat strings starting with digits followed by spaces as integer.

#include "OSCParser.h"

#ifdef WIN32
#define snprintf _snprintf
#define atoll _atoi64
#endif

////////////////////////////////////////////////////////////////////////////////

const char OSCParser::OSC_BUNDLE_PREFIX[] = {'#','b','u','n','d','l','e',0};
const char OSCParser::OSC_LIST_TAG[] = {'l','i','s','t',0};

// http://www.rfc-editor.org/rfc/rfc1055.txt
#define SLIP_END		0xc0    /* indicates end of packet */
#define SLIP_ESC		0xdb    /* indicates byte stuffing */
#define SLIP_ESC_END	0xdc    /* ESC ESC_END means END data byte */
#define SLIP_ESC_ESC	0xdd    /* ESC ESC_ESC means ESC data byte */

#define SLIP_CHAR(x)	static_cast<char>(static_cast<unsigned char>(x))

////////////////////////////////////////////////////////////////////////////////

#define OSC_COMPILE_TIME_ASSERT(name,exp) struct name{char a[(exp)?1:-1];}

class OSCCompileTimeChecks
{
	OSC_COMPILE_TIME_ASSERT(osc_size_check_char8,	sizeof(char)==1);
	OSC_COMPILE_TIME_ASSERT(osc_size_check_int32,	sizeof(int32_t)==4);
	OSC_COMPILE_TIME_ASSERT(osc_size_check_int64,	sizeof(int64_t)==8);
	OSC_COMPILE_TIME_ASSERT(osc_size_check_float32,	sizeof(float)==4);
	OSC_COMPILE_TIME_ASSERT(osc_size_check_float64,	sizeof(double)==8);
	OSC_COMPILE_TIME_ASSERT(osc_alignment_check,	sizeof(OSCParser::OSC_BUNDLE_PREFIX)==8);
};

////////////////////////////////////////////////////////////////////////////////

OSCStream::OSCStream(EnumFrameMode frameMode)
	: m_FrameMode(frameMode)
	, m_Buf(0)
	, m_Capacity(0)
	, m_Size(0)
{
}

////////////////////////////////////////////////////////////////////////////////

OSCStream::~OSCStream()
{
	Clear();
}

////////////////////////////////////////////////////////////////////////////////

void OSCStream::Clear()
{
	if( m_Buf )
	{
		delete[] m_Buf;
		m_Buf = 0;
	}

	m_Capacity = m_Size = 0;
}

////////////////////////////////////////////////////////////////////////////////

void OSCStream::Reset()
{
	m_Size = 0;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCStream::Add(const char *buf, size_t size)
{
	if(buf && size!=0)
	{
		const size_t GROW_MULTIPLIER = 3;

		if( m_Buf )
		{
			size_t avail = (m_Capacity - m_Size);
			if(avail < size)
			{
				m_Capacity = ((m_Capacity + size - avail) * GROW_MULTIPLIER);
				if(m_Capacity > MAX_BUF_SIZE)
				{
					Clear();
					return false;
				}

				char *temp = m_Buf;
				m_Buf = new char[m_Capacity];
				if(m_Size != 0)
					memcpy(m_Buf, temp, m_Size);
				delete[] temp;
			}
		}
		else
		{
			m_Capacity = (size * GROW_MULTIPLIER);
			if(m_Capacity > MAX_BUF_SIZE)
			{
				Clear();
				return false;
			}

			m_Buf = new char[m_Capacity];
			m_Size = 0;
		}

		memcpy(&m_Buf[m_Size], buf, size);
		m_Size += size;
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////

char* OSCStream::GetNextFrame(size_t &size)
{
	switch( m_FrameMode )
	{
		case FRAME_MODE_1_0:	return GetNextFrame_Mode_1_0(size);
		case FRAME_MODE_1_1:	return GetNextFrame_Mode_1_1(size);
		default:				break;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

char* OSCStream::GetNextFrame_Mode_1_0(size_t &size)
{
	int32_t packetSizeHeader = 0;
	if(m_Buf && m_Size>=sizeof(packetSizeHeader))
	{
		memcpy(&packetSizeHeader, m_Buf, sizeof(packetSizeHeader));
		OSCArgument::Swap32(&packetSizeHeader);
		if(packetSizeHeader > 0)
		{
			size = static_cast<size_t>(packetSizeHeader);
			if(packetSizeHeader <= MAX_FRAME_SIZE)
			{
				if((m_Size-sizeof(packetSizeHeader)) >= size)
				{
					char *frame = new char[size];
					memcpy(frame, &m_Buf[sizeof(packetSizeHeader)], size);
					Chop(size + sizeof(packetSizeHeader));
					return frame;
				}
			}
		}
		else
			Chop( sizeof(packetSizeHeader) );
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

char* OSCStream::GetNextFrame_Mode_1_1(size_t &size)
{
	if(m_Buf && m_Size!=0)
	{
		size_t frameStart = m_Size;
		for(size_t i=0; i<m_Size; i++)
		{
			if(m_Buf[i] == SLIP_CHAR(SLIP_END))
			{
				if(frameStart < m_Size)
				{
					size = 0;
					for(size_t j=frameStart; j<i; j++)
					{
						if(m_Buf[j] != SLIP_CHAR(SLIP_ESC))
							size++;
					}

					char *frame = 0;

					if(size != 0)
					{
						frame = new char[size];
						size_t bufIndex = frameStart;
						for(size_t j=0; j<size; j++)
						{
							frame[j] = m_Buf[bufIndex++];
							if(frame[j] == SLIP_CHAR(SLIP_ESC))
							{
								frame[j] = m_Buf[bufIndex++];
								if(frame[j] == SLIP_CHAR(SLIP_ESC_END))
									frame[j] = SLIP_CHAR(SLIP_END);
								else if(frame[j] == SLIP_CHAR(SLIP_ESC_ESC))
									frame[j] = SLIP_CHAR(SLIP_ESC);
							}
						}
					}

					Chop(i+1);
					return frame;
				}
			}
			else if(frameStart >= m_Size)
				frameStart = i;
		}

		if(frameStart >= m_Size)
			Reset();	// m_Buf only contains SLIP_END's
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

void OSCStream::Chop(size_t size)
{
	if(m_Buf && size!=0)
	{
		if(m_Size > size)
		{
			m_Size -= size;
			memmove(m_Buf, &m_Buf[size], m_Size);
		}
		else
			m_Size = 0;
	}
}

////////////////////////////////////////////////////////////////////////////////

char* OSCStream::CreateFrame(const char *buf, size_t &size) const
{
	return CreateFrame(m_FrameMode, buf, size);
}

////////////////////////////////////////////////////////////////////////////////

char* OSCStream::CreateFrame(EnumFrameMode frameMode, const char *buf, size_t &size)
{
	switch( frameMode )
	{
		case FRAME_MODE_1_0:	return CreateFrame_Mode_1_0(buf, size);
		case FRAME_MODE_1_1:	return CreateFrame_Mode_1_1(buf, size);
		default:				break;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

char* OSCStream::CreateFrame_Mode_1_0(const char *buf, size_t &size)
{
	if(buf && size!=0 && size<=MAX_FRAME_SIZE)
	{
		int32_t packetSizeHeader = static_cast<int32_t>(size);
		size += sizeof(packetSizeHeader);
		char *frame = new char[size];
		memcpy(frame, &packetSizeHeader, sizeof(packetSizeHeader));
		OSCArgument::Swap32(frame);
		memcpy(&frame[sizeof(packetSizeHeader)], buf, packetSizeHeader);
		return frame;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

char* OSCStream::CreateFrame_Mode_1_1(const char *buf, size_t &size)
{
	if(buf && size!=0 && size<=MAX_FRAME_SIZE)
	{
		// compute encoded size
		size_t encodedSize = size;
		for(size_t i=0; i<size; i++)
		{
			if(buf[i]==SLIP_CHAR(SLIP_END) || buf[i]==SLIP_CHAR(SLIP_ESC))
				encodedSize++;
		}

		bool hasReplacements = (encodedSize > size);

		char *encoded = new char[encodedSize + 2];

		encodedSize = 0;
		encoded[encodedSize++] = SLIP_CHAR(SLIP_END);

		if( hasReplacements )
		{
			for(size_t i=0; i<size; i++)
			{
				if(buf[i] == SLIP_CHAR(SLIP_END))
				{
					encoded[encodedSize++] = SLIP_CHAR(SLIP_ESC);
					encoded[encodedSize++] = SLIP_CHAR(SLIP_ESC_END);
				}
				else if(buf[i] == SLIP_CHAR(SLIP_ESC))
				{
					encoded[encodedSize++] = SLIP_CHAR(SLIP_ESC);
					encoded[encodedSize++] = SLIP_CHAR(SLIP_ESC_ESC);
				}
				else
					encoded[encodedSize++] = buf[i];
			}
		}
		else
		{
			// shortcut
			memcpy(&encoded[encodedSize], buf, size);
			encodedSize += size;
		}

		encoded[encodedSize++] = SLIP_CHAR(SLIP_END);
		size = encodedSize;
		return encoded;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int OSCArgument::sRGBA::toUInt() const
{
	return (	((static_cast<unsigned int>(r)<<24) & 0xff000000) |
				((static_cast<unsigned int>(g)<<16) & 0x00ff0000) |
				((static_cast<unsigned int>(b)<<8)  & 0x0000ff00) |
				(static_cast<unsigned int>(a)       & 0x0000000ff) );
}

////////////////////////////////////////////////////////////////////////////////

void OSCArgument::sRGBA::fromUInt(unsigned int n)
{
	r = static_cast<unsigned char>((n >> 24) & 0xff);
	g = static_cast<unsigned char>((n >> 16) & 0xff);
	b = static_cast<unsigned char>((n >> 8) & 0xff);
	a = static_cast<unsigned char>(n & 0xff);
}

////////////////////////////////////////////////////////////////////////////////

OSCArgument::OSCArgument()
	: m_Type(OSC_TYPE_INVALID)
	, m_pBuf(0)
	, m_Size(0)
{
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::Init(EnumArgumentTypes type, char *buf, size_t size)
{
	m_Type = type;
	m_pBuf = buf;

	switch( type )
	{
		case OSC_TYPE_CHAR:
		case OSC_TYPE_INT32:
		case OSC_TYPE_FLOAT32:
		case OSC_TYPE_RGBA32:
		case OSC_TYPE_MIDI:
			{
				if(size >= 4)
				{
					m_Size = 4;
					return true;
				}
			}
			break;

		case OSC_TYPE_INT64:
		case OSC_TYPE_TIME:
			{
				if(size >= 8)
				{
					m_Size = 8;
					return true;
				}
			}
			break;

		case OSC_TYPE_FLOAT64:
			{
				if(size >= 8)
				{
					m_Size = 8;
					return true;
				}
			}
			break;

		case OSC_TYPE_STRING:
			{
				if(size >= 4)
				{
					// find null terminator
					char *bufEnd = &buf[size-1];
					char *p = buf;
					do
					{
						if(*p++ == 0)
						{
							p = Get32BitAligned(buf, p);
							break;
						}
					}
					while(p < bufEnd);

					if(p <= ++bufEnd)
					{
						m_Size = (p - buf);
						return true;
					}
				}
			}
			break;

		case OSC_TYPE_BLOB:
			{
				if(size >= 4)
				{
					size_t dataBytes = static_cast<size_t>( GetInt32FromBuf(buf) );
					m_Size = (sizeof(int32_t) + Get32BitAlignedSize(dataBytes));
					return true;
				}
			}
			break;

		case OSC_TYPE_TRUE:
		case OSC_TYPE_FALSE:
		case OSC_TYPE_NULL:
		case OSC_TYPE_INFINITY:
			m_Size = 0;
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::GetFloat(float &f) const
{
	switch( m_Type )
	{
		case OSC_TYPE_CHAR:
		case OSC_TYPE_INT32:
			f = static_cast<float>( GetInt32FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_INT64:
			f = static_cast<float>( GetInt64FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_FLOAT32:
			f = GetFloat32FromBuf(m_pBuf);
			return true;

		case OSC_TYPE_FLOAT64:
			f = static_cast<float>( GetFloat64FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_STRING:
			{
				if(m_Size!=0 && IsFloatString(m_pBuf))
				{
					f = static_cast<float>( atof(m_pBuf) );
					return true;
				}
			}
			break;

		case OSC_TYPE_TRUE:
			f = 1.0f;
			return true;

		case OSC_TYPE_FALSE:
		case OSC_TYPE_NULL:
			f = 0;
			return true;

		case OSC_TYPE_INFINITY:
			f = FLT_MAX;
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::GetDouble(double &d) const
{
	switch( m_Type )
	{
		case OSC_TYPE_CHAR:
		case OSC_TYPE_INT32:
			d = static_cast<double>( GetInt32FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_INT64:
			d = static_cast<double>( GetInt64FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_FLOAT32:
			d = static_cast<double>( GetFloat32FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_FLOAT64:
			d = GetFloat64FromBuf(m_pBuf);
			return true;

		case OSC_TYPE_STRING:
			{
				if(m_Size!=0 && IsFloatString(m_pBuf))
				{
					d = atof(m_pBuf);
					return true;
				}
			}
			break;

		case OSC_TYPE_TRUE:
			d = 1.0;
			return true;

		case OSC_TYPE_FALSE:
		case OSC_TYPE_NULL:
			d = 0;
			return true;

		case OSC_TYPE_INFINITY:
			d = DBL_MAX;
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::GetInt(int &n) const
{
	switch( m_Type )
	{
		case OSC_TYPE_CHAR:
		case OSC_TYPE_INT32:
			n = static_cast<int>( GetInt32FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_INT64:
			n = static_cast<int>( GetInt64FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_FLOAT32:
			n = OSC_ROUNDF( GetFloat32FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_FLOAT64:
			n = OSC_ROUND( GetFloat64FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_STRING:
			{
				if(m_Size != 0)
				{
					if( IsIntString(m_pBuf) )
					{
						n = atoi(m_pBuf);
						return true;
					}
					else if( IsFloatString(m_pBuf) )
					{
						n = OSC_ROUND( atof(m_pBuf) );
						return true;
					}
				}
			}
			break;

		case OSC_TYPE_TRUE:
			n = 1;
			return true;

		case OSC_TYPE_FALSE:
		case OSC_TYPE_NULL:
			n = 0;
			return true;

		case OSC_TYPE_INFINITY:
			n = INT_MAX;
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::GetUInt(unsigned int &n) const
{
	switch( m_Type )
	{
		case OSC_TYPE_CHAR:
		case OSC_TYPE_INT32:
			n = static_cast<unsigned int>( GetUInt32FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_INT64:
			n = static_cast<unsigned int>( GetUInt64FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_FLOAT32:
			n = static_cast<unsigned int>( OSC_ROUNDF64(GetFloat32FromBuf(m_pBuf)) );
			return true;

		case OSC_TYPE_FLOAT64:
			n = static_cast<unsigned int>( OSC_ROUND64(GetFloat64FromBuf(m_pBuf)) );
			return true;

		case OSC_TYPE_STRING:
			{
				if(m_Size != 0)
				{
					if( IsIntString(m_pBuf) )
					{
						n = static_cast<unsigned int>( atoll(m_pBuf) );
						return true;
					}
					else if( IsFloatString(m_pBuf) )
					{
						n = static_cast<unsigned int>( OSC_ROUND64(atof(m_pBuf)) );
						return true;
					}
				}
			}
			break;

		case OSC_TYPE_TRUE:
			n = 1;
			return true;

		case OSC_TYPE_FALSE:
		case OSC_TYPE_NULL:
			n = 0;
			return true;

		case OSC_TYPE_INFINITY:
			n = UINT_MAX;
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::GetInt64(int64_t &n) const
{
	switch( m_Type )
	{
		case OSC_TYPE_CHAR:
		case OSC_TYPE_INT32:
			n = static_cast<int64_t>( GetInt32FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_INT64:
			n = GetInt64FromBuf(m_pBuf);
			return true;

		case OSC_TYPE_FLOAT32:
			n = OSC_ROUNDF64( GetFloat32FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_FLOAT64:
			n = OSC_ROUND64( GetFloat64FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_STRING:
			{
				if(m_Size != 0)
				{
					if( IsIntString(m_pBuf) )
					{
						n = atoll(m_pBuf);
						return true;
					}
					else if( IsFloatString(m_pBuf) )
					{
						n = OSC_ROUND64( atof(m_pBuf) );
						return true;
					}
				}
			}
			break;

		case OSC_TYPE_TRUE:
			n = 1;
			return true;

		case OSC_TYPE_FALSE:
		case OSC_TYPE_NULL:
			n = 0;
			return true;

		case OSC_TYPE_INFINITY:
			n = INT64_MAX;
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::GetUInt64(uint64_t &n) const
{
	switch( m_Type )
	{
		case OSC_TYPE_CHAR:
		case OSC_TYPE_INT32:
			n = static_cast<uint64_t>( GetUInt32FromBuf(m_pBuf) );
			return true;

		case OSC_TYPE_INT64:
			n = GetUInt64FromBuf(m_pBuf);
			return true;

		case OSC_TYPE_FLOAT32:
			n = static_cast<uint64_t>( OSC_ROUNDF64(GetFloat32FromBuf(m_pBuf)) );
			return true;

		case OSC_TYPE_FLOAT64:
			n = static_cast<uint64_t>( OSC_ROUND64(GetFloat64FromBuf(m_pBuf)) );
			return true;

		case OSC_TYPE_STRING:
			{
				if(m_Size != 0)
				{
					if( IsIntString(m_pBuf) )
					{
						n = static_cast<uint64_t>( atoll(m_pBuf) );
						return true;
					}
					else if( IsFloatString(m_pBuf) )
					{
						n = static_cast<uint64_t>( OSC_ROUND64(atof(m_pBuf)) );
						return true;
					}
				}
			}
			break;

		case OSC_TYPE_TRUE:
			n = 1;
			return true;

		case OSC_TYPE_FALSE:
		case OSC_TYPE_NULL:
			n = 0;
			return true;

		case OSC_TYPE_INFINITY:
			n = UINT64_MAX;
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::GetRGBA(sRGBA &rgba) const
{
	unsigned int n;
	if( GetUInt(n) )
	{
		rgba.fromUInt(n);
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::GetString(std::string &str) const
{
	switch( m_Type )
	{
		case OSC_TYPE_CHAR:
		case OSC_TYPE_INT32:
		case OSC_TYPE_INT64:
			{
				int n;
				if( GetInt(n) )
				{
					char buf[33];
					snprintf(buf, sizeof(buf), "%d", n);
					str = buf;
					return true;
				}
			}
			break;

		case OSC_TYPE_FLOAT32:
		case OSC_TYPE_FLOAT64:
			{
				float f;
				if( GetFloat(f) )
				{
					char buf[33];
					snprintf(buf, sizeof(buf), "%.3f", f);
					str = buf;
					return true;
				}
			}
			break;

		case OSC_TYPE_STRING:
			str = m_pBuf;
			return true;

		case OSC_TYPE_TRUE:
			str = "True";
			return true;

		case OSC_TYPE_FALSE:
			str = "False";
			return true;

		case OSC_TYPE_NULL:
			str = "Null";
			return true;

		case OSC_TYPE_INFINITY:
			str = "Infinity";
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::GetBool(bool &b) const
{
	switch( m_Type )
	{
		case OSC_TYPE_CHAR:
		case OSC_TYPE_INT32:
			b = (GetInt32FromBuf(m_pBuf) != 0);
			return true;

		case OSC_TYPE_INT64:
			b = (GetInt64FromBuf(m_pBuf) != 0);
			return true;

		case OSC_TYPE_FLOAT32:
			b = !OSC_IS_ABOUTF(GetFloat32FromBuf(m_pBuf),0.0f);
			return true;

		case OSC_TYPE_FLOAT64:
			b = !OSC_IS_ABOUT(GetFloat64FromBuf(m_pBuf),0.0);
			return true;

		case OSC_TYPE_STRING:
			{
				if(m_Size != 0)
				{
					switch( toupper(*m_pBuf) )
					{
						case 'F':
							b = false;
							return true;

						case 'T':
							b = true;
							return true;

						default:
							{
								int n;
								if( GetInt(n) )
								{
									b = (n != 0);
									return true;
								}
							}
							break;
					}
				}
			}
			return false;

		case OSC_TYPE_TRUE:
		case OSC_TYPE_INFINITY:
			b = true;
			return true;

		case OSC_TYPE_FALSE:
		case OSC_TYPE_NULL:
			b = false;
			return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

OSCArgument* OSCArgument::GetArgs(char *buf, size_t size, size_t &count)
{
	size_t requestedCount = count;

	OSCArgument *args = 0;
	count = 0;

	if(requestedCount!=0 && buf && size!=0)
	{
		const char *bufEnd = &buf[size-1];

		// skip past OSC address to OSC type tag string that starts with a comma ','
		char *typeTag = buf;
		while(typeTag < bufEnd)
		{
			if(*typeTag++ == ',')
				break;
		}

		if(typeTag < bufEnd)
		{
			// now typeTag should point to the string with the list of OSC argument types, ex: "ii"

			// find where the binary data starts, after the type tag string null terminator (32-bit aligned)
			size_t argCount = 0;
			char *binaryData = typeTag;
			do
			{
				if(*binaryData++ == 0)
				{
					binaryData = OSCArgument::Get32BitAligned(typeTag-1, binaryData);
					break;
				}
				argCount++;
			}
			while(binaryData < bufEnd);

			if(requestedCount > argCount)
				requestedCount = argCount;

			if(requestedCount != 0)
			{
				if(binaryData > bufEnd)
					binaryData = 0;	// still invalid, some OSC types do not have any binary data

				// now binaryData should point to the first argument's binary data
				args = new OSCArgument[requestedCount];
				for(; count<requestedCount; count++)
				{
					OSCArgument::EnumArgumentTypes argType = OSCArgument::GetArgumentTypeFromChar( typeTag[count] );
					if(argType == OSCArgument::OSC_TYPE_INVALID)
						break;	// unhandled argument type

					size_t binaryDataSize = (binaryData ? (bufEnd-binaryData+1) : 0);
					if( !args[count].Init(argType,binaryData,binaryDataSize) )
						break;	// unhandled argument

					if( binaryData )
					{
						binaryData += args[count].GetSize();

						if(binaryData > bufEnd)
							binaryData = 0; // still invalid, some OSC types do not have any binary data
					}
				}
			}
		}
	}

	return args;
}

////////////////////////////////////////////////////////////////////////////////

OSCArgument::EnumArgumentTypes OSCArgument::GetArgumentTypeFromChar(char c)
{
	switch( c )
	{
		case 'c':	return OSC_TYPE_CHAR;
		case 'i':	return OSC_TYPE_INT32;
		case 'h':	return OSC_TYPE_INT64;
		case 'f':	return OSC_TYPE_FLOAT32;
		case 'd':	return OSC_TYPE_FLOAT64;
		case 's':	return OSC_TYPE_STRING;
		case 'b':	return OSC_TYPE_BLOB;
		case 't':	return OSC_TYPE_TIME;
		case 'r':	return OSC_TYPE_RGBA32;
		case 'm':	return OSC_TYPE_MIDI;
		case 'T':	return OSC_TYPE_TRUE;
		case 'F':	return OSC_TYPE_FALSE;
		case 'N':	return OSC_TYPE_NULL;
		case 'I':	return OSC_TYPE_INFINITY;
	}

	return OSC_TYPE_INVALID;
}

////////////////////////////////////////////////////////////////////////////////

char OSCArgument::GetCharFromArgumentType(EnumArgumentTypes type)
{
	switch( type )
	{
		case OSC_TYPE_CHAR:		return 'c';
		case OSC_TYPE_INT32:	return 'i';
		case OSC_TYPE_INT64:	return 'h';
		case OSC_TYPE_FLOAT32:	return 'f';
		case OSC_TYPE_FLOAT64:	return 'd';
		case OSC_TYPE_STRING:	return 's';
		case OSC_TYPE_BLOB:		return 'b';
		case OSC_TYPE_TIME:		return 't';
		case OSC_TYPE_RGBA32:	return 'r';
		case OSC_TYPE_MIDI:		return 'm';
		case OSC_TYPE_TRUE:		return 'T';
		case OSC_TYPE_FALSE:	return 'F';
		case OSC_TYPE_NULL:		return 'N';
		case OSC_TYPE_INFINITY:	return 'I';
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

char* OSCArgument::Get32BitAligned(char *start, char *p)
{
	size_t m = ((p-start) % 4);
	return ((m==0) ? p : (p+4-m));
}

////////////////////////////////////////////////////////////////////////////////

size_t OSCArgument::Get32BitAlignedSize(size_t size)
{
	size_t m = (size % 4);
	return ((m==0) ? size : (size+4-m));
}

////////////////////////////////////////////////////////////////////////////////

void OSCArgument::Swap32(void *buf)
{
#ifndef __BIG_ENDIAN__
	unsigned char *p = reinterpret_cast<unsigned char*>(buf);
	uint32_t ul =(static_cast<uint32_t>(p[3]) |
			(static_cast<uint32_t>(p[2]) << 8) |
			(static_cast<uint32_t>(p[1]) << 16) |
			(static_cast<uint32_t>(p[0]) << 24));
	memcpy(buf, &ul, sizeof(ul));
#endif
}

////////////////////////////////////////////////////////////////////////////////

void OSCArgument::Swap64(void *buf)
{
#ifndef __BIG_ENDIAN__
	unsigned char *p = reinterpret_cast<unsigned char*>(buf);
	uint64_t ull =	(static_cast<uint64_t>(p[7]) |
			(static_cast<uint64_t>(p[6]) << 8) |
			(static_cast<uint64_t>(p[5]) << 16) |
			(static_cast<uint64_t>(p[4]) << 24) |
			(static_cast<uint64_t>(p[3]) << 32) |
			(static_cast<uint64_t>(p[2]) << 40) |
			(static_cast<uint64_t>(p[1]) << 48) |
			(static_cast<uint64_t>(p[0]) << 56));
	memcpy(buf, &ull, sizeof(ull));
#endif
}

////////////////////////////////////////////////////////////////////////////////

const char* OSCArgument::GetSafeString(const char*buf, size_t size)
{
	if( buf )
	{
		// only return original string if null pointer found
		for(size_t i=0; i<size; i++)
		{
			if(buf[i] == 0)
				return buf;
		}
	}

	// invalid string
	return "";
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::IsIntString(const char *buf)
{
	if( buf )
	{
		bool gotDigit = false;
		bool gotSign = false;

		for(;;buf++)
		{
			switch( *buf )
			{
				case 0:
					return gotDigit;	// done, did we get at least one digit?

				case '+':
				case '-':
					{
						if(gotDigit || gotSign)
							return false;	// fail, got a sign after a digit or another sign
						gotSign = true;
					}
					break;

				default:
					{
						if(isdigit(*buf) == 0)
						{
							if(isspace(*buf) != 0)
							{
								if(gotDigit || gotSign)
									//return gotDigit;	// trailing space, treat as string end

									// Addition by Tim Henning:
									// if there are trailing spaces, this is not an Int
									// because some strings start with a digit followed by space and text
									return false;

								// ignore leading spaces
							}
							else
								return false;	// fail, non-digit and non-space
						}
						else
							gotDigit = true;
					}
					break;
			}
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCArgument::IsFloatString(const char *buf)
{
	if( buf )
	{
		bool gotDigit = false;
		bool gotSign = false;
		bool gotDecimal = false;

		for(;;buf++)
		{
			switch( *buf )
			{
				case 0:
					return gotDigit;	// done, did we get at least one digit?

				case '+':
				case '-':
					{
						if(gotDigit || gotSign || gotDecimal)
							return false;	// fail, got a sign after a digit, another sign, or a decimal
						gotSign = true;
					}
					break;

				case '.':
					{
						if( gotDecimal )
							return false;	// fail, got another decimal already
						gotDecimal = true;
					}
					break;

				default:
					{
						if(isdigit(*buf) == 0)
						{
							if(isspace(*buf) != 0)
							{
								if(gotDigit || gotSign || gotDecimal)
									//return gotDigit;	// trailing space, treat as string end

									// Addition by Tim Henning:
									// if there are trailing spaces, this is not an Int
									// because some strings start with a digit followed by space and text
									return false;

								// ignore leading spaces
							}
							else
								return false;	// fail, non-digit and non-space
						}
						else
							gotDigit = true;
					}
					break;
			}
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

int32_t OSCArgument::GetInt32FromBuf(const char *buf)
{
	int32_t l;
	memcpy(&l, buf, sizeof(l));
	Swap32(&l);
	return l;
}

////////////////////////////////////////////////////////////////////////////////

uint32_t OSCArgument::GetUInt32FromBuf(const char *buf)
{
	uint32_t l;
	memcpy(&l, buf, sizeof(l));
	Swap32(&l);
	return l;
}

////////////////////////////////////////////////////////////////////////////////

int64_t OSCArgument::GetInt64FromBuf(const char *buf)
{
	int64_t ll;
	memcpy(&ll, buf, sizeof(ll));
	Swap64(&ll);
	return ll;
}

////////////////////////////////////////////////////////////////////////////////

uint64_t OSCArgument::GetUInt64FromBuf(const char *buf)
{
	uint64_t ll;
	memcpy(&ll, buf, sizeof(ll));
	Swap64(&ll);
	return ll;
}

////////////////////////////////////////////////////////////////////////////////

float OSCArgument::GetFloat32FromBuf(const char *buf)
{
	float f;
	memcpy(&f, buf, sizeof(f));
	Swap32(&f);
	return f;
}

////////////////////////////////////////////////////////////////////////////////

double OSCArgument::GetFloat64FromBuf(const char *buf)
{
	double d;
	memcpy(&d, buf, sizeof(d));
	Swap64(&d);
	return d;
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter::sArgInfo::sArgInfo()
	: type(OSCArgument::OSC_TYPE_INVALID)
	, size(0)
{
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter::sArgInfo::sArgInfo(const sArgInfo &other)
	: type(OSCArgument::OSC_TYPE_INVALID)
	, size(0)
{
	*this = other;
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter::sArgInfo::~sArgInfo()
{
	clear();
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter::sArgInfo& OSCPacketWriter::sArgInfo::operator=(const sArgInfo &other)
{
	clear();

	type = other.type;
	size = other.size;

	switch( type )
	{
		case OSCArgument::OSC_TYPE_STRING:
		case OSCArgument::OSC_TYPE_BLOB:
			{
				if(other.data.binaryData && size!=0)
				{
					data.binaryData = new char[size];
					memcpy(data.binaryData, other.data.binaryData, size);
				}
				else
					data.binaryData = 0;
			}
			break;

		default:
			memcpy(&data, &other.data, size);
			break;
	}

	return (*this);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::sArgInfo::clear()
{
	switch( type )
	{
		case OSCArgument::OSC_TYPE_STRING:
		case OSCArgument::OSC_TYPE_BLOB:
			{
				if( data.binaryData )
				{
					delete[] data.binaryData;
					data.binaryData = 0;
				}
			}
			break;
	}

	type = OSCArgument::OSC_TYPE_INVALID;
	size = 0;
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter::OSCPacketWriter()
{
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter::OSCPacketWriter(const std::string &path)
	: m_Path(path)
{
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter::~OSCPacketWriter()
{
	for(ARG_Q::iterator i=m_Q.begin(); i!=m_Q.end(); i++)
		delete (*i);
	m_Q.clear();
}

////////////////////////////////////////////////////////////////////////////////

size_t OSCPacketWriter::ComputeSize() const
{
	// OSC address tag
	sArgInfo addressTag;
	SetString(addressTag, m_Path);
	size_t size = addressTag.size;

	// OSC arguments tag
	// comma prefix + null terminator + num args (ex: {',','i','f',0}
	size += OSCArgument::Get32BitAlignedSize(m_Q.size() + 2);

	// OSC argument binary data
	for(ARG_Q::const_iterator i=m_Q.begin(); i!=m_Q.end(); i++)
		size += (*i)->size;

	return size;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCPacketWriter::Write(char *buf, size_t size) const
{
	if( buf )
	{
		// OSC address tag
		sArgInfo argAddress;
		SetString(argAddress, m_Path);
		if(size >= argAddress.size)
		{
			memcpy(buf, argAddress.data.binaryData, argAddress.size);
			buf += argAddress.size;
			size -= argAddress.size;

			// OSC arguments tag
			std::string strArgs;
			strArgs.reserve(m_Q.size() + 1);
			strArgs.append(",");
			for(ARG_Q::const_iterator i=m_Q.begin(); i!=m_Q.end(); i++)
			{
				char c = OSCArgument::GetCharFromArgumentType( (*i)->type );
				if(c != 0)
					strArgs.append(1, c);
			}

			sArgInfo argArgs;
			SetString(argArgs, strArgs);
			if(size >= argArgs.size)
			{
				memcpy(buf, argArgs.data.binaryData, argArgs.size);
				buf += argArgs.size;
				size -= argArgs.size;

				// OSC argument binary data
				for(ARG_Q::const_iterator i=m_Q.begin(); i!=m_Q.end(); i++)
				{
					const sArgInfo *info = *i;
					if(OSCArgument::GetCharFromArgumentType(info->type) != 0)
					{
						if(size < info->size)
							return false;

						WriteArg(buf, *info);
						buf += info->size;
						size -= info->size;
					}
				}

				return true;
			}
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::WriteArg(char *buf, const sArgInfo &info) const
{
	switch( info.type )
	{
		case OSCArgument::OSC_TYPE_CHAR:
		case OSCArgument::OSC_TYPE_INT32:
		case OSCArgument::OSC_TYPE_MIDI:
			memcpy(buf, &info.data.int32Data, 4);
			OSCArgument::Swap32(buf);
			break;

		case OSCArgument::OSC_TYPE_INT64:
		case OSCArgument::OSC_TYPE_TIME:
			memcpy(buf, &info.data.int64Data, 8);
			OSCArgument::Swap64(buf);
			break;

		case OSCArgument::OSC_TYPE_FLOAT32:
			memcpy(buf, &info.data.float32Data, 4);
			OSCArgument::Swap32(buf);
			break;

		case OSCArgument::OSC_TYPE_FLOAT64:
			memcpy(buf, &info.data.float64Data, 8);
			OSCArgument::Swap64(buf);
			break;

		case OSCArgument::OSC_TYPE_STRING:
		case OSCArgument::OSC_TYPE_BLOB:
			memcpy(buf, info.data.binaryData, info.size);
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

char* OSCPacketWriter::Create(size_t &size) const
{
	char *buf = 0;

	size = ComputeSize();
	if(size != 0)
	{
		buf = new char[size];
		if( !Write(buf,size) )
		{
			delete[] buf;
			buf = 0;
			size = 0;
		}
	}

	return buf;
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::SetString(sArgInfo &info, const std::string &str) const
{
	info.type = OSCArgument::OSC_TYPE_STRING;
	size_t len = (str.c_str() ? strlen(str.c_str()) : 0);
	info.size = OSCArgument::Get32BitAlignedSize(len + 1);
	info.data.binaryData = new char[info.size];
	if(len != 0)
		memcpy(info.data.binaryData, str.c_str(), len);
	memset(&info.data.binaryData[len], 0, info.size-len);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddBool(bool b)
{
	if( b )
		AddTrue();
	else
		AddFalse();
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddChar(char c)
{
	AddInt32(c);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddInt32(int32_t n)
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_INT32;
	arg->data.int32Data = n;
	arg->size = 4;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddUInt32(uint32_t n)
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_INT32;
	arg->data.int32Data = static_cast<int32_t>(n);
	arg->size = 4;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddInt64(const int64_t &n)
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_INT64;
	arg->data.int64Data = n;
	arg->size = 8;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddUInt64(const uint64_t &n)
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_INT64;
	arg->data.int64Data = static_cast<int64_t>(n);
	arg->size = 8;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddFloat32(float f)
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_FLOAT32;
	arg->data.float32Data = f;
	arg->size = 4;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddFloat64(const double &d)
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_FLOAT64;
	arg->data.float64Data = d;
	arg->size = 8;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddRGBA(const OSCArgument::sRGBA &rgba)
{
	AddUInt32( rgba.toUInt() );
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddString(const std::string &str)
{
	sArgInfo *arg = new sArgInfo;
	SetString(*arg, str);
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddBlob(const char *data, size_t size)
{
	int32_t bytes = static_cast<int32_t>(size);
	if(bytes >= 0)
	{
		sArgInfo *arg = new sArgInfo;
		arg->type = OSCArgument::OSC_TYPE_BLOB;
		arg->size = (4 + OSCArgument::Get32BitAlignedSize(size));
		arg->data.binaryData = new char[arg->size];
		memcpy(arg->data.binaryData, &bytes, 4);
		OSCArgument::Swap32(arg->data.binaryData);
		if(size != 0)
		{
			if( data )
				memcpy(&arg->data.binaryData[4], data, size);
			else
				memset(&arg->data.binaryData[4], 0, size);
		}
		size_t padding = (arg->size - 4 - size);
		if(padding != 0)
			memset(&arg->data.binaryData[arg->size-padding], 0, padding);
		m_Q.push_back(arg);
	}
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddTrue()
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_TRUE;
	arg->size = 0;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddFalse()
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_FALSE;
	arg->size = 0;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddNull()
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_NULL;
	arg->size = 0;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddInfinity()
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_INFINITY;
	arg->size = 0;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddMidi(int32_t n)
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_MIDI;
	arg->data.int32Data = n;
	arg->size = 4;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddTime(const int64_t &n)
{
	sArgInfo *arg = new sArgInfo;
	arg->type = OSCArgument::OSC_TYPE_TIME;
	arg->data.int64Data = n;
	arg->size = 8;
	m_Q.push_back(arg);
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddOSCArg(const OSCArgument &arg)
{
	switch( arg.GetType() )
	{
		case OSCArgument::OSC_TYPE_CHAR:
			{
				int n;
				if( arg.GetInt(n) )
					AddChar( static_cast<char>(n) );
			}
			break;

		case OSCArgument::OSC_TYPE_INT32:
			{
				int n;
				if( arg.GetInt(n) )
					AddInt32( static_cast<int32_t>(n) );
			}
			break;

		case OSCArgument::OSC_TYPE_INT64:
			{
				int64_t n;
				if( arg.GetInt64(n) )
					AddInt64(n);
			}
			break;

		case OSCArgument::OSC_TYPE_FLOAT32:
			{
				float f;
				if( arg.GetFloat(f) )
					AddFloat32(f);
			}
			break;

		case OSCArgument::OSC_TYPE_FLOAT64:
			{
				double d;
				if( arg.GetDouble(d) )
					AddFloat64(d);
			}
			break;

		case OSCArgument::OSC_TYPE_STRING:
			{
				std::string str;
				if( arg.GetString(str) )
					AddString(str);
			}
			break;

		case OSCArgument::OSC_TYPE_BLOB:
			AddBlob(arg.GetRaw(), arg.GetSize());
			break;

		case OSCArgument::OSC_TYPE_TIME:
			{
				int64_t n;
				if( arg.GetInt64(n) )
					AddTime(n);
			}
			break;

		case OSCArgument::OSC_TYPE_RGBA32:
			{
				OSCArgument::sRGBA rgba;
				if( arg.GetRGBA(rgba) )
					AddRGBA(rgba);
			}
			break;

		case OSCArgument::OSC_TYPE_MIDI:
			{
				int n;
				if( arg.GetInt(n) )
					AddMidi( static_cast<int32_t>(n) );
			}
			break;

		case OSCArgument::OSC_TYPE_TRUE:
			AddTrue();
			break;

		case OSCArgument::OSC_TYPE_FALSE:
			AddFalse();
			break;

		case OSCArgument::OSC_TYPE_NULL:
			AddNull();
			break;

		case OSCArgument::OSC_TYPE_INFINITY:
			AddInfinity();
			break;
	}
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::AddOSCArgList(const OSCArgument *args, size_t count)
{
	if(args && count!=0)
	{
		for(size_t i=0; i<count; i++)
			AddOSCArg( args[i] );
	}
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter* OSCPacketWriter::CreatePacketWriterForString(const char *str)
{
	if(str && str[0]==OSC_ADDR_SEPARATOR)
	{
		// find args
		const char *args = strrchr(str, '=');
		if( args )
		{
			// create packet with path
			OSCPacketWriter *packet = new OSCPacketWriter( std::string(str,args-str) );

			// append comma delimited args
			for(const char *i=++args; ;)
			{
				if(*i == 0)
				{
					if(i > args)
					{
						if( OSCArgument::IsIntString(args) )
							packet->AddInt32( atoi(args) );
						else if( OSCArgument::IsFloatString(args) )
							packet->AddFloat32( static_cast<float>(atof(args)) );
						else
							packet->AddString(args);
					}

					// done
					break;
				}
				else if(*i == ',')
				{
					size_t len = (i - args);
					if(len != 0)
					{
						char *copy = new char[len + 1];
						memcpy(copy, args, len);
						copy[len] = 0;

						if( OSCArgument::IsIntString(copy) )
							packet->AddInt32( atoi(copy) );
						else if( OSCArgument::IsFloatString(copy) )
							packet->AddFloat32( static_cast<float>(atof(copy)) );
						else
							packet->AddString(copy);

						delete[] copy;
					}

					args = ++i;
				}
				else
					++i;
			}

			return packet;
		}
		else
			return (new OSCPacketWriter(str));	// simple case, no args
	}

	// not an osc string
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

char* OSCPacketWriter::CreateForString(const char *str, size_t &outSize)
{
	char *buf = 0;
	outSize = 0;

	OSCPacketWriter *packet = CreatePacketWriterForString(str);
	if( packet )
	{
		buf = packet->Create(outSize);
		delete packet;
	}

	return buf;
}

////////////////////////////////////////////////////////////////////////////////

char* OSCPacketWriter::CreateForString(const char *str, size_t strSize, size_t &outSize)
{
	char *buf = 0;
	outSize = 0;

	OSCPacketWriter *packet = CreatePacketWriterForString(str, strSize);
	if( packet )
	{
		buf = packet->Create(outSize);
		delete packet;
	}

	return buf;
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter* OSCPacketWriter::CreatePacketWriterForString(const char *str, size_t size)
{
	if(str && size!=0 && str[0]==OSC_ADDR_SEPARATOR)
	{
		// find null terminator
		for(size_t i=0; i<size; i++)
		{
			if(str[i] == 0)
				return CreatePacketWriterForString(str);
		}

		// none found
		char *copy = new char[size + 1];
		memcpy(copy, str, size);
		copy[size] = 0;
		OSCPacketWriter *packet = CreatePacketWriterForString(str);
		delete[] copy;
		return packet;
	}

	// not an osc string
	return 0;
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter** OSCPacketWriter::CreateList(const OSCPacketWriter &packet, size_t &count)
{
	return CreateList(packet, /*maxPacketSize*/512, count);
}

////////////////////////////////////////////////////////////////////////////////

OSCPacketWriter** OSCPacketWriter::CreateList(const OSCPacketWriter &packet, size_t maxPacketBytes, size_t &count)
{
	OSCPacketWriter **packets = 0;

	if( packet.m_Q.empty() )
	{
		// return an empty list
		count = 1;
		packets = new OSCPacketWriter*[count];
		std::string listPath(packet.m_Path);
		MakeListPath(0, 0, listPath);
		packets[0] = new OSCPacketWriter(listPath);
	}
	else
	{
		std::deque<OSCPacketWriter*> q;
		OSCPacketWriter *listPacket = 0;
		size_t listIndex = 0;

		for(ARG_Q::const_iterator i=packet.m_Q.begin(); i!=packet.m_Q.end(); )
		{
			if( !listPacket )
			{
				std::string listPath(packet.m_Path);
				MakeListPath(packet.m_Q.size()-1, packet.m_Q.size(), listPath);		// temporarily reserve size for maximum list path length
				listPacket = new OSCPacketWriter(listPath);
			}

			listPacket->m_Q.push_back(new sArgInfo(**i));

			// check if packet size exceeds maxPacketBytes
			if(listPacket->ComputeSize() > maxPacketBytes)
			{
				// yup, remove last arg if possible
				if(listPacket->m_Q.size() > 1)
				{
					delete listPacket->m_Q.back();
					listPacket->m_Q.pop_back();
				}
				else
					i++;

				// add to q
				std::string listPath(packet.m_Path);
				MakeListPath(listIndex, packet.m_Q.size(), listPath);
				listIndex += listPacket->m_Q.size();
				listPacket->SetPath(listPath);
				q.push_back(listPacket);
				listPacket = 0;
			}
			else
				i++;
		}

		// list in progress?
		if( listPacket )
		{
			std::string listPath(packet.m_Path);
			MakeListPath(listIndex, packet.m_Q.size(), listPath);
			listPacket->SetPath(listPath);
			q.push_back(listPacket);
		}

		count = q.size();
		if(count != 0)
		{
			size_t index = 0;
			packets = new OSCPacketWriter*[count];
			for(std::deque<OSCPacketWriter*>::const_iterator i=q.begin(); i!=q.end(); i++)
				packets[index++] = *i;
		}
	}

	return packets;
}

////////////////////////////////////////////////////////////////////////////////

void OSCPacketWriter::MakeListPath(size_t index, size_t total, std::string &path)
{
	const char sep[] = {OSC_ADDR_SEPARATOR, 0};

	// <path>/
	path.append(sep);

	// <path>/list
	path.append(OSCParser::OSC_LIST_TAG);

	// <path>/list/
	path.append(sep);

	// <path>/list/<index>
	char buf[33];
	snprintf(buf, sizeof(buf), "%u", static_cast<unsigned int>(index));
	path.append(buf);

	// <path>/list/<index>/
	path.append(sep);

	// <path>/list/<index>/<total>
	snprintf(buf, sizeof(buf), "%u", static_cast<unsigned int>(total));
	path.append(buf);
}

////////////////////////////////////////////////////////////////////////////////

OSCBundleWriter::OSCBundleWriter()
{
}

////////////////////////////////////////////////////////////////////////////////

OSCBundleWriter::~OSCBundleWriter()
{
	for(PACKET_Q::iterator i=m_Q.begin(); i!=m_Q.end(); i++)
		delete (*i);
	m_Q.clear();
}

////////////////////////////////////////////////////////////////////////////////

size_t OSCBundleWriter::ComputeSize() const
{
	// OSC bundle prefix
	size_t size = sizeof(OSCParser::OSC_BUNDLE_PREFIX);

	// OSC bundle time-tag
	size += 8;

	// bundle elements
	for(PACKET_Q::const_iterator i=m_Q.begin(); i!=m_Q.end(); i++)
	{
		// element size
		size += 4;

		// element data
		size += (*i)->ComputeSize();
	}

	return size;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCBundleWriter::Write(char *buf, size_t size) const
{
	if( buf )
	{
		// OSC bundle prefix
		if(size >= sizeof(OSCParser::OSC_BUNDLE_PREFIX))
		{
			memcpy(buf, OSCParser::OSC_BUNDLE_PREFIX, sizeof(OSCParser::OSC_BUNDLE_PREFIX));
			buf += sizeof(OSCParser::OSC_BUNDLE_PREFIX);
			size -= sizeof(OSCParser::OSC_BUNDLE_PREFIX);

			// OSC bundle time-tag
			if(size >= 8)
			{
				memset(buf, 0, 8);
				buf += 8;
				size -= 8;

				// bundle elements
				for(PACKET_Q::const_iterator i=m_Q.begin(); i!=m_Q.end(); i++)
				{
					const OSCPacketElement *packet = *i;
					size_t packetSize = packet->ComputeSize();
					if(size < (4+packetSize))
						return false;

					// element size
					int32_t l = static_cast<int32_t>(packetSize);
					memcpy(buf, &l, 4);
					OSCArgument::Swap32(buf);
					buf += 4;
					size -= 4;

					// element
					if( !packet->Write(buf,size) )
						return false;

					buf += packetSize;
					size -= packetSize;
				}

				// success
				return (size == 0);
			}
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

char* OSCBundleWriter::Create(size_t &size) const
{
	char *buf = 0;

	size = ComputeSize();
	if(size != 0)
	{
		buf = new char[size];
		if( !Write(buf,size) )
		{
			delete[] buf;
			buf = 0;
			size = 0;
		}
	}

	return buf;
}

////////////////////////////////////////////////////////////////////////////////

void OSCBundleWriter::AddPacket(OSCPacketElement *packet)
{
	if( packet )
		m_Q.push_back(packet);
}

////////////////////////////////////////////////////////////////////////////////

OSCMethod::OSCMethod()
{
}

////////////////////////////////////////////////////////////////////////////////

OSCMethod::~OSCMethod()
{
	Clear();
}

////////////////////////////////////////////////////////////////////////////////

void OSCMethod::Clear()
{
	for(METHOD_TABLE::const_iterator i=m_MethodTable.begin(); i!=m_MethodTable.end(); i++)
		delete i->second;
	m_MethodTable.clear();
}

////////////////////////////////////////////////////////////////////////////////

void OSCMethod::AddMethod(const char *name, OSCMethod *method)
{
	if(name && method)
	{
		std::string methodName(name);
		if( !methodName.empty() )
		{
			METHOD_TABLE::iterator i = m_MethodTable.find(name);
			if(i == m_MethodTable.end())
			{
				m_MethodTable[methodName] = method;
			}
			else if(method != i->second)
			{
				delete i->second;
				i->second = method;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////

bool OSCMethod::ProcessPacket(OSCParserClient &client, char *buf, size_t size)
{
	// extract the next method name
	if(buf && size!=0)
	{
		// skip first separator
		if(*buf == OSC_ADDR_SEPARATOR)
		{
			buf++;
			size--;
		}

		if(size != 0)
		{
			char *methodName = 0;
			char *nextBuf = 0;
			bool last = true;

			char *p = buf;
			char *bufEnd = &buf[size-1];
			for(; p<=bufEnd; p++)
			{
				if(*p==0 || *p==OSC_ADDR_SEPARATOR)
				{
					// got our method name
					methodName = buf;

					// this is our next method (or binary data)
					nextBuf = p;

					// finished with OSC address tag?
					if(*p == 0)
						break;

					// null terminate method name string
					*p = 0;

					// advance
					if(nextBuf < bufEnd)
						nextBuf++;

					last = false;
					break;
				}
			}

			// do we have a method name?
			if( methodName )
			{
				std::string name(methodName);
				if( !name.empty() )
				{
					// yup, do we have a handler
					METHOD_TABLE::const_iterator i = m_MethodTable.find(name);
					if(i != m_MethodTable.end())
						return i->second->ProcessPacket(client, nextBuf, size-(nextBuf-buf));

					if(!last && nextBuf!=buf)
					{
						if( !ExecuteMethod(client,/*last*/false,buf,size) )
							return false;

						return ProcessPacket(client, nextBuf, size-(nextBuf-buf));
					}
				}
			}
		}
	}

	return ExecuteMethod(client, /*last*/true, buf, size);
}

////////////////////////////////////////////////////////////////////////////////

bool OSCMethod::PrintPacket(OSCParserClient &client, char *buf, size_t size)
{
	// find osc path null terminator
	if( buf )
	{
		for(size_t i=0; i<size; i++)
		{
			if(buf[i] == 0)
			{
				std::string desc("[OSC Packet] ");
				if(buf[0] == 0)
					desc.append("<null>");
				else
					desc.append(buf);

				size_t count = 0xffffffff;
				OSCArgument *args = OSCArgument::GetArgs(buf, size, count);
				if( args )
				{
					for(size_t j=0; j<count; j++)
					{
						OSCArgument &arg = args[j];

						// value
						desc.append(", ");
						std::string value;
						arg.GetString(value);
						desc.append(value);

						// abbreviation
						desc.append("(");
						char abbrev[2];
						abbrev[1] = 0;
						abbrev[0] = OSCArgument::GetCharFromArgumentType( arg.GetType() );
						if(abbrev[0] == 0)
							abbrev[0] = '?';
						desc.append(abbrev);
						desc.append(")");
					}

					delete[] args;
				}

				client.OSCParserClient_Log(desc);
				return true;
			}
		}
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCMethod::ExecuteMethod(OSCParserClient& /*client*/, bool /*last*/, char* /*buf*/, size_t /*size*/)
{
	return true;
}

////////////////////////////////////////////////////////////////////////////////

void OSCMethod::Print(OSCParserClient &client) const
{
	PrintPrivate(client, /*depth*/0);
}

////////////////////////////////////////////////////////////////////////////////

void OSCMethod::PrintPrivate(OSCParserClient &client, unsigned int depth) const
{
	if(depth == 0)
		client.OSCParserClient_Log("[OSC Mapping]");

	for(METHOD_TABLE::const_iterator i=m_MethodTable.begin(); i!=m_MethodTable.end(); i++)
	{
		std::string message;
		if(depth != 0)
			message.append(depth, ' ');
		message.append(i->first);
		client.OSCParserClient_Log(message);
		i->second->PrintPrivate(client, depth+1);
	}
}

////////////////////////////////////////////////////////////////////////////////

OSCParser::OSCParser()
	: m_Root(0)
{
}

////////////////////////////////////////////////////////////////////////////////

OSCParser::~OSCParser()
{
	SetRoot(0);
}

////////////////////////////////////////////////////////////////////////////////

void OSCParser::SetRoot(OSCMethod *root)
{
	if(m_Root && m_Root!=root)
		delete m_Root;

	m_Root = root;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCParser::ProcessPacket(OSCParserClient &client, char *buf, size_t size)
{
	return ProcessPacketPrivate(client, buf, size, /*print*/false);
}

////////////////////////////////////////////////////////////////////////////////

bool OSCParser::ProcessPacketPrivate(OSCParserClient &client, char *buf, size_t size, bool print)
{
	if(m_Root && buf)
	{
		if(size>=sizeof(OSC_BUNDLE_PREFIX) && memcmp(buf,OSC_BUNDLE_PREFIX,sizeof(OSC_BUNDLE_PREFIX))==0)
			return ProcessBundle(client, &buf[sizeof(OSC_BUNDLE_PREFIX)], size-sizeof(OSC_BUNDLE_PREFIX), print);
		else if( print )
			return m_Root->PrintPacket(client, buf, size);
		else
			return m_Root->ProcessPacket(client, buf, size);
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCParser::ProcessBundle(OSCParserClient &client, char *buf, size_t size, bool print)
{
	// expecting OSC time tag (8 bytes) followed by bundle size (4 bytes - int32)
	static const size_t OSC_MIN_BUNDLE_SIZE = 12;
	if(size < OSC_MIN_BUNDLE_SIZE)
	{
		char msg[256];
		sprintf(msg, "error, malformed bundle of size %u, expecting minimum size of %u", static_cast<unsigned int>(size), static_cast<unsigned int>(OSC_MIN_BUNDLE_SIZE));
		client.OSCParserClient_Log(msg);
		return false;
	}

	// skip past time tag
	buf += 8;
	size -= 8;

	bool processedAllBundleElements = true;

	do
	{
		// get bundle size
		size_t bundleSize = static_cast<size_t>( OSCArgument::GetInt32FromBuf(buf) );
		buf += 4;
		size -= 4;

		if(bundleSize != 0)
		{
			if(bundleSize <= size)
			{
				if( !ProcessPacketPrivate(client,buf,bundleSize,print) )
					processedAllBundleElements = false;

				// onto the next bundle
				buf += bundleSize;
				size -= bundleSize;
			}
			else
			{
				char msg[256];
				sprintf(msg, "error, malformed bundle with reported size %u, but packet size only %u", static_cast<unsigned int>(bundleSize), static_cast<unsigned int>(size));
				client.OSCParserClient_Log(msg);
				return false;
			}
		}
		else
			client.OSCParserClient_Log("warning, empty bundle element");
	}
	while(size >= OSC_MIN_BUNDLE_SIZE);

	return processedAllBundleElements;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCParser::PrintPacket(OSCParserClient &client, const char *buf, size_t size)
{
	bool success = false;

	if(buf && size!=0)
	{
		// make a copy, so this packet can also be processed
		char *copy = new char[size];
		memcpy(copy, buf, size);

		if( ProcessPacketPrivate(client,copy,size,/*print*/true) )
			success = true;

		delete[] copy;
	}

	return success;
}

////////////////////////////////////////////////////////////////////////////////

bool OSCParser::IsOSCPacket(const char *buf, size_t size)
{
	if(buf && size!=0)
	{
		if(*buf == OSC_ADDR_SEPARATOR)
			return true;	// osc address path

		if(size>=sizeof(OSC_BUNDLE_PREFIX) && memcmp(buf,OSC_BUNDLE_PREFIX,sizeof(OSC_BUNDLE_PREFIX))==0)
			return true;	// osc bundle
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
