#ifndef ANALYSE_H
#define ANALYSE_H
#include"constant.h"
#include<stack>
#include<functional>

namespace lsf {//namespace start

struct TreeNode;

enum  class NodeC:char{Obj,Arr,String,Number,Keyword,Null,Error};

template<auto token>
struct Jnode;

class ParserResultBuilder
{
public:
    virtual ~ParserResultBuilder(){}
protected:
    friend class JsonParser;
    friend class R_JsonParser;
    virtual void before_build()=0;
    virtual void after_build()=0;
    virtual void build_obj()=0;
    virtual void build_arr()=0;
    virtual void build_string(std::wstring str)=0;
    virtual void build_number(std::wstring str)=0;
    virtual void build_keyword(std::wstring str)=0;
    virtual void build_Null(std::wstring str)=0;
    virtual void set_memberkey(std::wstring key)=0;
    virtual void build_null_mbr()=0;
    virtual void can_start_iteration()=0;
    virtual void move_next()=0;
    virtual void finish_iteration()=0;
};

/// 第一个指向root_,第二个指向null_
using Tree=std::tuple<TreeNode *,TreeNode *>;

class Treebuilder :public ParserResultBuilder
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
    TreeNode * root_{nullptr};
    std::stack<std::tuple<TreeNode *,int>> mbr_node_{};
    std::vector<TreeNode *> clear_{};
    TreeNode * null_{nullptr};
};

///********************Visitor*****************
//以下产生5个纯虚函数,且返回类型都是void,可通过Visitor::Rtype访问到
//virtual void visit(Jnode<NodeC::Obj> &)=0;
//virtual void visit(Jnode<NodeC::Arr> &)=0;
//virtual void visit(Jnode<NodeC::String> & str)=0;
//virtual void visit(Jnode<NodeC::Number> & num)=0;
//virtual void visit(Jnode<NodeC::Keyword> & key)=0;
//所以需要在子类手动覆盖
//参数第一个是返回类型，剩下的是要访问的类型
class Visitor : public lsf::BaseVisitor<
        void,   //返回类型
        Jnode<NodeC::Obj>,
        Jnode<NodeC::Arr>,
        Jnode<NodeC::String>,
        Jnode<NodeC::Number>,
        Jnode<NodeC::Keyword>,
        Jnode<NodeC::Null>>
{
public:
    virtual ~Visitor(){}
    virtual void visit_BFS(Tree root,std::function<void ()> round_callback);
};
///********************************
/// 检查对象的重复key，以及数组中每个元素类型是否相同
///**********************************
class WeakTypeChecker: public lsf::BaseVisitor<
        bool,   //返回类型
        Jnode<NodeC::Obj>,
        Jnode<NodeC::Arr>,
        Jnode<NodeC::String>,
        Jnode<NodeC::Number>,
        Jnode<NodeC::Keyword>,
        Jnode<NodeC::Null>>
{
public:
    virtual bool visit(Jnode<NodeC::Obj> & obj)override;
    virtual bool visit(Jnode<NodeC::Arr> & arr)override;
    virtual bool visit(Jnode<NodeC::String> & str)override;
    virtual bool visit(Jnode<NodeC::Number> & num)override;
    virtual bool visit(Jnode<NodeC::Keyword> & key)override;
    virtual bool visit(Jnode<NodeC::Null> & null)override;
public:
    [[nodiscard]]
    bool check_type(Tree root);
    bool do_check(std::size_t first, std::size_t another);
    std::string_view get_error();
private:
    NodeC current_type{NodeC::Error};
    TreeNode * null_{};
    std::vector<NodeC> jtype_{};
    std::string error_{};
};

class PrintNodes: public Visitor
{
public:
    void set_null(TreeNode * nul);
private:
    virtual void visit(Jnode<NodeC::Obj> &)override;
    virtual void visit(Jnode<NodeC::Arr> &)override;
    virtual void visit(Jnode<NodeC::String> & str)override;
    virtual void visit(Jnode<NodeC::Number> & num)override;
    virtual void visit(Jnode<NodeC::Keyword> & key)override;
    virtual void visit(Jnode<NodeC::Null> & null)override;
    TreeNode * faken_{};
};

#define AcceptImp virtual Visitor::Rtype accept(Visitor & v){return v.visit(*this);}
#define TypeCheckerImp(TypeChecker) virtual TypeChecker::Rtype accept_check(TypeChecker & v){return v.visit(*this);}

struct TreeNode
{
    TreeNode * left_child_{nullptr};    ///左孩子
    TreeNode * right_bro_{nullptr};     ///右兄弟
    std::string key_;                  ///作为对象成员的key
    NodeC ele_type_{NodeC::Error};
    virtual  Visitor::Rtype accept(Visitor & v)=0;
    virtual WeakTypeChecker::Rtype accept_check(WeakTypeChecker & v)=0;
    virtual ~TreeNode(){}
    void operator delete  ( void* ptr, std::size_t sz );
    void* operator new  ( std::size_t count );
};

template<>
struct Jnode<NodeC::Obj>:TreeNode
{
    int n_;
    AcceptImp
    TypeCheckerImp(WeakTypeChecker)
};

template<>
struct Jnode<NodeC::Arr>:TreeNode
{
    int n_;
    AcceptImp
    TypeCheckerImp(WeakTypeChecker)
};

template<>
struct Jnode<NodeC::String> :TreeNode
{
    std::string data_;                  ///对于obj和arr，表示成员数量
    AcceptImp
    TypeCheckerImp(WeakTypeChecker)
};

template<>
struct Jnode<NodeC::Number> :TreeNode
{
    std::string data_;                  ///对于obj和arr，表示成员数量
    AcceptImp
    TypeCheckerImp(WeakTypeChecker)
};

template<>
struct Jnode<NodeC::Keyword> :TreeNode
{
    bool b_{false};
    AcceptImp
    TypeCheckerImp(WeakTypeChecker)
};

template<>
struct Jnode<NodeC::Null> : TreeNode
{
    AcceptImp
    TypeCheckerImp(WeakTypeChecker)
};

//namespace end
}

#endif // ANALYSE_H
