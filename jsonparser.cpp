#include <cassert>
#include "jsonparser.h"
#include "lexer.h"

namespace lsf {

JsonParser::JsonParser(GenToken gen):gen_(gen),
    expect_array_{}
{

}

bool JsonParser::parser()
{
    assert(gen_.next_||gen_.current_);
    expect_array_.clear();//之后push_back或assign都一样
    gen_.next_();
    return json();
}

const std::vector<Type> &JsonParser::get_expect_token()const
{
    return expect_array_;
}

//  json -> element
bool JsonParser::json()
{
    if(element())
        return isTerminator(TType::END);
    return false;
}

//element -> unuse value unuse
bool JsonParser::element()
{
    unuse();
    if(value())
        return unuse();
    return false;
}

//value -> obj | array | String | Number | KeyWord
bool JsonParser::value()
{
    switch (gen_.current_().type_) {
    case Type::LBRACE:
        return obj();
    case Type::LSQUARE:
        return array();
    case Type::String:
    case Type::Number:
    case Type::KeyWord:
        gen_.next_();
        return true;
    default:
        expect_array_.assign({Type::String,Type::Number,Type::KeyWord});
        return false;
    }
}

//obj -> '{' mb_ws '}'
bool JsonParser::obj()
{
    if(isTerminator(TType::LBRACE)){
        gen_.next_();
        if(mb_ws()){
            if(isTerminator(TType::RBRACE)){
                gen_.next_();
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
bool JsonParser::mb_ws()
{
    unuse();
    return mb_ws_r();
}

//mb_ws_r -> String unuse ':' element memberL | e
bool JsonParser::mb_ws_r()
{
    if(isTerminator(TType::String)){
        gen_.next_();
        unuse();
        if(isTerminator(TType::COLON)){
            gen_.next_();
            if(element())
                return memberL();
            else
                return false;
        }
        expect_array_.push_back(TType::COLON);
        return false;
    }else if (isTerminator(TType::RBRACE)) {
        return true;
    }else{
         expect_array_.push_back(TType::String);
         expect_array_.push_back(TType::RBRACE);
        return false;
    }
}

//memberL -> ',' member memberL | e
bool JsonParser::memberL()
{
    if(isTerminator(TType::RBRACE)){
        return true;
    }
    while (isTerminator(TType::COMMA)) {
        gen_.next_();
        if(member()){
            if(isTerminator(TType::RBRACE))
                return true;
            continue;
        }
        return false;
    }
    expect_array_.push_back(TType::RBRACE);
    expect_array_.push_back(TType::COMMA);
    return false;
}

//member -> unuse String unuse ':' element
bool JsonParser::member()
{
    unuse();
    if(isTerminator(TType::String)){
        gen_.next_();
        unuse();
        if(isTerminator(TType::COLON)){
            gen_.next_();
            return element();
        }
        expect_array_.push_back(TType::COLON);
        return false;
    }
    expect_array_.push_back(TType::String);
    return false;
}

//array -> '[' arr_ws ']'
bool JsonParser::array()
{
    if(isTerminator(TType::LSQUARE)){
        gen_.next_();
        if(arr_ws()){
            if(isTerminator(TType::RSQUARE)){
                gen_.next_();
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

//arr_ws -> unuse | arr_ws_r
bool JsonParser::arr_ws()
{
    unuse();
    return arr_ws_r();
}

//arr_ws_r -> value unuse elementsL | e
bool JsonParser::arr_ws_r()
{
    switch (gen_.current_().type_) {
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
        return true;
    default:
        expect_array_.assign({TType::RSQUARE});
        return false;
    }
}

//elementsL -> ',' element elementsL | e
bool JsonParser::elementsL()
{
    if(isTerminator(Type::RSQUARE))
        return true;
    while(isTerminator(Type::COMMA)){
        gen_.next_();
        if(element()){
            if(isTerminator(Type::RSQUARE))
                return true;
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
        gen_.next_();
    }
    return true;
}

bool JsonParser::isTerminator(JsonParser::TType type)
{
    return gen_.current_().type_==type;
}

}
