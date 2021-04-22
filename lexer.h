#ifndef LEXER_H
#define LEXER_H


//Code point <-> UTF-8 conversion
//First code point	Last code point	Byte 1      Byte 2      Byte 3      Byte 4
//U+0000            U+007F          0xxxxxxx
//U+0080            U+07FF          110xxxxx	10xxxxxx
//U+0800            U+FFFF          1110xxxx	10xxxxxx	10xxxxxx
//U+10000           U+10FFFF        11110xxx	10xxxxxx	10xxxxxx	10xxxxxx
//Byte 1,Byte 2,Byte 3,Byte 4依次从高位到低位
//
//wchar_t(0x4F60) L'\x4F60'
//char s[]="\xe4\xbd\xa0";
//wchar_t w;
//mbtowc(&w,s,3);
//宽字符与utf8编码 ，他们都是"你"的utf8编码
//https://en.wikipedia.org/wiki/Universal_Character_Set_characters#Surrogates
#include <string>
#include <set>
#include <memory>
#include "constant.h"
namespace lsf {

class BuffBase;
struct Statistic;

struct Token
{
    Type type_;
    std::wstring value_{};
    bool operator ==(const Token & c)const
    {
        return type_==c.type_;
    }
};

class LexerError:public std::runtime_error
{
    using std::runtime_error::runtime_error;
};

class Lexer
{
public:
    Lexer(std::shared_ptr<BuffBase> input);
    Token & next_token();
    Token & get_token();
    const std::wstring & get_error();
    const std::wstring & get_error(Statistic stat);
private:
    bool run();
    bool try_number(wchar_t c);
    bool try_comment(wchar_t c);
private:
    std::shared_ptr<BuffBase> input_;
    std::set<std::wstring> symbol_;
    bool has_error_{false};
    Token current_token_{};
    std::wstring error_{};
};

}
#endif // LEXER_H
