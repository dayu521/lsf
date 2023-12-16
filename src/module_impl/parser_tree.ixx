module;

module lsf:parser_tree;

namespace lsf
{
    enum class NodeC : char
    {
        Obj,
        Arr,
        String,
        Number,
        Keyword,
        Null,
        Error
    };

    struct TreeNode
    {
        TreeNode *left_child_{nullptr}; /// 左孩子
        TreeNode *right_bro_{nullptr};  /// 右兄弟
        std::string key_;               /// 作为对象成员的key
        NodeC ele_type_{NodeC::Error};
        virtual Visitor::Rtype accept(Visitor &v) = 0;
        virtual WeakTypeChecker::Rtype accept_check(WeakTypeChecker &v) = 0;
        virtual ~TreeNode() {}
        void operator delete(void *ptr, std::size_t sz);
        void *operator new(std::size_t count);
    };

    template <auto token>
    struct Jnode;

#define AcceptImp \
    virtual Visitor::Rtype accept(Visitor &v) { return v.visit(*this); }
#define TypeCheckerImp(TypeChecker) \
    virtual TypeChecker::Rtype accept_check(TypeChecker &v) { return v.visit(*this); }

    template <>
    struct Jnode<NodeC::Obj> : TreeNode
    {
        int n_;
        AcceptImp
        TypeCheckerImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::Arr> : TreeNode
    {
        int n_;
        AcceptImp
        TypeCheckerImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::String> : TreeNode
    {
        std::string data_; /// 对于obj和arr，表示成员数量
        AcceptImp
        TypeCheckerImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::Number> : TreeNode
    {
        std::string data_; /// 对于obj和arr，表示成员数量
        AcceptImp
        TypeCheckerImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::Keyword> : TreeNode
    {
        bool b_{false};
        AcceptImp
        TypeCheckerImp(WeakTypeChecker)
    };

    template <>
    struct Jnode<NodeC::Null> : TreeNode
    {
        AcceptImp
        TypeCheckerImp(WeakTypeChecker)
    };

    class BuilderInterface
    {
    public:
        virtual ~BuilderInterface() {}

    protected:
        friend class JsonParser;
        friend class R_JsonParser;
        virtual void before_build() = 0;
        virtual void after_build() = 0;
        virtual void build_obj() = 0;
        virtual void build_arr() = 0;
        virtual void build_string(std::wstring str) = 0;
        virtual void build_number(std::wstring str) = 0;
        virtual void build_keyword(std::wstring str) = 0;
        virtual void build_Null(std::wstring str) = 0;
        virtual void set_memberkey(std::wstring key) = 0;
        virtual void build_null_mbr() = 0;
        virtual void can_start_iteration() = 0;
        virtual void move_next() = 0;
        virtual void finish_iteration() = 0;
    };

    /// 第一个指向root_,第二个指向null_
    using Tree = std::tuple<TreeNode *, TreeNode *>;

    class Treebuilder : public BuilderInterface
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
        TreeNode *root_{nullptr};
        std::stack<std::tuple<TreeNode *, int>> mbr_node_{};
        std::vector<TreeNode *> clear_{};
        TreeNode *null_{nullptr};
    };

} // namespace lsf
