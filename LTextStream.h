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

	bool eof() const { return m_file.eof(); }
	char16_t readChar();

private:
	void init();
	typedef char16_t (LTextStream::*ReadCharFunction)();
	char16_t readUtf16LEChar();
	char16_t readUtf8Char();

private:
	LFile m_file;
	CodePage m_codepage{ cpUtf8 };
	ReadCharFunction m_fnReadChar{ &LTextStream::readUtf8Char };
};

