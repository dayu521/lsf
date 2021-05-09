#include "analyse.h"
#include "error.h"
#include<iostream>
#include<queue>

namespace lsf {

Tree Treebuilder::get_ast()
{
    return {root_,null_};
}

void Treebuilder::before_build()
{
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
    n->str_=std::move(std::move(str));
    n->left_child_=null_;
    n->right_bro_=n;
    root_=n;
}

void Treebuilder::build_number(std::wstring str)
{
    auto n=new Jnode<NodeC::Number>;
    n->str_repst=std::move(str);
    n->left_child_=null_;
    n->right_bro_=n;
    root_=n;
}

void Treebuilder::build_keyword(std::wstring str)
{
    auto n=new Jnode<NodeC::Keyword>;
    n->v_=std::move(str);
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

void PrintNodes::set_null(TreeNode *nul)
{
    faken_=nul;
}

void PrintNodes::visit(Jnode<NodeC::Obj> & obj)
{
//    std::cout<<lsf::to_cstring(obj.key_)<<" ";
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
    if(obj.left_child_==null_){
        current_type=NodeC::Obj;
        return true;
    }
    std::set<std::wstring> cset{};
    auto j=obj.left_child_;
    do{
        if(!j->accept_check(*this))
            return false;
        if(cset.contains(j->key_)){
            std::cout<<"重复key_:"<<lsf::to_cstring(j->key_)<<std::endl;
            return false;
        }
        cset.insert(j->key_);
        j=j->right_bro_;
    }while(j!=obj.left_child_);
    current_type=NodeC::Obj;
    return true;
}

bool TypeChecker::visit(Jnode<NodeC::Arr> &arr)
{

    if(arr.left_child_==null_){
        current_type=NodeC::Arr;
        return true;
    }

    auto j=arr.left_child_;
    do{
        if(!j->accept_check(*this))
            return false;
        j=j->right_bro_;
    }while(j!=arr.left_child_);
    current_type=NodeC::Arr;
    return true;
}

bool TypeChecker::visit(Jnode<NodeC::String> &str)
{
    current_type=NodeC::String;
    return true;
}

bool TypeChecker::visit(Jnode<NodeC::Number> &num)
{
    current_type=NodeC::Number;
    return true;
}

bool TypeChecker::visit(Jnode<NodeC::Keyword> &key)
{
    current_type=NodeC::Keyword;
    return true;
}

bool TypeChecker::check_type(Tree roott)
{
//    cur_node_=root;
    null_=std::get<1>(roott);
    auto root=std::get<0>(roott);
    return root->accept_check(*this);
}

bool TypeChecker::advance_and_check(TreeNode *one, TreeNode *another)
{
    return false;
}

}
