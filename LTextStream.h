#pragma once
#include "LFile.h"

class LTextStream
{
	enum CodePage
	{
		cpUtf8,
		cpUtf16LE,
		cpMultiByte,
	};
public:
	LTextStream();
	~LTextStream();

	bool openRead(const char* szFile);
	bool openRead(const wchar_t* szFile);

	bool eof() const { return (m_pos >= m_file.size()); }
	fpos_t pos() const { return m_pos; }
	char16_t readChar();
	char16_t peekChar() const { return m_nextChar; }

private:
	void init();
	char16_t read();
	typedef char16_t (LTextStream::*ReadCharFunction)();
	char16_t readUtf16LEChar();
	char16_t readUtf8Char();

private:
	LFile m_file;
	CodePage m_codepage{ cpUtf8 };
	ReadCharFunction m_fnReadChar{ &LTextStream::readUtf8Char };

	char16_t m_prevChar{ 0 };
	char16_t m_currChar{ 0 };
	char16_t m_nextChar{ 0 };
	fpos_t m_pos{ 0 };
	fpos_t m_row{ 0 };
	fpos_t m_col{ 0 };
};

