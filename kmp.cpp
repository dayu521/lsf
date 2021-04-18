#include "kmp.h"
#include<iostream>

KMP::KMP(const std::wstring &p_):pattern(p_)
{
    pattern_len=pattern.size();
    pi.resize(pattern_len);
    piFunc();
}

int KMP::match(const std::wstring &str) const
{
    int k=pi[0];
    for(std::size_t i=0;i<str.size();i++){
        while(k>0&&pattern[k]!=str[i])
            k=pi[k-1];
        if(pattern[k]==str[i])
            k++;
        if(k==pattern_len){
            using std::wcout;
            wcout<<str.substr(0,i-pattern_len+1)<<L"("<<
                  pattern<<L")"<<str.substr(i+1)<<std::endl;
//            k=pi[k];
//            break;
            k=pi[0];
        }
    }
    return str.size()-k;
}

KMP::~KMP()
{
}

void KMP::piFunc()
{
    pi[0]=0;
    int k=pi[0];
    for(int i=1;i<pattern_len;i++){
        while(k>0&&pattern[k]!=pattern[i]){
            k=pi[k-1];
        }
        if(pattern[k]==pattern[i]){
            k++;
        }
        pi[i]=k;
    }
    for(int i=0;i<pattern_len;i++)
        std::cout<<pi[i]<<" ";
    std::cout<<std::endl;
}
namespace lsf {

MBuff::MBuff():
//    buff_(std::make_shared<wchar_t [2*MaxTokenLen]>()),
    buff_(new wchar_t[2*BuffLen],[](auto p){delete [] p;}),
    state_(State::S0)
{
    f_.imbue(std::locale(""));
}

MBuff::~MBuff()
{
}

MBuff::MBuff(const std::string &file_name):
    /* 根本原因就是当前库未实现make_shared.
     * 根据大佬推测，库只是分配了一个指向数组的指针，所以当把这个这个指针作为数组读写时，
     * 造成之后其他对象分配的数据空间遭到破坏.于是错误反而在其他对象的数据析构时才发现，
     * 于是可能会被误导为其他对象出了问题
     *
     * 总结:要看各个标准库发行文档以及注记
     * libstc++文档中写了未实现
     *      https://gcc.gnu.org/onlinedocs/libstdc++/manual/status.html#status.iso.2017
     * 相关提案P0674R1
     *      http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2017/p0674r1.html
     */
//    buff_(std::make_shared<wchar_t [2*MaxTokenLen]>()),
    buff_(new wchar_t[2*BuffLen],[](auto p){delete [] p;}),
    f_(file_name),
    state_(State::S0)
{
    f_.imbue(std::locale(""));
    if(!f_.is_open())
        throw std::runtime_error("failed to open file:"+file_name);
}

void MBuff::open(const std::string &file_name)
{
    if(f_.is_open()){
        f_.close();
    }
    f_.open(file_name);
    if(!f_.is_open())
        throw std::runtime_error("failed to open file:"+file_name);
}

///不能一直调用此函数,需要调用current_token或discard来消耗当前已分析的字符，
/// 否则当前已分析的这些字符被新读取的字符覆盖,而且lexeme_begin_也表示的是原先分析的字符的开始处
wchar_t MBuff::next_char()
{
    //无法达到2*BuffLen，因为在这之前已经被转移到下一状态了，从而forward_和lexeme_begin_被重置了
    [[unlikely]]if(forward_-lexeme_begin_>=2*BuffLen-1)
        throw std::runtime_error("exceed the max length of token,should not peek next char");
    switch (state_) {
        case State::S0:{
            lexeme_begin_=forward_=fence_=0;
            read(forward_);
            state_=State::S1;
        }
        break;
        case State::S1:{
            forward_++;
            if(forward_==BuffLen){
                read(forward_);
                state_=State::S2;
            }
        }
        break;
        case State::S2:{
            forward_++;
            if(forward_==2*BuffLen){
                forward_=0;
                fence_=BuffLen;
                //因此,在S3中所有索引都是负的,从而任何时刻
                //lexeme_begin_和forward_都是相同符号
                lexeme_begin_=lexeme_begin_-2*BuffLen;
                read(forward_);
                state_=State::S3;
            }
        }
        break;
        case State::S3:{
            forward_++;
            if(forward_==BuffLen){
                fence_=0;
                read(forward_);
                state_=State::S2;
            }
        }
        break;
    }
    return buff_[(forward_+BuffLen*2)%(BuffLen*2)];
}

wchar_t MBuff::current_char() const
{
    //体会这种思想
    return buff_[(forward_+BuffLen*2)%(BuffLen*2)];
}

std::wstring MBuff::current_chars()
{
    switch (state_) {
        case State::S0:throw std::out_of_range("there is not currently token,current S0");break;
        case State::S1:
        case State::S2:{
            std::wstring s=std::wstring(buff_.get()+lexeme_begin_,forward_-lexeme_begin_+1);
//            lexeme_begin_=forward_+1;
            return s;
        }
        case State::S3:{
            std::wstring s(forward_-lexeme_begin_+1,wchar_t());
            if(lexeme_begin_>=0||forward_<0)
                s.assign(buff_.get()+(lexeme_begin_+BuffLen*2)%(BuffLen*2),forward_-lexeme_begin_+1);
            else{
                s.assign(buff_.get()+lexeme_begin_+BuffLen*2,BuffLen);
                s.append(buff_.get(),forward_+1);
            }
//            lexeme_begin_=forward_+1;
            return s;
        }
    }
}

std::wstring MBuff::current_token()
{
    auto s=current_chars();
    lexeme_begin_=forward_+1;
    return s;
}

void MBuff::discard_token()
{
    if(state_==State::S0)
        throw std::runtime_error("can not discard,we are in S0");
    lexeme_begin_=forward_+1;
}

int MBuff::get_char_count() const
{
    return forward_-lexeme_begin_+1;
}

///回滚不超过1个token
///溢出未考虑，留给调用者自己决定
void MBuff::roll_back_char(int len)
{
    switch (state_) {
        case State::S0:{
            throw std::out_of_range("roolback fail,current S0");
        }
        break;
        case State::S1:
        case State::S2:
        case State::S3:{
            if(forward_-len<lexeme_begin_-1){
                throw std::out_of_range("roolback fail,current S2");
            }
            forward_-=len;
        }
        break;
    }
}

bool MBuff::is_eof() const
{
    return buff_[forward_]==Eof;
}

void MBuff::init()
{
    state_=State::S0;
    forward_=-1;
    lexeme_begin_=fence_=0;
}

/// 调用之前必须先打开,否则未定义
void MBuff::read(int begin, int length)
{
    auto pb=&buff_[0]+begin;
    f_.read(pb,length);
    auto c=f_.gcount();
    if(c<length)
        pb[c]=WEOF;
}

}
