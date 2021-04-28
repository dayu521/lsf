#ifndef JSONPARSER_H
#define JSONPARSER_H

#include<memory>
#include<functional>
#include"constant.h"

namespace lsf {

struct Token;

class PError;

enum  NodeCategory{Obj,Arr,String,Number,Keyword};

class Visitor;

struct TreeNode
{
    virtual ~TreeNode(){}
    virtual void accept(Visitor & v)=0;
    TreeNode * left_child_{nullptr};
    TreeNode * right_bro_{nullptr};
};

template<auto token>
struct Jnode;

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

    //检查重复key
    bool check_obj_key();
    //检查数组元素类型是否相同
    bool check_arr();
private:
    GenToken gen_;
    std::vector<lsf::Type> expect_array_;
};

class Visitor : public lsf::BaseVisitor<Jnode<Obj>,Jnode<Arr>,Jnode<String>,Jnode<Number>>
{
public:
    virtual ~Visitor(){}
};

#define AcceptImp virtual void accept(Visitor & v){v.visit(*this);}

template<>
struct Jnode<Obj>:TreeNode
{
    std::string key_;
    AcceptImp
};

template<>
struct Jnode<Arr>:TreeNode
{
    std::string key_;
    AcceptImp
};

class Generator
{

};

//namespace end
}
#endif // JSONPARSER_H
