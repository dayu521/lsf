module;
#include <string>
#include <set>
#include <memory>

export module lsf:lexer;

import :constant;

namespace lsf
{

    class BuffBase;

    class LexerError : public lsf::BaseError
    {
        using BaseError::BaseError;
    };

    class Lexer
    {
    public:
        Lexer(std::shared_ptr<BuffBase> input);
        Token &next_token();
        Token &get_token();
        const Token &get_error();

    private:
        bool run();
        bool try_number(wchar_t c);
        bool try_comment(wchar_t c);
        [[nodiscard]] bool is_escaped(wchar_t &rc);
        [[nodiscard]] bool is_unicode_code_point(wchar_t &rc);

    private:
        std::shared_ptr<BuffBase> input_;
        std::set<std::wstring> symbol_;
        bool has_error_{false};
        Token current_token_{};
        std::wstring error_;
    };

}