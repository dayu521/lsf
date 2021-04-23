#ifndef ERROR_H
#define ERROR_H
#include<string>
#include"kmp.h"
#include"constant.h"
#include"jsonparser.h"
#include"lexer.h"

namespace lsf {
//namespace start
std::string parser_messages(Statistic stat_for_rc,Token lex_token,std::vector<lsf::Type> expects);
std::string lexer_messages(Statistic stat_for_rc,Token lex_token);
std::string to_cstring(const std::wstring & s);

class Error
{
public:
    std::string message_chars();
    std::wstring message_wchars();

private:
    Statistic s;
    Token::Type t;
};

//namespace end
}
#endif // ERROR_H
