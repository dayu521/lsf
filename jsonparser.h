#ifndef JSONPARSER_H
#define JSONPARSER_H

#include<memory>
#include<functional>
#include"constant.h"

namespace lsf {

class Token;

class PError;

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

class ParserError: public lsf::BaseError
{
public:
    using BaseError::BaseError;
};

class JsonParser
{
public:
    JsonParser(GenToken gen);
    bool parser();
    const std::vector<lsf::Type> & get_error()const;
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
    std::vector<lsf::Type> error_array_;
    std::unique_ptr<Visitor> visitor_;
};

}
#endif // JSONPARSER_H
