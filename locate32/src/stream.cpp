#include "stream.h"
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <wchar.h>

CReader::CReader(const wchar_t* szPath)
{
    m_fp = NULL;
#if _MSC_VER < 1400
    m_fp = _wfopen(szPath, L"rb");
#else
    _wfopen_s(&m_fp, szPath, L"rb");
#endif
}

CReader::~CReader()
{
    Close();
}

void CReader::Close()
{
    if(m_fp){
        ::fclose(m_fp);
        m_fp = NULL;
    }
}

bool CReader::IsValid() const
{
    return m_fp != NULL;
}


class OutputBuffer{
public:
    OutputBuffer(wchar_t* pBuf, int nBufLen)
    : m_pBuf(pBuf), m_pEnd(pBuf+nBufLen)
    {
        m_pCur = m_pBuf;
    }
    bool Push(const wchar_t* begin, const wchar_t* end)
    {
        int len = (int)(end-begin);
        if(m_pCur+len+1 >= m_pEnd){ //※+1は終端ヌル分
            throw myexception("file line is too long.");
            return false;
        }
        ::wmemcpy(m_pCur, begin, len);
        m_pCur[len] = 0;
        m_pCur += len; //※終端ヌル分は進めない
        return true;
    }
    int Length() const
    {
        return (int)(m_pCur - m_pBuf);
    }
private:
    wchar_t*    m_pBuf;
    wchar_t*    m_pEnd;
    wchar_t*    m_pCur;
};

wchar_t     m_buf[1024];
const wchar_t*  p_buf_begin = m_buf;
const wchar_t*  p_buf_end = m_buf;

//1行読み込み。改行コードを含む。
bool CReader::ReadLine(wchar_t* pBuf, int nBufLen)
{
    OutputBuffer result(pBuf, nBufLen);

    while(1){
        //p_buf_beginが既に終端に達している場合、さらに読み込み
        if(p_buf_begin >= p_buf_end){
            p_buf_begin = m_buf;
            size_t nRead = ::fread(m_buf, sizeof(wchar_t), _countof(m_buf), m_fp);
            p_buf_end = m_buf + nRead;
            if(nRead==0)break; //もう読み込めないのでこれにて終了
        }

        //改行コードを探す
        const wchar_t* q = wcschr(p_buf_begin, L'\n');

        //正常格納
        if(q){
            result.Push(p_buf_begin, q);
            p_buf_begin = q + 1;
            break;
        }

        //残りを全部格納
        result.Push(p_buf_begin, p_buf_end);
        p_buf_begin = p_buf_end;
    }
    return result.Length() > 0;
}

