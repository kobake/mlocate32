#include <windows.h>
#include <stdio.h>
#include <string>
#include <exception>
#include <stdarg.h>
#include <list>
#include <algorithm> //sort
#include "stream.h"
using namespace std;

int main_routine();
void enum_and_print_start(CFileOutputStream& out, wchar_t* szDir);
void enum_and_print(CFileOutputStream& out, wchar_t* szDir, int nIndent);

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

int main_routine()
{
	setlocale(LC_ALL, "");

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
	wstring strDatabaseFile = strDrive + L"\\updatedb";
	wprintf(L"Output to [%ls] ...\n", strDatabaseFile.c_str());

	//書き込み開始
	CFileOutputStream out(strDatabaseFile.c_str());
	if(!out.IsValid())throw myexception("Can't write to [%ls].", strDatabaseFile.c_str());
//	FILE* fp = NULL;
//	errno_t e = _wfopen_s(&fp, strDatabaseFile.c_str(), L"wb");
//	if(e!=0 || !fp)throw myexception("Can't write to [%ls].", strDatabaseFile.c_str());

	//書き込み
	wchar_t szRootDir[XP_MAX_PATH];
	wcscpy_s(szRootDir, _countof(szRootDir), strDrive.c_str());
	enum_and_print_start(out, szRootDir);

	//書き込み終了
	out.Close();

	//現在のドライブを取得
//	const wchar_t* p = wcschr(szCurDir, L'\\');
//	if(!p)p = wcschr(szCurDir, L'\0');
	return 0;
}


void enum_and_print_start(CFileOutputStream& out, wchar_t* szDir)
{
	out.Write(szDir);
	out.Write(L"\n");
	enum_and_print(out, szDir, 1);
}

void enum_and_print(CFileOutputStream& out, wchar_t* szDir, int nIndent)
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
	std::list<WIN32_FIND_DATA>::const_iterator p = files.begin(), q = files.end();
	for(; p != q; p++){
		//インデント出力
		for(int i=0;i<nIndent;i++)out.Write(L"\t");

		//ファイル名出力
		out.Write(p->cFileName);
		out.Write(L"\n");
//		::Sleep(100);

		//ディレクトリを再帰巡回
		if(p->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){
			wcscpy_s(pTitle, nTitleMax, p->cFileName);
			enum_and_print(out, szPath, nIndent+1);
		}
	}
}
