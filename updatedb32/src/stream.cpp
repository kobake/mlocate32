#include "stream.h"

CFileOutputStream::CFileOutputStream(const wchar_t* szPath)
{
	m_hFile = ::CreateFile(
		szPath,
		GENERIC_WRITE,
		FILE_SHARE_READ,
		NULL,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL
	);
	m_dwPos = 0;
}

CFileOutputStream::~CFileOutputStream()
{
	Close();
}

void CFileOutputStream::Close()
{
	if(m_hFile != INVALID_HANDLE_VALUE){
		::CloseHandle(m_hFile);
		m_hFile = INVALID_HANDLE_VALUE;
	}
}

bool CFileOutputStream::IsValid() const
{
	return m_hFile != INVALID_HANDLE_VALUE;
}

//ファイル出力実装
void CFileOutputStream::WriteF(const wchar_t* szText, ...)
{
	wchar_t buf[1024 * 4];
	va_list v;
	va_start(v, szText);
	_vsnwprintf_s(buf, _countof(buf) - 1, szText, v);
	va_end(v);
	Write(buf);
}
void CFileOutputStream::Write(const wchar_t* szText)
{
	//wprintf(L"%ls", szText);
	DWORD nLen = (DWORD)(wcslen(szText) * sizeof(wchar_t));
	DWORD dwWrote = 0;
	BOOL bRet = ::WriteFile(
		m_hFile,
		szText,
		nLen,
		&dwWrote,
		NULL
	);
	m_dwPos += dwWrote;
	if(!bRet || dwWrote!=nLen){
		throw myexception("Write process failed.");
	}

//	size_t nResult= fwrite(szText, sizeof(wchar_t), nLen, fp);
//	if(nLen!=nResult){
//		throw myexception("Write process failed.");
//	}
}
long long CFileOutputStream::Tell()
{
	return (long long)m_dwPos;
	/*
	DWORD dw = 0;
	::GetFileSize(m_hFile, &dw);
	return (long long)dw;
	*/
}

void CFileOutputStream::Seek(long long pos)
{
	::SetFilePointer(m_hFile, (LONG)pos, NULL, FILE_BEGIN);
	m_dwPos = (DWORD)pos;
}

