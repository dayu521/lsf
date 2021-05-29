#include "error.h"
#include <sstream>
#include <cassert>
#include "inner_imp.h"
namespace lsf{

std::string parser_messages(Statistic stat_for_rc, Token lex_token, std::vector<Type> expects)
{
    std::stringstream s{};
    stat_for_rc.column_last_-=lex_token.value_.length();
    s<<"当前语法期待以下词法单元:\n";
    for(const auto & i:expects)
        s<<"  "<<tokentype_to_string(i)<<'\n';
    s<<lexer_messages(stat_for_rc,lex_token)<<'\n';
    return s.str();
}

std::string lexer_messages(Statistic stat_for_rc, Token lex_token)
{
    std::stringstream s{};
    s<<"当前词法单元:\n"
        <<"  "<<lsf::tokentype_to_string(lex_token.type_)<<":"<<to_cstring(lex_token.value_)
    <<"\n位于:"<<stat_for_rc.line_<<"行,"<<stat_for_rc.column_last_<<"列";
    return s.str();
}

std::string to_cstring(const std::wstring &s)
{

    char cc[64];
    std::string r{};
    auto cstr = s.c_str();
    std::mbstate_t state{};
    auto n =wcsrtombs(cc,&cstr,std::extent_v<decltype(cc)>,&state);

    while (n!= static_cast<std::size_t>(-1)) {
        r.append(cc, n);
        if(cstr==nullptr)
            break;
        n =wcsrtombs(cc,&cstr,std::extent_v<decltype(cc)>,&state);
    }

    return r;
}


}
