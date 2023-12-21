module;
#include <functional>
#include <memory>
#include <stack>
#include <tuple>
#include <string>

export module lsf;

export import :util;
export import :struct_help;

namespace lsf
{
    class FilterBuff;
    class Lexer;
    class FunnyTokenGen;
    class JsonParser;
    class TreeBuilder;
    // enum class NodeC;
}

// TODO 修改接口,此接口很迷惑

// 流,解析器,
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
        Json(const std::string &filename);
        Json(const Json &) = delete;
        Json(Json &&) = default;
        // https://en.cppreference.com/w/cpp/memory/unique_ptr
        ~Json();
        /// 回调函数f,仅仅是提醒错误需要处理,没有其他想法
        [[nodiscard]] bool run(std::function<void(ErrorType et, const std::string &message)> f);
        /// 回调函数f,仅仅是提醒错误需要处理,没有其他想法
        [[nodiscard]] bool weak_type_check(std::function<void(ErrorType et, const std::string &message)> f);

        std::string get_errors() const;

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