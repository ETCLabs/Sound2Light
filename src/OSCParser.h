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

#pragma once
#ifndef OSC_PARSER_H
#define OSC_PARSER_H

#include <map>
#include <deque>
#include <string>
#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>


// Addition by Tim Henning to compile with MinGW:
#ifndef UINT64_MAX
	#include <limits>
	#define UINT64_MAX std::numeric_limits<uint64_t>::max()
	#define INT64_MAX std::numeric_limits<int64_t>::max()
#endif


////////////////////////////////////////////////////////////////////////////////

// TODO: use OSC time-tag instead of ignoring

////////////////////////////////////////////////////////////////////////////////

#define OSC_ADDR_SEPARATOR	'/'

////////////////////////////////////////////////////////////////////////////////

#define OSC_ROUND(x)		((x>=0)?static_cast<int>(x+0.5):static_cast<int>(x-0.5))
#define OSC_ROUNDF(x)		((x>=0)?static_cast<int>(x+0.5f):static_cast<int>(x-0.5f))
#define OSC_ROUND64(x)		((x>=0)?static_cast<int64_t>(x+0.5):static_cast<int64_t>(x-0.5))
#define OSC_ROUNDF64(x)		((x>=0)?static_cast<int64_t>(x+0.5f):static_cast<int64_t>(x-0.5f))
#define OSC_IS_ABOUT(x,y)	(fabs(x-y)<0.00001)
#define OSC_IS_ABOUTF(x,y)	(fabsf(x-y)<0.00001f)

////////////////////////////////////////////////////////////////////////////////

class OSCParserClient
{
public:
	virtual void OSCParserClient_Log(const std::string &message) = 0;
	virtual void OSCParserClient_Send(const char *buf, size_t size) = 0;
};

////////////////////////////////////////////////////////////////////////////////

class OSCStream
{
public:
	enum EnumFrameMode
	{
		FRAME_MODE_1_0		= 0,	// 4-byte size header
		FRAME_MODE_1_1,				// http://www.rfc-editor.org/rfc/rfc1055.txt

		FRAME_MODE_COUNT,

		FRAME_MODE_DEFAULT	= FRAME_MODE_1_1,
		FRAME_MODE_INVALID
	};

	enum EnumConstants
	{
		MAX_FRAME_SIZE	= 524288,	// 512k
		MAX_BUF_SIZE	= 2097152	// 2g
	};

	OSCStream(EnumFrameMode frameMode);
	virtual ~OSCStream();

	virtual void Clear();
	virtual void Reset();
	virtual bool Add(const char *buf, size_t size);
	virtual char* GetNextFrame(size_t &size);
	virtual char* CreateFrame(const char *buf, size_t &size) const;

	static char* CreateFrame(EnumFrameMode frameMode, const char *buf, size_t &size);
	static char* CreateFrame_Mode_1_0(const char *buf, size_t &size);
	static char* CreateFrame_Mode_1_1(const char *buf, size_t &size);

protected:
	EnumFrameMode	m_FrameMode;
	char			*m_Buf;
	size_t			m_Capacity;
	size_t			m_Size;

	virtual char* GetNextFrame_Mode_1_0(size_t &size);
	virtual char* GetNextFrame_Mode_1_1(size_t &size);
	virtual void Chop(size_t size);
};

////////////////////////////////////////////////////////////////////////////////

class OSCArgument
{
public:
	enum EnumArgumentTypes
	{
		OSC_TYPE_INVALID,

		OSC_TYPE_CHAR,			// c
		OSC_TYPE_INT32,			// i
		OSC_TYPE_INT64,			// h
		OSC_TYPE_FLOAT32,		// f
		OSC_TYPE_FLOAT64,		// d (double)
		OSC_TYPE_STRING,		// s
		OSC_TYPE_BLOB,			// b
		OSC_TYPE_TIME,			// t (OSC-timetag)
		OSC_TYPE_RGBA32,		// r
		OSC_TYPE_MIDI,			// m (4 byte MIDI message. Bytes from MSB to LSB are: port id, status byte, data1, data2)
		OSC_TYPE_TRUE,			// T (True, 0 bytes)
		OSC_TYPE_FALSE,			// F (False, 0 bytes)
		OSC_TYPE_NULL,			// N (Null, 0 bytes
		OSC_TYPE_INFINITY,		// I (Infinity, 0 bytes)

		OSC_TYPE_COUNT
	};

	struct sRGBA
	{
		unsigned char r;
		unsigned char g;
		unsigned char b;
		unsigned char a;

		unsigned int toUInt() const;
		void fromUInt(unsigned int n);
	};

	OSCArgument();

	bool Init(EnumArgumentTypes type, char *buf, size_t size);
	EnumArgumentTypes GetType() const {return m_Type;}
	const char* GetRaw() const {return m_pBuf;}
	size_t GetSize() const {return m_Size;}
	bool GetFloat(float &f) const;
	bool GetDouble(double &d) const;
	bool GetInt(int &n) const;
	bool GetUInt(unsigned int &n) const;
	bool GetInt64(int64_t &n) const;
	bool GetUInt64(uint64_t &n) const;
	bool GetRGBA(sRGBA &rgba) const;
	bool GetString(std::string &str) const;
	bool GetBool(bool &b) const;

	static OSCArgument* GetArgs(char *buf, size_t size, size_t &count);
	static EnumArgumentTypes GetArgumentTypeFromChar(char c);
	static char GetCharFromArgumentType(EnumArgumentTypes type);
	static char* Get32BitAligned(char *start, char *p);
	static size_t Get32BitAlignedSize(size_t size);
	static void Swap32(void *buf);
	static void Swap64(void *buf);
	static const char* GetSafeString(const char*buf, size_t size);
	static bool IsIntString(const char *buf);
	static bool IsFloatString(const char *buf);
	static int32_t GetInt32FromBuf(const char *buf);
	static uint32_t GetUInt32FromBuf(const char *buf);
	static int64_t GetInt64FromBuf(const char *buf);
	static uint64_t GetUInt64FromBuf(const char *buf);
	static float GetFloat32FromBuf(const char *buf);
	static double GetFloat64FromBuf(const char *buf);

protected:
	EnumArgumentTypes	m_Type;
	const char*			m_pBuf;
	size_t				m_Size;
};

////////////////////////////////////////////////////////////////////////////////

class OSCPacketElement
{
public:
	virtual ~OSCPacketElement() {}
	virtual size_t ComputeSize() const = 0;
	virtual bool Write(char *buf, size_t size) const = 0;
};

////////////////////////////////////////////////////////////////////////////////

class OSCPacketWriter
	: public OSCPacketElement
{
public:
	OSCPacketWriter();
	OSCPacketWriter(const std::string &path);
	virtual ~OSCPacketWriter();

	virtual size_t ComputeSize() const;
	virtual bool Write(char *buf, size_t size) const;
	virtual char* Create(size_t &size) const;
	virtual const std::string& GetPath() const {return m_Path;}
	virtual void SetPath(const std::string &path) {m_Path = path;}
	virtual bool empty() const {return m_Q.empty();}
	virtual size_t size() const {return m_Q.size();}

	virtual void AddBool(bool b);
	virtual void AddChar(char c);
	virtual void AddInt32(int32_t n);
	virtual void AddUInt32(uint32_t n);
	virtual void AddInt64(const int64_t &n);
	virtual void AddUInt64(const uint64_t &n);
	virtual void AddFloat32(float f);
	virtual void AddFloat64(const double &d);
	virtual void AddRGBA(const OSCArgument::sRGBA &rgba);
	virtual void AddString(const std::string &str);
	virtual void AddBlob(const char *data, size_t size);
	virtual void AddTrue();
	virtual void AddFalse();
	virtual void AddNull();
	virtual void AddInfinity();
	virtual void AddMidi(int32_t n);
	virtual void AddTime(const int64_t &n);
	virtual void AddOSCArg(const OSCArgument &arg);
	virtual void AddOSCArgList(const OSCArgument *args, size_t count);

	// must be in the following format
	// /x/y/z=1.0,2.0
	static OSCPacketWriter* CreatePacketWriterForString(const char *str);
	static OSCPacketWriter* CreatePacketWriterForString(const char *str, size_t size);
	static char* CreateForString(const char *str, size_t &outSize);
	static char* CreateForString(const char *str, size_t strSize, size_t &outSize);

	// split into "list" form
	static OSCPacketWriter** CreateList(const OSCPacketWriter &packet, size_t &count);
	static OSCPacketWriter** CreateList(const OSCPacketWriter &packet, size_t maxPacketBytes, size_t &count);

private:
	// not allowed
	OSCPacketWriter(const OSCPacketWriter&) {}
	OSCPacketWriter& operator=(const OSCPacketWriter&) {return *this;}

protected:
	union uArgData
	{
		char*		binaryData;
		int32_t		int32Data;
		int64_t	int64Data;
		float		float32Data;
		double		float64Data;
	};

	struct sArgInfo
	{
		sArgInfo();
		sArgInfo(const sArgInfo &other);
		~sArgInfo();
		sArgInfo& operator=(const sArgInfo &other);
		void clear();
		OSCArgument::EnumArgumentTypes	type;
		uArgData						data;
		size_t							size;
	};

	typedef std::deque<sArgInfo*> ARG_Q;

	std::string	m_Path;
	ARG_Q		m_Q;

	virtual void SetString(sArgInfo &info, const std::string &str) const;
	virtual void WriteArg(char *buf, const sArgInfo &info) const;

	static void MakeListPath(size_t index, size_t total, std::string &path);
};

////////////////////////////////////////////////////////////////////////////////

class OSCBundleWriter
	: public OSCPacketElement
{
public:
	OSCBundleWriter();
	virtual ~OSCBundleWriter();

	virtual size_t ComputeSize() const;
	virtual bool Write(char *buf, size_t size) const;
	virtual char* Create(size_t &size) const;
	virtual void AddPacket(OSCPacketElement *packet);

private:
	// not allowed
	OSCBundleWriter(const OSCBundleWriter&) {}
	OSCBundleWriter& operator=(const OSCBundleWriter&) {return *this;}

protected:
	typedef std::deque<OSCPacketElement*> PACKET_Q;

	PACKET_Q	m_Q;
};

////////////////////////////////////////////////////////////////////////////////

class OSCMethod
{
public:
	OSCMethod();
	virtual ~OSCMethod();

	virtual void Clear();
	virtual void AddMethod(const char *path, OSCMethod *method);
	virtual bool ProcessPacket(OSCParserClient &client, char *buf, size_t size);
	virtual bool PrintPacket(OSCParserClient &client, char *buf, size_t size);
	virtual void Print(OSCParserClient &client) const;

private:
	// not allowed
	OSCMethod(const OSCMethod &) {}
	OSCMethod& operator=(const OSCMethod &) {return *this;}

protected:
	typedef std::map<std::string,OSCMethod*> METHOD_TABLE;

	METHOD_TABLE	m_MethodTable;

	virtual bool ExecuteMethod(OSCParserClient &client, bool last, char *buf, size_t size);
	virtual void PrintPrivate(OSCParserClient &client, unsigned int depth) const;
};

////////////////////////////////////////////////////////////////////////////////

class OSCParser
{
public:
	OSCParser();
	virtual ~OSCParser();

	virtual OSCMethod* GetRoot() {return m_Root;}
	virtual const OSCMethod* GetRootConst() const {return m_Root;}
	virtual void SetRoot(OSCMethod *root);
	virtual bool ProcessPacket(OSCParserClient &client, char *buf, size_t size);
	virtual bool PrintPacket(OSCParserClient &client, const char *buf, size_t size);

	static bool IsOSCPacket(const char *buf, size_t size);

	static const char OSC_BUNDLE_PREFIX[];
	static const char OSC_LIST_TAG[];

protected:
	OSCMethod	*m_Root;

	virtual bool ProcessBundle(OSCParserClient &client, char *buf, size_t size, bool print);
	virtual bool ProcessPacketPrivate(OSCParserClient &client, char *buf, size_t size, bool print);
};

////////////////////////////////////////////////////////////////////////////////

#endif
