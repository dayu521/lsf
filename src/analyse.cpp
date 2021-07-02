#include<iostream>
#include<queue>
#include<cassert>
#include<algorithm>

#include "analyse.h"
#include "error.h"
#include "inner_imp.h"

namespace lsf {

Treebuilder::~Treebuilder()
{
    dealloc_node();
}

Tree Treebuilder::get_ast()
{
    return {root_,null_};
}

void Treebuilder::before_build()
{
    dealloc_node();
    root_=null_=new Jnode<NodeC::Obj>;
    null_->left_child_=null_->right_bro_=null_;
    null_->key_="Never used!";
    clear_.push_back(null_);
}

void Treebuilder::after_build()
{
    root_->key_ = "root";
    root_->right_bro_=root_;
    null_->left_child_=null_->right_bro_=null_;
}

void Treebuilder::build_obj()
{
    auto n=new Jnode<NodeC::Obj>;
    n->ele_type_=NodeC::Obj;
    n->left_child_=root_;
    n->right_bro_=n;
    n->n_=std::get<1>(mbr_node_.top());
    root_=n;
    mbr_node_.pop();
    clear_.push_back(root_);
}

void Treebuilder::build_arr()
{
    auto n=new Jnode<NodeC::Arr>;
    n->ele_type_=NodeC::Arr;
    n->left_child_=root_;
    n->right_bro_=n;
    n->n_=std::get<1>(mbr_node_.top());
    root_=n;
    mbr_node_.pop();
    clear_.push_back(root_);
}

void Treebuilder::build_string(std::wstring str)
{
    auto n=new Jnode<NodeC::String>; 
    n->data_=to_cstring(str);
    n->ele_type_=NodeC::String;
    n->left_child_=null_;
    n->right_bro_=n;
    root_=n;
    clear_.push_back(root_);
}

void Treebuilder::build_number(std::wstring str)
{
    auto n=new Jnode<NodeC::Number>;
    n->data_=to_cstring(str);
    n->ele_type_=NodeC::Number;
    n->left_child_=null_;
    n->right_bro_=n;
    root_=n;
    clear_.push_back(root_);
}

void Treebuilder::build_keyword(std::wstring str)
{
    auto n=new Jnode<NodeC::Keyword>;
    if(str==L"true")
        n->b_=true;
    n->ele_type_=NodeC::Keyword;
    n->left_child_=null_;
    n->right_bro_=n;
    root_=n;
    clear_.push_back(root_);
}

void Treebuilder::build_Null(std::wstring str)
{
    auto n=new Jnode<NodeC::Null>;
    n->ele_type_=NodeC::Null;
    n->left_child_=null_;
    n->right_bro_=n;
    root_=n;
    clear_.push_back(root_);
}

void Treebuilder::set_memberkey(std::wstring key)
{
    root_->key_=to_cstring(key);
}

void Treebuilder::build_null_mbr()
{
    root_=null_;
    mbr_node_.push({nullptr,0});
}

//当前已经有一个元素了
void Treebuilder::can_start_iteration()
{
    mbr_node_.push({root_,1});//构建完成后正好清空
}

void Treebuilder::move_next()
{
    auto & [current,n]=mbr_node_.top();
    n++;
    root_->right_bro_=current->right_bro_;
    current->right_bro_=root_;
    current=root_;
}

void Treebuilder::finish_iteration()
{
    root_=root_->right_bro_;
//    mbr_node_.pop();  //放到arr或obj中清空，因为那时候需要存储的个数
}

void Treebuilder::dealloc_node()
{
    for(auto & i:clear_){
        delete i;
        i=nullptr;
    }
    clear_.clear();
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
            std::cout<<j->key_<<" ";
            j=j->right_bro_;
        } while (j!=obj.left_child_);
    }
}

void PrintNodes::visit(Jnode<NodeC::Arr> &arr)
{

}

void PrintNodes::visit(Jnode<NodeC::String> &str)
{
    std::cout<<str.data_<<" ";
}

void PrintNodes::visit(Jnode<NodeC::Number> &num)
{
    std::cout<<num.data_<<" ";
}

void PrintNodes::visit(Jnode<NodeC::Keyword> &key_word)
{
    std::cout<<(key_word.b_?"true":"false")<<" ";
}

void PrintNodes::visit(Jnode<NodeC::Null> &null)
{
    std::cout<<"null"<<" ";
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
bool WeakTypeChecker::visit(Jnode<NodeC::Obj> &obj)
{
    current_type=NodeC::Obj;
    //arr begin
    jtype_.push_back(current_type);
    if(obj.left_child_==null_){
        //arr end
        jtype_.push_back(current_type);
        return true;
    }
    std::set<std::string> cset{};
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
            error_+="重复key_:";
            error_+=j->key_;
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

bool WeakTypeChecker::visit(Jnode<NodeC::Arr> &arr)
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
            error_+="数组类型不同:";
            error_+=arr.key_;
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

bool WeakTypeChecker::visit(Jnode<NodeC::String> &str)
{
    current_type=NodeC::String;
    jtype_.push_back(current_type);
    return true;
}

bool WeakTypeChecker::visit(Jnode<NodeC::Number> &num)
{
    current_type=NodeC::Number;
    jtype_.push_back(current_type);
    return true;
}

bool WeakTypeChecker::visit(Jnode<NodeC::Keyword> &key)
{
    current_type=NodeC::Keyword;
    jtype_.push_back(current_type);
    return true;
}

bool WeakTypeChecker::visit(Jnode<NodeC::Null> &null)
{
    current_type=NodeC::Null;
    jtype_.push_back(current_type);
    return true;
}

bool WeakTypeChecker::check_type(Tree roott)
{
    jtype_.clear();
    null_=std::get<1>(roott);
    auto root=std::get<0>(roott);
    return root->accept_check(*this);
}

bool WeakTypeChecker::do_check(std::size_t first, std::size_t another)
{
    auto first_beg=jtype_.begin()+first;
    auto another_beg=jtype_.begin()+another;
    assert(first_beg!=jtype_.end()&&another_beg!=jtype_.end());
    auto r= std::equal(first_beg,another_beg,another_beg,jtype_.end());
    return r;
}

std::string_view WeakTypeChecker::get_error()
{
    return error_;
}

void TreeNode::operator delete(void *ptr, std::size_t sz)
{
    if(sz>Inner::MyAllocator::MaxObjSize)
        ::operator delete(ptr);
    else{
        Inner::get_singleton<Inner::MyAllocator>().deallocate(ptr,sz);
    }
}

void *TreeNode::operator new(std::size_t count)
{
    if(count>Inner::MyAllocator::MaxObjSize)
        return ::operator new(count);
    auto p=Inner::get_singleton<Inner::MyAllocator>().allocate(count);
    if(p==nullptr)
//        throw std::bad_alloc("MyAllocator allocate failed");
        throw std::bad_alloc();
    return p;
}

}
