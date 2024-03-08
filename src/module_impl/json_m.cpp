module;
#include <tuple>
#include <string>
#include <vector>
#include <stdexcept>

#include <functional>
#include <memory>
#include <stack>

module lsf;
import :analyze;
import :inner_imp;
import :jsonparser;
import :util;
import :json_src;

namespace lsf
{
    void SerializeBuilder::write_value(const std::string &ele)
    {
        out_ += ele;
    }

    void SerializeBuilder::write_value(const char *ele)
    {
        out_ += ele;
    }

    void SerializeBuilder::add_quotation()
    {
        out_ += '"';
    }

    void SerializeBuilder::write_key(std::string key)
    {
        out_ += '"';
        out_ += key;
        out_ += '"';
        out_ += ": ";
    }
    void SerializeBuilder::arr_start()
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
    void SerializeBuilder::arr_end()
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
    void SerializeBuilder::obj_start()
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
    void SerializeBuilder::obj_end()
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
    void SerializeBuilder::forward_next()
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
    void SerializeBuilder::back(std::size_t i)
    {
        // todo:check overflow
        out_.resize(out_.size() - indent.top() * 4 - i);
    }
} // namespace lsf

namespace lsf
{

    void Visitable2string(Visitable *root, SerializeBuilder &sb);

    void json_to_string(std::shared_ptr<TreeBuilder> builder, SerializeBuilder &sb)
    {
        LocaleGuard lg;
        Visitable2string(std::get<0>(builder->get_ast()), sb);
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
                    sb.write_key(to_cstring(i->get_this()->get_ref_str_(i->get_this()->key_)));
                    Visitable2string(i->get_this(), sb);
                }
                else
                    sb.back(1);
            }
            else
            {
                do
                {
                    sb.write_key(to_cstring(i->get_this()->get_ref_str_(i->get_this()->key_)));
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
            auto str=static_cast<const Jnode<NodeC::String> *>(root);
            sb.write_value(to_cstring(str->get_ref_str_(str->data_)));
            sb.add_quotation();
        }
        else if (root->ele_type_ == NodeC::Number)
        {
            auto num=static_cast<const Jnode<NodeC::Number> *>(root);
            sb.write_value(to_cstring(num->get_ref_str_(num->data_)));
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