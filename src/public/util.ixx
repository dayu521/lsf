module;
#include <string>
#include <memory>
#include <functional>
#include <stack>

export module lsf:util;

import :analyze;
import :json_src;

namespace lsf
{
    export class SerializeBuilder
    {
    public:
        SerializeBuilder() { indent.push(0); }
        virtual ~SerializeBuilder() {}
        std::string_view get_jsonstring() const { return out_; }
        void clear() { out_.clear(); }

    public:
        virtual void write_value(const std::string &ele);

        virtual void write_value(const char *ele);
        virtual void add_quotation();
        virtual void write_key(std::string key);
        virtual void arr_start();
        virtual void arr_end();
        virtual void obj_start();
        virtual void obj_end();
        virtual void forward_next();
        virtual void back(std::size_t i = 2);

    protected:
        std::string out_{};
        std::stack<int> indent{};
    };

    export template <typename T>
    void write_value(const T &v, SerializeBuilder &builder);

    export template <typename T>
    void Deserialize(T &s, const Visitable *t);

    class DeserializeError : public BaseError
    {
        using BaseError::BaseError;
    };

    template <typename T>
    void deserialize(T &obj, const Visitable *t);

    template <typename T>
    void deserialize(T &obj, const Visitable *t)
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
            //        std::vector<Visitable*> vs{};
            Visitable *vs[member_size] = {};
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
    void deserialize(std::vector<T> &v, const Visitable *t)
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
    inline void Deserialize<std::string>(std::string &s, const Visitable *t)
    {
        if (t->ele_type_ == NodeC::String)
        {
            // 如果有错,语法分析会提前失败.下同
            auto str = static_cast<const Jnode<NodeC::String> *>(t);
            s = to_cstring(str->get_ref_str_(str->data_));
        }
        else
            throw DeserializeError("序列化std::string: 期待json String");
    }

    template <>
    inline void Deserialize<int>(int &s, const Visitable *t)
    {
        if (t->ele_type_ == NodeC::Number)
        {
            auto num = static_cast<const Jnode<NodeC::Number> *>(t);
            s = std::stoi(num->get_ref_str_(num->data_));
        }
        else
            throw DeserializeError("序列化int: 期待json Number");
    }

    template <>
    inline void Deserialize<double>(double &s, const Visitable *t)
    {
        if (t->ele_type_ == NodeC::Number)
        {
            auto dob = static_cast<const Jnode<NodeC::Number> *>(t);
            s = std::stod(dob->get_ref_str_(dob->data_));
        }
        else
            throw DeserializeError("序列化double: 期待json Number");
    }

    template <>
    inline void Deserialize<bool>(bool &b, const Visitable *t)
    {
        if (t->ele_type_ == NodeC::Keyword)
        {
            b = static_cast<const Jnode<NodeC::Keyword> *>(t)->b_;
        }
        else
            throw DeserializeError("序列化bool: 期待json bool");
    }

    ////////////////////序列化

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

} // namespace lsf
