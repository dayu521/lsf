#ifndef ERROR_H
#define ERROR_H
#include<string>
#include"constant.h"
#include"jsonparser.h"
#include"lexer.h"

namespace lsf {
//namespace start
struct Location;
std::string parser_messages(Location stat_for_rc,Token lex_token,std::vector<lsf::Type> expects);
std::string lexer_messages(Location stat_for_rc,Token lex_token);

namespace detail {
template<std::size_t >
struct AnyType{};
}

template<std::size_t N=64>
std::string to_cstring(const std::wstring &s,detail::AnyType<N> none={})
{
    std::string::value_type cc[N];
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

//namespace end
}
#endif // ERROR_H
