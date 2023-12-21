module;
#include <utility>

export module lsf:struct_help;

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