#include<cassert>
#include "inner_imp.h"
#include "lexer.h"
#include <iostream>

namespace lsf {

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

///*************************///
void Ltokens::next_()
{
    lexer_->next_token();
}

Token &Ltokens::current_()
{
    return lexer_->get_token();
}

}
