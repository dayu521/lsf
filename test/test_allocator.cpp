#include<iostream>
#include"doctest/doctest.h"

#include"inner_imp.h"

using namespace lsf::Inner;
//using namespace Loki;

TEST_CASE("allocator")
{
    FixedAllocator f{sizeof (double)};
//    double* ds=(double *)f.allocate();
    constexpr std::size_t AS=510;
    double * da[AS];
    for(auto i=0;i<AS;i++){
        da[i]=(double *)f.allocate();
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

