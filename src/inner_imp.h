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

struct Location
{
    size_t line_{0};
    size_t column_{0};
//    size_t column_last_{0};
};

class FilterBuff:public BuffBase
{
public:
    virtual wchar_t next_char();
    virtual void rollback_char(std::size_t len=1);
    virtual void rollback_all_chars();
    virtual void discard_token();
    virtual std::wstring get_token();
public:
    FilterBuff(std::unique_ptr<BuffBase> buff );
    ~FilterBuff();
    ///调用前需要已经调用了this->discard_token()或this->get_token()
    Location begin_location();
    void record_location();
    bool test_and_skipBOM();
private:
    std::unique_ptr<BuffBase> b_;
    std::vector<std::size_t> history_{};
    //begin position
    Location p_begin_{};
    //current position
//    Location p_current_{};
    std::size_t number_{0};
};

class Lexer;

class FunnyTokenGen : public lsf::GenToken
{
public:
    FunnyTokenGen(std::shared_ptr<lsf::Lexer> l,std::shared_ptr<FilterBuff> b):lexer_(l),buff_(b),tk_begin_(b->begin_location()){}
    ///token 开始的位置,相对于文件的行与列
    Location token_position()const;
    // GenToken interface
public:
    virtual void next_() override;
    virtual lsf::Token &current_() override;
private:
    std::shared_ptr<lsf::Lexer> lexer_{};
    std::shared_ptr<FilterBuff> buff_{};
    Location tk_begin_{};
};

}

#endif // IMP_H
