#include "stdafx.h"
#include "LString.h"
#include <codecvt>
#ifndef _MAP_
#include <map>
#endif
#ifndef GSL_SPAN_H
#include <gsl/span>
#endif
#ifndef assert
#include <cassert>
#endif
#include <windows.h>

std::locale LString::s_defaultLocale;

LString& LString::assign(CStrPtr pcstr, size_t size, const std::locale& locale)
{
	clear();

	std::vector<wchar_t> buf(size);
	std::mbstate_t state = { 0 };
	const char* from_next = nullptr;
	wchar_t* to_next = nullptr;

	const converter_type& converter = std::use_facet<converter_type>(locale);
	const converter_type::result result = converter.in(
		state, pcstr, pcstr + size, from_next,
		buf.data(), buf.data() + buf.size(), to_next);

	if (result == converter_type::ok || result == converter_type::noconv)
	{
		Base::assign(buf.data(), buf.size());
	}
	else
	{
		auto span = gsl::make_span(pcstr, size);
		LStrBuilder builder(LStrBuilder::modeJoin, L"\\x");
		for (const char ch : span)
			builder.arg(static_cast<Byte>(ch), 2, 16, L'0');
		(*this) = LString(L"\\x") + builder.apply();
	}
	return (*this);
}

LString& LString::setNum(int val, int base /*= 10*/)
{
	const size_t buffsize = 33;
	wchar_t buff[buffsize] = { 0 };
	const errno_t err = _itow_s(val, buff, base);
	assert(0 == err);
	Base::assign(buff);
	return (*this);
}

LString& LString::setNum(unsigned int val, int base /*= 10*/)
{
	const size_t buffsize = 33;
	wchar_t buff[buffsize] = { 0 };
	const errno_t err = _ultow_s(val, buff, base);
	assert(0 == err);
	Base::assign(buff);
	return (*this);
}

LString& LString::setNum(float val, int prec /*= 6*/)
{
	int decimal = 0, sign = 0;
	char buff[_CVTBUFSIZE + 1] = { 0 };
	_fcvt_s(buff, val, prec, &decimal, &sign);
	buff[_CVTBUFSIZE] = 0;
	size_t sourceLen = strlen(buff);
	size_t dstLen = sourceLen + (sign ? 2 : 1);
	resize(dstLen);
	
#if _HAS_CXX17
	pointer pData = data();
#else
	pointer pData = &operator[](0);
#endif
	if (sign)
	{
		*pData = L'-';
		++pData;
	}
	for (size_t i = 0; i < (size_t)decimal; ++i)
	{
		pData[i] = buff[i];
	}
	pData[decimal] = L'.';
	pData += decimal + 1;
	for (size_t i = 0; i < sourceLen - decimal; ++i)
	{
		pData[i] = buff[decimal + i];
	}
	return (*this);
}

LString& LString::setNum(unsigned long val, int base /*= 10*/)
{
	const size_t buffsize = 33;
	wchar_t buff[buffsize] = { 0 };
	errno_t err = _ultow_s(val, buff, base);
	assert(0 == err);
	Base::assign(buff);
	return (*this);
}

LString& LString::setPtr32(void* ptr)
{
	const size_t buffsize = 35;
	const size_t prefixSize = 2;
	wchar_t buff[buffsize] = { L'0', L'x', 0 };
	errno_t err = _ultow_s((unsigned long)ptr, (wchar_t*)buff + prefixSize, buffsize - prefixSize, 16);
	assert(0 == err);
	Base::assign(buff);
	return (*this);
}

std::string LString::toUtf8() const
{
#if _HAS_CXX17
	if (empty())
		return std::string();

	std::string result;
	int szNew = WideCharToMultiByte(CP_UTF8, 0, c_str(), size(), NULL, NULL, NULL, NULL);
	if (szNew)
	{
		result.resize(szNew);
		szNew = WideCharToMultiByte(CP_UTF8, 0, c_str(), size(), result.data(), result.size(), NULL, NULL);
	}
	if (szNew)
		return result;
	else
		return std::string();
#else
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	return conv.to_bytes(c_str());
#endif
}

void LString::tolower()
{
	for (auto iter = begin(); iter != end(); ++iter)
	{
		wchar_t& ch = *iter;
		if (ch < u'\xF0')
			continue;

		ch = towlower(ch);
	}
}

LString LString::toLowerString() const
{
	LString result(*this);
	result.tolower();
	return result;
}

int LString::toInt(bool* pOk) const
{
	const int result = _wtoi(c_str());
#if _DEBUG
	if (ERANGE == errno)
		assert(!L"toInt: number overflow");
	else if (EINVAL == errno)
		assert(!L"toInt: invalid argument");
#endif
	if (pOk)
		*pOk = (0 == errno);
	return result;
}

double LString::toNumber(bool* pOk) const
{
	const double result = _wtof(c_str());
#if _DEBUG
	if (ERANGE == errno)
		assert(!L"toNumber: number overflow");
	else if (EINVAL == errno)
		assert(!L"toNumber: invalid argument");
#endif
	if (pOk)
		*pOk = (0 == errno);
	return result;
}

LString LString::fromUtf8(const std::string& bytes)
{
#if _HAS_CXX17
	if (bytes.empty())
		return LString();

	LString result;
	int szNew = MultiByteToWideChar(CP_UTF8, 0, bytes.c_str(), bytes.size(), NULL, 0);
	if (szNew)
	{
		result.resize(szNew, 0);
		szNew = MultiByteToWideChar(CP_UTF8, 0, bytes.c_str(), bytes.size(), result.data(), result.size());
	}
	if (szNew)
		return result;
	else
		return LString();
#else
	std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
	return conv.from_bytes(bytes);
#endif
}

//////////////////////////////////////////////////////////////////////////
LStrBuilder::LStrBuilder(CWStrPtr pPattern)
{
	resetPattern(pPattern);
}

LStrBuilder::LStrBuilder(CStrPtr pPattern)
{
	resetPattern(pPattern);
}

LStrBuilder::LStrBuilder(Mode mode, CWStrPtr pArg)
	: m_mode(mode)
{
	if (m_mode == modePattern)
		resetPattern(pArg);
	else
		m_pattern = pArg;
}

LStrBuilder::~LStrBuilder()
{

}

void LStrBuilder::resetPattern(CWStrPtr pPattern)
{
	m_pattern = pPattern;
	reset(modePattern);
}

void LStrBuilder::resetPattern(CStrPtr pPattern)
{
	m_pattern = pPattern;
	reset(modePattern);
}

LString LStrBuilder::apply() const
{
	if (m_mode == modePattern)
		return applyPattern();
	else
		return applyJoin();
}

void LStrBuilder::reset(Mode mode)
{
	m_mode = mode;
	m_argCount = 0;
	m_chpxes.clear();
	m_args.clear();
	analyzePattern();
}

LString LStrBuilder::applyPattern() const
{
	if (m_chpxes.empty() || m_args.empty())
		return m_pattern;

	LString result;
	CWStrPtr pBegin = m_pattern.c_str();
	CWStrPtr pPos = pBegin;
	for (auto iter = m_chpxes.begin(); iter != m_chpxes.end(); ++iter)
	{
		const ChpxInfo& info = *iter;

		size_t prevLen = pBegin + info.begin - pPos;
		result.append(pPos, prevLen);
		result.append(m_args[info.argID]);

		pPos = pBegin + info.begin + info.len;
	}

	if (pPos < pBegin + m_pattern.size())
		result.append(pPos);

	return result;
}

LString LStrBuilder::applyJoin() const
{
	if (m_args.empty())
		return LString();

	size_t cap = m_pattern.size() * (m_args.size() - 1);
	for (const LString& arg : m_args)
		cap += arg.size();

	LString result;
	result.reserve(cap);
	for (const LString& arg : m_args)
	{
		if (!result.empty())
			result.append(m_pattern);
		result.append(arg);
	}
	return result;
}

void LStrBuilder::analyzePattern()
{
	if (m_pattern.empty())
		return;

	const wchar_t* pBegin = m_pattern.c_str();
	const wchar_t* pEnd = pBegin + m_pattern.size();

	std::map<size_t, size_t> idMap;
	for (const wchar_t* pch = pBegin; pch < pEnd; ++pch)
	{
		if (*pch != L'%')
			continue;

		++pch;
		if (pch >= pEnd)
			break;

		const wchar_t ch = *pch;

		int num = ch - L'0';
		if (num <= 0 || num > 9)
			continue;

		const wchar_t* pArgBegin = pch - 1;
		if (pch + 1 < pEnd)
		{
			const int num2 = pch[1] - L'0';
			if (num2 >= 0 && num2 <= 9)
			{
				++pch;
				num *= 10;
				num += num2;
			}
		}

		ChpxInfo info;
		info.begin = pArgBegin - pBegin;
		info.len = pch - pArgBegin + 1;
		info.argID = num;
		idMap[num] = 0;
		m_chpxes.push_back(info);
	}
	if (idMap.empty())
		return;

	size_t relID = 0;
	for (auto iter = idMap.begin(); iter != idMap.end(); ++iter, ++relID)
		iter->second = relID;

	for (auto iter = m_chpxes.begin(); iter != m_chpxes.end(); ++iter)
		iter->argID = idMap[iter->argID];
}

bool LStrBuilder::isFull()
{
	if (m_args.size() >= m_argCount)
	{
		assert(!"Argument number mismatch!");
		return true;
	}
	return false;
}

LStrBuilder& LStrBuilder::arg(CWStrPtr val)
{
	m_args.push_back(val);
	return (*this);
}

LStrBuilder& LStrBuilder::arg(int val)
{
	m_args.emplace_back(LString::number(val));
	return (*this);
}

LStrBuilder& LStrBuilder::arg(int val, size_t fieldWidth, int base, wchar_t fillChar)
{
	LString str = LString::number(val, base);
	if (str.size() < fieldWidth)
	{
		m_args.emplace_back(LString(fieldWidth, fillChar));
		memcpy(&m_args.back()[fieldWidth - str.size()], str.c_str(), str.size());
	}
	else
	{
		m_args.push_back(str);
	}
	return (*this);
}

LStrBuilder& LStrBuilder::arg(float val)
{
	m_args.emplace_back(LString::number(val));
	return (*this);
}

LStrBuilder& LStrBuilder::arg(void* ptr)
{
	LString str;
	str.setNum((unsigned int)ptr, 16);
	if (!str.empty() && str.size() <= 8)
	{
		m_args.emplace_back(LString(L"0x00000000"));
		memcpy(&m_args.back()[m_args.back().size() - str.size()], str.c_str(), sizeof(wchar_t) * str.size());
	}
	return (*this);
}

LStrBuilder& LStrBuilder::arg(unsigned long val)
{
	m_args.push_back(LString().setNum(val));
	return (*this);
}
