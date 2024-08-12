#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include <memory>
#include <string>
#include <vector>

#include "lsf/xx.h"

import lsf;

/*
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
    int age = 77;
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

    lsf::parse_cpp_type(lf, wj);

    std::cout << wj.get_jsonstring() << std::endl;
}

TEST_CASE("Test 2")
{
    People lf;
    lsf::Json j;
    auto res2 = j.run(std::make_unique<lsf::StrSource>(R"(
    {
        // "age": 18,
        // "is_fat": true,
        "friends": [
            {
                "name": "dog",
                "age": 2
            },
            {
                "name": "duck",
                "age": 1
            },
            {
                "name": "cat",
                "age": 3
            }
        ]
    }
    )"));
    if (!res2)
    {
        std::cout << j.get_errors() << std::endl;
        CHECK(false);
        return;
    }
    // auto root = std::get<0>((*res2)->get_ast());
    // lsf::ReadJsonStr rj(root);
    // lsf::parse_cpp_type(lf, rj);

    lsf::ReadJsonStrExt rj2((*res2)->get_ast());
    lsf::parse_cpp_type(lf, rj2);

    lsf::WriteJsonStr wj;
    lsf::parse_cpp_type(lf, wj);
    std::cout << wj.get_jsonstring() << std::endl;
}
*/
struct InterceptSet
{
    std::string host_;      // 请求的主机.标识key查询拦截
    std::string svc_;       // 端口或服务名.例如 http https
    std::string url_;       // TODO 替换url
    std::string real_host_; // TODO 替换的主机名

    bool doh_ = false;         // 使用doh查询主机ip
    std::string doh_provider_; // TODO 当前主机使用此提供商查询,而非全局配置的提供商

    bool mitm_ = false;
    bool close_sni_ = false; // 不发送sni信息.默认就是false.优先级高于sni_host_
    std::string sni_host_;   // 发送假的sni信息
};

struct host_mapping
{
    std::string host_;
    std::string port_;
    std::vector<std::string> ips_;

    JS_OBJECT(JS_MEMBER(host_), JS_MEMBER(port_), JS_MEMBER(ips_));
};

struct dns_provider
{
    std::string provider_;
    std::string host_;

    JS_OBJECT(JS_MEMBER(provider_), JS_MEMBER(host_));
};

struct proxy_service : InterceptSet
{
    // std::string host_;
    // std::string svc_; // 端口或服务名.例如 http https
    // std::string url_;
    // bool mitm_ = false;
    // bool doh_ = false;
    // bool close_sni_ = false; // 默认就是false.优先级高于sni_host_
    // std::string sni_host_;   // 假的主机名

    JS_OBJECT(JS_MEMBER(host_),
              JS_MEMBER(svc_),
              JS_MEMBER(url_),
              JS_MEMBER(mitm_),
              JS_MEMBER(doh_),
              JS_MEMBER(close_sni_),
              JS_MEMBER(sni_host_));
};

struct config_struct
{
    // TODO 可以重载json解析 uint16_t
    int port_ = 55555;
    std::string ca_cert_path_ = "";
    std::string ca_pkey_path_ = "";
    std::string host_mapping_path_ = "";
    std::string proxy_service_path_ = "";
    std::vector<proxy_service> proxy_service_;
    std::vector<dns_provider> dns_provider_ = {{.provider_ = "1.1.1.1", .host_ = "1.1.1.1"}, {.provider_ = "ali", .host_ = "dns.alidns.com"}};

    JS_OBJECT(JS_MEMBER(port_),
              JS_MEMBER(ca_cert_path_),
              JS_MEMBER(ca_pkey_path_),
              JS_MEMBER(host_mapping_path_),
              JS_MEMBER(proxy_service_path_),
              JS_MEMBER(proxy_service_),
              JS_MEMBER(dns_provider_));
};

TEST_CASE("Test 3")
{
    lsf::Json j;
    //  直接获取json字符串
            // "dns_provider_": [
        //     {
        //         "provider_": "1.1.1.1", //取一个名字
        //         "host_": "9.9.9.9"
        //     }
        // ],
                // "dns_provider_": [
        //     {
        //         "provider_": "1.1.1.1", //取一个名字
        //         "host_": "9.9.9.9"
        //     }
        // ],
                //,
        // "proxy_service_": [
        //     {
        //         "host_":"github.com",
        //         "svc_":"443",   //端口或服务名.例如 http https
        //         "url_":"",      //暂未使用
        //         "mitm_":true,   //使用mimt代理
        //         "doh_":true,
        //         "sni_host_":"www.baidu.com",     //假的主机名
        //         "close_sni_":false  //默认就是false.设置为true时,优先级高于sni_host_
        //     }
        // ]
    auto res2 = j.run(std::make_unique<lsf::StrSource>(R"(
    {
                "dns_provider_": [
            {
                "provider_": "1.1.1.1", //取一个名字
                "host_": "9.9.9.9"
            }
        ]
                        ,
        "proxy_service_": [
            {
                "host_":"github.com",
                "svc_":"443",   //端口或服务名.例如 http https
                "url_":"",      //暂未使用
                "mitm_":true,   //使用mimt代理
                "doh_":true,
                "sni_host_":"www.baidu.com",     //假的主机名
                "close_sni_":false  //默认就是false.设置为true时,优先级高于sni_host_
            }
        ]
    }
    )"));
    if (!res2)
    {
        std::cout << j.get_errors() << std::endl;
        CHECK(false);
        return;
    }
    // bu.clear();
    //  把解析好的json表示再输出为格式化的json字符串
    // lsf::json_to_string(*res2, bu);
    // std::cout << bu.get_jsonstring() << std::endl;

    //  把json表示转换成struct
    config_struct cs;
    // lsf::json_to_struct(*res2, peo);
    lsf::json_to_struct_ignore_absence(*res2, cs);
}