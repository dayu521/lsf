### json 解析器

解析json文件,反序列化到c++结构;以及从c++结构序列化到json.用到了c++20标准([module](https://en.cppreference.com/w/cpp/language/modules)).

感谢[json_struct](https://github.com/jorgen/json_struct)与[cista](https://github.com/felixguendling/cista)提供的序列化实践.
感谢[dcotest](https://github.com/onqtam/doctest).

### 编译(c++20以上)

---

cmake:

gcc13

> 当前[cmake3.28](https://cmake.org/cmake/help/v3.28/manual/cmake-cxxmodules.7.html) 提供了module支持,它需要gcc14,但我并未编译通过.

clang16

```bash
    cd <project-path>
    mkdir build
    cd build 
    CXX=clang++ CC=clang cmake -GNinja ..
    cmake --build . -v
```
---

xmake:

clang16

```bash
    cd <project-path>
    xmake c -va 
    xmake f -m debug --toolchain=clang
    xmake -rvDw
```
### 使用(c++20)
#### 1.xmake

如果你的项目是通过xmake进行构建的

在你的工程目录下clone当前项目

然后在你的xmake.lua工程文件中引用即可

```lua
includes("lsf")
target("your project name")
    add_deps("lsf")
    add_includedirs("lsf/src/public")
```

#### 例子代码 ####

```cpp
//包含合适的头文件
#include "json.h"
#include <string>
#include <vector>
#include <iostream>

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
#if __cpp_modules
    
    lsf::Json j;
    lsf::SerializeBuilder bu;

    // 从文件获取json字符串
    auto res = j.run(std::make_unique<lsf::FileSource>(argv[1]));
    if (!res)
    {
        std::cout << j.get_errors() << std::endl;
        return -1;
    }
    //  把解析好的json表示再输出为格式化的json字符串
    lsf::json_to_string(*res, bu);
    std::cout << bu.get_jsonstring() << std::endl;

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
    std::cout << bu.get_jsonstring() << std::endl;

    //  把json表示转换成struct
    //  这里,会发现 json中的null不能被很好地表示,
    People2 peo;
    lsf::json_to_struct(*res2,peo);

    //  把struct转换成json字符串
    People lf = {18, true, {{"dog", 2}, {"duck", 1}, {"cat", 3}}};
    bu.clear();
    lsf::struct_to_jsonstr(peo, bu);
    std::cout << bu.get_jsonstring() << std::endl;
#else
   return 0;
#endif
}

int main()
{
    return t();
}
```

#### 说明 ####

目前支持:

* `std::string`   <->json string
* `double`  <->json number
* `int`  <->json number
* `bool`   <-> json true false
* `std::vector`  <-> json array
* 指针(不建议使用) <->json object | null
* 普通struct(聚合类型aggregate)  <->json object

如果需要支持其他**基本类型**(非std::vector和普通struct类型)，需要特化:

```cpp
//从TreeNode中反序列化到类型T
template<typename T>
void lsf::Deserialize(T & s,const TreeNode * t);

//序列化T类型到json字符串
template<typename T>
void lsf::write_value(const T & v,SerializeBuilder & builder);
```

#### 问题 ####

- 不处理字符编码问题，因为实在搞不定(因为采用了宽字符,并且默认与当前系统字符编码一致,但愿一般不会出问题- -)
- 不支持继承
- 保留空白.标准json不接受注释,但也是保留c和c++风格的注释(留下了支持的接口)
- 详细文档都会开放,其实也没什么,大家都能找到

    - 标准来自rfc8259和https://www.json.org/json-zh.html 网站

    - 处理各个词法单元的自动机，词法分析器是手写的
    
    - ll(1)语法分析器的各个产生式

ps: 到目前为止,感觉自己越写越觉得垃圾
