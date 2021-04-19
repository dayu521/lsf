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
    if(gen_.current_().type_==lsf::Type::WHITESPACE)
        return element();
    else if(gen_.current_().type_==lsf::Type::END)
        return true;
    else
        return false;
}

bool JsonParser::element()
{
    switch (gen_.current_().type_) {
    case Type::WHITESPACE:
    case Type::LBRACE:
    case Type::LSQUARE:
    case Type::String:
    case Type::Number:
    case Type::KeyWord:
        if(ws())
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
        return true;
    case lsf::Type::Number:
        return true;
    case lsf::Type::KeyWord:
        return true;
    default:
        return false;
    }
}

bool JsonParser::obj()
{
    if(gen_.current_().type_==lsf::Type::LBRACE){
        gen_.next_();
        return mb_ws();
    }else
        return false;
}

bool JsonParser::mb_ws()
{

}

bool JsonParser::elements()
{
    switch (gen_.current_().type_) {
    case Type::WHITESPACE:
    case Type::LBRACE:
    case Type::LSQUARE:
    case Type::String:
    case Type::Number:
    case Type::KeyWord:
        if(element())
            return elementsL();
        return false;
    default:
        return false;
    }
}

bool JsonParser::elementsL()
{
    auto t=gen_.current_();
    if(t.type_==Type::COMMA){
        gen_.next_();
        return elements();
    }else if(t.type_==Type::RSQUARE)
        return true;
    else
        return false;
}

bool JsonParser::ws()
{

}

}
