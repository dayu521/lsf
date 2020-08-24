#ifndef KMP_H
#define KMP_H
#include <string>
#include <vector>

inline int al=80;

class KMP
{
public:
    KMP(const std::string &p_);
    KMP(const KMP &)=delete ;
    int match(const std::string & s) const;
    ~KMP();
private:
    void piFunc();
private:
    std::string pattern{};
    int s{0};
    std::vector<int> pi{};
};

class MBuff
{
public:
    MBuff();
    char getnext();
    std::string currentBytes();
private:
    unsigned int lexemeBegin{0};
    unsigned int forward{0};
    //0000 0100	4	04	EOT	传输结束
    //作为哨兵 eof
    static constexpr char Eof=4;
    static constexpr unsigned int le=16;
    char b[2*le]{4};
    enum class State{S1,S2,S3,S4};
    //L,F分别代表lexeme和forward
    //L F |         S1
    //L | F         S2
    // | L F        S3
    //F | L         S4
    State state{State::S1};
private:
    void switchPos();
    void read(char * b,int length=le-1);
};

#endif // KMP_H
