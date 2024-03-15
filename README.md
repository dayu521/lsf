### json 解析器

c++结构体与json字符串互相转换.用到了c++20标准([module](https://en.cppreference.com/w/cpp/language/modules)).

## special thanks

- [json_struct](https://github.com/jorgen/json_struct)

- [cista](https://github.com/felixguendling/cista).

- [dcotest](https://github.com/onqtam/doctest)

### 编译

c++20

- linux,clang17

- windows,msvc latest

---

使用xmake最新版:

```bash
    cd <project-path>
    xmake c -va 
    xmake -vDwr
```
### 使用
#### xmake

如果你的项目是通过xmake进行构建的

在你的xmake.lua工程文件中添加以下内容

```lua
package("lsf")
    set_description("json.")
    set_urls(https://github.com/dayu521/lsf.git)
    -- set_sourcedir(path.join(os.scriptdir(), "lib/lsf"))
    
    on_install(function (package)
        os.cp("src/lsf", package:installdir("include"))
        -- os.cp("libjpeg.lib", package:installdir("lib"))
        import("package.tools.xmake").install(package,{})
    end)
package_end()
add_requires("lsf")

target(...)
    #其他配置
    #....
    #添加包依赖
    add_packages("lsf")
```

#### cmake

TODO

#### 例子代码 ####

```cpp
//包含合适的头文件
#include <lsf/xx.h>//当前编译器模块支持不成熟,需要放到第一行

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
    std::cout << bu.get_jsonstring() << std::endl;

    //  把json表示转换成struct
    //  这里,会发现 json中的null不能被很好地表示,
    People2 peo;
    lsf::json_to_struct(*res2,peo);
    // 不需要所有成员都有对应的json属性
    // lsf::json_to_struct_ignore_absence(*res2,peo);

    //  把struct转换成json字符串
    People lf = {18, true, {{"dog", 2}, {"duck", 1}, {"cat", 3}}};
    bu.clear();
    lsf::struct_to_jsonstr(peo, bu);
    std::cout << bu.get_jsonstring() << std::endl;
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

#### 说明 ####

- 从普通字符串转化为宽字符串受locale影响(默认与当前系统字符编码一致)

- 保留空白.标准json不接受注释,但也是保留c和c++风格的注释(留下了支持的接口)

- 标准来自rfc8259和https://www.json.org/json-zh.html 网站

- 处理各个词法单元的自动机，词法分析器是手写的

- ll(1)语法分析器的各个产生式

#### TODO

- [ ] 简化的定义struct的宏

- [ ] 记录词法分析过程中token在原始输入流的位置

- [ ] 合并TreeBuilder和SerializeBuilder接口

- [ ] json中的null在c++的struct中不能很好地对应.c++每个实体都有值.null映射指针还算合理