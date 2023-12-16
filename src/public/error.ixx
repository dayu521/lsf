module;
#include <string>
#include <cassert>
#include <vector>

export module lsf:error;
namespace lsf
{
    // namespace start
    struct Location;
    struct Token;
    enum class Type;
    std::string parser_messages(Location stat_for_rc, Token lex_token, std::vector<lsf::Type> expects);
    std::string lexer_messages(Location stat_for_rc, Token lex_token);

    template <std::size_t N = 64>
    std::string to_cstring(const std::wstring &s)
    {
        std::string::value_type cc[N];
        std::string r{};
        auto cstr = s.c_str();
        std::mbstate_t state{};
        auto n = wcsrtombs(cc, &cstr, std::extent_v<decltype(cc)>, &state);

        while (n != static_cast<std::size_t>(-1))
        {
            r.append(cc, n);
            if (cstr == nullptr)
                return r;
            n = wcsrtombs(cc, &cstr, std::extent_v<decltype(cc)>, &state);
        }
        // 转换错误
        // https://en.cppreference.com/w/cpp/string/multibyte/wcsrtombs
        assert(false);
        return r;
    }

    // namespace end
}