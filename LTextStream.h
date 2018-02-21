#pragma once
#include "LFile.h"

class LString;
class LTextStream
{
public:
	typedef size_t row_t;
	typedef size_t col_t;
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
	bool openRead(const void* pRawData, size_t size, CodePage cp = cpUtf8);

	bool eof() const { return m_bEof; }
	fpos_t posChar() const { return m_chpos; }
	row_t row() const { return m_row; }
	col_t col() const { return m_col; }
	char16_t readChar();
	char16_t peekChar() const { return m_nextChar; }
	char16_t currChar() const { return m_currChar; }
	char16_t prevChar() const { return m_prevChar; }
	char16_t readIf(char16_t ch) { return (m_nextChar == ch) ? readChar() : u'\0'; }

private:
	void init(CodePage cp = cpUtf8);
	char16_t read();
	typedef char16_t (LTextStream::*ReadCharFunction)();
	char16_t readUtf16LEChar();
	char16_t readUtf8Char();

private:
	std::unique_ptr<LFile> m_spFile;
	CodePage m_codepage{ cpUtf8 };
	ReadCharFunction m_fnReadChar{ &LTextStream::readUtf8Char };

	int16_t  m_readedCharWide{ 1 };
	char16_t m_prevChar{ 0 };
	char16_t m_currChar{ 0 };
	char16_t m_nextChar{ 0 };
	fpos_t m_chpos{ 0 };
	row_t m_row{ 0 };
	col_t m_col{ 0 };
	bool m_bEof{ false };
};

