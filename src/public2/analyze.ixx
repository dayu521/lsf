module;
#include <cassert>
#include <stack>
#include <functional>
#include <string>
#include <vector>
#include <queue>
#include <set>
#include <utility>

export module lsf:analyze;

import :constant;

import :parser_tree;
import :tree_builder;

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

    struct str_ref : std::pair<std::size_t, std::size_t>
    {
        auto begin()
        {
            return std::get<0>(*this);
        }
        auto end()
        {
            return std::get<1>(*this);
        }
    };

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
        virtual ~Visitor() = default;
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
        virtual ~WeakTypeChecker() = default;
        [[nodiscard]] bool check_type(Tree root);
        bool do_check(std::size_t first, std::size_t another);
        std::pair<std::string, std::wstring> get_error();

    private:
        NodeC current_type{NodeC::Error};
        Visitable *null_{};
        std::vector<NodeC> jtype_{};
        std::string error_{};
        std::wstring src_{};
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
    virtual Visitorx::Rtype accept(Visitorx &v) override { return v.visit(*this); }

    using ff = std::function<std::wstring(str_ref)>;

    template <>
    struct VisitableTree<Visitor, WeakTypeChecker> : public TreeNode<VisitableTree<Visitor, WeakTypeChecker>>
    {
        // 用于访问
        virtual Visitor::Rtype accept(Visitor &v) = 0;
        virtual WeakTypeChecker::Rtype accept(WeakTypeChecker &v) = 0;
        virtual ~VisitableTree() = default;

        /// 作为对象成员的key
        // 因为基本上所有
        str_ref key_;
        ff get_ref_str_;
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
        str_ref data_;
        ff get_ref_str_;
        AcceptImp(Visitor)
            AcceptImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::Number> : Visitable
    {
        str_ref data_;
        ff get_ref_str_;
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

    class TreeBuilder : public ParserResultBuilder
    {
    public:
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
        /// Fixme 这里无法自定义析构函数 2023-12-17 当前使用clang 16.0.6
        // ~TreeBuilder() { dealloc_node(); }
        ~TreeBuilder() = default;
        // ~TreeBuilder(){}
    public:
        // TODO 当此函数被调用后,对象就由接收方负责
        Tree get_ast();
        /// BUG
        /// FIXME 销毁时,这里需要手动调用.
        void dealloc_node();

        std::wstring get_ref_str(str_ref sr)
        {
            return str_cache_.substr(sr.first, sr.second);
        }

    private:
        Visitable *root_{nullptr};
        std::stack<std::tuple<Visitable *, int>> mbr_node_{};
        std::vector<Visitable *> clear_{};
        Visitable *null_{nullptr};

        std::wstring str_cache_;
    };

} // namespace lsf
