#include "jsonparser.h"
#include "lexer.h"

namespace lsf {

JsonParser::JsonParser(GenToken gen):gen_(gen)
{

}

bool JsonParser::parser()
{
    if(!gen_.next_||!gen_.current_)
        return false;
    gen_.next_();
    return json();
}

bool JsonParser::json()
{
    return element();
}

bool JsonParser::element()
{
    auto x=gen_.current_();
    switch (gen_.current_().type_) {
    case Type::WHITESPACE:
    case Type::LBRACE:
    case Type::LSQUARE:
    case Type::String:
    case Type::Number:
    case Type::KeyWord:
        ws();
        if(value())
            return ws();
        return false;
    default:
        return false;
    }
}

bool JsonParser::value()
{
    switch (gen_.current_().type_) {
    case Type::LBRACE:
        return obj();
    case Type::LSQUARE:
        return array();
    case lsf::Type::String:
    case lsf::Type::Number:
    case lsf::Type::KeyWord:
        gen_.next_();
        return true;
    default:
        return false;
    }
}

bool JsonParser::obj()
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

bool JsonParser::mb_ws()
{
    ws();
    return mb_ws_r();
}

bool JsonParser::mb_ws_r()
{
    if(isTerminator(TType::String)){
        gen_.next_();
        ws();
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
    return false;
}

bool JsonParser::member()
{
    if(isTerminator(TType::WHITESPACE)){
        ws();
        if(isTerminator(TType::String)){
            gen_.next_();
            ws();
            if(isTerminator(TType::COLON)){
                gen_.next_();
                return element();
            }
        }
    }
    return false;
}

bool JsonParser::array()
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

bool JsonParser::arr_ws()
{
    if(isTerminator(TType::WHITESPACE)){
        ws();
        return arr_ws_r();
    }
    return false;
}

bool JsonParser::arr_ws_r()
{
    switch (gen_.current_().type_) {
    case TType::LBRACE:
    case TType::LSQUARE:
    case TType::String:
    case TType::Number:
    case TType::KeyWord:{
        if(value()){
            ws();
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

bool JsonParser::elementsL()
{
    if(isTerminator(Type::COMMA)){
        gen_.next_();
        if(element())
            return elementsL();
        return false;
    }else if(isTerminator(Type::RSQUARE))
        return true;
    else
        return false;
}

bool JsonParser::ws()
{
    if(isTerminator(TType::WHITESPACE)){
        gen_.next_();
    }
    return true;
}

bool JsonParser::isTerminator(JsonParser::TType type)
{
    return gen_.current_().type_==type;
}

}
