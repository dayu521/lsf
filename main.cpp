#include <iostream>
#include"kmp.h"

using namespace std;
constexpr int s=8;
int main()
{
    cout << "Hello World!" << endl;
    KMP k("ababaa");
    KMP k2("ababaca");
    k.match("abababaab");
    k.match("abababbaa");
    k2.match("bacbababacacbab");
//    al=150;
    int a=al;
    MBuff m;
    while(a--)
        m.getnext();
    return 0;
}
