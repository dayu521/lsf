#include <iostream>
#include <fstream>
#include <memory>
#include"kmp.h"
//#include <json/json.h>
#include "lexer.h"
using namespace std;
template<class T1, class T2>
struct P {
 T1 x1;
 T2 x2;
 friend auto operator<=>(const P&, const P&) = default;
};
int main()
{
//    setlocale(LC_ALL,"zh_CN.UTF-8");
    //std::locale("").name().c_str()
    std::locale::global(std::locale(""));
//    MBuff m("文本文件.txt");
//    m.next_char();
//    while (!m.is_eof()) {
//        auto ss=m.current_char();
//        std::wcout<<ss;
//        m.next_char();
//    }
//    std::wcout.flush();
//    char s[]="\xe4\xbd\xa0";
//    wchar_t w;
//    mbtowc(&w,s,3);
//    wcout<<w<<endl;
//    wcout<<L'\x4F60'<<endl;
//    wcout<<wchar_t(0x4F60)<<endl;
//    char c2[]="\x50\n";
    lsf::Lexer lex(std::make_unique<lsf::MBuff>("文本文件.txt"));
    auto tok=lex.next();
    auto end=lsf::Token{lsf::Type::END};
    while (tok!=end) {
        std::wcout<<tok.value_<<std::endl;
        tok=lex.next();
    }
    return 0;
}

