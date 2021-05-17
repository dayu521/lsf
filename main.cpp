#include <iostream>
#include <memory>
#include <cassert>
#include "lexer.h"
#include "jsonparser.h"
#include "mbuff.h"
#include "error.h"
#include "inner_imp.h"
#include "analyse.h"
#include "json.h"
//#include<loki/Visitor.h>
//#include<loki/AbstractFactory.h>
using namespace std;
struct Pe
{
    std::string name;
    int age;
//    double ss;
};
struct Hellos
{
    int a;
    std::string s;
    bool bs;
    Pe * p;
};

constexpr auto sie=lsf::detail::arity<Hellos>();

int main()
{
//    setlocale(LC_ALL,"zh_CN.UTF-8");
    //std::locale("").name().c_str()

    //设置全局c++环境,所有之后std::locale()的实例都是此locale的副本，
    //同时设置本地c环境为用户偏好的locale，默认c环境的name好像是"C"
//    std::locale::global(std::locale(""));
    auto old=std::setlocale(LC_ALL,nullptr);
    std::setlocale(LC_ALL,std::locale("").name().c_str());

    //输入抽象
    auto f1="2.txt";
    auto buff=std::make_shared<lsf::FilterBuff>(std::make_unique<lsf::MBuff>(f1));
    buff->test_and_skipBOM();

    //创建词法分析器
    auto lex=std::make_shared<lsf::Lexer>(buff);

    //创建语法分析器
    lsf::JsonParser parser(std::make_unique<lsf::Ltokens>(lex));

    auto builder=std::make_shared<lsf::Treebuilder>();

    //设置节点构建器
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

    //广度优先输出
    lsf::PrintNodes p;
    p.set_null(std::get<1>(builder->get_ast()));
    p.visit_BFS(builder->get_ast(),[]{std::cout<<std::endl;});

    //类型检查
    lsf::TypeChecker typer;
    if(!typer.check_type(builder->get_ast())){
        return -1;
    }

    std::cout<<"合法json"<<endl;

    Hellos lf;
    lf.p=new Pe;
    lsf::deserialize(lf,std::get<0>(builder->get_ast()));

    std::setlocale(LC_ALL,old);
    return 0;
}

