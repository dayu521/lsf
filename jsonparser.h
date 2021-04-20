#ifndef JSONPARSER_H
#define JSONPARSER_H

#include<memory>
#include<functional>
#include"constant.h"

namespace lsf {

class Token;

class Visitor
{
    template<lsf::Type T>
    void visit();
};

struct GenToken
{
    std::function<void ()> next_;
    std::function<const Token & ()> current_;
};

class JsonParser
{
public:
    JsonParser(GenToken gen);
    bool parser();
private:
    using TType=lsf::Type;
    bool json();
    bool element();
    bool value();
    bool obj();
    bool mb_ws();
    bool mb_ws_r();
    bool memberL();
    bool member();
    bool array();
    bool arr_ws();
    bool arr_ws_r();
    bool elementsL();
    bool unuse();

    bool isTerminator(TType type);
private:
    GenToken gen_;
    Token * c_token_{nullptr};
    std::unique_ptr<Visitor> visitor_;
};

}
#endif // JSONPARSER_H
