#include <iostream>
#include <fstream>
#include <memory>

/* 根本原因就是当前库未实现make_shared.
 * 根据大佬推测，库只是分配了一个指向数组的指针，所以当把这个这个指针作为数组读写时，
 * 造成之后其他对象分配的数据空间遭到破坏.于是错误反而在其他对象的数据析构时才发现，
 * 于是可能会被误导为其他对象出了问题
 *
 * 总结:要看各个标准库发行文档以及注记
 * libstc++文档中写了未实现
 *      https://gcc.gnu.org/onlinedocs/libstdc++/manual/status.html#status.iso.2017
 * 相关提案P0674R1
 *      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0674r1.html
 */
//    buff_(std::make_shared<wchar_t [2*MaxTokenLen]>())
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
