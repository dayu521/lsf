#include <iostream>
#include <fstream>
#include <memory>
//#include <json/json.h>
#include "lexer.h"
using namespace std;

std::string to_cstring(const std::wstring & s)
{
    char cc[MB_CUR_MAX];
    std::string r{};
    for(const auto c:s){
        auto l=wctomb(cc,c);
        r.append(cc,l);
    }
    return r;
}

int main()
{
//    setlocale(LC_ALL,"zh_CN.UTF-8");
    //std::locale("").name().c_str()
//    std::locale::global(std::locale(""));
    auto f1="2.txt";
    auto f2="文本文件.txt";
    auto old=std::setlocale(LC_ALL,nullptr);
    std::setlocale(LC_ALL,std::locale("").name().c_str());
    lsf::Lexer lex(std::make_unique<lsf::MBuff>(f2));
    auto ok=lex.run();
    auto end=lsf::Token{lsf::Type::END};
    while (ok) {
        std::cout<<to_cstring(lex.get_token().value_);
        ok=lex.run();
        if(end==lex.get_token())
            break;
    }
    std::setlocale(LC_ALL,old);
    return 0;
}

