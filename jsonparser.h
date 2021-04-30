#ifndef JSONPARSER_H
#define JSONPARSER_H

#include<memory>
#include<functional>
#include"constant.h"

namespace lsf {

struct Token;

class PError;

struct TreeNode;

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
    std::function<void ()> next_;
    std::function<const Token & ()> current_;
};

class ParserError: public lsf::BaseError
{
public:
    using BaseError::BaseError;
};

class JsonParser
{
public:
    JsonParser(GenToken gen);
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

    //检查重复key
    bool check_obj_key();
    //检查数组元素类型是否相同
    bool check_arr();
private:
    GenToken gen_;
    std::vector<lsf::Type> expect_array_;
    TreeNode * root_;
    TreeNode * fake_n;
};

/// 内部表示

enum  class NodeC{Obj,Arr,String,Number,Keyword};

class Visitor;

struct TreeNode
{
    TreeNode * left_child_{nullptr};
    TreeNode * right_bro_{nullptr};
    //在array中表示数量，类型是int
    //在obj或keyword中表示key,类型是std::string
    //在number中表示数值,类型是double
    std::wstring key_;
    virtual ~TreeNode(){}
    virtual void accept(Visitor & v)=0;
};

template<auto token>
struct Jnode;

class Visitor : public lsf::BaseVisitor<
        Jnode<NodeC::Obj>,
        Jnode<NodeC::Arr>,
        Jnode<NodeC::String>,
        Jnode<NodeC::Number>,
        Jnode<NodeC::Keyword>>
{
public:
    virtual ~Visitor(){}
};

#define AcceptImp virtual void accept(Visitor & v){v.visit(*this);}

template<>
struct Jnode<NodeC::Obj>:TreeNode
{
    //key保存在key_中
    AcceptImp
};

template<>
struct Jnode<NodeC::Arr>:TreeNode
{
    //key保存在key_中
    AcceptImp
};

template<>
struct Jnode<NodeC::String> :TreeNode
{
    //保存在str_中
    std::wstring str_;
    AcceptImp
};

template<>
struct Jnode<NodeC::Number> :TreeNode
{
    //很多时候，可能会把整数修改为浮点数
    double number_;
    //保存字符串表示在str_repst中
    std::wstring str_repst;
    AcceptImp
};

template<>
struct Jnode<NodeC::Keyword> :TreeNode
{
    //保存在v_中
    std::wstring v_;
    AcceptImp
};

class PrintNodes: public Visitor
{
public:
    virtual void visit(Jnode<NodeC::Obj> &)override;
    virtual void visit(Jnode<NodeC::Arr> &)override;
    virtual void visit(Jnode<NodeC::String> & str)override;
    virtual void visit(Jnode<NodeC::Number> & num)override;
    virtual void visit(Jnode<NodeC::Keyword> & key)override;
public:
    void v(TreeNode *root,TreeNode * faken);
};

//namespace end
}
#endif // JSONPARSER_H
