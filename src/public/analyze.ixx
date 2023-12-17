module;
#include <cassert>
#include <stack>
#include <functional>
#include <string>
#include <vector>
#include <queue>
#include <set>

export module lsf:analyze;

import :constant;
import :error;
import :inner_imp;
import :parser_tree;

// 如果需要主模块的声明,则也需要显式导入
//  import lsf;

namespace lsf
{
    template <auto token>
    struct Jnode;

    template <typename T1, typename T2>
    struct VisitableTree;

    class Visitor;
    class WeakTypeChecker;

    using Visitable = VisitableTree<Visitor, WeakTypeChecker>;

    /// 第一个指向root_,第二个指向null_
    using Tree = std::tuple<Visitable *, Visitable *>;

    ///********************Visitor*****************
    // 以下产生5个纯虚函数,且返回类型都是void,可通过Visitor::Rtype访问到
    // virtual void visit(Jnode<NodeC::Obj> &)=0;
    // virtual void visit(Jnode<NodeC::Arr> &)=0;
    // virtual void visit(Jnode<NodeC::String> & str)=0;
    // virtual void visit(Jnode<NodeC::Number> & num)=0;
    // virtual void visit(Jnode<NodeC::Keyword> & key)=0;
    // 所以需要在子类手动覆盖
    // 参数第一个是返回类型，剩下的是要访问的类型
    class Visitor : public lsf::BaseVisitor<
                        void, // 返回类型
                        Jnode<NodeC::Obj>,
                        Jnode<NodeC::Arr>,
                        Jnode<NodeC::String>,
                        Jnode<NodeC::Number>,
                        Jnode<NodeC::Keyword>,
                        Jnode<NodeC::Null>>
    {
    public:
        virtual ~Visitor() {}
        virtual void visit_BFS(Tree root, std::function<void()> round_callback);
    };
    ///********************************
    /// 检查对象的重复key，以及数组中每个元素类型是否相同
    ///**********************************
    class WeakTypeChecker : public lsf::BaseVisitor<
                                bool, // 返回类型
                                Jnode<NodeC::Obj>,
                                Jnode<NodeC::Arr>,
                                Jnode<NodeC::String>,
                                Jnode<NodeC::Number>,
                                Jnode<NodeC::Keyword>,
                                Jnode<NodeC::Null>>
    {
    public:
        virtual bool visit(Jnode<NodeC::Obj> &obj) override;
        virtual bool visit(Jnode<NodeC::Arr> &arr) override;
        virtual bool visit(Jnode<NodeC::String> &str) override;
        virtual bool visit(Jnode<NodeC::Number> &num) override;
        virtual bool visit(Jnode<NodeC::Keyword> &key) override;
        virtual bool visit(Jnode<NodeC::Null> &null) override;

    public:
        [[nodiscard]] bool check_type(Tree root);
        bool do_check(std::size_t first, std::size_t another);
        std::string_view get_error();

    private:
        NodeC current_type{NodeC::Error};
        Visitable *null_{};
        std::vector<NodeC> jtype_{};
        std::string error_{};
    };

    class PrintNodes : public Visitor
    {
    public:
        void set_null(Visitable *nul);

    private:
        virtual void visit(Jnode<NodeC::Obj> &) override;
        virtual void visit(Jnode<NodeC::Arr> &) override;
        virtual void visit(Jnode<NodeC::String> &str) override;
        virtual void visit(Jnode<NodeC::Number> &num) override;
        virtual void visit(Jnode<NodeC::Keyword> &key) override;
        virtual void visit(Jnode<NodeC::Null> &null) override;
        Visitable *faken_{};
    };

#define AcceptImp(Visitorx) \
    virtual Visitorx::Rtype accept(Visitorx &v) override{ return v.visit(*this); }

    template <>
    struct VisitableTree<Visitor, WeakTypeChecker> : public TreeNode<VisitableTree<Visitor, WeakTypeChecker>>
    {
        virtual Visitor::Rtype accept(Visitor &v) = 0;
        virtual WeakTypeChecker::Rtype accept(WeakTypeChecker &v) = 0;
        virtual ~VisitableTree()=default;
    };

    template <>
    struct Jnode<NodeC::Obj> : Visitable
    {
        int n_;
        AcceptImp(Visitor)
            AcceptImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::Arr> : Visitable
    {
        int n_;
        AcceptImp(Visitor)
            AcceptImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::String> : Visitable
    {
        std::string data_; /// 对于obj和arr，表示成员数量
        AcceptImp(Visitor)
            AcceptImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::Number> : Visitable
    {
        std::string data_; /// 对于obj和arr，表示成员数量
        AcceptImp(Visitor)
            AcceptImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::Keyword> : Visitable
    {
        bool b_{false};
        AcceptImp(Visitor)
            AcceptImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::Null> : Visitable
    {
        AcceptImp(Visitor)
            AcceptImp(WeakTypeChecker)
    };

    class Treebuilder : public ParserResultBuilder
    {
    public:
        virtual ~Treebuilder();

    protected:
        friend class JsonParser;
        virtual void before_build() override;
        virtual void after_build() override;
        virtual void build_obj() override;
        virtual void build_arr() override;
        virtual void build_string(std::wstring str) override;
        virtual void build_number(std::wstring str) override;
        virtual void build_keyword(std::wstring str) override;
        virtual void build_Null(std::wstring str) override;
        virtual void set_memberkey(std::wstring key) override;
        virtual void build_null_mbr() override;
        virtual void can_start_iteration() override;
        virtual void move_next() override;
        virtual void finish_iteration() override;

    public:
        Tree get_ast();
        void dealloc_node();

    private:
        Visitable *root_{nullptr};
        std::stack<std::tuple<Visitable *, int>> mbr_node_{};
        std::vector<Visitable *> clear_{};
        Visitable *null_{nullptr};
    };

} // namespace lsf

namespace lsf
{
    Treebuilder::~Treebuilder()
    {
        dealloc_node();
    }

    Tree Treebuilder::get_ast()
    {
        return {root_, null_};
    }

    void Treebuilder::before_build()
    {
        dealloc_node();
        root_ = null_ = new Jnode<NodeC::Obj>;
        null_->left_child_ = null_->right_bro_ = null_;
        null_->key_ = "Never used!";
        clear_.push_back(null_);
    }

    void Treebuilder::after_build()
    {
        root_->key_ = "root";
        root_->right_bro_ = root_;
        null_->left_child_ = null_->right_bro_ = null_;
    }

    void Treebuilder::build_obj()
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

    void Treebuilder::build_arr()
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

    void Treebuilder::build_string(std::wstring str)
    {
        auto n = new Jnode<NodeC::String>;
        n->data_ = to_cstring(str);
        n->ele_type_ = NodeC::String;
        n->left_child_ = null_;
        n->right_bro_ = n;
        root_ = n;
        clear_.push_back(root_);
    }

    void Treebuilder::build_number(std::wstring str)
    {
        auto n = new Jnode<NodeC::Number>;
        n->data_ = to_cstring(str);
        n->ele_type_ = NodeC::Number;
        n->left_child_ = null_;
        n->right_bro_ = n;
        root_ = n;
        clear_.push_back(root_);
    }

    void Treebuilder::build_keyword(std::wstring str)
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

    void Treebuilder::build_Null(std::wstring str)
    {
        auto n = new Jnode<NodeC::Null>;
        n->ele_type_ = NodeC::Null;
        n->left_child_ = null_;
        n->right_bro_ = n;
        root_ = n;
        clear_.push_back(root_);
    }

    void Treebuilder::set_memberkey(std::wstring key)
    {
        root_->key_ = to_cstring(key);
    }

    void Treebuilder::build_null_mbr()
    {
        root_ = null_;
        mbr_node_.push({nullptr, 0});
    }

    // 当前已经有一个元素了
    void Treebuilder::can_start_iteration()
    {
        mbr_node_.push({root_, 1}); // 构建完成后正好清空
    }

    void Treebuilder::move_next()
    {
        auto &[current, n] = mbr_node_.top();
        n++;
        root_->right_bro_ = current->right_bro_;
        current->right_bro_ = root_;
        current = root_;
    }

    void Treebuilder::finish_iteration()
    {
        //    root_=root_->right_bro_;
        //    mbr_node_.pop();  //放到arr或obj中清空，因为那时候需要存储的个数
    }

    void Treebuilder::dealloc_node()
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
        std::set<std::string> cset{};
        auto j = obj.left_child_;
        do
        {
            if (!j->get_this()->accept(*this))
                return false;
#if __cplusplus >= 202002L
            auto haskey = cset.contains(j->key_);
#else
            auto haskey = cset.end() != cset.find(j->key_);
#endif
            if (haskey)
            {
                error_ += "重复key_:";
                error_ += j->key_;
                return false;
            }
            cset.insert(j->key_);
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
                error_ += arr.key_;
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

    std::string_view WeakTypeChecker::get_error()
    {
        return error_;
    }

} // namespace lsf
