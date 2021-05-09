#ifndef KMP_H
#define KMP_H
#include <string>
#include <fstream>
#include <array>

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
    std::array<wchar_t,2*BuffLen> buff_{};

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

//namespace end
}
#endif // KMP_H
