#pragma once

#ifndef _VECTOR_
#include <vector>
#endif
#ifndef assert
#include <cassert>
#endif
#define CheckSize(lhs, rhs) assert(lhs == rhs);
#ifdef max
#undef max
#endif

class LFile
{
public:
	LFile() {}
	~LFile() { close(); }

	typedef const wchar_t* CWStrPtr;
	typedef const char* CStrPtr;
	typedef std::vector<unsigned char> ByteArray;

	bool openRead(CStrPtr pFileName);
	bool openRead(CWStrPtr pFileName);
	template <typename T> size_t readAs(T& var);
	template <typename T> size_t readAs(T* pBuf, size_t cnt);
	template <typename T> T read();
	ByteArray readToEnd();
	template <typename T> size_t getAs(T& var);

	bool openWrite(CStrPtr pFileName);
	bool openWrite(CWStrPtr pFileName);
	template <typename T> size_t write(T& var);
	template <typename T> size_t write(T* pBuf, size_t cnt);

	void close() { if (m_hFile) fclose(m_hFile); }
	void reset() { fseek(m_hFile, 0, SEEK_SET); }
	void skip(long sz) { fseek(m_hFile, sz, SEEK_CUR); }

	bool opened() const { return m_hFile != nullptr; }
	fpos_t size() const { return m_size; }
	errno_t error() const { return ferror(m_hFile); }
	bool eof() const
	{
		fpos_t cur = 0;
		fgetpos(m_hFile, &cur);
		return (cur >= m_size || 0 != feof(m_hFile));
	}

private:
	template <typename CharType, typename Function>
	bool open(const CharType* pFileName, Function fnOpen_s, const CharType* pMode);

private:
	FILE* m_hFile	{ nullptr };
	fpos_t m_size	{ 0 };
};

template <typename T>
size_t LFile::readAs(T& var)
{
	size_t sz = fread_s((void*)&var, sizeof(T), sizeof(T), 1, m_hFile);
	CheckSize(1, sz);
	return sizeof(T);
}

template <typename T>
size_t LFile::readAs(T* pBuf, size_t cnt)
{
	size_t sz = fread_s((void*)pBuf, sizeof(T) * cnt, sizeof(T), cnt, m_hFile);
	CheckSize(cnt, sz);
	return sz;
}

template <typename T>
T LFile::read()
{
	T res;
	readAs(res);
	return res;
}

template <typename T>
size_t LFile::getAs(T& var)
{
	static_assert(sizeof(T) <= std::numeric_limits<long>::max(), "size too large");
	size_t sz = readAs(var);
	fseek(m_hFile, -(long)sz, SEEK_CUR);
	return sz;
}

template <typename T>
size_t LFile::write(T& var)
{
	size_t sz = fwrite((void*)&var, sizeof(T), 1, m_hFile);
	CheckSize(sizeof(T), sz);
	return sz;
}

template <typename T>
size_t LFile::write(T* pBuf, size_t cnt)
{
	size_t sz = fwrite(reinterpret_cast<const void*>(pBuf), sizeof(T), cnt, m_hFile);
	CheckSize(sizeof(T) * cnt, sz);
	return sz;
}

template <typename CharType, typename Function>
bool LFile::open(const CharType* pFileName, Function fnOpen_s, const CharType* pMode)
{
	if (m_hFile)
		return false;

	errno_t err = fnOpen_s(&m_hFile, pFileName, pMode);
	return (0 == err);
}
