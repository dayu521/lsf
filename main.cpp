#include <iostream>
#include <memory>
#include <cassert>
#include "lexer.h"
#include "jsonparser.h"
#include "mbuff.h"
#include "error.h"
#include "inner_imp.h"
#include "analyse.h"
//#include<loki/Visitor.h>
//#include<loki/AbstractFactory.h>
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
    std::shared_ptr<lsf::Lexer> lex=std::make_shared<lsf::Lexer>(buff);
    lsf::JsonParser parser(std::make_unique<lsf::Ltokens>(lex));
    auto builder=std::make_shared<lsf::Treebuilder>();
    parser.set_builder(builder);
    try {
        if(!parser.parser()){
            std::cout<<lsf::parser_messages(buff->get_stat(),lex->get_token(),parser.get_expect_token());
            return -1;
        }
    }  catch (const lsf::LexerError & e) {
        std::cout<<e.what();
        std::cout<<lsf::lexer_messages(buff->get_stat(),lex->get_token());
        return -1;
    }   catch(const std::runtime_error &e){
        std::cout<<e.what();
        return -1;
    }
    lsf::PrintNodes p;
    p.visit_BFS(builder->get_ast(),[]{std::cout<<std::endl;});
    lsf::TypeChecker typer;
    if(!typer.check_type(builder->get_ast())){
        return -1;
    }
    std::cout<<"合法json"<<endl;
    std::setlocale(LC_ALL,old);
    return 0;
}

