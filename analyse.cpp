#include "analyse.h"
#include "error.h"
#include<iostream>
#include<queue>
#include<cassert>
#include<algorithm>

namespace lsf {

Treebuilder::~Treebuilder()
{
    if(root_!=nullptr&&null_!=nullptr)
        dealloc_node();
}

Tree Treebuilder::get_ast()
{
    return {root_,null_};
}

void Treebuilder::before_build()
{
    if(root_!=nullptr&&null_!=nullptr)
        dealloc_node();
    root_=null_=new Jnode<NodeC::Obj>;
    null_->left_child_=null_->right_bro_=null_;
    null_->key_=L"Never used!";
}

void Treebuilder::after_build()
{
    root_->key_ = L"\"root\"";
    root_->right_bro_=root_;
    null_->left_child_=null_->right_bro_=null_;
}

void Treebuilder::build_obj()
{
    auto n=new Jnode<NodeC::Obj>;
    n->left_child_=root_;
    n->right_bro_=n;
    root_=n;
}

void Treebuilder::build_arr()
{
    auto n=new Jnode<NodeC::Arr>;
    n->left_child_=root_;
    n->right_bro_=n;
    root_=n;
}

void Treebuilder::build_string(std::wstring str)
{
    auto n=new Jnode<NodeC::String>;
    n->data_=std::move(std::move(str));
    n->ele_type_=NodeC::String;
    n->left_child_=null_;
    n->right_bro_=n;
    root_=n;
}

void Treebuilder::build_number(std::wstring str)
{
    auto n=new Jnode<NodeC::Number>;
    n->data_=std::move(str);
    n->ele_type_=NodeC::Number;
    n->left_child_=null_;
    n->right_bro_=n;
    root_=n;
}

void Treebuilder::build_keyword(std::wstring str)
{
    auto n=new Jnode<NodeC::Keyword>;
    n->data_=std::move(str);
    n->ele_type_=NodeC::Keyword;
    n->left_child_=null_;
    n->right_bro_=n;
    root_=n;
}

void Treebuilder::set_memberkey(std::wstring key)
{
    root_->key_=key;
}

void Treebuilder::build_null_mbr()
{
    root_=null_;
}

void Treebuilder::start_iteration()
{
    mbr_node_.push(root_);//构建完成后正好清空
}

void Treebuilder::move_next()
{
    root_->right_bro_=mbr_node_.top()->right_bro_;
    mbr_node_.top()->right_bro_=root_;
    mbr_node_.top()=root_;
}

void Treebuilder::finish_iteration()
{
    root_=root_->right_bro_;
    mbr_node_.pop();
}

void Treebuilder::dealloc_node()
{
    assert(root_!=nullptr&&null_!=nullptr);
    std::queue< TreeNode*> c{};
    auto root=root_;
    c.push(root);
    while (!c.empty()) {
        auto i=c.front();
        c.pop();

        if(i->left_child_!=null_){
            auto j=i->left_child_;
            do {
                c.push(j);
                j=j->right_bro_;
            } while (j!=i->left_child_);
        }
        delete i;
        i=nullptr;
    }
    delete null_;
    root_=null_=nullptr;
}

void PrintNodes::set_null(TreeNode *nul)
{
    faken_=nul;
}

void PrintNodes::visit(Jnode<NodeC::Obj> & obj)
{
    [[likely]]if(obj.left_child_!=faken_){
        auto j=obj.left_child_;
        do {
            std::cout<<lsf::to_cstring(j->key_)<<" ";
            j=j->right_bro_;
        } while (j!=obj.left_child_);
    }
}

void PrintNodes::visit(Jnode<NodeC::Arr> &arr)
{

}

void PrintNodes::visit(Jnode<NodeC::String> &str)
{
    std::cout<<lsf::to_cstring(str.data_)<<" ";
}

void PrintNodes::visit(Jnode<NodeC::Number> &num)
{
    std::cout<<lsf::to_cstring(num.data_)<<" ";
}

void PrintNodes::visit(Jnode<NodeC::Keyword> &key)
{
    std::cout<<lsf::to_cstring(key.data_)<<" ";
}

void Visitor::visit_BFS(Tree roott, std::function<void ()> round_callback)
{
    std::queue< TreeNode*> c{};
    auto [root,faken]=roott;
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
            round_callback();
        }
    }
}

///*************TypeChecker*************
bool TypeChecker::visit(Jnode<NodeC::Obj> &obj)
{
    current_type=NodeC::Obj;
    //arr begin
    jtype_.push_back(current_type);
    if(obj.left_child_==null_){
        //arr end
        jtype_.push_back(current_type);
        return true;
    }
    std::set<std::wstring> cset{};
    auto j=obj.left_child_;
    do{
        if(!j->accept_check(*this))
            return false;
#if __cplusplus >= 202002L
        auto haskey=cset.contains(j->key_);
#else
        auto haskey=cset.end()!=cset.find(j->key_);
#endif
        if(haskey){
            std::cout<<"重复key_:"<<lsf::to_cstring(j->key_)<<std::endl;
            return false;
        }
        cset.insert(j->key_);
        j=j->right_bro_;
    }while(j!=obj.left_child_);
    //因为是共享的,所以需要重新赋值.其他地方类似
    current_type=NodeC::Obj;
    //end
    jtype_.push_back(current_type);
    return true;
}

bool TypeChecker::visit(Jnode<NodeC::Arr> &arr)
{
    current_type=NodeC::Arr;
    //arr begin
    jtype_.push_back(current_type);
    if(arr.left_child_==null_){
        //arr end
        jtype_.push_back(current_type);
        return true;
    }
    auto first_beg=jtype_.size();
    auto j=arr.left_child_;
    if(!j->accept_check(*this))
        return false;
    auto another_beg=jtype_.size();
    j=j->right_bro_;
    while (j!=arr.left_child_) {
        if(!j->accept_check(*this)||!do_check(first_beg,another_beg)){
            std::cout<<"数组类型不同:"<<lsf::to_cstring(arr.key_)<<std::endl;
            return false;
        }
        jtype_.resize(another_beg);
        j=j->right_bro_;
    }
    current_type=NodeC::Arr;
    //end
    jtype_.push_back(current_type);
    return true;
}

bool TypeChecker::visit(Jnode<NodeC::String> &str)
{
    current_type=NodeC::String;
    jtype_.push_back(current_type);
    return true;
}

bool TypeChecker::visit(Jnode<NodeC::Number> &num)
{
    current_type=NodeC::Number;
    jtype_.push_back(current_type);
    return true;
}

bool TypeChecker::visit(Jnode<NodeC::Keyword> &key)
{
    current_type=NodeC::Keyword;
    jtype_.push_back(current_type);
    return true;
}

bool TypeChecker::check_type(Tree roott)
{
    jtype_.clear();
    null_=std::get<1>(roott);
    auto root=std::get<0>(roott);
    return root->accept_check(*this);
}

bool TypeChecker::do_check(int first, int another)
{
    auto first_beg=jtype_.begin()+first;
    auto another_beg=jtype_.begin()+another;
    assert(first_beg!=jtype_.end()&&another_beg!=jtype_.end());
    auto r= std::equal(first_beg,another_beg,another_beg,jtype_.end());
    return r;
}

}
