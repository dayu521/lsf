#ifndef JSONPARSER_H
#define JSONPARSER_H

#include<memory>
#include<functional>
#include<set>
#include"constant.h"

namespace lsf {

struct Token;

class PError;

struct TreeNode;

class Treebuilder;

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

struct GenToken
{
    virtual ~GenToken(){}
    virtual void next_()=0;
    virtual Token & current_()=0;
};

class ParserError: public lsf::BaseError
{
public:
    using BaseError::BaseError;
};

class JsonParser
{
public:
    JsonParser(std::unique_ptr<GenToken> gen);
    bool parser();
    TreeNode * get_ast();
    TreeNode * get_faken()const{return fake_n;}
    const std::vector<lsf::Type> & get_expect_token()const;
private:
    using TType=lsf::Type;
    bool json();
    bool element();
    bool value();
    bool obj();
    bool mb_ws();
    bool mb_ws_r();
    bool memberL();
    bool member();
    bool array();
    bool arr_ws();
    bool arr_ws_r();
    bool elementsL();
    bool unuse();

    bool isTerminator(TType type);

    void denodes(TreeNode * root_);

private:
    std::unique_ptr<GenToken> gen_;
    std::vector<lsf::Type> expect_array_;
//    std::unique_ptr<Treebuilder> builder_;
    TreeNode * root_;
    TreeNode * fake_n;
};

/// 内部表示

enum  class NodeC{Obj,Arr,String,Number,Keyword,None,Error};

class Visitor;

template<auto token>
struct Jnode;

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
    bool check_type(TreeNode * root ,TreeNode * faken);
private:
    NodeC current_type{NodeC::Error};
//    TreeNode * cur_node_;
    TreeNode * faken_{};
    std::set<std::wstring> set_;   //检查对象成员是否存在相同key
};

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
    virtual void visit_BFS(TreeNode *root,TreeNode * faken,std::function<void ()> round);
};

#define AcceptImp virtual Visitor::Rtype accept(Visitor & v){return v.visit(*this);}
#define TypeCheckerImp virtual TypeChecker::Rtype accept_check(TypeChecker & v){return v.visit(*this);}

enum class TypeState:char{Uncheck,TypeOk,Error};

struct TreeNode
{
    TreeNode * left_child_{nullptr};
    TreeNode * right_bro_{nullptr};

    std::wstring key_;
    TypeState check_;
    virtual ~TreeNode(){}
    virtual  Visitor::Rtype accept(Visitor & v)=0;
    virtual  TypeChecker::Rtype accept_check(TypeChecker & v)=0;
};

template<>
struct Jnode<NodeC::Obj>:TreeNode
{
    //key保存在key_中
    AcceptImp
    TypeCheckerImp
};

template<>
struct Jnode<NodeC::Arr>:TreeNode
{
    //key保存在key_中
    AcceptImp
    TypeCheckerImp
};

template<>
struct Jnode<NodeC::String> :TreeNode
{
    //保存在str_中
    std::wstring str_;
    AcceptImp
    TypeCheckerImp
};

template<>
struct Jnode<NodeC::Number> :TreeNode
{
    //很多时候，可能会把整数修改为浮点数
    double number_;
    //保存字符串表示在str_repst中
    std::wstring str_repst;
    AcceptImp
    TypeCheckerImp
};

template<>
struct Jnode<NodeC::Keyword> :TreeNode
{
    //保存在v_中
    std::wstring v_;
    AcceptImp
    TypeCheckerImp
};

class PrintNodes: public Visitor
{
public:
    virtual void visit(Jnode<NodeC::Obj> &)override;
    virtual void visit(Jnode<NodeC::Arr> &)override;
    virtual void visit(Jnode<NodeC::String> & str)override;
    virtual void visit(Jnode<NodeC::Number> & num)override;
    virtual void visit(Jnode<NodeC::Keyword> & key)override;
    TreeNode * faken{};
};

//namespace end
}
#endif // JSONPARSER_H
