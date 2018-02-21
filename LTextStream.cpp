#include "stdafx.h"
#include "LTextStream.h"

LTextStream::LTextStream()
{
}


LTextStream::~LTextStream()
{
}

bool LTextStream::openRead(const char* szFile)
{
	m_spFile = std::make_unique<LDiskFile>();
	LDiskFile* pFile = reinterpret_cast<LDiskFile*>(m_spFile.get());
	if (!pFile->openRead(szFile))
		return false;
	init();
	return true;
}

bool LTextStream::openRead(const wchar_t* szFile)
{
	m_spFile = std::make_unique<LDiskFile>();
	LDiskFile* pFile = reinterpret_cast<LDiskFile*>(m_spFile.get());
	if (!pFile->openRead(szFile))
		return false;
	init();
	return true;
}

bool LTextStream::openRead(const void* pRawData, size_t size, CodePage cp /*= cpUtf8*/)
{
	m_spFile = std::make_unique<LMemoryFile>();
	LMemoryFile* pFile = reinterpret_cast<LMemoryFile*>(m_spFile.get());
	pFile->openRead(pRawData, size);
	init(cp);
	return true;
}

void LTextStream::init(CodePage cp /*= cpUtf8*/)
{
	m_chpos = 0;
	char head[4] = { 0 };
	if (m_spFile->size() >= 4)
		m_spFile->getAs(head);

	const char bomUtf8[] = { '\xEF', '\xBB', '\xBF' };
	const char bomUtf16le[] = { '\xFF', '\xFE' };

	m_codepage = cp;
	if (0 == memcmp(head, bomUtf16le, sizeof(bomUtf16le)))
	{
		m_codepage = cpUtf16LE;
		m_spFile->skip(sizeof(bomUtf16le));
	}
	else if (0 == memcmp(head, bomUtf8, sizeof(bomUtf8)))
	{
		m_codepage = cpUtf8;
		m_spFile->skip(sizeof(bomUtf8));
	}
	else if (head[1] == 0 && head[3] == 0 && head[0] != 0 && head[2] != 0)
	{
		m_codepage = cpUtf16LE;
	}
	m_bEof = m_spFile->end();

	switch (m_codepage)
	{
	case LTextStream::cpUtf16LE:
		m_fnReadChar = &LTextStream::readUtf16LEChar;
		m_readedCharWide = sizeof(char16_t);
		break;
	case LTextStream::cpUtf8:
	default:
		m_fnReadChar = &LTextStream::readUtf8Char;
		break;
	}
	readChar();
	m_col = 0;
	m_chpos = 0;
}

char16_t LTextStream::readChar()
{
	if (!m_bEof)
	{
		m_bEof = m_spFile->end();
		++m_col;
		++m_chpos;
		m_prevChar = m_currChar;
		m_currChar = m_nextChar;
		if (!m_bEof)
			m_nextChar = read();
		else
			m_nextChar = 0;
		if (m_prevChar == u'\n')
		{
			++m_row;
			m_col = 0;
		}
	}
	return m_currChar;
}

inline char16_t LTextStream::read()
{
	return ((*this).*m_fnReadChar)();
}

char16_t LTextStream::readUtf16LEChar()
{
	return m_spFile->read<char16_t>();
}

char16_t LTextStream::readUtf8Char()
{
	//UTF8 ±àÂë¸ñÊ½£º
	//     U-00000000 - U-0000007F: 0xxxxxxx  
	//     U-00000080 - U-000007FF: 110xxxxx 10xxxxxx  
	//     U-00000800 - U-0000FFFF: 1110xxxx 10xxxxxx 10xxxxxx  
	//     U-00010000 - U-001FFFFF: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx  
	//     U-00200000 - U-03FFFFFF: 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
	//     U-04000000 - U-7FFFFFFF: 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx  
	char byte = m_spFile->read<char>();
	if ((byte & 0x80) == 0)
	{
		m_readedCharWide = 1;
		return byte;
	}
	else if ((byte & 0xE0) == 0xC0)
	{
		m_readedCharWide = 2;
		char16_t wch = (byte & 0x1F);
		char byte2 = (m_spFile->read<char>() & 0x3F);
		return ((wch << 6) | byte2);
	}
	else if ((byte & 0xF0) == 0xE0)
	{
		m_readedCharWide = 3;
		char16_t wch = (byte & 0x0F);

		char bytes[2] = { 0 };
		m_spFile->readAs(bytes);

		char byte2 = (bytes[0] & 077);
		char byte3 = (bytes[1] & 077);
		return ((((wch << 6) | byte2) << 6) | byte3);
	}
	else // utf32 not support
	{
		m_readedCharWide = 1;
		if ((byte & 0xF8) == 0xF0)
		{
			m_readedCharWide = 4;
			m_spFile->skip(3);
		}
		else if ((byte & 0xFC) == 0xF8)
		{
			m_readedCharWide = 5;
			m_spFile->skip(4);
		}
		else if ((byte & 0xFE) == 0xFC)
		{
			m_readedCharWide = 6;
			m_spFile->skip(5);
		}
		return u'_';
	}
}
