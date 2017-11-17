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
	wchar_t readChar();

private:
	typedef wchar_t (LTextStream::*ReadCharFunction)();
	wchar_t readUtf16LEChar();
	wchar_t readUtf8Char();

private:
	LFile m_file;
	CodePage m_codepage{ cpUtf8 };
	ReadCharFunction m_fnReadChar{ &readUtf8Char };
};

