#ifndef KMP_H
#define KMP_H
#include <string>
#include <vector>
#include <fstream>
#include <memory>

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

class MBuff
{
public:
    MBuff();
    ~MBuff();
    MBuff(const std::string &file_name);
    MBuff(const MBuff & )=delete;
    void open(const std::string & file_name);
    wchar_t next_char();
    wchar_t current_char()const;
    std::wstring current_token();
    void roll_back();
    bool is_eof()const;
    static constexpr auto Eof=WEOF;
private:
    int fence_ {0};
    int lexeme_begin_ {0};
    int forward_ {0};

    static constexpr  int MaxTokenLen=1024;
    std::shared_ptr<wchar_t []> buff_{};

    std::wifstream f_ {};
//    wchar_t buff_[2*MaxTokenLen]{4};
    enum class State{S0,S1,S2,S3};
    //fence,forward分别代表fence和forward
    //fence forward |         S0
    //fence | forward         S1
    // | fence forward        S2
    //forward | fence         S3
    State state_{State::S0};
private:
    void read(int begin, int length=MaxTokenLen);
};

#endif // KMP_H
