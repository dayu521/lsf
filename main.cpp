#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <cassert>
#include "lexer.h"
#include "jsonparser.h"
#include "kmp.h"
using namespace std;

std::string to_cstring(const std::wstring & s)
{
    char cc[MB_CUR_MAX];
    std::string r{};
    for(const auto c:s){
        auto l=wctomb(cc,c);
        assert(l>=0);
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
    auto f3="3.txt";
    auto old=std::setlocale(LC_ALL,nullptr);
    std::setlocale(LC_ALL,std::locale("").name().c_str());
    auto buff=std::make_shared<lsf::FilterBuff>(std::make_unique<lsf::MBuff>(f2));
    lsf::Lexer lex(buff);
    lsf::JsonParser parser({[&lex]()->void
                            {
                                lex.next_token();
                            },
                            [&lex]()->const lsf::Token &
                            {
                             return lex.get_token();}
                           });
    try {
        parser.parser();
    }  catch (const lsf::LexerError & e) {
        std::cout<<e.what();
        std::cout<<to_cstring(lex.get_error(buff->get_stat()));
        throw ;
    }   catch(const std::runtime_error e){
        std::cout<<e.what();
        throw ;
    }

    std::cout<<"合法json"<<endl;
    std::setlocale(LC_ALL,old);
    return 0;
}

