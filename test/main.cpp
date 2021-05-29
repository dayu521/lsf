/*
#include <iostream>
#include "json.h"

using namespace std;
struct Pe
{
    std::string name;
    int age;
    std::vector<int> fucks;
    //默认json的键是成员名的字符串,即"name","age","fucks"
    JS_OBJECT(JS_MEMBER(name),JS_MEMBER(age),JS_MEMBER(fucks));
    //为成员名指定不同的json键
//    JS_OBJECT(JS_MEMBER(name,"xxxkey1"),JS_MEMBER(age,"xxxkey2"),JS_MEMBER(fucks,"xxxkey3"));
};
struct Hellos
{
    double a;
    std::string s;
    bool bs;
    Pe p;
    JS_OBJECT(JS_MEMBER(a,"a"),JS_MEMBER(s),JS_MEMBER(bs,"bs"),JS_MEMBER(p,"p"));
};

int main()
{

    lsf::Json j("2.txt");
    auto ok=j.run([&](auto t,const std::string &s){
        std::cout<<s<<std::endl;
    });

    std::setlocale(LC_ALL,old);
    Hellos lf;
    try {
        lsf::deserialize(lf,j.get_output());
    }  catch (const lsf::DeserializeError &ex) {
        std::cout<<ex.what()<<std::endl;
        return -1;
    }

    lsf::SerializeBuilder bu;
    lsf::serialize(lf,bu);

    std::cout<<bu.get_jsonstring()<<std::endl;

    return 0;
}
*/

#include "json.h"
#include <string>
#include <vector>
#include <iostream>
#include "doctest/doctest.h"

//首先创建自己的结构体
struct Pet
{
    std::string name;
    int age;
    //默认key是成员名字的字符串,此处即"age","name"
    //struct成员顺序与json中的obj成员顺序无关，只要它们成员之间一一对应
    JS_OBJECT(JS_MEMBER(age), JS_MEMBER(name));
};
struct People
{
    int age;
    bool is_fat;
    std::vector<Pet> pets;
    //也可以指定一个key
    JS_OBJECT(JS_MEMBER(age,"age"), JS_MEMBER(is_fat),JS_MEMBER(pets,"friends"));
};

struct Fn
{
    std::vector<People> lp;
    std::string sd;
    Pet pt;
    JS_OBJECT(JS_MEMBER(lp,"太多人"), JS_MEMBER(sd,"蜜汁"),JS_MEMBER(pt,"疲劳"));
};

TEST_CASE("testing json serialization and deserialization")
{
//    People lf={18,true,{{"dog",2},{"duck鸭子",1},{"cat",3}}};
    Fn lf={
        {{18,true,{{"dog",2},{"duck鸭子",1},{"cat",3}}},{},{}},
        "liad",
        {"pl",56}
    };

    lsf::SerializeBuilder bu;
    lsf::serialize(lf,bu);

    std::cout<<bu.get_jsonstring()<<std::endl;

    std::ofstream f("example.txt");
    if(f.is_open()){
        f<<bu.get_jsonstring();
        f.close();
    }

    //从文件读入
    lsf::Json j("example.txt");
    auto ok=j.run([&](auto t,const std::string &s){
        lsf::ErrorType sd=t;
        std::cout<<s<<std::endl;
    });
    CHECK(ok==true);
//    People ml{};
    Fn ml{};
    try {
        lsf::deserialize(ml,j.get_output());
    }  catch (const lsf::DeserializeError &ex) {
        std::cout<<ex.what()<<std::endl;
        throw ex;
    }

}
