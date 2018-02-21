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
	virtual ~LFile() { }

	typedef const wchar_t* CWStrPtr;
	typedef const char* CStrPtr;
	typedef std::vector<unsigned char> ByteArray;

	template <typename T> size_t readAs(T& var);
	template <typename T> size_t readAs(T* pBuf, size_t cnt);
	template <typename T> T read();
	ByteArray readToEnd();
	template <typename T> size_t getAs(T& var);

	template <typename T> size_t write(T& var);
	template <typename T> size_t write(T* pBuf, size_t cnt);

	virtual int close() = 0;
	virtual bool opened() const = 0;
	virtual errno_t error() const = 0;
	virtual fpos_t pos() const = 0;

	void reset() { seek(0, SEEK_SET); }
	void skip(long sz) { seek(sz, SEEK_CUR); }

	fpos_t size() const { return m_size; }
	bool end() const
	{
		if (eof())
			return true;

		return (pos() >= m_size);
	}

protected:
	virtual int seek(long offset, int origin) = 0;
	virtual bool eof() const = 0;
	virtual size_t read(void* buffer, size_t bufSize, size_t elemSize, size_t elemCount) = 0;
	virtual size_t write(void* buffer, size_t elemSize, size_t elemCount) = 0;

protected:
	fpos_t m_size{ 0 };
};

template <typename T>
size_t LFile::readAs(T& var)
{
	size_t sz = read((void*)&var, sizeof(T), sizeof(T), 1);
	CheckSize(1, sz);
	return sizeof(T);
}

template <typename T>
size_t LFile::readAs(T* pBuf, size_t cnt)
{
	size_t sz = read((void*)pBuf, sizeof(T) * cnt, sizeof(T), cnt);
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
	seek(-(long)sz, SEEK_CUR);
	return sz;
}

template <typename T>
size_t LFile::write(T& var)
{
	size_t sz = write((void*)&var, sizeof(T), 1);
	CheckSize(sizeof(T), sz);
	return sz;
}

template <typename T>
size_t LFile::write(T* pBuf, size_t cnt)
{
	size_t sz = write(reinterpret_cast<const void*>(pBuf), sizeof(T), cnt);
	CheckSize(sizeof(T) * cnt, sz);
	return sz;
}

//////////////////////////////////////////////////////////////////////////


class LDiskFile : public LFile
{
public:
	~LDiskFile() final { close(); }
	bool openRead(CStrPtr pFileName);
	bool openRead(CWStrPtr pFileName);
	bool openWrite(CStrPtr pFileName);
	bool openWrite(CWStrPtr pFileName);

public:
	int close() final { if (m_hFile) return fclose(m_hFile); else return 0; }
	bool opened() const final { return m_hFile != nullptr; }
	errno_t error() const final { return ferror(m_hFile); }
	fpos_t pos() const final
	{
		fpos_t cur = 0;
		fgetpos(m_hFile, &cur);
		return cur;
	}

protected:
	int seek(long offset, int origin) final { return fseek(m_hFile, offset, origin); }
	bool eof() const final { return (0 != feof(m_hFile)); }
	size_t read(void* buffer, size_t bufSize, size_t elemSize, size_t elemCount) final
	{
		return fread_s(buffer, bufSize, elemSize, elemCount, m_hFile);
	}
	size_t write(void* buffer, size_t elemSize, size_t elemCount) final
	{
		return fwrite(buffer, elemSize, elemCount, m_hFile);
	}

private:
	template <typename CharType, typename Function>
	bool open(const CharType* pFileName, Function fnOpen_s, const CharType* pMode)
	{
		if (m_hFile)
			return false;

		errno_t err = fnOpen_s(&m_hFile, pFileName, pMode);
		return (0 == err);
	}

private:
	FILE* m_hFile{ nullptr };
};

class LMemoryFile : public LFile
{
public:
	void openRead(const void* pData, size_t size);

public:
	int close() final { return 0; }
	bool opened() const final { return m_pMemory != nullptr; }
	errno_t error() const final { return 0; }
	fpos_t pos() const final { return m_pCurrent - m_pMemory; }

protected:
	int seek(long offset, int origin) final;
	bool eof() const final { return (m_pCurrent >= m_pMemory + m_size); }
	size_t read(void* buffer, size_t bufSize, size_t elemSize, size_t elemCount) final;
	size_t write(void* buffer, size_t elemSize, size_t elemCount) final
	{
		return 0;
	}

private:
	const char* m_pMemory{ nullptr };
	const char* m_pCurrent{ nullptr };
};
