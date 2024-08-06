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

参考目录example

```cpp

//将会转换 json字符串json_str 与 c++结构体People

struct People
{
    std::string dasd;
    int  a2;
    std::vector<int> n;
    // 也可以指定一个key
    JS_OBJECT(JS_MEMBER(dasd), JS_MEMBER(a2,"faa阿萨德撒安抚"), JS_MEMBER(n, "达到loo5"));
};

auto json_str=R"(
        {
            "dasd":"aasdaffa",
            "faa阿萨德撒安抚":5,
            "达到loo5":null
        }
    )";

```

```bash
    cd <project-path>/example
    xmake -vD -P .
    xmake run -P .
```

#### cmake

需要cmake3.28,在你的工程目录clone当前项目,然后添加以下内容到你的CMakeLists.txt

```text

# 自己的模块库,目前需要c++版本一致,所以需要设置库版本
add_subdirectory(lsf)
# 默认使用c++20,如果你使用更高的标准,你可以使用下述修改.
# 这是必须的,目前,库和你的应用的标准需要一致.
# set_property(TARGET lsf PROPERTY CXX_STANDARD 23)

target_link_libraries(${appname} lsf)

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