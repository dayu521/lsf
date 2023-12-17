module;
#include <tuple>
#include <string>
#include <vector>
#include <stdexcept>

export module lsf:x;
import lsf;
import :analyze;
import :inner_imp;

namespace lsf
{

    class DeserializeError : public BaseError
    {
        using BaseError::BaseError;
    };

    template <typename T>
    void deserialize(T &obj, const Visitable *t);

    template <typename T>
    void Deserialize(T &s, const Visitable *t);

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
            s = static_cast<const Jnode<NodeC::String> *>(t)->data_;
        }
        else
            throw DeserializeError("序列化std::string: 期待json String");
    }

    template <>
    inline void Deserialize<int>(int &s, const Visitable *t)
    {
        if (t->ele_type_ == NodeC::Number)
        {
            s = std::stoi(static_cast<const Jnode<NodeC::Number> *>(t)->data_);
        }
        else
            throw DeserializeError("序列化int: 期待json Number");
    }

    template <>
    inline void Deserialize<double>(double &s, const Visitable *t)
    {
        if (t->ele_type_ == NodeC::Number)
        {
            s = std::stod(static_cast<const Jnode<NodeC::Number> *>(t)->data_);
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

    void Visitable2string(Visitable *root, SerializeBuilder &sb);

    export void json_to_string(Json &json, SerializeBuilder &sb)
    {

        Visitable2string(std::get<0>(json.builder->get_ast()), sb);
    }

    export template <typename S>
    void struct_to_jsonstr(const S &obj, SerializeBuilder &builder)
    {
        serialize(obj, builder);
    }

    export template <typename S>
    void json_to_struct(const Json &json, S &s)
    {
        deserialize(s, std::get<0>(json.builder->get_ast()));
    }

    // I can't find out a better function name
    void Visitable2string(Visitable *root, SerializeBuilder &sb)
    {
        if (root->left_child_ == root)
        {
            return;
        }
        if (root->ele_type_ == NodeC::Obj)
        {
            sb.obj_start();
            auto i = root->left_child_;
            // fixed: empty member takes up a new line
            if (i->right_bro_ == i)
            {
                if (i->left_child_ != i)
                {
                    sb.write_key(i->key_);
                    Visitable2string(i->get_this(), sb);
                }
                else
                    sb.back(1);
            }
            else
            {
                do
                {
                    sb.write_key(i->key_);
                    Visitable2string(i->get_this(), sb);
                    sb.forward_next();
                    i = i->right_bro_;
                } while (root->left_child_ != i);
                sb.back();
            }
            sb.obj_end();
        }
        else if (root->ele_type_ == NodeC::Arr)
        {
            sb.arr_start();
            auto i = root->left_child_;
            if (i->right_bro_ == i)
            {
                if (i->left_child_ != i)
                    Visitable2string(i->get_this(), sb);
                else
                    sb.back(1);
            }
            else
            {
                do
                {
                    Visitable2string(i->get_this(), sb);
                    sb.forward_next();
                    i = i->right_bro_;
                } while (root->left_child_ != i);
                sb.back();
            }
            sb.arr_end();
        }
        else if (root->ele_type_ == NodeC::String)
        {
            sb.add_quotation();
            sb.write_value(static_cast<const Jnode<NodeC::String> *>(root)->data_);
            sb.add_quotation();
        }
        else if (root->ele_type_ == NodeC::Number)
        {
            sb.write_value(static_cast<const Jnode<NodeC::Number> *>(root)->data_);
        }
        else if (root->ele_type_ == NodeC::Keyword)
        {
            sb.write_value(static_cast<const Jnode<NodeC::Keyword> *>(root)->b_ ? "true" : "false");
        }
        else if (root->ele_type_ == NodeC::Null)
        {
            sb.write_value("null");
        }
        else
            throw std::runtime_error("Visitable2string() failed"); // never be here
    }
    // namespace end
}