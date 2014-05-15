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
	if(!bRet || dwWrote!=nLen){
		throw myexception("Write process failed.");
	}

//	size_t nResult= fwrite(szText, sizeof(wchar_t), nLen, fp);
//	if(nLen!=nResult){
//		throw myexception("Write process failed.");
//	}
}

