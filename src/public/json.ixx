module;
#include <functional>
#include <memory>
#include <stack>
#include <tuple>
#include <string>

export module lsf;

import :analyze;

namespace lsf
{
    class FilterBuff;
    class Lexer;
    class FunnyTokenGen;
    class JsonParser;
    // class TreeBuilder;
    // enum class NodeC;
}

//TODO 修改接口,此接口很迷惑
export namespace lsf
{

    enum class ErrorType
    {
        LexError,
        ParserError,
        WeakTypeCheckError,
        UnknowError
    };

    class SerializeBuilder;

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

    class SerializeBuilder
    {
    public:
        SerializeBuilder() { indent.push(0); }
        virtual ~SerializeBuilder() {}
        std::string_view get_jsonstring() const { return out_; }
        void clear() { out_.clear(); }

    public:
        virtual void write_value(const std::string &ele)
        {
            out_ += ele;
        }

        virtual void write_value(const char *ele)
        {
            out_ += ele;
        }

        virtual void add_quotation()
        {
            out_ += '"';
        }

        virtual void write_key(std::string key)
        {
            out_ += '"';
            out_ += key;
            out_ += '"';
            out_ += ": ";
        }
        virtual void arr_start()
        {
            out_ += '[';
            out_ += '\n';
            indent.push(indent.top() + 1);
            auto i = indent.top();
            while (i > 0)
            {
                out_ += "    ";
                i--;
            }
        }
        virtual void arr_end()
        {
            out_ += '\n';
            auto i = indent.top();
            indent.pop();
            while (i - 1 > 0)
            {
                out_ += "    ";
                i--;
            }
            out_ += ']';
        }
        virtual void obj_start()
        {
            out_ += '{';
            out_ += '\n';
            indent.push(indent.top() + 1);
            auto i = indent.top();
            while (i > 0)
            {
                out_ += "    ";
                i--;
            }
        }
        virtual void obj_end()
        {
            out_ += '\n';
            auto i = indent.top();
            indent.pop();
            while (i - 1 > 0)
            {
                out_ += "    ";
                i--;
            }
            out_ += '}';
        }
        virtual void forward_next()
        {
            out_ += ',';
            out_ += '\n';
            auto i = indent.top();
            while (i > 0)
            {
                out_ += "    ";
                i--;
            }
        }
        virtual void back(std::size_t i = 2)
        {
            // todo:check overflow
            out_.resize(out_.size() - indent.top() * 4 - i);
        }

    protected:
        std::string out_{};
        std::stack<int> indent{};
    };

    // template <typename S>
    // void struct_to_jsonstr(const S &obj, SerializeBuilder &builder);

    // template <typename S>
    // void json_to_struct(const Json &json, S &s);

    // void json_to_string(Json &json, SerializeBuilder &sb);

} // namespace lsf

// 宏使用
export namespace lsf::detail
{

    template <typename T, typename U, typename NAMETUPLE>
    struct MI
    {
        NAMETUPLE name;
        T U::*member;
        typedef T type;
    };

    template <typename T, typename U>
    constexpr auto makeMemberInfo(const char *name, T U::*member)
        -> MI<T, U, const char *>
    {
        return {name, member};
    }

    struct instance
    {
        template <typename Type>
        operator Type() const;
    };
    // struct Hellos
    //{
    //     int a;
    //     std::string s;
    //     bool bs;
    // };
    // struct Mk:Hellos
    //{
    // };
    // int tr(Mk n);
    // excess elements in struct initializer
    // instance()转换成Hellos,所以无法初始化基类成员
    // auto xst=tr(Mk{instance(),instance()});

    template <typename Aggregate, typename IndexSequence = std::index_sequence<>,
              // 当下面偏特化中的std::void_t替换失败时，选择这个主模板
              typename = void>
    struct arity_impl : IndexSequence
    {
    };

    template <typename Aggregate, std::size_t... Indices>
    struct arity_impl<Aggregate, std::index_sequence<Indices...>,
                      // 偏特化与主模板之间的SFINAE
                      std::void_t<
                          decltype(Aggregate{
                              // 丢弃前一个表达式的值，然后返回后一个表达式的值
                              (static_cast<void>(Indices), std::declval<instance>())...,
                              std::declval<instance>()})>>
        : arity_impl<Aggregate,
                     // 当前参数包与当前参数包的个数,它们都是std::size_t类型
                     std::index_sequence<Indices..., sizeof...(Indices)>>
    {
    };

    template <typename T>
    constexpr std::size_t arity()
    {
        return detail::arity_impl<std::decay_t<T>>().size();
    }

    // namespace detail end
}