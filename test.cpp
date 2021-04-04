#include <iostream>
#include <fstream>
#include <memory>

void test_shared_ptr()
{
    constexpr int bb=1024;
//    std::shared_ptr<wchar_t [bb]> spp=std::make_shared<wchar_t [bb]>();
//    std::shared_ptr<wchar_t [bb]> spp=std::make_shared<wchar_t [bb]>(bb);
//    std::shared_ptr<wchar_t [bb]> spp(new wchar_t[bb],[](auto p){delete [] p;});
    std::shared_ptr<wchar_t []> spp=std::make_shared<wchar_t []>(bb);

    /**********/
//    std::wifstream f("文本文件.txt");
//    if(f.is_open()){
//        f.read(spp.get(),bb);
//    }
    /**********/

    //下面和上面任意一处都会造成崩溃

    /*******/
    auto p=new int[512];
    for(int i=0;i<bb;i++)
        spp[i]=i;
    delete [] p;
    /********/
}
