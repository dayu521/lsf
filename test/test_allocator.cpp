#include<iostream>
#include"doctest/doctest.h"
#include<thread>

#include"inner_imp.h"

using namespace lsf::Inner;
//using namespace Loki;

TEST_CASE("allocator")
{
    FixedAllocator f{sizeof (int)};
//    double* ds=(double *)f.allocate();
    constexpr std::size_t AS=510;
    int * da[AS];
    for(auto i=0;i<AS;i++){
        da[i]=(int *)f.allocate();
        *(da[i])=i;
    }
//    for(auto i=0;i<AS;i++){
//        f.deallocate(da[i]);
//    }
    for(long i=AS-1;i>=0;i--){
        f.deallocate(da[i]);
    }
//    f.deallocate(ds);
}

constexpr int d[]={1,6,3,55,4,63,25,99,44,11,36,28};

TEST_CASE("binary search tree")
{
    BST<int,int> b{};
    for(auto i=0;i<std::extent_v<decltype (d)>;i++){
        b.insert(d[i],i);
    }
    CHECK(b.remove(28));
    CHECK(!b.remove(28));
    CHECK(b.remove(36));
    CHECK(b.remove(55));
    CHECK(b.remove(44));
    CHECK(!b.remove(0));
}

#include <future>

TEST_CASE("test thread_local singleton")
{
    std::promise<MyAllocator *> p;
    auto f=p.get_future();
    std::thread t([&p]{
//       p.set_value(&SingletonHolder<MyAllocator>::instance());
        get_singleton<MyAllocator>();
    });
    t.detach();
//    CHECK(f.get()==&SingletonHolder<MyAllocator>::instance());
    get_singleton<MyAllocator>();
}

inline thread_local int a=0;

TEST_CASE("test atexit")
{

}
