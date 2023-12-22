module;
#include <functional>
#include <memory>
#include <stack>
#include <tuple>
#include <string>

export module lsf;

export import :util;
export import :struct_help;

export import :json_src;

import :analyze;
import :lexer;
import :jsonparser;
import :inner_imp;
import :tp;

// namespace lsf
// {
//     class FilterBuff;
//     class Lexer;
//     class FunnyTokenGen;
//     class JsonParser;
//     class TreeBuilder;
//     // enum class NodeC;
// }

// TODO 修改接口,此接口很迷惑

// 流,解析器,解析树,json字符串构建器,struct_to_jsonstr,解析树到struct,解析树到jsonstr
export namespace lsf
{

    enum class ErrorType
    {
        LexError,
        ParserError,
        WeakTypeCheckError,
        UnknowError
    };

    // class Json2
    // {
    // public:
    //     ~Json2() = default;

    // public:
    //     virtual bool run();
    //     virtual std::tuple<ErrorType,std::string> get_error() const;
    //     virtual std::tuple<ErrorType,std::string> get_error() const;
    // }

    class Json
    {
    public:
        Json();
        Json(const Json &) = delete;
        Json(Json &&) = default;
        // https://en.cppreference.com/w/cpp/memory/unique_ptr
        ~Json();
        /// 回调函数f,仅仅是提醒错误需要处理,没有其他想法
        [[nodiscard]] bool run(std::function<void(ErrorType et, const std::string &message)> f);
        /// 回调函数f,仅仅是提醒错误需要处理,没有其他想法
        [[nodiscard]] bool weak_type_check(std::function<void(ErrorType et, const std::string &message)> f);

        std::string get_errors() const;

        template <InputSource T>
        void set_input(std::unique_ptr<T> input);

    public:
        friend void json_to_string(Json &json, SerializeBuilder &sb);

        template <typename S>
        friend void json_to_struct(const Json &json, S &s);

    private:
        std::shared_ptr<FilterBuff> buff_;
        std::shared_ptr<Lexer> lexer_;
        std::shared_ptr<FunnyTokenGen> wrap_lexer_;
        std::unique_ptr<JsonParser> parser_;
        std::shared_ptr<TreeBuilder> builder;
        std::string error_msg_;
    };

    template <InputSource T>
    void Json::set_input(std::unique_ptr<T> input)
    {
        input->before_read();
        // 输入抽象
        buff_->set_buff_base(std::make_unique<MBuff<T>>( std::move(input)));
    }

    // template <typename S>
    // void struct_to_jsonstr(const S &obj, SerializeBuilder &builder);

    // template <typename S>
    // void json_to_struct(const Json &json, S &s);

    void json_to_string(Json &json, SerializeBuilder &sb);

    template <typename S>
    void struct_to_jsonstr(const S &obj, SerializeBuilder &builder)
    {
        serialize(obj, builder);
    }

    template <typename S>
    void json_to_struct(const Json &json, S &s)
    {
        deserialize(s, std::get<0>(json.builder->get_ast()));
    }

} // namespace lsf