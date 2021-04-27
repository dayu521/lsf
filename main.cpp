#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>
#include <cassert>
#include "lexer.h"
#include "jsonparser.h"
#include "kmp.h"
#include "error.h"
#include<loki/Visitor.h>
using namespace std;

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
    auto buff=std::make_shared<lsf::FilterBuff>(std::make_unique<lsf::MBuff>(f1));
    buff->test_and_skipBOM();
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
        if(!parser.parser()){
            std::cout<<lsf::parser_messages(buff->get_stat(),lex.get_token(),parser.get_expect_token());
            return -1;
        }
    }  catch (const lsf::LexerError & e) {
        std::cout<<e.what();
        std::cout<<lsf::lexer_messages(buff->get_stat(),lex.get_token());
        return -1;
    }   catch(const std::runtime_error &e){
        std::cout<<e.what();

    }

    std::cout<<"合法json"<<endl;
    std::setlocale(LC_ALL,old);
    return 0;
}

