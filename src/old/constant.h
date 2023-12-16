#ifndef CONSTANT_H
#define CONSTANT_H
#include<stdexcept>
namespace lsf {

enum class Type{
    String,
    Number,
    KeyWord,
    Null,
    Comment,
    LBRACE, RBRACE,     //{,}
    LSQUARE, RSQUARE,   //[,]
//    LPAREN, RPAREN,     //(,)
    COLON,               //:
    COMMA,              //,
    WHITESPACE,         //
    END
};

struct Location
{
    size_t line_{0};
    size_t column_{0};
};

inline const char * tokentype_to_string(Type type)
{
    switch (type) {
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

template<typename ...T>
struct BaseVisitor;

template<typename R,typename T>
struct BaseVisitor<R,T>
{
    using Rtype=R;
    virtual R visit(T & a)=0;
};

template<typename R,typename T,typename ...Others>
struct BaseVisitor<R,T,Others...> : BaseVisitor<R,Others...>
{
    using BaseVisitor<R,Others...>::visit;
    virtual R visit(T & a)=0;
};

template <typename T>
struct TypeTag
{
    typedef T OriginalType;
};

template<typename T>
struct DoBuilder
{
    virtual ~DoBuilder(){}
    virtual void build(TypeTag<T>)=0;
};

template <typename ...T>
struct BaseBuilder;

template<typename T>
struct BaseBuilder<T>: DoBuilder<T>
{

};

template<typename T,typename ...Left>
struct BaseBuilder<T,Left...>: DoBuilder<T>, BaseBuilder<Left...>
{
    using DoBuilder<T>::build;
    using BaseBuilder<Left...>::build;
};

template<typename ...T>
struct OpneBuilder:BaseBuilder<T...>
{
    template<typename U>
    void build()
    {
        build(TypeTag<U>());
    }
};

}
#endif // CONSTANT_H
