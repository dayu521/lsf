#ifndef KMP_H
#define KMP_H
#include <string>
#include <vector>
#include <fstream>
#include <memory>

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
namespace lsf {

class BuffBase
{
public:
    BuffBase(){}
    virtual ~BuffBase(){}
    virtual wchar_t next_char()=0;
    virtual void roll_back_char(int len=1)=0;
    virtual void discard_token()=0;
    virtual std::wstring get_token()=0;
    static constexpr auto Eof_w=WEOF;
    static constexpr  int BuffLen=512;
};

class MBuff :public BuffBase
{

public:
    virtual wchar_t next_char();
    virtual void roll_back_char(int len=1);
    virtual void discard_token();
    virtual std::wstring get_token();
public:
    MBuff();
    ~MBuff();
    MBuff(const std::string &file_name);
    MBuff(const MBuff & )=delete;
    void open(const std::string & file_name);

    std::wstring current_chars();

    wchar_t current_char()const;
    bool is_eof()const;

    void init();

private:
    int fence_ {0};
    int lexeme_begin_ {0};
    int forward_ {-1};

    //2*BuffLen
    std::shared_ptr<wchar_t []> buff_{};

    std::wifstream f_ {};

    enum class State{S0,S1,S2,S3};
    //fence,forward分别代表fence和forward
    //  init state            S0
    //fence forward |         S1
    //fence | forward         S2
    //forward | fence         S3
    State state_{State::S0};

private:
    void read(int begin, int length=BuffLen);
};

struct Statistic
{
    unsigned int column_curr_{0};
    unsigned int line_{0};
    unsigned int column_last_{0};
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
    Statistic get_stat()const;
    bool test_and_skipBOM();
private:
    std::unique_ptr<BuffBase> b_;
    std::vector<int> history_{};
    Statistic stat_{};
};

//namespace end
}
#endif // KMP_H
