#include <stdio.h>
#include <exception>
#include <string>
#include <stdarg.h>
#include <windows.h>
#include "stream.h"
using namespace std;

int main_routine(const char* pattern);

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



//#include "interpreter.hpp"
#include <boost/program_options.hpp>
#include <iostream>
namespace po = boost::program_options;

int main2(const std::vector<std::string>& args);

std::string w2a(std::wstring wstr)
{
	int n = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), NULL, 0, NULL, NULL);
	std::vector<char> buf(n + 1);
	n = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), wstr.length(), &buf[0], n, NULL, NULL);
	buf[n] = 0;
	return &buf[0];
}

std::wstring a2w(std::string str)
{
	int n = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), NULL, 0);
	std::vector<wchar_t> buf(n + 1);
	n = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), &buf[0], n);
	buf[n] = 0;
	return &buf[0];
}

int wmain(int argc, wchar_t* argv[])
{
	setlocale(LC_ALL, "jpn");

	std::vector<std::string> args;
	for(int i = 1; i < argc; i++){
		args.push_back(w2a(argv[i]));
	}
	/*
	char* argv[] = {"locate32.exe", "こんにちは", "hoge", "fuga", "piyo", "xxx", "yyy", "--size=10G", "ppp", "vvv"};
	//wchar_t* argv_w[] = {L"locate32.exe", L"こんにちは"};
	int argc = _countof(argv);
	*/
	return main2(args);
}

inline bool IS_DIGIT(char c){ return c >= '0' && c <= '9'; }

class SizeOption{
public:
	// [+-]N[bcwkMG]
	/*
	-size n[bcwkMG]- this will return a list of files having size “n” (of course as discussed at the beginning of part II the list can consist of files bigger than n (+n), smaller (-n) or equal n. The letter after the number define size unit. Use:
	b – for 512 bytes blocks (b for blocks);
	c – for bytes (c for – I have no idea, probably b was already in use);
	w – for two bytes words (w for words);
	k – for Kilobytes (1024 bytes);
	M – for Megabytes (1048576 bytes);
	G – for Gigabytes (1073741824 bytes).
	*/
	SizeOption(const char* str = "")
	{
		m_kind = 999;
		m_size = 0;
		// パース
		const char* p = str;
		if(!p)return;
		if(!*p)return;
		char c;
		// 先頭文字
		c = *p;
		if(c == '+')m_kind = 1, p++; // bigger
		else if(c == '-')m_kind = -1, p++; // smaller
		//else if(c == '=')m_kind = 0, p++; // equal
		else if(IS_DIGIT(c))m_kind = 0; // equal
		else{
			std::string s = (std::string)"Invalid size specification: " + str;
			throw std::exception(s.c_str());
		}
		// 後続数値
		if(!IS_DIGIT(*p)){
			std::string s = (std::string)"Invalid size specification: " + str;
			throw std::exception(s.c_str());
		}
		m_size = 0;
		while(IS_DIGIT(c = *p++)){
			m_size *= 10;
			long long n = c - '0';
			m_size += n;
		}
		// 末尾単位
		long long unit = 512; // default: 512bytes block
		switch(c){
		case '\0': unit = 512; break; // 512bytes block (default)
		case ' ': unit =  512; break; // 512bytes block (default)
		case 'b': unit =  512; break; // 512bytes block
		case 'c': unit =    1; break; // byte
		case 'w': unit =    2; break; // 2byte word
		case 'k': unit = 1024; break; // KBytes (1024 bytes)
		case 'M': unit = 1024 * 1024; break;        // MBytes (1024 * 1024 = 1048576 bytes)
		case 'G': unit = 1024 * 1024 * 1024; break; // GBytes (1024 * 1024 * 1024 = 1073741824 bytes)
		default:
			{
				std::string s = (std::string)"Invalid size specification: " + str;
				throw std::exception(s.c_str());
			}
		}
		// 格納
		m_size *= unit;
	}
	bool Match(const wchar_t* attributes)
	{
		if(m_kind == 999){
			// 条件チェック無し
			return true;
		}

		// アイテムのサイズ
		long long attrsize = attr2size(attributes);

		// 等
		if(m_kind == 0){
			return attrsize == m_size;
		}
		// より上
		else if(m_kind == 1){
			return attrsize >= m_size;
		}
		// より下
		else if(m_kind == -1){
			return attrsize <= m_size;
		}

		// 例外 (条件チェック無し)
		return true;
	}

	// FolderSize:xxx または Size:xxx を検出
	static long long attr2size(const wchar_t* attrs)
	{
		const wchar_t* p;
		p = wcsstr(attrs, L"FolderSize:");
		if(p){
			p += (_countof(L"FolderSize:") - 1);
			return ::_wtoi64(p);
		}
		p = wcsstr(attrs, L"Size:");
		if(p){
			p += (_countof(L"Size:") - 1);
			return ::_wtoi64(p);
		}
		return 0;
	}
private:
	int			m_kind; // 比較方法  999:無し 0:equal 1:bigger -1:smaller
	long long	m_size; // 比較基準サイズ
};
SizeOption g_sizeOption;

int main2(const std::vector<std::string>& args)
{
	try{
		// オプション一覧
		std::string sizeOption = "";

		// オプション
		po::options_description gen("Options");
		gen.add_options()
			("help,h", "Display this help message")
			("size,s", po::value<std::string>(&sizeOption)->value_name("[+-]N[bcwkMG]"),  "File size for filtering (Specifying it as one of 'find' command options)")
			;

		// 非表示オプション
		po::options_description hidden("Hidden options");
		hidden.add_options()
			("input-file", po::value< vector<string> >(), "input file"); // 任意個受け取りたいものは、vector<T>で。

		// 非オプション引数
		po::positional_options_description pos;
		pos.add("input-file", -1); // -1とすることで、全てのpositionalオプションが、"input-file"であると解釈される（vector<string>に、ざくざく入っていく）
		
		// 全て
		po::options_description cmdline_options;
		cmdline_options.add(gen).add(hidden);

		po::variables_map values;
		//po::store(po::parse_command_line(argc, argv, desc), values);
		po::store(po::command_line_parser(args).options(cmdline_options).positional(pos).run(), values);
		po::notify(values);

		// ヘルプが押されたら
		if (values.count("help")) {
			std::cout << "Usage: locate32 [options] <file-name-pattern>\n";
			std::cout << "Search for entries in a mlocate32 database.\n";
			std::cout << "\n";
			std::cout << gen << "\n";
			std::cout << "Sample usage:\n";
			std::cout << "  locate32 hoge.exe      # Match named \"*hoge.exe*\".\n";
			std::cout << "  locate32 .dat -s +10M  # Match named \"*.dat*\" and size is bigger than 10MB.\n";
			std::cout << "  locate32 .dat -s +10G  # Match named \"*.dat*\" and size is bigger than 10GB.\n";
			return 1;
		}

		// サイズオプション取得
		/*
		if(values.count("size")){
			sizeOption = values["size"].as<std::string>();
			std::cout << "size = " << sizeOption << "\n";
		}
		*/
		//std::cout << "sizeOption = " << sizeOption << "\n";

		// 通常引数
		std::string pattern;
		if (values.count("input-file"))
		{
			const std::vector<std::string>& v = values["input-file"].as< std::vector<std::string> >();
			/*
			std::cout << "---------------------\n";
			for(int i = 0; i < (int)v.size(); i++){
				std::cout << i << ":" << v[i] << "\n";
			}
			std::cout << "---------------------\n";
			*/
			//std::cout << "Input files are: " <<  << "\n";
			pattern = v[0];
		}
		else{
			std::cout << "locate32: no pattern to search for specified\n";
			return 1;
		}

		// 処理
		g_sizeOption = SizeOption(sizeOption.c_str());
		return main_routine(pattern.c_str());
	}
	catch(const std::exception& ex){
		std::cout << "Error: " << ex.what() << "\n";
		return 1;
	}

	// サイズ指定があったら

	/*
	auto interpreter = Interpreter();

	if (vm.count("xboard")) {
	interpreter.is_running_over_xboard = true;
	}
	interpreter.prompt();
	*/
}
/*
int wmain(int argc, wchar_t* argv[])
{
	try{
		return wmain_routine(argc, argv);
	}
	catch(const exception& e){
		wprintf(L"Error: %hs\n", e.what());
	}
}*/

int main_routine(const char* pattern)
{
	std::wstring wstrPattern = a2w(pattern);

	const wchar_t* szKeyword = wstrPattern.c_str();

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
			aIndentNames[nIndent].assign(pLine, tab - pLine);
		}
		else{
			aIndentNames[nIndent] = pLine;
		}
		const wchar_t* pCurrentName = aIndentNames[nIndent].c_str();

		//キーワードマッチ
		if(wcsistr(pCurrentName, szKeyword)){
			// サイズチェック
			if(!g_sizeOption.Match(tab))continue;

			// フルパス出力
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
