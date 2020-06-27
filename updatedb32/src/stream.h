#pragma once

#include <windows.h> //HANDLE

#ifndef _countof
    #define _countof(A) (sizeof(A) / sizeof(A[0]))
#endif
#if _MSC_VER < 1400
    #define vsprintf_s _vsnprintf
    #define wcscat_s(A,B,C) wcsncat(A,C,B)
    #define wcscpy_s(A,B,C) wcsncpy(A,C,B)
#endif

class CFileOutputStream{
public:
    CFileOutputStream(const wchar_t* szPath);
    ~CFileOutputStream();
    bool IsValid() const;
    void Write(const wchar_t* szText);
    void WriteF(const wchar_t* szText, ...);
    void Close();
    long long Tell();
    void Seek(long long pos);
private:
    HANDLE  m_hFile;
    DWORD   m_dwPos;
};


#include <exception>
#include <stdio.h>

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
