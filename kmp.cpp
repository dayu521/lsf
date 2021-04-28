#include "kmp.h"
#include<iostream>
#include<assert.h>

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

namespace Private {

}

MBuff::MBuff():
//    buff_(std::make_shared<wchar_t [2*MaxTokenLen]>()),
    buff_(new wchar_t[2*BuffLen],[](wchar_t * p){delete [] p;}),
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
    buff_(new wchar_t[2*BuffLen],[](wchar_t * p){delete [] p;}),
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
            s.assign(buff_.get()+(lexeme_begin_+BuffLen*2)%(BuffLen*2),forward_-lexeme_begin_+1);
        else{
            s.assign(buff_.get()+lexeme_begin_+BuffLen*2,BuffLen);
            s.append(buff_.get(),forward_+1);
        }
        return s;
    }else
        return std::wstring(buff_.get()+lexeme_begin_,forward_-lexeme_begin_+1);
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
    if(forward_-len<lexeme_begin_-1){
        throw std::out_of_range("roolback fail,current S2");
    }
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

FilterBuff::FilterBuff(std::unique_ptr<BuffBase> buff):b_(std::move(buff)),history_(1,0),stat_{1,1,1}
{
}

FilterBuff::~FilterBuff()
{

}

wchar_t FilterBuff::next_char()
{
    auto c=b_->next_char();
    if(c==L'\n'){
        stat_.line_++;
        //    \n \n  换行符
        //  5  2  1     数量
        //  0  1  2     数组索引
        //除第一次外，每次遇到换行就从数组下一位开始从1计数，直到遇到换行符，然后重复过程，数组的大小减一就是当前token的换行符个数
        //在回滚的时候从后往前，依次减掉回滚的个数，然后遇到换行符就把stat_.line_减一
        //所以上述图示表示,前5个字符不是换行符，第6个字符是换行符，第7个字符不是换行符，第8个是换行符
        //好吧 我描述的不太好，仔细想想就明白了
        history_.push_back(0);
        stat_.column_curr_=1;
    }else
        stat_.column_curr_++;
    history_[history_.size()-1]++;
    return c;
}

void FilterBuff::roll_back_char(int len)
{
    b_->roll_back_char(len);
    stat_.column_curr_--;
    for(auto i=history_.crbegin();i!=history_.crend();i++){
        if(len-*i<0){
            history_.resize(1);
            history_[0]=0;
            break;
        }
        len-=*i;
        stat_.line_--;
    }
    assert(stat_.line_>=0&&stat_.column_curr_>=0);
}

void FilterBuff::discard_token()
{
    b_->discard_token();
    stat_.column_last_=stat_.column_curr_;
    history_.resize(1);
    history_[0]=0;
}

std::wstring FilterBuff::get_token()
{
    stat_.column_last_=stat_.column_curr_;
    history_.resize(1);
    history_[0]=0;
    return b_->get_token();
}

Statistic FilterBuff::get_stat() const
{
    return stat_;
}

bool FilterBuff::test_and_skipBOM()
{
    wchar_t head[3]={};
    for (auto i=0;i<3;i++){
        head[i]=b_->next_char();
    }
    if(wcscmp(head,L"\xEF\xBB\xBF")==0){
        b_->discard_token();
        return true;
    }
    b_->roll_back_char(3);
    return false;
}

}
