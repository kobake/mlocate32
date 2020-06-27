#include <windows.h>
#include <stdio.h>
#include <string>
#include <exception>
#include <stdarg.h>
#include <list>
#include <algorithm> //sort
#include <map>
#include "stream.h"
#include <locale.h>
using namespace std;

int main_routine();
void enum_and_print_start(CFileOutputStream& out, wchar_t* szDir);
long long enum_and_print(CFileOutputStream& out, wchar_t* szDir, int nIndent);

#ifdef _DEBUG
    #define DBFILENAME L"updatedb_debug"
#else
    #define DBFILENAME L"updatedb"
#endif


//※WindowsXPでは32768文字のパスをサポートする。
#define XP_MAX_PATH (32768+1)

bool operator < (const WIN32_FIND_DATA& lhs, const WIN32_FIND_DATA& rhs)
{
    bool lhsDir = ((lhs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    bool rhsDir = ((rhs.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0);
    if(lhsDir != rhsDir){
        return lhsDir?true:false;
    }
    return _wcsicmp(lhs.cFileName, rhs.cFileName) < 0;
}

int wmain(int argc, wchar_t* argv[])
{
    try{
        return main_routine();
    }
    catch(const exception& e){
        printf("Error: %hs\n", e.what());
    }
}

std::map<long long, long long> g_sizes;

int main_routine()
{
    setlocale(LC_ALL, "jpn");

    //現在のディレクトリを取得
    //例「D:\abc\def」「\\landisk\abc\def」「D:」
    wchar_t szCurDir[XP_MAX_PATH];
    ::GetCurrentDirectory(_countof(szCurDir), szCurDir);
    {
        //末尾の「\」は削る
        wchar_t* p = wcsrchr(szCurDir, L'\\');
        if(p && *(p+1)==L'\0')*p = L'\0';
    }

    //現在のドライブを取得
    //例「C:」「D:」
    wstring strDrive;
    {
        const wchar_t* p = szCurDir;
        while(*p==L'\\')p++;
        const wchar_t* q = wcschr(p, L'\\');
        if(!q)q = wcschr(p, L'\0');
        strDrive.assign(p, q);
    }

    //データベースファイル名を生成
    //例「D:\updatedb」
    wstring strDatabaseFile = strDrive + L"\\" + DBFILENAME;
    wprintf(L"Output to [%ls] ...\n", strDatabaseFile.c_str());

    //書き込み開始
    CFileOutputStream out(strDatabaseFile.c_str());
    if(!out.IsValid())throw myexception("Can't write to [%ls].", strDatabaseFile.c_str());
//  FILE* fp = NULL;
//  errno_t e = _wfopen_s(&fp, strDatabaseFile.c_str(), L"wb");
//  if(e!=0 || !fp)throw myexception("Can't write to [%ls].", strDatabaseFile.c_str());

    //書き込み
    g_sizes.clear();
    wchar_t szRootDir[XP_MAX_PATH];
    wcscpy_s(szRootDir, _countof(szRootDir), strDrive.c_str());
    enum_and_print_start(out, szRootDir);

    // 2stage
    for(std::map<long long, long long>::const_iterator itr = g_sizes.begin(); itr != g_sizes.end(); itr++){
        long long pos = itr->first;
        long long size = itr->second;
        out.Seek(pos);
        out.WriteF(L"%020lld", size);
    }
    g_sizes.clear();

    //書き込み終了
    out.Close();

    //現在のドライブを取得
//  const wchar_t* p = wcschr(szCurDir, L'\\');
//  if(!p)p = wcschr(szCurDir, L'\0');
    return 0;
}


void enum_and_print_start(CFileOutputStream& out, wchar_t* szDir)
{
    out.Write(szDir);
    out.Write(L"\n");
    enum_and_print(out, szDir, 1);
}

// アイテムのサイズを返す
long long enum_and_print(CFileOutputStream& out, wchar_t* szDir, int nIndent)
{
    //CPUをあんまり食いつぶさないように。
    ::Sleep(1);

    //ワイルドカード式ファイルパスを構築
    wchar_t* szPath = szDir;
    wcscat_s(szPath, XP_MAX_PATH, L"\\*");

    //ファイルタイトル部分を覚えておく
    wchar_t* pTitle = wcschr(szPath, L'\0') - 1;
    int nTitleMax = XP_MAX_PATH - (int)(pTitle - szPath);

    //ファイル一覧収集
    WIN32_FIND_DATA fd;
    HANDLE hFind = ::FindFirstFile(szPath, &fd);
    std::list<WIN32_FIND_DATA> files;
    if(hFind!=INVALID_HANDLE_VALUE){
        do{
            //「.」と「..」はスキップ
            if(wcscmp(fd.cFileName, L".")==0)continue;
            if(wcscmp(fd.cFileName, L"..")==0)continue;

            //リスト構築
            files.push_back(fd);
        }
        while(::FindNextFile(hFind, &fd));
    }
    ::FindClose(hFind);

    //ファイル一覧ソート
    files.sort();

    //ファイル一覧処理
    long long totalsize = 0;
    std::list<WIN32_FIND_DATA>::const_iterator p = files.begin(), q = files.end();
    for(; p != q; p++){
        //インデント出力
        for(int i=0;i<nIndent;i++)out.Write(L"\t");

        // ファイル名出力
        const wchar_t* test = p->cFileName;
        const WIN32_FIND_DATA* test2 = &(*p);
        
        out.Write(p->cFileName);

        // タブ区切りで属性値を出力
        out.Write(L"\t");
        long long size = p->nFileSizeLow | (((long long)p->nFileSizeHigh) << 32);
        out.WriteF(L"Size:%lld", size);
        //out.Seek();

        //ディレクトリを再帰巡回
        if(p->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
            if(p->dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT){
                // シンボリックリンクまたはジャンクションなので配下は無視
                out.Write(L",Type:ReparsePoint\n");
            }
            else{
                // フォルダ情報出力スペース予約
                out.WriteF(L",Type:Folder,FolderSize:xxxxxxxxxxxxxxxxxxxx", size);
                long long foldersizepos = out.Tell() - 20 * 2;
                out.Write(L"\n");
                // 配下情報
                wcscpy_s(pTitle, nTitleMax, p->cFileName);
                size += enum_and_print(out, szPath, nIndent+1);
                size += 10;
                // 保存すべきサイズをひとまず保持（2stage目でこの値は吐き出す）
                g_sizes[foldersizepos] = size;
            }
        }
        else{
            out.Write(L",Type:File\n");
        }

        // サイズ加算
        totalsize += size;
    }
    return totalsize;
}
