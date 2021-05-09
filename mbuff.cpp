#include "mbuff.h"
#include<assert.h>

namespace lsf {

namespace Private {

}

MBuff::MBuff():
    state_(State::S0)
{
    f_.imbue(std::locale(""));
}

MBuff::~MBuff()
{
}

MBuff::MBuff(const std::string &file_name):
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

/// 因为当缓冲区重新填充时，当前已分析的这些字符被新读取的字符覆盖,且lexeme_begin_也表示的是原先分析的token的开始处,
/// 因此需要在合适的时候调用current_token或discard来消耗当前已分析的字符.
/// 否则current_token返回的是被污染的token
wchar_t MBuff::next_char()
{
    //无法达到2*BuffLen，因为在这之前已经被转移到下一状态了，从而forward_和lexeme_begin_被重置了
    //exceed the max length of token,should not peek next char
//    assert(forward_-lexeme_begin_<2*BuffLen-1);
    switch (state_) {
        case State::S0:{
            lexeme_begin_=forward_=fence_=0;
            read(forward_);
            state_=State::S1;
        }
        break;
        case State::S1:
        case State::S3:{
            forward_++;
            if(forward_==BuffLen){
                fence_=0;
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
    assert(state_!=State::S0);//there is not currently token,current S0
    if(state_==State::S3){
        std::wstring s(forward_-lexeme_begin_+1,wchar_t());
        if(lexeme_begin_>=0||forward_<0)
            s.assign(buff_.data()+(lexeme_begin_+BuffLen*2)%(BuffLen*2),(-lexeme_begin_)+forward_+1);
        else{
            s.assign(buff_.data()+lexeme_begin_+BuffLen*2,(-lexeme_begin_));
            s.append(buff_.data(),forward_+1);
        }
        return s;
    }else
        return std::wstring(buff_.data()+lexeme_begin_,(-lexeme_begin_)+forward_+1);
}

std::wstring MBuff::get_token()
{
    assert(state_!=State::S0);
    auto s=current_chars();
    lexeme_begin_=forward_+1;
    return s;
}

void MBuff::discard_token()
{
    //can not discard,we are in S0
    assert(state_!=State::S0);
    lexeme_begin_=forward_+1;
}

///回滚不超过1个token
///溢出未考虑，留给调用者自己决定
void MBuff::roll_back_char(int len)
{
    //roolback fail
    assert(state_!=State::S0);
    //roolback fail,len overflows
    assert(forward_-len>=lexeme_begin_-1);
    forward_-=len;

}

bool MBuff::is_eof() const
{
    return buff_[forward_]==Eof_w;
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
    if(f_.bad())
        throw std::runtime_error("read file failed!");
    auto c=f_.gcount();
    if(c<length)
        pb[c]=Eof_w;
}

}
