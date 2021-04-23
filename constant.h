#ifndef CONSTANT_H
#define CONSTANT_H
#include<stdexcept>
namespace lsf {

enum class Type{
    String,
    Number,
    KeyWord,
    Comment,
    LBRACE, RBRACE,     //{,}
    LSQUARE, RSQUARE,   //[,]
//    LPAREN, RPAREN,     //(,)
    COLON,               //:
    COMMA,              //,
    WHITESPACE,         //
    END
};

inline const char * tokentype_to_string(Type type)
{
    switch (type) {
    case Type::String:
        return "<String>";
    case Type::Number:
        return "<Number>";
    case Type::KeyWord:
        return "<KeyWord>";
    case Type::Comment:
        return "<Comment>";
    case Type::LBRACE:
        return "<LBRACE>";
    case Type::RBRACE:
        return "<RBRACE>";
    case Type::LSQUARE:
        return "<LSQUARE>";
    case Type::RSQUARE:
        return "<RSQUARE>";
    case Type::COLON:
        return "<COLON>";
    case Type::COMMA:
        return "<COMMA>";
    case Type::WHITESPACE:
        return "<WHITESPACE>";
    case Type::END:
        return "<END>";
    }
}

class BaseError : public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

}
#endif // CONSTANT_H
