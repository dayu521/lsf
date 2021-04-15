#include "lexer.h"
#include <tuple>
#include <iostream>
namespace lsf {

lsf::Lexer::Lexer(std::unique_ptr<MBuff> input):input_(std::move(input))
{
    const wchar_t * l[]={
        L"true",
        L"false",
        L"null"
    };
    for(auto key_word:l)
        symbol_.insert(key_word);
}

Token Lexer::next()
{
    auto c=input_->next_char();
    L:if (!input_->is_eof()) {
        if(c==L'/'){//注释
            c=input_->next_char();
            if(c==L'*'){
                input_->discard_token();
            }else if(c==L'/'){
                while (c!=L'\n'&&c!=L'\t'&&c!=L'\r') {
                    c=input_->next_char();
                }               
            }else{
                std::wcout<<L"错误"<<std::endl;//错误
                error=true;
            }
            input_->discard_token();
        }
        if(c==L'\"'){//字符串
            std::wstring s{};
            c=input_->next_char();
            input_->discard_token();
            while (c!=L'\"') {               
                if(c==L'\\'){//转义字符
                    c=input_->next_char();
                    input_->discard_token();
                    switch (c) {
                    case L'\"':
                        s+=L'\"';
                        break;
                    case L'\\':
                        s+=L'\\';
                        break;
                    case L'/':
                        s+=L'/';
                        break;
                    case L'b':
                        s+=L'\b';
                        break;
                    case L'f':
                        s+=L'\f';
                        break;
                    case L'n':
                        s+=L'\n';
                        break;
                    case L'r':
                        s+=L'\r';
                        break;
                    case L't':
                        s+=L'\t';
                        break;
                    case L'u':
                        for(int i=0;i<4;i++){
                            c=input_->next_char();
//                            input_->discard_token();
                            if((c>=L'A'&&c<=L'F')||(c>=L'0'&&c<=L'9')||(c>=L'a'&&c<=L'f'))
                                continue;
                            error=true;
                            break;
                        }
                        s+=input_->current_chars();
                        break;
                    default:
                        std::wcout<<L"错误"<<std::endl;//错误
                        error=true;
                    }
                }else if(c>=L'\u0020'&&c<=0x10FFFF){
                    s+=c;
                }else{
                    std::wcout<<L"错误"<<std::endl;//错误
                    error=true;
                }
                c=input_->next_char();
                input_->discard_token();
            }
            if(symbol_.contains(s))
                return Token{Type::KeyWord,s};
            return Token({Type::String,s});
        }else if(c==L'{')
            return Token{Type::LBRACE,L"{"};
        else if(c==L'}')
            return Token{Type::RBRACE,L"}"};
        else if(c==L'[')
            return Token{Type::LSQUARE,L"["};
        else if(c==L']')
            return Token{Type::RSQUARE,L"]"};
        else if(c==L':')
            return Token{Type::COLON,L":"};
        else if(c==L',')
            return Token{Type::COMMA,L","};
        else if(c==L'\u0020'||c==L'\u000a'||c==L'\u000d'||c==L'\u0009'){
            while (c==L'\u0020'||c==L'\u000a'||c==L'\u000d'||c==L'\u0009') {
                c=input_->next_char();
                input_->discard_token();
            }
            goto L;
        }else
            ;//出错

    }
    return Token{Type::END};
}

bool Lexer::has_error()
{
    auto old=error;
    error=false;
    return old;
}

bool Lexer::get_string()
{

}

}
