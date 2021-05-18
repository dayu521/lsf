### json 解析器

从文件读取json到c++结构，以及从c++结构反序列化到json

感谢[json_struct](https://github.com/jorgen/json_struct)与[cista](https://github.com/felixguendling/cista)提供的序列化实践

- 不处理字符编码问题，因为实在搞不定

- 保留空白，标准json不接受注释，但也是保留c和c++风格的注释

- 详细文档都会开放，其实也没什么，大家都能找到

    - 标准来自rfc8259和https://www.json.org/json-zh.html 网站

    - 处理各个词法单元的自动机，词法分析器是手写的
    
    - ll(1)语法分析器的各个产生式
