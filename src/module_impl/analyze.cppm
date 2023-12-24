module;
#include <cassert>
#include <stack>
#include <functional>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <utility>

module lsf:analyze.imp;

import :analyze;
/*
namespace lsf
{
    // TreeBuilder::~TreeBuilder()
    // {
    //     dealloc_node();
    // }

    Tree TreeBuilder::get_ast()
    {
        return {root_, null_};
    }

    void TreeBuilder::before_build()
    {
        dealloc_node();
        root_ = null_ = new Jnode<NodeC::Obj>;
        null_->left_child_ = null_->right_bro_ = null_;
        clear_.push_back(null_);
    }

    void TreeBuilder::after_build()
    {
        // root_->key_ = "root";
        root_->right_bro_ = root_;
        null_->left_child_ = null_->right_bro_ = null_;
    }

    void TreeBuilder::build_obj()
    {
        auto n = new Jnode<NodeC::Obj>;
        n->ele_type_ = NodeC::Obj;
        n->left_child_ = root_->right_bro_;
        n->right_bro_ = n;
        n->n_ = std::get<1>(mbr_node_.top());
        root_ = n;
        mbr_node_.pop();
        clear_.push_back(root_);
    }

    void TreeBuilder::build_arr()
    {
        auto n = new Jnode<NodeC::Arr>;
        n->ele_type_ = NodeC::Arr;
        n->left_child_ = root_->right_bro_;
        n->right_bro_ = n;
        n->n_ = std::get<1>(mbr_node_.top());
        root_ = n;
        mbr_node_.pop();
        clear_.push_back(root_);
    }

    void TreeBuilder::build_string(std::wstring str)
    {
        auto n = new Jnode<NodeC::String>;
        n->data_.first = str_cache_.size();
        n->data_.second = str.size();
        n->get_ref_str_ = [this](str_ref a)
        { return this->get_ref_str(a); };
        str_cache_ += std::move(str);
        n->ele_type_ = NodeC::String;
        n->left_child_ = null_;
        n->right_bro_ = n;
        root_ = n;
        clear_.push_back(root_);
    }

    void TreeBuilder::build_number(std::wstring str)
    {
        auto n = new Jnode<NodeC::Number>;
        n->data_.first = str_cache_.size();
        n->data_.second = str.size();
        n->get_ref_str_ = [this](str_ref a)
        { return this->get_ref_str(a); };
        str_cache_ += std::move(str);
        n->ele_type_ = NodeC::Number;
        n->left_child_ = null_;
        n->right_bro_ = n;
        root_ = n;
        clear_.push_back(root_);
    }

    void TreeBuilder::build_keyword(std::wstring str)
    {
        auto n = new Jnode<NodeC::Keyword>;
        if (str == L"true")
            n->b_ = true;
        n->ele_type_ = NodeC::Keyword;
        n->left_child_ = null_;
        n->right_bro_ = n;
        root_ = n;
        clear_.push_back(root_);
    }

    void TreeBuilder::build_Null(std::wstring str)
    {
        auto n = new Jnode<NodeC::Null>;
        n->ele_type_ = NodeC::Null;
        n->left_child_ = null_;
        n->right_bro_ = n;
        root_ = n;
        clear_.push_back(root_);
    }

    void TreeBuilder::set_memberkey(std::wstring key)
    {
        // root_->key_ = to_cstring(key);
        auto n = root_;
        n->key_.first = str_cache_.size();
        n->key_.second = key.size();
        if (n->get_ref_str_ == nullptr)
        {
            n->get_ref_str_ = [this](str_ref a)
            { return this->get_ref_str(a); };
        }
        str_cache_ += std::move(key);
    }

    void TreeBuilder::build_null_mbr()
    {
        root_ = null_;
        mbr_node_.push({nullptr, 0});
    }

    // 当前已经有一个元素了
    void TreeBuilder::can_start_iteration()
    {
        mbr_node_.push({root_, 1}); // 构建完成后正好清空
    }

    void TreeBuilder::move_next()
    {
        auto &[current, n] = mbr_node_.top();
        n++;
        root_->right_bro_ = current->right_bro_;
        current->right_bro_ = root_;
        current = root_;
    }

    void TreeBuilder::finish_iteration()
    {
        //    root_=root_->right_bro_;
        //    mbr_node_.pop();  //放到arr或obj中清空，因为那时候需要存储的个数
    }

    void TreeBuilder::dealloc_node()
    {
        for (auto &i : clear_)
        {
            delete i;
            i = nullptr;
        }
        clear_.clear();
    }

    void PrintNodes::set_null(Visitable *nul)
    {
        faken_ = nul;
    }

    void PrintNodes::visit(Jnode<NodeC::Obj> &obj)
    {
        [[likely]] if (obj.left_child_ != faken_)
        {
            auto j = obj.left_child_;
            do
            {
                //            std::cout<<j->key_<<" ";
                j = j->right_bro_;
            } while (j != obj.left_child_);
        }
    }

    void PrintNodes::visit(Jnode<NodeC::Arr> &arr)
    {
    }

    void PrintNodes::visit(Jnode<NodeC::String> &str)
    {
        //    std::cout<<str.data_<<" ";
    }

    void PrintNodes::visit(Jnode<NodeC::Number> &num)
    {
        //    std::cout<<num.data_<<" ";
    }

    void PrintNodes::visit(Jnode<NodeC::Keyword> &key_word)
    {
        //    std::cout<<(key_word.b_?"true":"false")<<" ";
    }

    void PrintNodes::visit(Jnode<NodeC::Null> &null)
    {
        //    std::cout<<"null"<<" ";
    }

    void Visitor::visit_BFS(Tree roott, std::function<void()> round_callback)
    {
        std::queue<Visitable *> c{};
        auto [root, faken] = roott;
        c.push(root);
        int x = 1, y = 0;
        while (!c.empty())
        {
            auto i = c.front();
            c.pop();
            x--;
            i->accept(*this);
            if (i->left_child_ != faken)
            {
                auto j = i->left_child_;
                do
                {
                    y++;
                    c.push(j->get_this());
                    j = j->right_bro_;
                } while (j != i->left_child_);
            }
            if (x == 0)
            {
                x = y;
                y = 0;
                round_callback();
            }
        }
    }

    ///*************TypeChecker*************
    bool WeakTypeChecker::visit(Jnode<NodeC::Obj> &obj)
    {
        current_type = NodeC::Obj;
        // arr begin
        jtype_.push_back(current_type);
        if (obj.left_child_ == null_)
        {
            // arr end
            jtype_.push_back(current_type);
            return true;
        }
        std::set<str_ref> cset{};
        auto j = obj.left_child_;
        do
        {
            if (!j->get_this()->accept(*this))
                return false;
#if __cplusplus >= 202002L
            auto haskey = cset.contains(j->get_this()->key_);
#else
            auto haskey = cset.end() != cset.find(j->get_this()->key_);
#endif
            if (haskey)
            {
                error_ += "重复key_:";
                src_ += j->get_this()->get_ref_str_(j->get_this()->key_);
                return false;
            }
            cset.insert(j->get_this()->key_);
            j = j->right_bro_;
        } while (j != obj.left_child_);
        // 因为是共享的,所以需要重新赋值.其他地方类似
        current_type = NodeC::Obj;
        // end
        jtype_.push_back(current_type);
        return true;
    }

    bool WeakTypeChecker::visit(Jnode<NodeC::Arr> &arr)
    {
        current_type = NodeC::Arr;
        // arr begin
        jtype_.push_back(current_type);
        if (arr.left_child_ == null_)
        {
            // arr end
            jtype_.push_back(current_type);
            return true;
        }
        auto first_beg = jtype_.size();
        auto j = arr.left_child_;
        if (!j->get_this()->accept(*this))
            return false;
        auto another_beg = jtype_.size();
        j = j->right_bro_;
        while (j != arr.left_child_)
        {
            if (!j->get_this()->accept(*this) || !do_check(first_beg, another_beg))
            {
                error_ += "数组类型不同:";
                src_ += arr.get_this()->get_ref_str_(arr.get_this()->key_);
                return false;
            }
            jtype_.resize(another_beg);
            j = j->right_bro_;
        }
        current_type = NodeC::Arr;
        // end
        jtype_.push_back(current_type);
        return true;
    }

    bool WeakTypeChecker::visit(Jnode<NodeC::String> &str)
    {
        current_type = NodeC::String;
        jtype_.push_back(current_type);
        return true;
    }

    bool WeakTypeChecker::visit(Jnode<NodeC::Number> &num)
    {
        current_type = NodeC::Number;
        jtype_.push_back(current_type);
        return true;
    }

    bool WeakTypeChecker::visit(Jnode<NodeC::Keyword> &key)
    {
        current_type = NodeC::Keyword;
        jtype_.push_back(current_type);
        return true;
    }

    bool WeakTypeChecker::visit(Jnode<NodeC::Null> &null)
    {
        current_type = NodeC::Null;
        jtype_.push_back(current_type);
        return true;
    }

    bool WeakTypeChecker::check_type(Tree roott)
    {
        jtype_.clear();
        null_ = std::get<1>(roott);
        auto root = std::get<0>(roott);
        return root->accept(*this);
    }

    bool WeakTypeChecker::do_check(std::size_t first, std::size_t another)
    {
        auto first_beg = jtype_.begin() + first;
        auto another_beg = jtype_.begin() + another;
        assert(first_beg != jtype_.end() && another_beg != jtype_.end());
        auto r = std::equal(first_beg, another_beg, another_beg, jtype_.end());
        return r;
    }

    std::pair<std::string, std::wstring> WeakTypeChecker::get_error()
    {
        return {error_, src_};
    }

} // namespace lsf
*/