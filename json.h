#ifndef JSON_H
#define JSON_H

namespace lsf {

#define Jfk(s)   \
template<typename T>    \
constexpr const char * Fx(const char * p=s)   \
{   \
    T::name=p;  \
    return p;   \
}

//Jfk("haha")
struct Test
{
    static const char * name;
};

//auto m=Fx<Test>(" ");

class json
{
public:
    json();
};

#endif // JSON_H

}
