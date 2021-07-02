#include<iostream>
#include"doctest/doctest.h"

#include"jsonparser.h"
#include"inner_imp.h"
#include"analyse.h"
#include"lexer.h"
#include"json.h"

TEST_CASE("test jsonparser")
{
    //输入抽象
    auto buff_=std::make_shared<lsf::FilterBuff>(std::make_unique<lsf::MBuff>("2.txt"));
    buff_->test_and_skipBOM();

    //创建词法分析器
    auto lexer_=std::make_shared<lsf::Lexer>(buff_);

    ///************************
    auto wrap_lexer_=std::make_shared<lsf::FunnyTokenGen>(lexer_,buff_);

    //创建语法分析器
    auto parser_=std::make_unique<lsf::R_JsonParser>(wrap_lexer_);

    //节点构建器
    auto builder=std::make_shared<lsf::Treebuilder>();

    parser_->set_builder(builder);

    auto old=std::setlocale(LC_ALL,nullptr);
    std::setlocale(LC_ALL,std::locale("").name().c_str());

    auto ok=parser_->parser();
    std::string error_msg;
    if(!ok){
        error_msg+=lsf::parser_messages(wrap_lexer_->token_position(),lexer_->get_token(),parser_->get_expect_token());
        std::cout<<error_msg<<std::endl;
    }
    CHECK_EQ(ok,true);

#ifdef MSVC_SPECIAL
        old = "C";
#endif // MSVC_SPECIAL
    std::setlocale(LC_ALL, old);

}


//TEST_CASE("test jsonparser")
//{
//    lsf::Json j("22.txt");
//    auto ok=j.run([&](auto t,const std::string &s){
//        lsf::ErrorType sd=t;
//        std::cout<<s<<std::endl;
//    });
//    CHECK(ok==true);
//}
