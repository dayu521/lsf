module;
#include "analyse.h"
#include "error.h"
#include <tuple>
#include <string>
#include "inner_imp.h"

module lsf;

namespace lsf
{

    class DeserializeError : public BaseError
    {
        using BaseError::BaseError;
    };

    namespace detail
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

    /*****************************/

    class JsonContext
    {
    public:
        virtual ~JsonContext() {}

    public:
        virtual void set_filename(const std::string &file_name) = 0;
        virtual void run() = 0;
    };

    template <typename T>
    void deserialize(T &obj, const TreeNode *t);

    template <typename T>
    void Deserialize(T &s, const TreeNode *t);

    template <typename T>
    void deserialize(T &obj, const TreeNode *t)
    {
        auto temp = t->left_child_;
        if (temp == t)
        {
            throw DeserializeError("json 过早结束");
        }
        using TT = std::decay_t<T>;
        if constexpr (std::is_pointer_v<TT>)
        { // if constexpr间接提供了"函数偏特化"
            static_assert(!std::is_pointer_v<std::decay_t<decltype(*t)>>, "不支持多级指针");
            if (t == nullptr)
            {
                throw DeserializeError("struct成员为nullptr");
            }
            deserialize(*obj, t);
        }
        else if constexpr (std::is_aggregate_v<TT> && !std::is_union_v<TT>)
        {
            auto member_info = TT::template JsonStructBase<TT>::js_static_meta_data_info();
            constexpr auto member_size = std::tuple_size<decltype(member_info)>::value;
            if (t->ele_type_ == NodeC::Null)
            {
                obj = {};
                return;
            }
            if (t->ele_type_ != NodeC::Obj)
            {
                throw DeserializeError("期待json对象");
            }
            auto ot = static_cast<const Jnode<NodeC::Obj> *>(t);
            if (member_size != ot->n_)
            {
                // 元素数量不等
                throw DeserializeError("struct成员数量与json成员数量不同");
            }
            //        std::vector<TreeNode*> vs{};
            TreeNode *vs[member_size] = {};
            int n = 0;
            do
            {
                vs[n] = temp;
                n++;
                temp = temp->right_bro_;
            } while (temp != t->left_child_);
            n = 0;
            std::apply([&](auto &&...args)
                       {
            auto lam=[&](auto && arg,auto n)->bool{
                for(std::size_t i=n;i<member_size;i++){
                    if(arg.name==vs[i]->key_){
                        if(n!=i)
                            std::swap(vs[n],vs[i]);
                        return true;
                    }
                }
                throw DeserializeError(std::string("找不到key:")+arg.name);
            };
            //(lam(args,n++)&&...)好像是等价的!
            //msvc不支持!
//            return (...&&lam(args,n++));
            //因为&&是短路求值,所以折叠后的表达式的括号不影响求值顺序?
            return (lam(args,n++)&&...); },
                       member_info);
            n = 0;
            std::apply([&](auto &&...args)
                       { (deserialize(obj.*(args.member), vs[n++]), ...); },
                       member_info);
        }
        else
            Deserialize(obj, t);
    }

    template <typename T>
    void deserialize(std::vector<T> &v, const TreeNode *t)
    {
        auto temp = t->left_child_;
        if (temp == t)
        {
            throw DeserializeError("序列化std::vector: json过早结束");
        }
        if (t->ele_type_ == NodeC::Null)
        {
            v = {};
            return;
        }
        if (t->ele_type_ != NodeC::Arr)
            throw DeserializeError("期待json数组");
        auto at = static_cast<const Jnode<NodeC::Arr> *>(t);
        T m{};
        v.resize(at->n_);
        for (std::size_t i = 0; i < at->n_; i++)
        {
            deserialize(m, temp);
            v[i] = m;
            temp = temp->right_bro_;
        }
    }

    template <>
    inline void Deserialize<std::string>(std::string &s, const TreeNode *t)
    {
        if (t->ele_type_ == NodeC::String)
        {
            // 如果有错,语法分析会提前失败.下同
            s = static_cast<const Jnode<NodeC::String> *>(t)->data_;
        }
        else
            throw DeserializeError("序列化std::string: 期待json String");
    }

    template <>
    inline void Deserialize<int>(int &s, const TreeNode *t)
    {
        if (t->ele_type_ == NodeC::Number)
        {
            s = std::stoi(static_cast<const Jnode<NodeC::Number> *>(t)->data_);
        }
        else
            throw DeserializeError("序列化int: 期待json Number");
    }

    template <>
    inline void Deserialize<double>(double &s, const TreeNode *t)
    {
        if (t->ele_type_ == NodeC::Number)
        {
            s = std::stod(static_cast<const Jnode<NodeC::Number> *>(t)->data_);
        }
        else
            throw DeserializeError("序列化double: 期待json Number");
    }

    template <>
    inline void Deserialize<bool>(bool &b, const TreeNode *t)
    {
        if (t->ele_type_ == NodeC::Keyword)
        {
            b = static_cast<const Jnode<NodeC::Keyword> *>(t)->b_;
        }
        else
            throw DeserializeError("序列化bool: 期待json bool");
    }

    template <typename T>
    void write_value(const T &v, SerializeBuilder &builder);

    template <>
    inline void write_value<std::string>(const std::string &ele, SerializeBuilder &builder)
    {
        builder.add_quotation();
        builder.write_value(ele);
        builder.add_quotation();
    }

    template <>
    inline void write_value<bool>(const bool &ele, SerializeBuilder &builder)
    {
        builder.write_value(ele == true ? "true" : "false");
    }

    template <>
    inline void write_value<int>(const int &ele, SerializeBuilder &builder)
    {
        builder.write_value(std::to_string(ele));
    }

    template <>
    inline void write_value<double>(const double &ele, SerializeBuilder &builder)
    {
        builder.write_value(std::to_string(ele));
    }

    template <typename T>
    void serialize(const T &obj, SerializeBuilder &builder)
    {
        using TT = std::decay_t<T>;
        if constexpr (std::is_pointer_v<TT>)
        { // if constexpr间接提供了"函数偏特化"
            static_assert(!std::is_pointer_v<std::decay_t<decltype(*obj)>>, "不支持多级指针");
            assert(obj != nullptr);
            if (obj == nullptr)
            {
                write_value("null", builder);
            }
            else
                serialize(*obj, builder);
        }
        else if constexpr (std::is_aggregate_v<TT> && !std::is_union_v<TT>)
        {
            auto member_info = TT::template JsonStructBase<TT>::js_static_meta_data_info();
            builder.obj_start();
            std::apply([&](auto &&...args)
                       { ((builder.write_key(args.name), serialize(obj.*(args.member), builder), builder.forward_next()), ...); },
                       member_info);
            if (std::tuple_size<decltype(member_info)>::value > 0)
                builder.back();
            builder.obj_end();
        }
        else
        {
            write_value(obj, builder);
        }
    }

    template <typename T>
    void serialize(const std::vector<T> &v, SerializeBuilder &builder)
    {
        builder.arr_start();
        for (auto &&i : v)
        {
            serialize(i, builder);
            builder.forward_next();
        }
        if (v.size() > 0)
            builder.back();
        builder.arr_end();
    }

    // namespace end
}