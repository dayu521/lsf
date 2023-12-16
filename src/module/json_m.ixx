module;
#include "analyse.h"
#include "error.h"
#include <tuple>
#include <string>
#include "inner_imp.h"

module lsf;

/*
namespace lsf
{
    class Json
    {
    public:
        Json(const std::string &filename);
        Json(const Json &) = delete;
        Json(Json &&) = default;
        /// 回调函数f,仅仅是提醒错误需要处理,没有其他想法
        [[nodiscard]] bool run(std::function<void(ErrorType et, const std::string &message)> f);
        /// 回调函数f,仅仅是提醒错误需要处理,没有其他想法
        [[nodiscard]] bool weak_type_check(std::function<void(ErrorType et, const std::string &message)> f);
        /// 不要使用这个函数,因为返回值的寿命是当前对象*this负责的
        TreeNode *get_output() const;
        std::string get_errors() const;

    private:
        std::shared_ptr<FilterBuff> buff_;
        std::shared_ptr<Lexer> lexer_;
        std::shared_ptr<FunnyTokenGen> wrap_lexer_;
        std::unique_ptr<JsonParser> parser_;
        std::shared_ptr<Treebuilder> builder;
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

    template <typename S>
    void struct_to_json(const S &obj, SerializeBuilder &builder)
    {
        serialize(obj, builder);
    }

    template <typename S>
    void json_to_struct(S &s, const Json &json)
    {
        deserialize(s, json.get_output());
    }

    // I can't find out a better function name
    inline void TreeNode2string(TreeNode *root, SerializeBuilder &sb)
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
                    TreeNode2string(i, sb);
                }
                else
                    sb.back(1);
            }
            else
            {
                do
                {
                    sb.write_key(i->key_);
                    TreeNode2string(i, sb);
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
                    TreeNode2string(i, sb);
                else
                    sb.back(1);
            }
            else
            {
                do
                {
                    TreeNode2string(i, sb);
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
            throw std::runtime_error("TreeNode2string() failed"); // never be here
    }
} // namespace lsf

*/
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

    // enum class ErrorType
    // {
    //     LexError,
    //     ParserError,
    //     WeakTypeCheckError,
    //     UnknowError
    // };

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