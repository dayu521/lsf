//包含合适的头文件
#include <lsf/json.h>//当前编译器模块支持不成熟,需要放到第一行

#include <string>
#include <vector>
#include <iostream>
#include <memory>

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

struct People2
{
    std::string dasd;
    int  a2;
    std::vector<int> n;
    // 也可以指定一个key
    JS_OBJECT(JS_MEMBER(dasd), JS_MEMBER(a2,"faa阿萨德撒安抚"), JS_MEMBER(n, "达到loo5"));
};

int t()
{    
    lsf::Json j;
    lsf::SerializeBuilder bu;

    // // 从文件获取json字符串
    // auto res = j.run(std::make_unique<lsf::FileSource>(argv[1]));
    // if (!res)
    // {
    //     std::cout << j.get_errors() << std::endl;
    //     return -1;
    // }
    // //  把解析好的json表示再输出为格式化的json字符串
    // lsf::json_to_string(*res, bu);
    // std::cout << bu.get_jsonstring() << std::endl;

    //  直接获取json字符串
    auto res2 = j.run(std::make_unique<lsf::StrSource>(R"(
        {
            "dasd":"aasdaffa",
            "faa阿萨德撒安抚":5,
            "达到loo5":null
        }
    )"));
    if (!res2)
    {
        std::cout << j.get_errors() << std::endl;
        return -1;
    }
    bu.clear();
    //  把解析好的json表示再输出为格式化的json字符串
    lsf::json_to_string(*res2, bu);
    std::cout <<"读到的json字符串:\n"<< bu.get_jsonstring() << std::endl;

    People2 peo;
    //  把json表示转换成struct
    //  这里,会发现 json中的null不能被很好地表示,
    lsf::json_to_struct(*res2,peo);
    // 不需要所有成员都有对应的json属性
    // lsf::json_to_struct_ignore_absence(*res2,peo);
    bu.clear();
    lsf::struct_to_jsonstr(peo,bu);
    std::cout << bu.get_jsonstring() << std::endl;

    //  把struct转换成json字符串
    People lf = {18, true, {{"dog", 2}, {"duck", 1}, {"cat", 3}}};
    bu.clear();
    lsf::struct_to_jsonstr(lf, bu);
    std::cout << bu.get_jsonstring() << std::endl;
    return 0;
}

int main()
{
    return t();
}