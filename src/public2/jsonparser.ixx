module;
#include <memory>
#include <vector>
#include <string>

export module lsf:jsonparser;

import :constant;
import :tree_builder;
import :tk_generator;

namespace lsf
{
    class JsonParser
    {
    public:
        JsonParser(std::shared_ptr<GenToken> gen);
        [[nodiscard]] bool parser(std::shared_ptr<ParserResultBuilder>);
        const std::vector<lsf::Type> &get_expect_token() const;

    private:
        using TType = lsf::Type;
        bool json();
        bool element();
        bool value();
        bool obj();
        bool mb_ws();
        bool mb_ws_r();
        bool memberL();
        bool member();
        bool array();
        bool arr_ws();
        bool arr_ws_r();
        bool elementsL();
        bool unuse();

        bool isTerminator(TType type);

    private:
        std::shared_ptr<GenToken> gen_;
        std::shared_ptr<ParserResultBuilder> builder_;
        std::vector<lsf::Type> expect_array_;
    };

    // 把递归下降转换成循环
    class R_JsonParser
    {
    public:
        R_JsonParser(std::shared_ptr<GenToken> gen);
        void set_builder(std::shared_ptr<ParserResultBuilder> b);
        [[nodiscard]] bool parser();
        const std::vector<lsf::Type> &get_expect_token() const;

    private:
        using TType = lsf::Type;
        bool json();
        /// 不建议对此函数有任何想法,因为全是复杂的令人难过的转换逻辑,
        bool value();
        bool unuse();

        bool isTerminator(TType type);

    private:
        std::shared_ptr<GenToken> gen_;
        std::shared_ptr<ParserResultBuilder> builder_;
        std::vector<lsf::Type> expect_array_;
    };

    // namespace end
}
