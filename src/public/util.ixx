module;
#include <string>
#include <memory>
#include <functional>
#include <stack>
#include <vector>
#include <type_traits>
#include <limits>

#include <unordered_map>

export module lsf:util;

import :analyze;
import :json_src;
import :struct_help;
import :tp;

namespace lsf
{
    // enum CppNestType
    // {
    //     None,
    //     Struct,
    //     STD_VECTOR
    // };

    class DeserializeError : public BaseError
    {
        using BaseError::BaseError;
    };

    namespace CppNestType
    {
        struct Struct
        {
        };

        struct STD_VECTOR
        {
        };
    };

    export class FetchCppType : public BaseGuard<void, bool, long, double, std::string>,
                                public BaseNest<void, CppNestType::Struct, CppNestType::STD_VECTOR>
    {
    public:
        virtual ~FetchCppType();

    public:
        virtual void nest_begin(TypeTag<CppNestType::Struct>, std::size_t n) override {}
        virtual void nest_end(TypeTag<CppNestType::Struct>) override {}

        virtual void nest_begin(TypeTag<CppNestType::STD_VECTOR>, std::size_t n) override {}
        virtual void nest_end(TypeTag<CppNestType::STD_VECTOR>) override {}

    public:
        virtual void find_obj_mem(std::string s) = 0;
        virtual void find_arr_mem() = 0;
        virtual std::size_t arr_size() = 0;
    };

    FetchCppType::~FetchCppType() = default;

    export class ReadJsonStr : public FetchCppType
    {
    public:
        ReadJsonStr(Visitable *root)
        {
            mem_nest_context_.push(root);
            root_ = root;
        }
        virtual ~ReadJsonStr();

    public:
        virtual void find(bool &b) override;
        virtual void find(long &l) override;
        virtual void find(double &d) override;
        virtual void find(std::string &s) override;
        virtual std::size_t arr_size() override;

    public:
        virtual void nest_begin(TypeTag<CppNestType::Struct>, std::size_t n) override;
        virtual void nest_end(TypeTag<CppNestType::Struct>) override;

        virtual void nest_begin(TypeTag<CppNestType::STD_VECTOR>, std::size_t n) override;
        virtual void nest_end(TypeTag<CppNestType::STD_VECTOR>) override;

    public:
        virtual void find_obj_mem(std::string s) override;
        virtual void find_arr_mem() override;

    private:
        Visitable *root_;

        std::stack<Visitable *> mem_nest_context_;
        std::unordered_map<std::string, Visitable *> key_index_;
        bool has_key_ = false;

        std::size_t arr_size_;
    };

    ReadJsonStr::~ReadJsonStr() = default;

    std::size_t ReadJsonStr::arr_size()
    {
        return arr_size_;
    }
    void ReadJsonStr::find(bool &b)
    {
        if (root_->ele_type_ == NodeC::Keyword)
        {
            b = static_cast<const Jnode<NodeC::Keyword> *>(root_)->b_;
        }
        else
            throw DeserializeError("序列化bool: 期待json bool");
    }
    void ReadJsonStr::find(long &l)
    {
        if (root_->ele_type_ == NodeC::Number)
        {
            auto num = static_cast<const Jnode<NodeC::Number> *>(root_);
            l = std::stol(num->get_ref_str_(num->data_));
        }
        else
            throw DeserializeError("序列化int: 期待json Number");
    }
    void ReadJsonStr::find(double &d)
    {
        if (root_->ele_type_ == NodeC::Number)
        {
            auto dob = static_cast<const Jnode<NodeC::Number> *>(root_);
            d = std::stod(dob->get_ref_str_(dob->data_));
        }
        else
            throw DeserializeError("序列化double: 期待json Number");
    }
    void ReadJsonStr::find(std::string &s)
    {
        if (root_->ele_type_ == NodeC::String)
        {
            // 如果有错,语法分析会提前失败.下同
            auto str = static_cast<const Jnode<NodeC::String> *>(root_);
            s = to_cstring(str->get_ref_str_(str->data_));
        }
        else
            throw DeserializeError("序列化std::string: 期待json String");
    }

    void ReadJsonStr::nest_begin(TypeTag<CppNestType::Struct>, std::size_t n)
    {
        mem_nest_context_.push(root_);
        key_index_.clear();

        auto begin = root_->left_child_->get_this();
        auto i = begin;
        do
        {
            auto key = to_cstring(i->get_ref_str_(i->key_));
            key_index_.insert({key, i});
            i = i->right_bro_->get_this();
        } while (i != begin);
    }

    void ReadJsonStr::nest_end(TypeTag<CppNestType::Struct>)
    {
        mem_nest_context_.pop();
        root_ = mem_nest_context_.top();
    }

    void ReadJsonStr::nest_begin(TypeTag<CppNestType::STD_VECTOR>, std::size_t n)
    {
        mem_nest_context_.push(root_);

        if (root_->ele_type_ != NodeC::Arr)
        {
            throw DeserializeError("序列化std::vector: 期待json Array");
        }
        arr_size_ = static_cast<const Jnode<NodeC::Arr> *>(root_)->n_; // 可以是0
        // root_ = root_->left_child_->get_this();
    }

    void ReadJsonStr::nest_end(TypeTag<CppNestType::STD_VECTOR>)
    {
        mem_nest_context_.pop();
        root_ = mem_nest_context_.top();
    }

    void ReadJsonStr::find_obj_mem(std::string s)
    {
        if (key_index_.find(s) == key_index_.end())
        {
            return;
        }
        root_ = key_index_.at(s);
    }

    void ReadJsonStr::find_arr_mem()
    {
        root_ = root_->right_bro_->get_this();
    }

    export class WriteJsonStr : public FetchCppType
    {
    public:
        virtual void find(bool &b) override;
        virtual void find(long &l) override;
        virtual void find(double &d) override;
        virtual void find(std::string &s) override;

        virtual void find_obj_mem(std::string s) override;
        virtual void find_arr_mem() override;

    public:
        virtual void nest_begin(TypeTag<CppNestType::Struct>, std::size_t n) override;
        virtual void nest_end(TypeTag<CppNestType::Struct>) override;

        virtual void nest_begin(TypeTag<CppNestType::STD_VECTOR>, std::size_t n) override;
        virtual void nest_end(TypeTag<CppNestType::STD_VECTOR>) override;
        virtual std::size_t arr_size() override;

    public:
        std::string_view get_jsonstring() const { return out_; }

        WriteJsonStr() { indent_.push(0); }
        virtual ~WriteJsonStr();

    private:
        std::string out_;
        std::stack<int> indent_;
        std::size_t mem_n_ = 0;
        std::size_t mem_index_ = 0;
    };

    WriteJsonStr::~WriteJsonStr() = default;

    void WriteJsonStr::find(bool &b)
    {
        auto bstr = b == true ? "true" : "false";
        out_ += bstr;
    }

    void WriteJsonStr::find(long &l)
    {
        out_ += std::to_string(l);
    }

    void WriteJsonStr::find(double &d)
    {
        out_ += std::to_string(d);
    }

    void WriteJsonStr::find(std::string &s)
    {
        out_ += '"' + s + '"';
    }

    void WriteJsonStr::find_obj_mem(std::string s)
    {
        find_arr_mem();

        // 写json key
        out_ += '"' + s + '"';
        out_ += ": ";
    }

    void WriteJsonStr::find_arr_mem()
    {
        if (mem_index_ > 0)
            out_ += ',';
        out_ += '\n';
        mem_index_++;

        // 对齐缩进
        auto i = indent_.top();
        while (i > 0)
        {
            out_ += "    ";
            i--;
        }
    }

    void WriteJsonStr::nest_begin(TypeTag<CppNestType::Struct>, std::size_t n)
    {
        // 初始化环境
        mem_index_ = 0;
        mem_n_ = n;

        out_ += '{';
        // out_ += '\n';

        // 开始新缩进
        indent_.push(indent_.top() + 1);
        auto i = indent_.top();
        while (i > 0)
        {
            out_ += "    ";
            i--;
        }
    }

    void WriteJsonStr::nest_end(TypeTag<CppNestType::Struct>)
    {
        out_ += '\n';
        auto i = indent_.top();
        indent_.pop();
        while (i - 1 > 0)
        {
            out_ += "    ";
            i--;
        }
        out_ += '}';
    }

    void WriteJsonStr::nest_begin(TypeTag<CppNestType::STD_VECTOR>, std::size_t n)
    {
        auto old_index = out_.size();

        nest_begin(TypeTag<CppNestType::Struct>(), n);

        out_[old_index] = '[';
    }

    void WriteJsonStr::nest_end(TypeTag<CppNestType::STD_VECTOR>)
    {
        nest_end(TypeTag<CppNestType::Struct>());
        out_.back() = ']';
    }

    std::size_t WriteJsonStr::arr_size()
    {
        return mem_n_;
    }

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

    enum Direct
    {
        DFS,
        BFS
    };

    // 在树中,沿着不同方向移动.然后返回新的节点
    template <Direct D>
    Visitable *move_DFS(Visitable *root);

    template <>
    Visitable *move_DFS<DFS>(Visitable *root);

    template <>
    Visitable *move_DFS<BFS>(Visitable *root);

    // TODO 新的名字
    export template <typename T>
    void parse_cpp_type(T &s, FetchCppType &fct);

    export template <StructConcept T>
    void parse_cpp_type(T &s, FetchCppType &fct)
    {
        auto member_info = T::template JsonStructBase<T>::js_static_meta_data_info();

        fct.nest_begin(TypeTag<CppNestType::Struct>{}, std::tuple_size<decltype(member_info)>::value);

        std::apply([&](auto &&...args)
                   { ((fct.find_obj_mem(args.name), parse_cpp_type(s.*(args.member), fct)), ...); },
                   member_info);

        fct.nest_end(TypeTag<CppNestType::Struct>{});
    }

    export template <typename T>
    void parse_cpp_type(std::vector<T> &s, FetchCppType &fct)
    {
        fct.nest_begin(TypeTag<CppNestType::STD_VECTOR>{}, s.size());
        s.resize(fct.arr_size());
        for (auto &v : s)
        {
            fct.find_arr_mem();
            parse_cpp_type(v, fct);
        }
        fct.nest_end(TypeTag<CppNestType::STD_VECTOR>{});
    }

    export template <>
    void parse_cpp_type(std::string &s, FetchCppType &fct)
    {
        fct.find(s);
    }

    export template <>
    void parse_cpp_type(long &s, FetchCppType &fct)
    {
        fct.find(s);
    }

    export template <>
    void parse_cpp_type(int &s, FetchCppType &fct)
    {
        long s_long = s;
        fct.find(s_long);
        if (s_long > std::numeric_limits<int>::max())
        {
            // TODO: throw exception
        }
        s = s_long;
    }

    export template <>
    void parse_cpp_type(double &s, FetchCppType &fct)
    {
        fct.find(s);
    }

    export template <>
    void parse_cpp_type(bool &s, FetchCppType &fct)
    {
        fct.find(s);
    }

    // TODO 新的名字
    export template <typename T>
    void write_cpp_type(const T &v, SerializeBuilder &builder);

    // TODO 新的名字
    export template <typename T>
    void read_cpp_type(T &s, const Visitable *t);

    export template <typename T>
    void write_value(const T &v, SerializeBuilder &builder);

    export template <typename T>
    void Deserialize(T &s, const Visitable *t);

    template <typename T>
    void deserialize(T &obj, const Visitable *t);

    template <typename T>
    void deserialize(std::vector<T> &v, const Visitable *t)
    {
        auto temp = t->left_child_->get_this();
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
            temp = temp->right_bro_->get_this();
        }
    }

    template <typename T>
    void deserialize(T &obj, const Visitable *t)
    {
        auto temp = t->left_child_->get_this();
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
                temp = temp->right_bro_->get_this();
            } while (temp != t->left_child_);
            n = 0;
            std::apply([&](auto &&...args)
                       {
                auto lam=[&](auto && arg,auto n)->bool{
                    for(std::size_t i=n;i<member_size;i++){
                        if(arg.name==to_cstring(vs[i]->get_ref_str_(vs[i]->key_))){
                            if(n!=i)
                                std::swap(vs[n],vs[i]);
                            return true;
                        }
                    }
                    throw DeserializeError(std::string("找不到key:")+arg.name);
                };
                //(lam(args,n++)&&...)好像是等价的!
                //msvc不支持!
            //    return (...&&lam(args,n++));
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
    void deserialize(T &obj, const Visitable *t, bool ignore_absence_tag)
    {
        auto temp = t->left_child_->get_this();
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
            if (t->ele_type_ == NodeC::Null)
            {
                obj = {};
                return;
            }
            if (t->ele_type_ != NodeC::Obj)
            {
                throw DeserializeError("期待json对象");
            }

            std::vector<Visitable *> vss;
            do
            {
                vss.push_back(temp);
                temp = temp->right_bro_->get_this();
            } while (temp != t->left_child_);
            auto try_deserialize = [&vss](auto &&arg, auto &&key)
            {
                for (auto i : vss)
                {
                    if (key == to_cstring(i->get_ref_str_(i->key_)))
                    {
                        deserialize(arg, i);
                        return;
                    }
                }
            };
            std::apply([&](auto &&...args)
                       { (try_deserialize(obj.*(args.member), args.name), ...); },
                       member_info);
        }
        else
            Deserialize(obj, t);
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
