import lsf;

/*宏定义开始*/
#define BRACKET_L() (
#define BRACKET_R() )

#define EVAL(...) __VA_ARGS__

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define JS_INTERNAL_BOOL(x) JS_INTERNAL_NOT(JS_INTERNAL_NOT(x))

#define JS_INTERNAL_IF_ELSE(condition) JS_INTERNAL__IF_ELSE(JS_INTERNAL_BOOL(condition))
#define JS_INTERNAL__IF_ELSE(condition) JS_INTERNAL_CAT(JS_INTERNAL__IF_, condition)

#define JS_INTERNAL__IF_1(...) __VA_ARGS__ JS_INTERNAL__IF_1_ELSE
#define JS_INTERNAL__IF_0(...) JS_INTERNAL__IF_0_ELSE

#define JS_INTERNAL__IF_1_ELSE(...)
#define JS_INTERNAL__IF_0_ELSE(...) __VA_ARGS__

#define GET_N(...) IN_PARAMETER_N(__VA_ARGS__, FOR_EACH_RSEQ_N())
/*#define IN_PARAMETER_N(...)  PARAMETER_N( __VA_ARGS__ )
    因为msvc会在转发可变参数时"优化"，于是PARAMETER_N不会展开__VA_ARGS__
    https://docs.microsoft.com/en-us/cpp/preprocessor/preprocessor-experimental-overview?view=msvc-160
*/
#define IN_PARAMETER_N(...) EVAL( PARAMETER_N( __VA_ARGS__ ) )
#define PARAMETER_N(_01, _02, _03, _04, _05, _06, _07, _08, _09, _10, _11, _12, _13, _14, _15, _16, N, ...) N
#define FOR_EACH_RSEQ_N() 16, 15, 14, 13, 12, 11, 10, 09, 08, 07, 06, 05, 04, 03, 02, 01, 00

#define JS_OBJECT_INTERNAL_IMPL(member_list)                                                               \
  template <typename JS_OBJECT_T>                                                                                      \
  struct JsonStructBase                                                                                                \
  {                                                                                                                    \
    using TT = decltype(member_list);                                                                                  \
    static inline constexpr const TT js_static_meta_data_info()                                                        \
    {                                                                                                                  \
      return member_list;                                                                                              \
    }                                                                                                                  \
  }

#define N_01(member,...) lsf::detail::makeMemberInfo(#member, &JS_OBJECT_T::member)
#define N_02(member,name1,...) lsf::detail::makeMemberInfo(name1, &JS_OBJECT_T::member)
#define N_03(member,name1,...)/*暂时不实现*/

/*#define JS_MEMBER(member,...) lsf::detail::makeMemberInfo(name, &JS_OBJECT_T::member)*/
#define JS_MEMBER(...) CAT(N_,GET_N(__VA_ARGS__))(__VA_ARGS__)

#define JS_OBJECT(...) JS_OBJECT_INTERNAL_IMPL(std::make_tuple(__VA_ARGS__))
/*宏定义结束*/