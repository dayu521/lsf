#include <cassert>
#include "jsonparser.h"
#include "lexer.h"
#include <queue>
#include <iostream>
#include "error.h"

namespace lsf {

JsonParser::JsonParser(GenToken gen):gen_(gen),
    expect_array_{}
{
    root_=fake_n=new Jnode<NodeC::Obj>;
    fake_n->left_child_=fake_n->right_bro_=root_;
    fake_n->key_=L"Never used!";
}

bool JsonParser::parser()
{
    assert(gen_.next_||gen_.current_);
    denodes(root_);
    expect_array_.clear();//之后push_back或assign都一样
    gen_.next_();
    return json();
}

TreeNode *JsonParser::get_ast()
{
    return root_;
}

const std::vector<Type> &JsonParser::get_expect_token()const
{
    return expect_array_;
}

//  json -> element
//  json.node=element.node
bool JsonParser::json()
{
    if(element())
        if(isTerminator(TType::END)){
            root_->key_=L"\"root\"";
            return true;
        }
    return false;
}

//element -> unuse value unuse
//element.node=value.node
bool JsonParser::element()
{
    unuse();
    if(value())
        return unuse();
    return false;
}

//value -> obj | array | String | Number | KeyWord
//value.node=obj.node,array.node
//value.node=new Jnode<string,number,keyword>;
bool JsonParser::value()
{
    switch (gen_.current_().type_) {
    case Type::LBRACE:
        return obj();
    case Type::LSQUARE:
        return array();
    case Type::String:{
        auto n=new Jnode<NodeC::String>;
        n->str_=std::move(gen_.current_().value_);
        n->left_child_=fake_n;
        n->right_bro_=n;
        root_=n;
        gen_.next_();
        return true;
    }
    case Type::Number:{
        auto n=new Jnode<NodeC::Number>;
        n->str_repst=std::move(gen_.current_().value_);
        n->left_child_=fake_n;
        n->right_bro_=n;
        root_=n;
        gen_.next_();
        return true;
    }
    case Type::KeyWord:{
        auto n=new Jnode<NodeC::Keyword>;
        n->v_=std::move(gen_.current_().value_);
        n->left_child_=fake_n;
        n->right_bro_=n;
        root_=n;
        gen_.next_();
        return true;
    }
    default:
        expect_array_.assign({Type::String,Type::Number,Type::KeyWord});
        return false;
    }
}

//obj -> '{' mb_ws '}'
//obj.node=new Jnode<obj>
//obj.node.left=mb_ws.node
//obj.node.right=obj.node
bool JsonParser::obj()
{
    if(isTerminator(TType::LBRACE)){
        gen_.next_();
        if(mb_ws()){
            if(isTerminator(TType::RBRACE)){
                gen_.next_();
                auto n=new Jnode<NodeC::Obj>;
                n->left_child_=root_;
                n->right_bro_=n;
                root_=n;
                return true;
            }
            return false;
        }
        return false;
    }
    expect_array_.push_back(TType::LBRACE);
    return false;
}

//mb_ws -> unuse mb_ws_r
//mb_ws.node=mb_ws_r.node
bool JsonParser::mb_ws()
{
    unuse();
    return mb_ws_r();
}

//mb_ws_r -> String unuse ':' element memberL | e
//element.node.key=string
//memberL.node=element.node+memberL.node
//mb_ws_r.node=memberL.node
bool JsonParser::mb_ws_r()
{
    if(isTerminator(TType::String)){
        auto key=std::move(gen_.current_().value_);
        gen_.next_();
        unuse();
        if(isTerminator(TType::COLON)){
            gen_.next_();
            if(element()){
                root_->key_=key;
                root_->right_bro_=root_;
                return memberL();
            }
            else
                return false;
        }
        expect_array_.push_back(TType::COLON);
        return false;
    }else if (isTerminator(TType::RBRACE)) {
        //root_
        return true;
    }else{
         expect_array_.push_back(TType::String);
         expect_array_.push_back(TType::RBRACE);
        return false;
    }
}

//memberL -> ',' member memberL | e
//memberL.node=member.node+memberL.node
bool JsonParser::memberL()
{
    if(isTerminator(TType::RBRACE)){
        return true;
    }
    auto m=root_;
    while (isTerminator(TType::COMMA)) {
        gen_.next_();
        if(member()){
            root_->right_bro_=m->right_bro_;
            m->right_bro_=root_;
            m=root_;
            if(isTerminator(TType::RBRACE)){
                root_=root_->right_bro_;
                return true;
            }
            continue;
        }
        return false;
    }
    expect_array_.push_back(TType::RBRACE);
    expect_array_.push_back(TType::COMMA);
    return false;
}

//member -> unuse String unuse ':' element
//member.node=element.node
//member.node.key=String
bool JsonParser::member()
{
    unuse();
    if(isTerminator(TType::String)){
        auto key=std::move(gen_.current_().value_);
        gen_.next_();
        unuse();
        if(isTerminator(TType::COLON)){
            gen_.next_();
            if(element()){
                root_->key_=key;
                return true;
            }
            return false;
        }
        expect_array_.push_back(TType::COLON);
        return false;
    }
    expect_array_.push_back(TType::String);
    return false;
}

//array -> '[' arr_ws ']'
//array.node=new Jnode<arr>
//array.node.right=array.node
//array.node.left=arr_ws.node
bool JsonParser::array()
{
    if(isTerminator(TType::LSQUARE)){
        gen_.next_();
        if(arr_ws()){
            if(isTerminator(TType::RSQUARE)){
                gen_.next_();
                auto n=new Jnode<NodeC::Arr>;
                n->left_child_=root_;
                n->right_bro_=n;
                root_=n;
                return true;
            }
            expect_array_.push_back(TType::RSQUARE);
            return false;
        }
        return false;
    }
    expect_array_.push_back(TType::LSQUARE);
    return false;
}

//arr_ws -> unuse arr_ws_r
//arr_ws.node=arr_ws_r.node
bool JsonParser::arr_ws()
{
    unuse();
    return arr_ws_r();
}

//arr_ws_r -> value unuse elementsL | e
//arr_ws_r.node=value.node+elementsL.node
bool JsonParser::arr_ws_r()
{
    switch (gen_.current_().type_) {
    case TType::LBRACE:
    case TType::LSQUARE:
    case TType::String:
    case TType::Number:
    case TType::KeyWord:{
        if(value()){
            unuse();
            return elementsL();
        }
        return false;
    }
    case TType::RSQUARE:
        //
        return true;
    default:
        expect_array_.assign({TType::RSQUARE});
        return false;
    }
}

//elementsL -> ',' element elementsL | e
//elementsL.node=element.node+elementsL.node
bool JsonParser::elementsL()
{
    if(isTerminator(Type::RSQUARE))
        return true;
    auto e=root_;
    while(isTerminator(Type::COMMA)){
        gen_.next_();
        if(element()){
            root_->right_bro_=e->right_bro_;
            e->right_bro_=root_;
            e=root_;
            if(isTerminator(Type::RSQUARE)){
                root_=root_->right_bro_;
                return true;
            }
            continue;
        }
        return false;
    }
    expect_array_.push_back(Type::RSQUARE);
    expect_array_.push_back(Type::COMMA);
    return false;
}

//unuse -> wc unuse | e
//wc -> WhiteSpace | Comment
bool JsonParser::unuse()
{
    while (isTerminator(TType::WHITESPACE)||isTerminator(TType::Comment)) {
        gen_.next_();
    }
    return true;
}

bool JsonParser::isTerminator(JsonParser::TType type)
{
    return gen_.current_().type_==type;
}

void JsonParser::denodes(TreeNode *root_)
{
//    if(root_!=fake_n){
//        denodes(root_)
//    }
}

void PrintNodes::visit(Jnode<NodeC::Obj> & obj)
{
//    std::cout<<lsf::to_cstring(obj.key_)<<" ";
    [[likely]]if(obj.left_child_!=faken){
        auto j=obj.left_child_;
        do {
            std::cout<<lsf::to_cstring(j->key_)<<" ";
            j=j->right_bro_;
        } while (j!=obj.left_child_);
    }
}

void PrintNodes::visit(Jnode<NodeC::Arr> &arr)
{
//    std::cout<<lsf::to_cstring(arr.key_)<<" ";
//    if(arr.left_child_!=faken){
//        auto j=arr.left_child_;
//        do {
//            j->accept(*this);
//            j=j->right_bro_;
//        } while (j!=arr.left_child_);
//    }
}

void PrintNodes::visit(Jnode<NodeC::String> &str)
{
    //<<lsf::to_cstring(str.key_)<<":"
    std::cout<<lsf::to_cstring(str.str_)<<" ";
}

void PrintNodes::visit(Jnode<NodeC::Number> &num)
{
    //<<lsf::to_cstring(num.key_)<<":"
    std::cout<<lsf::to_cstring(num.str_repst)<<" ";
}

void PrintNodes::visit(Jnode<NodeC::Keyword> &key)
{
    //<<lsf::to_cstring(key.key_)<<":"
    std::cout<<lsf::to_cstring(key.v_)<<" ";
}

void Visitor::visit_BFS(TreeNode *root, TreeNode * faken, std::function<void ()> round)
{
    std::queue< TreeNode*> c{};
    c.push(root);
    int x=1,y=0;
    while (!c.empty()) {
        auto i=c.front();
        c.pop();
        x--;
        i->accept(*this);
        if(i->left_child_!=faken){
            auto j=i->left_child_;
            do {
                y++;
                c.push(j);
                j=j->right_bro_;
            } while (j!=i->left_child_);
        }
        if(x==0){
            x=y;
            y=0;
            round();
        }
    }
}

void TypeChecker::check_type()
{

}

}
