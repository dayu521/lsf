#ifndef JSONPARSER_H
#define JSONPARSER_H

#include<memory>
#include<functional>

namespace lsf {

class Token;

class Visitor
{
    void visit_obj(Token *);
    void visit_array(Token *);
    void visit_keyword(Token *);
    void visit_string(Token *);
    void visit_number(Token *);
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
    bool json();
    bool element();
    bool value();
    bool obj();
    bool mb_ws();
    bool members();
    bool membersL();
    bool member();
    bool array();
    bool arr_ws();
    bool elements();
    bool elementsL();
    bool ws();
private:
    GenToken gen_;
    Token * c_token_{nullptr};
    std::unique_ptr<Visitor> visitor_;
};

}
#endif // JSONPARSER_H
