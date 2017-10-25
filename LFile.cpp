#include "stdafx.h"
#include "LFile.h"
#include <numeric>

bool LFile::openRead(CStrPtr pFileName)
{
	if (!open(pFileName, fopen_s, "rb"))
		return false;

	fseek(m_hFile, 0, SEEK_END);
	fgetpos(m_hFile, &m_size);
	fseek(m_hFile, 0, SEEK_SET);
	return true;
}

bool LFile::openRead(CWStrPtr pFileName)
{
	if (!open(pFileName, _wfopen_s, L"rb"))
		return false;

	fseek(m_hFile, 0, SEEK_END);
	fgetpos(m_hFile, &m_size);
	fseek(m_hFile, 0, SEEK_SET);
	return true;
}

bool LFile::openWrite(CStrPtr pFileName)
{
	return open(pFileName, fopen_s, "wb");
}

bool LFile::openWrite(CWStrPtr pFileName)
{
	return open(pFileName, _wfopen_s, L"wb");
}

LFile::ByteArray LFile::readToEnd()
{
	fpos_t cur = 0;
	fgetpos(m_hFile, &cur);
	const fpos_t remainSize = m_size - cur;
	const size_t maxSize = std::numeric_limits<size_t>::max();
	const size_t sizeToRead = (remainSize > maxSize) ? maxSize : static_cast<size_t>(remainSize);

	ByteArray buf(sizeToRead);
	fread_s(buf.data(), sizeToRead, sizeToRead, 1, m_hFile);
	return buf;
}
