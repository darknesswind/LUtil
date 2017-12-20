#include "stdafx.h"
#include "LTextStream.h"
// #include "windows.h"

LTextStream::LTextStream()
{
}


LTextStream::~LTextStream()
{
}

bool LTextStream::openRead(const char* szFile)
{
	if (!m_file.openRead(szFile))
		return false;
	init();
	return true;
}

bool LTextStream::openRead(const wchar_t* szFile)
{
	if (!m_file.openRead(szFile))
		return false;
	init();
	return true;
}

void LTextStream::init()
{
	char head[4] = { 0 };
	m_file.getAs(head);

	const char bomUtf8[] = { '\xEF', '\xBB', '\xBF' };
	const char bomUtf16le[] = { '\xFF', '\xFE' };

	if (0 == memcmp(head, bomUtf16le, sizeof(bomUtf16le)))
		m_codepage = cpUtf16LE;
	else if (0 == memcmp(head, bomUtf8, sizeof(bomUtf8)))
		m_codepage = cpUtf8;

	switch (m_codepage)
	{
	case LTextStream::cpUtf16LE:
		m_fnReadChar = &LTextStream::readUtf16LEChar;
		break;
	case LTextStream::cpUtf8:
	default:
		m_fnReadChar = &LTextStream::readUtf8Char;
		break;
	}
}

char16_t LTextStream::readChar()
{
	return ((*this).*m_fnReadChar)();
}

char16_t LTextStream::readUtf16LEChar()
{
	return m_file.read<char16_t>();
}

char16_t LTextStream::readUtf8Char()
{
	//UTF8 �����ʽ��
	//     U-00000000 - U-0000007F: 0xxxxxxx  
	//     U-00000080 - U-000007FF: 110xxxxx 10xxxxxx  
	//     U-00000800 - U-0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx  
	//     U-00010000 - U-001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  
	//     U-00200000 - U-03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
	//     U-04000000 - U-7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
	char byte = m_file.read<char>();
	if ((byte & 0x80) == 0)
	{
		return byte;
	}
	else if ((byte & 0xE0) == 0xC0)
	{
		char16_t wch = (byte & 0x1F);
		char byte2 = (m_file.read<char>() & 0x3F);
		return ((wch << 6) | byte2);
	}
	else if ((byte & 0xF0) == 0xE0)
	{
		char16_t wch = (byte & 0x0F);
		char byte2 = (m_file.read<char>() & 0x3F);
		char byte3 = (m_file.read<char>() & 0x3F);
		return ((((wch << 6) | byte2) << 6) | byte3);
	}
	else // utf32 not support
	{
		if ((byte & 0xF8) == 0xF0)
			m_file.skip(3);
		else if ((byte & 0xFC) == 0xF8)
			m_file.skip(4);
		else if ((byte & 0xFE) == 0xFC)
			m_file.skip(5);
		return u'_';
	}
}
