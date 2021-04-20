#ifndef CONSTANT_H
#define CONSTANT_H
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

}
#endif // CONSTANT_H
