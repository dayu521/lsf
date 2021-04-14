#include <iostream>
#include <fstream>
#include <memory>
#include"kmp.h"
#include <json/json.h>
using namespace std;
template<class T1, class T2>
struct P {
 T1 x1;
 T2 x2;
 friend auto operator<=>(const P&, const P&) = default;
};
int main()
{
//    setlocale(LC_ALL,"zh_CN.UTF-8");
    //std::locale("").name().c_str()
//    std::locale::global(std::locale(""));
//    MBuff m("文本文件.txt");
//    m.next_char();
//    while (!m.is_eof()) {
//        auto ss=m.current_char();
//        std::wcout<<ss;
//        m.next_char();
//    }
//    std::wcout.flush();
//    char s[]="\xe4\xbd\xa0";
//    wchar_t w;
//    mbtowc(&w,s,3);
//    wcout<<w<<endl;
//    wcout<<L'\x4F60'<<endl;
//    wcout<<wchar_t(0x4F60)<<endl;
//    char c2[]="\x50\n";
    Json::Value root;
//    cin>>root["ha"];
    root["ha"]="你";
    cout<<root;
    constexpr auto xx=-1>>1;
    constexpr auto x1=-5>>11;
    constexpr auto x2=-1/2;
    return 0;
}

