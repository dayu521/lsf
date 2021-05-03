#ifndef INNER_IMP_H
#define INNER_IMP_H

#include"mbuff.h"
#include"jsonparser.h"

namespace lsf {

//本来以为用得着kmp算法，但没想到没用
class KMP
{
public:
    KMP(const std::wstring &p_);
    KMP(const KMP &)=delete ;
    int match(const std::wstring &s) const;
    ~KMP();
private:
    void piFunc();
private:
    std::wstring pattern{};
    int pattern_len{0};
    std::vector<int> pi{};
};

struct Statistic
{
    size_t column_curr_{0};
    size_t line_{0};
    size_t column_last_{0};
};

class FilterBuff:public BuffBase
{
public:
    virtual wchar_t next_char();
    virtual void roll_back_char(int len=1);
    virtual void discard_token();
    virtual std::wstring get_token();
public:
    FilterBuff(std::unique_ptr<BuffBase> buff );
    ~FilterBuff();
    Statistic get_stat();
    bool test_and_skipBOM();
private:
    std::unique_ptr<BuffBase> b_;
    std::vector<int> history_{};
    Statistic stat_{};
    int chars_n_;
};

class Lexer;

class Ltokens : public lsf::GenToken
{
public:
    Ltokens(std::shared_ptr<lsf::Lexer> l):lexer_(l){}
    // GenToken interface
public:
    virtual void next_() override;
    virtual lsf::Token &current_() override;
private:
    std::shared_ptr<lsf::Lexer> lexer_;
};

}

#endif // IMP_H
