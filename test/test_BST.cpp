#include<iostream>
#include"doctest/doctest.h"
#include"inner_imp.h"

using namespace lsf::Inner;

TEST_CASE("test bst...") {
    MESSAGE("初始化BST数据");
    BST<int,int> t{};
//    const auto &&keys={1,2,3,8,574,89};
    const auto &&keys={1,891,51,23,14,37,28,100,66,58,59,2,6,55,13,9,3,8,574,89};
    for(auto i: keys){
        t.insert(i,i);
    }
    std::vector<std::tuple<int,int>> di_v{};
    std::vector<std::tuple<int,int>> v{};
    REQUIRE(di_v.size()==0);
    REQUIRE(v.size()==0);
    SUBCASE("=======中序遍历========")
    {
        t.set_fun([&v](const int & key,const int & value){
            v.push_back({key,value});
        });
        t.inorder_traversal();

        t.set_fun([&di_v](const int & key,const int & value){
            di_v.push_back({key,value});
        });
        t.inorder_recursion();
        CHECK(v==di_v);
    }
    SUBCASE("========先序遍历=======")
    {
        t.set_fun([&v](const int & key,const int & value){
            v.push_back({key,value});
        });
        t.preorder_traversal();

        t.set_fun([&di_v](const int & key,const int & value){
            di_v.push_back({key,value});
        });
        t.preorder_recursion();
        CHECK(v==di_v);
    }
    SUBCASE("========后序遍历=======")
    {
        t.set_fun([&v](const int & key,const int & value){
            v.push_back({key,value});
        });
        t.postorder_traversal();

        t.set_fun([&di_v](const int & key,const int & value){
            di_v.push_back({key,value});
        });
        t.postorder_recursion();
        CHECK(v==di_v);
    }
}
