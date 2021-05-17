#ifndef ANALYSE_H
#define ANALYSE_H
#include"constant.h"
#include<stack>
#include<functional>

namespace lsf {
//namespace start

/// 暂时不考虑使用allocator，因为要自己考虑析构函数调用
//template<typename T>
//class LsfAlloc
//{
//public:
//    static auto Allocate()->decltype (std::allocator<T>())
//    {
//        return std::allocator<T>();
//    }
//    static auto DeAllocate();
//};

//struct LinkNode
//{
//    LinkNode * left_child_{nullptr};
//    LinkNode * right_bro_{nullptr};
//    std::wstring key_;
//    virtual ~LinkNode(){}
//};

struct TreeNode;

enum  class NodeC{Obj,Arr,String,Number,Keyword,None,Error};

template<auto token>
struct Jnode;

/// 第一个指向root_,第二个指向null_
using Tree=std::tuple<TreeNode *,TreeNode *>;

class Treebuilder
{
public:
    Tree get_ast();
protected:
    friend class JsonParser;
    virtual void before_build();
    virtual void after_build();
    virtual void build_obj();
    virtual void build_arr();
    virtual void build_string(std::wstring str);
    virtual void build_number(std::wstring str);
    virtual void build_keyword(std::wstring str);
    virtual void set_memberkey(std::wstring key);
    virtual void build_null_mbr();
    virtual void start_iteration();
    virtual void move_next();
    virtual void finish_iteration();
private:
    TreeNode * root_;
    std::stack<TreeNode *> mbr_node_;
    TreeNode * null_;
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
        Jnode<NodeC::Keyword>>
{
public:
    virtual ~Visitor(){}
    virtual void visit_BFS(Tree root,std::function<void ()> round_callback);
};
///********************************
/// 检查对象的重复key，以及数组中每个元素类型是否相同
///**********************************
class TypeChecker: public lsf::BaseVisitor<
        bool,   //返回类型
        Jnode<NodeC::Obj>,
        Jnode<NodeC::Arr>,
        Jnode<NodeC::String>,
        Jnode<NodeC::Number>,
        Jnode<NodeC::Keyword>>
{
public:
    virtual bool visit(Jnode<NodeC::Obj> & obj)override;
    virtual bool visit(Jnode<NodeC::Arr> & arr)override;
    virtual bool visit(Jnode<NodeC::String> & str)override;
    virtual bool visit(Jnode<NodeC::Number> & num)override;
    virtual bool visit(Jnode<NodeC::Keyword> & key)override;
public:
    bool check_type(Tree root);
    bool do_check(int first,int another);
private:
    NodeC current_type{NodeC::Error};
    TreeNode * null_{};
    std::vector<NodeC> jtype_{};
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
    TreeNode * faken_{};
};

#define AcceptImp virtual Visitor::Rtype accept(Visitor & v){return v.visit(*this);}
#define TypeCheckerImp virtual TypeChecker::Rtype accept_check(TypeChecker & v){return v.visit(*this);}

struct TreeNode
{
    TreeNode * left_child_{nullptr};    ///左孩子
    TreeNode * right_bro_{nullptr};     ///右兄弟
    std::wstring key_;                  ///作为对象成员的key
    NodeC ele_type_{NodeC::Error};
    virtual  Visitor::Rtype accept(Visitor & v)=0;
    virtual TypeChecker::Rtype accept_check(TypeChecker & v)=0;
    virtual ~TreeNode(){}
};

template<>
struct Jnode<NodeC::Obj>:TreeNode
{
    AcceptImp
    TypeCheckerImp
};

template<>
struct Jnode<NodeC::Arr>:TreeNode
{
    AcceptImp
    TypeCheckerImp
};

template<>
struct Jnode<NodeC::String> :TreeNode
{
    std::wstring data_;
    AcceptImp
    TypeCheckerImp
};

template<>
struct Jnode<NodeC::Number> :TreeNode
{
    std::wstring data_;
    AcceptImp
    TypeCheckerImp
};

template<>
struct Jnode<NodeC::Keyword> :TreeNode
{
    std::wstring data_;
    AcceptImp
    TypeCheckerImp
};

//namespace end
}

#endif // ANALYSE_H
