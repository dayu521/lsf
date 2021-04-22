#ifndef JSONPARSER_H
#define JSONPARSER_H

#include<memory>
#include<functional>
#include"constant.h"

namespace lsf {

class Token;

enum class NodeCategory{Obj,Arr,String,Number,Keyword};

class Visitor
{
    void visit_obj(Token t);
    void visit_member(Token t);
    void visit_arr(Token t);
    void visit_element(Token t);
    void visit_string(Token t);
    void visit_number(Token t);
    void visit_keyword(Token t);
};


struct GenToken
{
    std::function<void ()> next_;
    std::function<const Token & ()> current_;
};

class ParserError: public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class JsonParser
{
public:
    JsonParser(GenToken gen);
    void parser();
private:
    using TType=lsf::Type;
    void json();
    void element();
    void value();
    void obj();
    void mb_ws();
    void mb_ws_r();
    void memberL();
    void member();
    void array();
    void arr_ws();
    void arr_ws_r();
    void elementsL();
    void unuse();

    bool isTerminator(TType type);
private:
    GenToken gen_;
    Token * c_token_{nullptr};
    std::unique_ptr<Visitor> visitor_;
};

}
#endif // JSONPARSER_H
