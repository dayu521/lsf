module;
#include "analyse.h"
#include "error.h"
#include <tuple>
#include <string>
#include "inner_imp.h"
export module lsf;

export namespace lsf
{
    enum class ErrorType
    {
        LexError,
        ParserError,
        WeakTypeCheckError,
        UnknowError
    };
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
