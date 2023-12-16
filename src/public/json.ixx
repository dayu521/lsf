module;
// #include "analyse.h"
#include <functional>
#include <memory>
#include <stack>
#include <tuple>
#include <string>
// #include "inner_imp.h"
export module lsf;

namespace lsf
{
    class FilterBuff;
    class Lexer;
    class FunnyTokenGen;
    class JsonParser;
    class Treebuilder;

    enum class NodeC;

    struct TreeNode;
}

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
        //https://en.cppreference.com/w/cpp/memory/unique_ptr
        ~Json();
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
    void struct_to_jsonstr(const S &obj, SerializeBuilder &builder)
    {
        serialize(obj, builder);
    }

    template <typename S>
    void json_to_struct(const Json &json,S &s)
    {
        deserialize(s, json.get_output());
    }

    void json_to_string(Json & json, SerializeBuilder &sb);
} // namespace lsf
