#ifndef ERROR_H
#define ERROR_H
#include<string>
#include"constant.h"
#include"jsonparser.h"
#include"lexer.h"

namespace lsf {
//namespace start
struct Statistic;
std::string parser_messages(Statistic stat_for_rc,Token lex_token,std::vector<lsf::Type> expects);
std::string lexer_messages(Statistic stat_for_rc,Token lex_token);
std::string to_cstring(const std::wstring & s);

//namespace end
}
#endif // ERROR_H
