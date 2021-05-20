#ifndef JSON_H
#define JSON_H
#include"analyse.h"
#include"error.h"
#include<tuple>
#include<cassert>
#include<string>

namespace lsf {

namespace detail{

/*宏定义开始*/
#define BRACKET_L() (
#define BRACKET_R() )

#define EXPAND(...) __VA_ARGS__

//#define CAT(x, y) x ## y
//#define PRIMITIVE_CAT(x, y) CAT(x, y)
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
//#define IN_PARAMETER_N(...)  PARAMETER_N ( __VA_ARGS__ )
    /* msvc需要这样的写法*/
#define IN_PARAMETER_N(...) EXPAND( PARAMETER_N BRACKET_L() __VA_ARGS__ BRACKET_R() )
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

//#define JS_MEMBER(member,...) lsf::detail::makeMemberInfo(name, &JS_OBJECT_T::member)
#define JS_MEMBER(...) CAT(N_,GET_N(__VA_ARGS__))(__VA_ARGS__)

#define JS_OBJECT(...) JS_OBJECT_INTERNAL_IMPL(std::make_tuple(__VA_ARGS__))
/*宏定义结束*/


template <typename T, typename U, typename NAMETUPLE>
struct MI
{
  NAMETUPLE name;
  T U::*member;
  typedef T type;
};

template <typename T, typename U>
constexpr auto makeMemberInfo(const char * name, T U::*member)
  -> MI<T, U, const char *>
{
  return {name, member};
}

struct instance {
  template <typename Type>
  operator Type() const;
};
//struct Hellos
//{
//    int a;
//    std::string s;
//    bool bs;
//};
//struct Mk:Hellos
//{
//};
//int tr(Mk n);
//excess elements in struct initializer
//instance()转换成Hellos,所以无法初始化基类成员
//auto xst=tr(Mk{instance(),instance()});

template <typename Aggregate, typename IndexSequence = std::index_sequence<>,
          //当下面偏特化中的std::void_t替换失败时，选择这个主模板
          typename = void>
struct arity_impl : IndexSequence {};

template <typename Aggregate, std::size_t... Indices>
struct arity_impl<Aggregate, std::index_sequence<Indices...>,
            //偏特化与主模板之间的SFINAE
                  std::void_t<
                decltype(
                    Aggregate{
                    //丢弃前一个表达式的值，然后返回后一个表达式的值
                      (static_cast<void>(Indices), std::declval<instance>())...,
                      std::declval<instance>()
                    }
                )
                >
            >
    : arity_impl<Aggregate,
                //当前参数包与当前参数包的个数,它们都是std::size_t类型
                 std::index_sequence<Indices..., sizeof...(Indices)>> {};

template <typename T>
constexpr std::size_t arity() {
  return detail::arity_impl<std::decay_t<T>>().size();
}


template <typename T>
inline auto to_tuple(T& t);

//namespace detail end
}

/*****************************/
template<typename T>
inline void Deserialize(T & s,TreeNode * t);

template<typename T>
void deserialize(T & obj,TreeNode * t)
{
    using TT=std::decay_t<T>;
    if constexpr (std::is_pointer_v<TT>) {//if constexpr间接提供了"函数偏特化"
        static_assert (!std::is_pointer_v<std::decay_t<decltype (*t)>>,"不支持多级指针");
        assert(t != nullptr);
        deserialize(*obj, t);
    } else if constexpr (std::is_aggregate_v<TT> && !std::is_union_v<TT>){
        auto member_info=TT::template JsonStructBase<TT>::js_static_meta_data_info();
//        auto tupes=detail::to_tuple(obj);
        size_t n=0;
        auto temp=t->left_child_;
        do{
            n++;
            temp=temp->right_bro_;
        }while(temp!=t->left_child_);
//        assert(std::tuple_size<decltype (tupes)>::value==n);
        assert(std::tuple_size<decltype (member_info)>::value==n);
        TreeNode * de=new Jnode<NodeC::Obj>;
        de->right_bro_=temp;
        temp=de;
        //不用循环，因为编译前已知次数
//        std::apply([&](auto&&... args) {
//            (deserialize(args,de=de->right_bro_),...);
//        },tupes);
        auto check_key=std::apply([&](auto&&... args) {
            return ((args.name==lsf::to_cstring((de=de->right_bro_)->key_))&&...);
        },member_info);
        if(check_key)
            std::apply([&](auto&&... args) {
                (deserialize(obj.*(args.member),de=de->right_bro_),...);
            },member_info);
        delete temp;
    }else
        Deserialize(obj,t);
}

template<typename T>
void deserialize(std::vector<T> &v,TreeNode * t)
{
    auto temp=t->left_child_;
    T m{};
    v.clear();
    do{
        deserialize(m,temp);
        v.push_back(m);
        temp=temp->right_bro_;
    }while(temp!=t->left_child_);
}

template<>
inline void Deserialize<std::string>(std::string & s,TreeNode * t)
{
    //如果有错,语法分析会提前失败.下同
    if(t->ele_type_==NodeC::String){
        auto v=static_cast<Jnode<NodeC::String> *>(t)->data_;
        char cc[6];
        std::string r{};
        s.clear();
        for(const auto c:v){
            auto l=wctomb(cc,c);
            assert(l>0);
            s.append(cc,l);
        }
    }
}

template<>
inline void Deserialize<int>(int & s,TreeNode * t)
{
    if(t->ele_type_==NodeC::Number){     
        s=std::stoi(static_cast<Jnode<NodeC::Number> *>(t)->data_);
    }
}

template<>
inline void Deserialize<bool>(bool & b,TreeNode * t)
{
    if(t->ele_type_==NodeC::Keyword){
        auto v=static_cast<Jnode<NodeC::Number> *>(t)->data_;
        if(v==L"true"){
            b=true;
        }else if (v==L"false") {
            b=false;
        }
    }
}

class SerializeBuilder
{
public:
    std::string get_jsonstring()const{return out_;}
public:
    virtual ~SerializeBuilder(){}

    template<typename T>
    void write_value(const T & ele);

    virtual void write_key(std::string key)
    {
        out_+='"';
        out_+=key;
        out_+='"';
        out_+=':';
    }
    virtual void arr_start()
    {
        out_+='[';
        out_+='\n';
    }
    virtual void arr_end()
    {
        out_+='\n';
        out_+=']';
    }
    virtual void obj_start()
    {
        out_+='{';
        out_+='\n';
//        auto i=indent_;
//        while (i>0) {
//            out_+="  ";
//            i--;
//        }
    }
    virtual void obj_end()
    {
        out_+='\n';
        out_+='}';
    }
    virtual void forward_next()
    {
        out_+=',';
        out_+='\n';
    }
    virtual void back()
    {
        out_.resize(out_.size()-2);
    }
protected:
    std::string out_;
    int indent_{0};
};

template<>
inline void SerializeBuilder::write_value<std::string>(const std::string & ele)
{
    out_+='"';
    out_+=ele;
    out_+='"';
}

template<>
inline void SerializeBuilder::write_value<bool>(const bool & ele)
{
    out_+=(ele==true?"true":"false");
}

template<>
inline void SerializeBuilder::write_value<int>(const int & ele)
{
    out_+=std::to_string(ele);
}

template<typename T>
void serialize(const T & obj,SerializeBuilder & builder)
{
    using TT=std::decay_t<T>;
    if constexpr (std::is_pointer_v<TT>) {//if constexpr间接提供了"函数偏特化"
        static_assert (!std::is_pointer_v<std::decay_t<decltype (*obj)>>,"不支持多级指针");
        assert(obj != nullptr);
        serialize(*obj, builder);
    } else if constexpr (std::is_aggregate_v<TT> && !std::is_union_v<TT>){
//        auto tupes=detail::to_tuple(obj);
//        builder.obj_start();
//        std::apply([&](auto&&... args) {
//            ((builder.write_key("fuck"),serialize(args,builder),builder.forward_next()),...);
//        },tupes);
//        if(std::tuple_size<decltype (tupes)>::value>0)
//            builder.back();
//        builder.obj_end();
        auto member_info=TT::template JsonStructBase<TT>::js_static_meta_data_info();
        builder.obj_start();
        std::apply([&](auto&&... args) {
            ((builder.write_key(args.name),serialize(obj.*(args.member),builder),builder.forward_next()),...);
        },member_info);
        if(std::tuple_size<decltype (member_info)>::value>0)
            builder.back();
        builder.obj_end();
    }else
        builder.write_value(obj);
}

template<typename T>
void serialize(const std::vector<T> &v,SerializeBuilder & builder)
{
    builder.arr_start();
    for(auto && i:v){
        serialize(i,builder);
        builder.forward_next();
    }
    if(v.size()>0)
        builder.back();
    builder.arr_end();
}
class json
{
public:
    json();
};

namespace detail {


template <typename T>
inline auto to_tuple(T& t) {
  constexpr auto const a = arity<T>();
  static_assert(a <= 64, "Max. supported members: 64");
  if constexpr (a == 0) {
    return std::tie();
  } else if constexpr (a == 1) {
    auto& [p1] = t;
    return std::tie(p1);
  } else if constexpr (a == 2) {
    auto& [p1, p2] = t;
    return std::tie(p1, p2);
  } else if constexpr (a == 3) {
    auto& [p1, p2, p3] = t;
    return std::tie(p1, p2, p3);
  } else if constexpr (a == 4) {
    auto& [p1, p2, p3, p4] = t;
    return std::tie(p1, p2, p3, p4);
  } else if constexpr (a == 5) {
    auto& [p1, p2, p3, p4, p5] = t;
    return std::tie(p1, p2, p3, p4, p5);
  } else if constexpr (a == 6) {
    auto& [p1, p2, p3, p4, p5, p6] = t;
    return std::tie(p1, p2, p3, p4, p5, p6);
  } else if constexpr (a == 7) {
    auto& [p1, p2, p3, p4, p5, p6, p7] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7);
  } else if constexpr (a == 8) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8);
  } else if constexpr (a == 9) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9);
  } else if constexpr (a == 10) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10);
  } else if constexpr (a == 11) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11);
  } else if constexpr (a == 12) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12);
  } else if constexpr (a == 13) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13);
  } else if constexpr (a == 14) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14);
  } else if constexpr (a == 15) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15);
  } else if constexpr (a == 16) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16);
  } else if constexpr (a == 17) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17);
  } else if constexpr (a == 18) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18);
  } else if constexpr (a == 19) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19);
  } else if constexpr (a == 20) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20);
  } else if constexpr (a == 21) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21);
  } else if constexpr (a == 22) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22);
  } else if constexpr (a == 23) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23);
  } else if constexpr (a == 24) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24);
  } else if constexpr (a == 25) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25);
  } else if constexpr (a == 26) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26);
  } else if constexpr (a == 27) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27);
  } else if constexpr (a == 28) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28);
  } else if constexpr (a == 29) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29);
  } else if constexpr (a == 30) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30);
  } else if constexpr (a == 31) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31);
  } else if constexpr (a == 32) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32);
  } else if constexpr (a == 33) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33);
  } else if constexpr (a == 34) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34);
  } else if constexpr (a == 35) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35);
  } else if constexpr (a == 36) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36);
  } else if constexpr (a == 37) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37);
  } else if constexpr (a == 38) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38);
  } else if constexpr (a == 39) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39);
  } else if constexpr (a == 40) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40);
  } else if constexpr (a == 41) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41);
  } else if constexpr (a == 42) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42);
  } else if constexpr (a == 43) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43);
  } else if constexpr (a == 44) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44);
  } else if constexpr (a == 45) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45);
  } else if constexpr (a == 46) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46);
  } else if constexpr (a == 47) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47);
  } else if constexpr (a == 48) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48);
  } else if constexpr (a == 49) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49);
  } else if constexpr (a == 50) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50);
  } else if constexpr (a == 51) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51);
  } else if constexpr (a == 52) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52);
  } else if constexpr (a == 53) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53);
  } else if constexpr (a == 54) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54);
  } else if constexpr (a == 55) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55);
  } else if constexpr (a == 56) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56);
  } else if constexpr (a == 57) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57);
  } else if constexpr (a == 58) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58);
  } else if constexpr (a == 59) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59);
  } else if constexpr (a == 60) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60);
  } else if constexpr (a == 61) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61);
  } else if constexpr (a == 62) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62);
  } else if constexpr (a == 63) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62, p63] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62, p63);
  } else if constexpr (a == 64) {
    auto& [p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62, p63, p64] = t;
    return std::tie(p1, p2, p3, p4, p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30, p31, p32, p33, p34, p35, p36, p37, p38, p39, p40, p41, p42, p43, p44, p45, p46, p47, p48, p49, p50, p51, p52, p53, p54, p55, p56, p57, p58, p59, p60, p61, p62, p63, p64);
  }
}

//namespace detail end

}

//namespace end
}

#endif // JSON_H
