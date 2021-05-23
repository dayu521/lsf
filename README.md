### json 解析器

解析json文件,反序列化到c++结构;以及从c++结构序列化到json.用到了c++17标准.

感谢[json_struct](https://github.com/jorgen/json_struct)与[cista](https://github.com/felixguendling/cista)提供的序列化实践.

#### 用法 ####

```cpp
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

int main()
{
    People lf={18,true,{{"dog",2},{"duck",1},{"cat",3}}};
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
    if(!ok)
        return -1;
    People ml{};
    try {
        lsf::deserialize(ml,j.get_output());
    }  catch (const lsf::DeserializeError &ex) {
        std::cout<<ex.what()<<std::endl;
        throw ex;
    }

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
//从TreeNode中反序列化到基本类型T
template<typename T>
inline void Deserialize(T & s,const TreeNode * t);

//序列化到json字符串
template<typename T>
void SerializeBuilder::write_value(const T & ele);
```

#### 问题 ####

- 不处理字符编码问题，因为实在搞不定
- 不支持继承
- 保留空白.标准json不接受注释,但也是保留c和c++风格的注释(留下了支持的接口)
- 详细文档都会开放,其实也没什么,大家都能找到

    - 标准来自rfc8259和https://www.json.org/json-zh.html 网站

    - 处理各个词法单元的自动机，词法分析器是手写的
    
    - ll(1)语法分析器的各个产生式

ps: 到目前为止,感觉自己越写越觉得垃圾
