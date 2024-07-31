#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include"lsf/xx.h"

import lsf;

struct Pet
{
    std::string name;
    int age;
    // 默认key是成员名字的字符串,此处即"age","name"
    // struct成员顺序与json中的obj成员顺序无关，只要它们成员之间一一对应
    JS_OBJECT(JS_MEMBER(age), JS_MEMBER(name));
};

struct People
{
    int age;
    bool is_fat;
    std::vector<Pet> pets;
    // 也可以指定一个key
    JS_OBJECT(JS_MEMBER(age, "age"), JS_MEMBER(is_fat), JS_MEMBER(pets, "friends"));
};

struct People2
{
    std::string dasd;
    int a2;
    std::vector<int> n;
    // 也可以指定一个key
    JS_OBJECT(JS_MEMBER(dasd), JS_MEMBER(a2, "faa阿萨德撒安抚"), JS_MEMBER(n, "达到loo5"));
};

TEST_CASE("Test 1")
{
    People lf = {18, true, {{"dog", 2}, {"duck", 1}, {"cat", 3}}};
    lsf::WriteJsonStr wj;

    lsf::parse_cpp_type(lf,wj);

    std::cout << wj.get_jsonstring() << std::endl;
}