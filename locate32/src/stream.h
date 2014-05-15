#pragma once

#include <stdio.h> //FILE

#ifndef _countof
	#define _countof(A) (sizeof(A) / sizeof(A[0]))
#endif
#if _MSC_VER < 1400
	#define vsprintf_s _vsnprintf
	#define wcscat_s(A,B,C) wcsncat(A,C,B)
	#define wcscpy_s(A,B,C) wcsncpy(A,C,B)
#endif

class CReader{
public:
	CReader(const wchar_t* szPath);
	~CReader();
	void Close();
	bool IsValid() const;
	bool ReadLine(wchar_t* pBuf, int nBufLen);
private:
	FILE* m_fp;
};


#include <exception>
#include <stdarg.h>
#include <windows.h> //_countof

class myexception : public std::exception{
public:
	myexception(const char* str, ...)
	{
		va_list v;
		va_start(v, str);
		vsprintf_s(m_str, _countof(m_str), str, v);
		va_end(v);
	}
	const char* what() const throw(){ return m_str; }
private:
	char m_str[256];
};
