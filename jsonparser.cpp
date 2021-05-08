#include <cassert>
#include "jsonparser.h"
#include "lexer.h"
#include "analyse.h"

namespace lsf {

JsonParser::JsonParser(std::unique_ptr<GenToken> gen):gen_(std::move(gen)),
    expect_array_{}
{

}

void JsonParser::set_builder(std::shared_ptr<Treebuilder> b)
{
    builder_=b;
}

bool JsonParser::parser()
{
    builder_->start_build();
    expect_array_.clear();//之后push_back或assign都一样
    gen_->next_();
    return json();
}

const std::vector<Type> &JsonParser::get_expect_token()const
{
    return expect_array_;
}

//  json -> element
//  json.node=element.node
bool JsonParser::json()
{
    if (element()) {
        if (isTerminator(TType::END)) {
            builder_->finish_build();
            return true;
        }
        expect_array_.push_back({ TType::END });
    }
    return false;
}

//element -> unuse value unuse
//element.node=value.node
bool JsonParser::element()
{
    unuse();
    if(value())
        return unuse();
    return false;
}

//value -> obj | array | String | Number | KeyWord
//value.node=obj.node,array.node
//value.node=new Jnode<string,number,keyword>;
bool JsonParser::value()
{
    switch (gen_->current_().type_) {
    case Type::LBRACE:
        return obj();
    case Type::LSQUARE:
        return array();
    case Type::String:{
        builder_->build_string(gen_->current_().value_);
        gen_->next_();
        return true;
    }
    case Type::Number:{
        builder_->build_number(gen_->current_().value_);
        gen_->next_();
        return true;
    }
    case Type::KeyWord:{
        builder_->build_keyword(gen_->current_().value_);
        gen_->next_();
        return true;
    }
    default:
        expect_array_.assign({Type::String,Type::Number,Type::KeyWord});
        return false;
    }
}

//obj -> '{' mb_ws '}'
//obj.node=new Jnode<obj>
//obj.node.left=mb_ws.node
//obj.node.right=obj.node
bool JsonParser::obj()
{
    if(isTerminator(TType::LBRACE)){
        gen_->next_();
        if(mb_ws()){
            if(isTerminator(TType::RBRACE)){
                gen_->next_();
                builder_->build_obj();
                return true;
            }
            return false;
        }
        return false;
    }
    expect_array_.push_back(TType::LBRACE);
    return false;
}

//mb_ws -> unuse mb_ws_r
//mb_ws.node=mb_ws_r.node
bool JsonParser::mb_ws()
{
    unuse();
    return mb_ws_r();
}

//mb_ws_r -> String unuse ':' element memberL | e
//element.node.key=string
//memberL.node=element.node+memberL.node
//mb_ws_r.node=memberL.node
bool JsonParser::mb_ws_r()
{
    if(isTerminator(TType::String)){
        auto key=std::move(gen_->current_().value_);
        gen_->next_();
        unuse();
        if(isTerminator(TType::COLON)){
            gen_->next_();
            if(element()){
                builder_->set_memberkey(key);
                return memberL();
            }
            else
                return false;
        }
        expect_array_.push_back(TType::COLON);
        return false;
    }else if (isTerminator(TType::RBRACE)) {
//        root_ = null_;
        builder_->build_null_mbr();
        return true;
    }else{
         expect_array_.push_back(TType::String);
         expect_array_.push_back(TType::RBRACE);
        return false;
    }
}

//memberL -> ',' member memberL | e
//memberL.node=member.node+memberL.node
bool JsonParser::memberL()
{
    if(isTerminator(TType::RBRACE)){
        return true;
    }
    builder_->start_iteration();
    while (isTerminator(TType::COMMA)) {
        gen_->next_();
        if(member()){
            builder_->move_next();
            if(isTerminator(TType::RBRACE)){
                builder_->finish_iteration();
                return true;
            }
            continue;
        }
        return false;
    }
    expect_array_.push_back(TType::RBRACE);
    expect_array_.push_back(TType::COMMA);
    return false;
}

//member -> unuse String unuse ':' element
//member.node=element.node
//member.node.key=String
bool JsonParser::member()
{
    unuse();
    if(isTerminator(TType::String)){
        auto key=std::move(gen_->current_().value_);
        gen_->next_();
        unuse();
        if(isTerminator(TType::COLON)){
            gen_->next_();
            if(element()){
                builder_->set_memberkey(key);
                return true;
            }
            return false;
        }
        expect_array_.push_back(TType::COLON);
        return false;
    }
    expect_array_.push_back(TType::String);
    return false;
}

//array -> '[' arr_ws ']'
//array.node=new Jnode<arr>
//array.node.right=array.node
//array.node.left=arr_ws.node
bool JsonParser::array()
{
    if(isTerminator(TType::LSQUARE)){
        gen_->next_();
        if(arr_ws()){
            if(isTerminator(TType::RSQUARE)){
                gen_->next_();
                builder_->build_arr();
                return true;
            }
            expect_array_.push_back(TType::RSQUARE);
            return false;
        }
        return false;
    }
    expect_array_.push_back(TType::LSQUARE);
    return false;
}

//arr_ws -> unuse arr_ws_r
//arr_ws.node=arr_ws_r.node
bool JsonParser::arr_ws()
{
    unuse();
    return arr_ws_r();
}

//arr_ws_r -> value unuse elementsL | e
//arr_ws_r.node=value.node+elementsL.node
bool JsonParser::arr_ws_r()
{
    switch (gen_->current_().type_) {
    case TType::LBRACE:
    case TType::LSQUARE:
    case TType::String:
    case TType::Number:
    case TType::KeyWord:{
        if(value()){
            unuse();
            return elementsL();
        }
        return false;
    }
    case TType::RSQUARE:
//        root_ = null_;
        builder_->build_null_mbr();
        return true;
    default:
        expect_array_.assign({TType::RSQUARE});
        return false;
    }
}

//elementsL -> ',' element elementsL | e
//elementsL.node=element.node+elementsL.node
bool JsonParser::elementsL()
{
    if(isTerminator(Type::RSQUARE))
        return true;
    builder_->start_iteration();
    while(isTerminator(Type::COMMA)){
        gen_->next_();
        if(element()){
            builder_->move_next();
            if(isTerminator(Type::RSQUARE)){
                builder_->finish_iteration();
                return true;
            }
            continue;
        }
        return false;
    }
    expect_array_.push_back(Type::RSQUARE);
    expect_array_.push_back(Type::COMMA);
    return false;
}

//unuse -> wc unuse | e
//wc -> WhiteSpace | Comment
bool JsonParser::unuse()
{
    while (isTerminator(TType::WHITESPACE)||isTerminator(TType::Comment)) {
        gen_->next_();
    }
    return true;
}

bool JsonParser::isTerminator(JsonParser::TType type)
{
    return gen_->current_().type_==type;
}

}
