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

MBuff::MBuff():buff_(std::make_shared<wchar_t [2*MaxTokenLen]>(2*MaxTokenLen)),
    state_(State::S0)
{

}

MBuff::~MBuff()
{
}

MBuff::MBuff(const std::string &file_name):
    buff_(std::make_shared<wchar_t [2*MaxTokenLen]>()),
    f_(file_name),
    state_(State::S0)
{
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

//不能一直调用此函数,需要调用current_token
wchar_t MBuff::next_char()
{
    switch (state_) {
        case State::S0:{
            forward_=fence_=0;
            read(forward_);
            state_=State::S1;
        }
        break;
        case State::S1:{
            forward_++;
            if(forward_==MaxTokenLen){
                read(forward_);
                state_=State::S2;
            }
        }
        break;
        case State::S2:{
            forward_++;
            if(forward_==2*MaxTokenLen){
                forward_=0;
                fence_=MaxTokenLen;
                read(forward_);
                state_=State::S3;
            }
        }
        break;
        case State::S3:{
            forward_++;
            if(forward_==MaxTokenLen){
                fence_=0;
                read(forward_);
                state_=State::S2;
            }
        }
        break;
    }
    return buff_[(forward_+MaxTokenLen*2)%(MaxTokenLen*2)];
}

wchar_t MBuff::current_char() const
{
    return buff_[(forward_+MaxTokenLen*2)%(MaxTokenLen*2)];
}

std::wstring MBuff::current_token()
{
    switch (state_) {
        case State::S0:throw std::out_of_range("there is not currently token,current S0");break;
        case State::S1:
        case State::S2:{
            std::wstring s{};
            if(lexeme_begin_<=forward_)
                s=std::wstring(buff_.get()+lexeme_begin_,forward_-lexeme_begin_);
            else
                throw std::out_of_range("rollback too much to find a token,current S1 or S2");
            lexeme_begin_=forward_+1;
            return s;
        }
        case State::S3:{
            std::wstring s{};
            if(lexeme_begin_>=0&&lexeme_begin_<=forward_)
                s=std::wstring(buff_.get()+lexeme_begin_,forward_-lexeme_begin_);
            else if(lexeme_begin_>MaxTokenLen&&forward_>=0)
                s=std::wstring(buff_.get(),forward_+1)+std::wstring(buff_.get()+lexeme_begin_,MaxTokenLen);
            else
                throw std::out_of_range("rollback too much to find a token,current S3");
            lexeme_begin_=forward_+1;
            return s;
        }
    }
    return L"";
}

void MBuff::roll_back()
{
    switch (state_) {
        case State::S0:{
            throw std::out_of_range("roolback fail,current S0");
        }
        break;
        case State::S1:{
            if(forward_<0){
                throw std::out_of_range("roolback fail,current S1");
            }
            forward_--;
        }
        break;
        case State::S2:{
            if(forward_<fence_){
                throw std::out_of_range("roolback fail,current S2");
            }
            forward_--;
        }
        break;
        case State::S3:{
            if(fence_-forward_>MaxTokenLen*2){
                throw std::out_of_range("roolback fail,current S3");
            }
            //[lexeme_begin_，0)
            forward_--;
        }
        break;
    }
}

bool MBuff::is_eof() const
{
    return (buff_.get())[forward_]==Eof;
}

/// 调用之前必须先打开,否则未定义
void MBuff::read(int begin, int length)
{
    auto pb=(buff_.get())+begin;
    f_.read(pb,length);
    auto c=f_.gcount();
//    if(f_.eof()){
//        if(c<length)
//            pb[c]=WEOF;
//    }
    if(c<length)
        pb[c]=WEOF;
}
