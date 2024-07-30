module;
#include <stdexcept>
#include <cassert>
#include <cstddef>

export module lsf:constant;

namespace lsf
{
    enum class Type
    {
        String,
        Number,
        KeyWord,
        Null,
        Comment,
        LBRACE,
        RBRACE, //{,}
        LSQUARE,
        RSQUARE,    //[,]
                    //    LPAREN, RPAREN,     //(,)
        COLON,      //:
        COMMA,      //,
        WHITESPACE, //
        END
    };

    struct Token
    {
        using Type = lsf::Type;
        Type type_;
        std::wstring value_{};
    };

    struct Location
    {
        size_t line_{0};
        size_t column_{0};
    };

    inline const char *tokentype_to_string(Type type)
    {
        switch (type)
        {
        case Type::String:
            return "<String>";
        case Type::Number:
            return "<Number>";
        case Type::KeyWord:
            return "<KeyWord>";
        case Type::Null:
            return "<Null>";
        case Type::Comment:
            return "<Comment>";
        case Type::LBRACE:
            return "<LBRACE>";
        case Type::RBRACE:
            return "<RBRACE>";
        case Type::LSQUARE:
            return "<LSQUARE>";
        case Type::RSQUARE:
            return "<RSQUARE>";
        case Type::COLON:
            return "<COLON>";
        case Type::COMMA:
            return "<COMMA>";
        case Type::WHITESPACE:
            return "<WHITESPACE>";
        case Type::END:
            return "<END>";
        default:
            return "error";
        }
    }

    class BaseError : public std::runtime_error
    {
        using std::runtime_error::runtime_error;
    };

    template <typename... T>
    struct BaseVisitor;

    template <typename R, typename T>
    struct BaseVisitor<R, T>
    {
        using Rtype = R;
        virtual R visit(T &a) = 0;
        virtual ~BaseVisitor() = default;
    };

    template <typename R, typename T, typename... Others>
    struct BaseVisitor<R, T, Others...> : BaseVisitor<R, Others...>
    {
        using BaseVisitor<R, Others...>::visit;
        virtual R visit(T &a) = 0;
        virtual ~BaseVisitor() = default;
    };

    template <typename T>
    struct TypeTag
    {
        typedef T OriginalType;
    };

    template <typename T>
    struct DoBuilder
    {
        virtual ~DoBuilder() {}
        virtual void build(TypeTag<T>) = 0;
    };

    template <typename... T>
    struct BaseBuilder;

    template <typename T>
    struct BaseBuilder<T> : DoBuilder<T>
    {
    };

    template <typename T, typename... Left>
    struct BaseBuilder<T, Left...> : DoBuilder<T>, BaseBuilder<Left...>
    {
        using DoBuilder<T>::build;
        using BaseBuilder<Left...>::build;
    };

    template <typename... T>
    struct OpneBuilder : BaseBuilder<T...>
    {
        template <typename U>
        void build()
        {
            build(TypeTag<U>());
        }
    };

    template <typename... T>
    struct BaseGuard;

    template <typename R, typename T>
    struct BaseGuard<R, T>
    {
        using Rtype = R;
        virtual R find(T &a) = 0;
        virtual ~BaseGuard() = default;
    };

    template <typename R, typename T, typename... Others>
    struct BaseGuard<R, T, Others...> : BaseGuard<R, Others...>
    {
        using BaseGuard<R, Others...>::find;
        virtual R find(T &a) = 0;
        virtual ~BaseGuard() = default;
    };

    template <typename... T>
    struct BaseNest;

    template <typename R, typename T>
    struct BaseNest<R, T>
    {
        using Rtype = R;
        virtual R nest_begin(TypeTag<T>, std::size_t n) = 0;
        virtual R nest_end(TypeTag<T>) = 0;
        virtual ~BaseNest() = default;
    };

    template <typename R, typename T, typename... Others>
    struct BaseNest<R, T, Others...> : BaseNest<R, Others...>
    {
        using BaseNest<R, Others...>::nest_begin;
        using BaseNest<R, Others...>::nest_end;
        virtual R nest_begin(TypeTag<T>, std::size_t n) = 0;
        virtual R nest_end(TypeTag<T>) = 0;
        virtual ~BaseNest() = default;
    };
} // namespace lsf
