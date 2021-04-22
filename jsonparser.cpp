#include <cassert>
#include "jsonparser.h"
#include "lexer.h"

namespace lsf {

JsonParser::JsonParser(GenToken gen):gen_(gen)
{

}

void JsonParser::parser()
{
    assert(gen_.next_||gen_.current_);
    gen_.next_();
    json();
}

//json -> element
void JsonParser::json()
{
    return element();
}

//element -> unuse value unuse
void JsonParser::element()
{
    unuse();
    if(value())
        return unuse();
    return false;
}

//value -> obj | array | String | Number | KeyWord
void JsonParser::value()
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
        return false;
    }
}

//obj -> '{' mb_ws '}'
void JsonParser::obj()
{
    if(isTerminator(lsf::Type::LBRACE)){
        gen_.next_();
        if(mb_ws()&&isTerminator(TType::RBRACE)){
            gen_.next_();
            return true;
        }
    }
    return false;
}

//mb_ws -> unuse mb_ws_r
void JsonParser::mb_ws()
{
    unuse();
    return mb_ws_r();
}

//mb_ws_r -> String unuse ':' element memberL | e
void JsonParser::mb_ws_r()
{
    if(isTerminator(TType::String)){
        gen_.next_();
        unuse();
        if(isTerminator(TType::COLON)){
            gen_.next_();
            if(element())
                return memberL();
        }
        return false;
    }else if (isTerminator(TType::RBRACE)) {
        return true;
    }else
        return false;
}

//memberL -> ',' member memberL | e
void JsonParser::memberL()
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
    return false;
}

//member -> unuse String unuse ':' element
void JsonParser::member()
{
    unuse();
    if(isTerminator(TType::String)){
        gen_.next_();
        unuse();
        if(isTerminator(TType::COLON)){
            gen_.next_();
            return element();
        }
    }
    return false;
}

//array -> '[' arr_ws ']'
void JsonParser::array()
{
    if(isTerminator(TType::LSQUARE)){
        gen_.next_();
        if(arr_ws()&&isTerminator(TType::RSQUARE)){
            gen_.next_();
            return true;
        }
    }
    return false;
}

//arr_ws -> unuse | arr_ws_r
void JsonParser::arr_ws()
{
    unuse();
    return arr_ws_r();
}

//arr_ws_r -> value unuse elementsL
void JsonParser::arr_ws_r()
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
        return false;
    }
}

//elementsL -> ',' element elementsL | e
void JsonParser::elementsL()
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
    return false;
}

//unuse -> wc unuse | e
//wc -> WhiteSpace | Comment
void JsonParser::unuse()
{
    while (isTerminator(TType::WHITESPACE)||isTerminator(TType::Comment)) {
        gen_.next_();
    }
}

bool JsonParser::isTerminator(JsonParser::TType type)
{
    return gen_.current_().type_==type;
}

}
