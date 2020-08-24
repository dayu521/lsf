#include "kmp.h"
#include<iostream>

KMP::KMP(const std::string & p_):pattern(p_)
{
    s=pattern.size();
    pi.resize(s);
    piFunc();
}

int KMP::match(const std::string &str) const
{
    int k=pi[0];
    for(std::size_t i=0;i<str.size();i++){
        while(k>0&&pattern[k]!=str[i])
            k=pi[k-1];
        if(pattern[k]==str[i])
            k++;
        if(k==s){
            using std::cout;
            cout<<str.substr(0,i-s+1)<<"("<<
                  pattern<<")"<<str.substr(i+1)<<std::endl;
            k=pi[k];
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
    for(int i=1;i<s;i++){
        while(k>0&&pattern[k]!=pattern[i]){
            k=pi[k-1];
        }
        if(pattern[k]==pattern[i]){
            k++;
        }
        pi[i]=k;
    }
    for(int i=0;i<s;i++)
        std::cout<<pi[i]<<" ";
    std::cout<<std::endl;
}

MBuff::MBuff()
{
    read(b);
    b[le-1]=Eof;
}

char MBuff::getnext()
{
    switch (b[forward]) {
    case Eof:if(forward==le-1){
            forward++;
            read(b+le);
            b[2*le-1]=Eof;
            state=State::S2;
        }else if(forward==2*le-1){
            forward++;
            read(b);
            b[le-1]=Eof;
            state=State::S4;
        }else{
            std::cout<<"词法分析结束"<<std::endl;
        }
    }
    char c=b[forward++];
    return c;
}

std::string MBuff::currentBytes()
{
    switch (state) {
    case State::S1:
    case State::S3:
        if(forward>=2*le)
            forward=-1;
        lexemeBegin=++forward;
        return std::string(b+lexemeBegin,forward-lexemeBegin);
    case State::S2:
        lexemeBegin=++forward;
        return std::string(b+lexemeBegin,le-2-lexemeBegin)+std::string(b+le-1,forward);
    case State::S4:
        lexemeBegin=++forward;
        return std::string(b+lexemeBegin,2*le-1)+std::string(b,forward);
    }
}

void MBuff::read(char *b, int length)
{
    char w=' ';
    srand(time(nullptr));
    if(al>length)
        al-=length;
    else{
        al=0;
        length=al;
    }
    for(int i=0;i<length;i++){
        int rnd=rand()%26;
        b[i]='a'+rnd;
        if(b[i]=='c')
            b[++i]=w;
    }
}
