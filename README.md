### json 解析器

解析json文件,反序列化到c++结构;以及从c++结构序列化到json.用到了c++20标准([module](https://en.cppreference.com/w/cpp/language/modules)).

最低也支持c++17.

感谢[json_struct](https://github.com/jorgen/json_struct)与[cista](https://github.com/felixguendling/cista)提供的序列化实践.
感谢[dcotest](https://github.com/onqtam/doctest).

### 编译(c++20以上)

---

cmake:

当前[cmake3.28](https://cmake.org/cmake/help/v3.28/manual/cmake-cxxmodules.7.html) 提供了module支持,它需要gcc14,但我并未编译通过.

且使用clang也不能编译成功

---

xmake:

使用clang16进行编译

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

> 再次,编译release版本出错,~ ~

```lua
includes("lsf_module")
target("your project name")
    add_deps("lsf")
    add_includedirs("lsf_module/src/public")
```

### 使用(c++17)
#### 1.通过cmake

```bash
cd <projectdir>	#你的工程源码目录
git clone --depth=1 https://github.com/dayu521/lsf.git
```

修改你自己的cmake工程文件`CMakeLists.txt`,添加以下行:

```cmake
add_subdirectory(lsf)
target_link_libraries(${PROJECT_NAME} lsf)
```

#### 2.复制源码

当前不依赖三方库,除了标准库,以及测试库`doctest`.
所以,把源码中`src/old`目录下的内容复制到你自己的工程内,它包含了所有需要的头文件与源文件.


#### 列子代码 ####

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

int main()
{
    People lf={18,true,{{"dog",2},{"duck",1},{"cat",3}}};
    
    lsf::SerializeBuilder bu;
    //序列化到bu中
    lsf::struct_to_json(lf,bu);
    //标准输出打印
    std::cout<<bu.get_jsonstring()<<std::endl;

    //写入到文件中
    std::ofstream f("example.txt");
    if(f.is_open()){
        f<<bu.get_jsonstring();
        f.close();
    }

    //从文件读入
    lsf::Json j("example.txt");
    //解析
    auto ok=j.run([&](auto t,const std::string &s){
        lsf::ErrorType sd=t;
        std::cout<<s<<std::endl;
    });
    if(!ok)
        return -1;
    
    People ml{};
    try {
        //序列化到People
        lsf::json_to_struct(ml,j);
    }  catch (const lsf::DeserializeError &ex) {
        std::cout<<ex.what()<<std::endl;
        throw ex;
    }
    //不可复制
    lsf::Json * lk=new lsf::Json(std::move(j));
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
inline void lsf::Deserialize(T & s,const TreeNode * t);

//序列化T类型到json字符串
template<typename T>
inline void lsf::write_value(const T & v,SerializeBuilder & builder);
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
