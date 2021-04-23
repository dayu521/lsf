#include "error.h"
#include <sstream>
#include <cassert>
namespace lsf{

std::string parser_messages(Statistic stat_for_rc, Token lex_token, std::vector<Type> expects)
{
    std::stringstream s{};
    stat_for_rc.column_last_-=lex_token.value_.length();
    s<<"当前语法期待:\n";
    for(const auto & i:expects)
        s<<tokentype_to_string(i)<<'\n';
    s<<lexer_messages(stat_for_rc,lex_token);
    return s.str();
}

std::string lexer_messages(Statistic stat_for_rc, Token lex_token)
{
    std::stringstream s{};
    s<<"当前词法:"
        <<lsf::tokentype_to_string(lex_token.type_)<<","<<to_cstring(lex_token.value_)
    <<"位于:"<<stat_for_rc.line_<<"行,"<<stat_for_rc.column_last_<<"列";
    return s.str();
}

std::string to_cstring(const std::wstring &s)
{
    char cc[MB_CUR_MAX];
    std::string r{};
    for(const auto c:s){
        auto l=wctomb(cc,c);
        assert(l>0);
        r.append(cc,l);
    }
    return r;
}


}
