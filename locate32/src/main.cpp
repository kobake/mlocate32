#include <stdio.h>
#include <exception>
#include <string>
#include <stdarg.h>
#include <windows.h>
#include "stream.h"
using namespace std;

int wmain_routine(int argc, wchar_t* argv[]);

//※WindowsXPでは32768文字のパスをサポートする。
#define XP_MAX_PATH (32768+1)


int ParseIndent(const wchar_t* szText)
{
	const wchar_t* p = szText;
	while(*p==L'\t')p++;
	return (int)(p-szText);
}

const wchar_t* wcsistr(const wchar_t* str1, const wchar_t* str2)
{
	int len1 = (int)wcslen(str1);
	int len2 = (int)wcslen(str2);
	int cnt = len1 - len2 + 1;
	const wchar_t* p = str1;
	for(int i=0;i<cnt;i++,p++){
		if(_wcsnicmp(p, str2, len2)==0)return p;
	}
	return NULL;
}


int wmain(int argc, wchar_t* argv[])
{
	try{
		return wmain_routine(argc, argv);
	}
	catch(const exception& e){
		wprintf(L"Error: %hs\n", e.what());
	}
}

int wmain_routine(int argc, wchar_t* argv[])
{
	setlocale(LC_ALL, "");

	//コマンドライン引数 -> szKeyword
	if(argc<2){
		wprintf(L"Usage: locate32 <keyword>\n");
		return 1;
	}
	const wchar_t* szKeyword = argv[1];

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
//	wprintf(L"Output to [%ls] ...\n", strDatabaseFile.c_str());

	//読み込み開始
	CReader in(strDatabaseFile.c_str());
	if(!in.IsValid())throw myexception("Can't read [%ls]", strDatabaseFile.c_str());

	//巡回
	wchar_t szLine[XP_MAX_PATH];
	wstring aIndentNames[512];
	while(in.ReadLine(szLine, _countof(szLine))){
		//インデント取得・インデント除去
		int nIndent = ParseIndent(szLine);
		wchar_t* pLine = szLine + nIndent;
		if(nIndent>=_countof(aIndentNames)){
			throw myexception("Indent [%d] is too long; than %d.", nIndent, (int)_countof(aIndentNames));
		}

		//インデント位置へのアイテム文字列記録
		const wchar_t* tab = wcschr(pLine, '\t');
		if(tab){
			aIndentNames[nIndent].assign(pLine, tab);
		}
		else{
			aIndentNames[nIndent] = pLine;
		}
		const wchar_t* pCurrentName = aIndentNames[nIndent].c_str();

		//キーワードマッチ
		if(wcsistr(pCurrentName, szKeyword)){
			//フルパス出力
			for(int i=0;i<nIndent;i++){
				wprintf(L"%ls\\", aIndentNames[i].c_str());
			}
			wprintf(L"%ls", pCurrentName);

			// 属性出力
			if(tab){
				wprintf(L" (%ls)", tab + 1);
			}

			// 改行
			wprintf(L"\n");
		}
	}

	//読み込み終了
	in.Close();

	return 0;
}
