#ifndef JSONPARSER_H
#define JSONPARSER_H

#include<memory>
#include<vector>
#include"constant.h"

namespace lsf {

struct Token;

class PError;

class BuilderInterface;

struct GenToken
{
    virtual ~GenToken(){}
    virtual void next_()=0;
    virtual Token & current_()=0;
};

class JsonParser
{
public:
    JsonParser(std::shared_ptr<GenToken> gen);
    void set_builder(std::shared_ptr<BuilderInterface> b);
    [[nodiscard]] bool parser();
    const std::vector<lsf::Type> & get_expect_token()const;
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
    std::shared_ptr<GenToken> gen_;
    std::shared_ptr<BuilderInterface> builder_;
    std::vector<lsf::Type> expect_array_;
};

class R_JsonParser
{
public:
    R_JsonParser(std::shared_ptr<GenToken> gen);
    void set_builder(std::shared_ptr<BuilderInterface> b);
    [[nodiscard]] bool parser();
    const std::vector<lsf::Type> & get_expect_token()const;
private:
    using TType=lsf::Type;
    bool json();
//    bool element();
    bool value();
//    bool obj();
//    bool mb_ws();
//    bool mb_ws_r();
//    bool memberL();
//    bool member();
//    bool array();
//    bool arr_ws();
//    bool arr_ws_r();
//    bool elementsL();
    bool unuse();

    bool isTerminator(TType type);

private:
    std::shared_ptr<GenToken> gen_;
    std::shared_ptr<BuilderInterface> builder_;
    std::vector<lsf::Type> expect_array_;
};

//namespace end
}
#endif // JSONPARSER_H
