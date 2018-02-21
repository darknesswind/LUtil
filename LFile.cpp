#include "stdafx.h"
#include "LFile.h"
#include <numeric>
#ifdef max
#undef max
#endif
bool LDiskFile::openRead(CStrPtr pFileName)
{
	if (!open(pFileName, fopen_s, "rb"))
		return false;

	fseek(m_hFile, 0, SEEK_END);
	fgetpos(m_hFile, &m_size);
	fseek(m_hFile, 0, SEEK_SET);
	return true;
}

bool LDiskFile::openRead(CWStrPtr pFileName)
{
	if (!open(pFileName, _wfopen_s, L"rb"))
		return false;

	fseek(m_hFile, 0, SEEK_END);
	fgetpos(m_hFile, &m_size);
	fseek(m_hFile, 0, SEEK_SET);
	return true;
}

bool LDiskFile::openWrite(CStrPtr pFileName)
{
	return open(pFileName, fopen_s, "wb");
}

bool LDiskFile::openWrite(CWStrPtr pFileName)
{
	return open(pFileName, _wfopen_s, L"wb");
}

LFile::ByteArray LFile::readToEnd()
{
	const fpos_t cur = pos();
	const fpos_t remainSize = m_size - cur;
	const size_t maxSize = std::numeric_limits<size_t>::max();
	const size_t sizeToRead = (remainSize > maxSize) ? maxSize : static_cast<size_t>(remainSize);

	ByteArray buf(sizeToRead);
	read(buf.data(), sizeToRead, sizeToRead, 1);
	return buf;
}

void LMemoryFile::openRead(const void* pData, size_t size)
{
	m_pMemory = reinterpret_cast<const char*>(pData);
	m_pCurrent = m_pMemory;
	m_size = size;
}

int LMemoryFile::seek(long offset, int origin)
{
	if (!m_pMemory || !m_pCurrent)
	{
		assert(false);
		errno = EBADF;
		return -1;
	}
	switch (origin)
	{
	case SEEK_SET:
		if (offset < 0 || offset > m_size)
		{
			assert(false);
			errno = EINVAL;
			return -1;
		}
		m_pCurrent = m_pMemory + offset;
		break;
	case SEEK_CUR:
		if (m_pCurrent + offset > m_pMemory + m_size ||
			m_pCurrent + offset < m_pMemory)
		{
			assert(false);
			errno = EINVAL;
			return -1;
		}
		m_pCurrent += offset;
		break;
	case SEEK_END:
		if (offset > 0 || offset + m_size < 0)
		{
			assert(false);
			errno = EINVAL;
			return -1;
		}
		m_pCurrent = m_pMemory + (m_size + offset);
		break;
	default:
		assert(false);
		errno = EINVAL;
		return -1;
	}
	return 0;
}

size_t LMemoryFile::read(void* buffer, size_t bufSize, size_t elemSize, size_t elemCount)
{
	if (!m_pMemory || !m_pCurrent)
	{
		errno = EBADF;
		return -1;
	}
	if (m_pCurrent < m_pMemory || m_pCurrent > m_pMemory + m_size)
	{
		errno = EINVAL;
		return -1;
	}

	size_t remindSize = static_cast<size_t>(m_size) - (m_pCurrent - m_pMemory);
	size_t elemWrite = std::min(elemCount, bufSize / elemSize);
	size_t elemRead = std::min(elemWrite, remindSize / elemSize);

	size_t bytesRead = elemRead * elemSize;
	if (bytesRead > 0)
	{
		memcpy(buffer, m_pCurrent, bytesRead);
		m_pCurrent += bytesRead;
	}
	return elemRead;
}
