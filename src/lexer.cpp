#include <assert.h>
#include "lexer.h"
#include "mbuff.h"

namespace lsf {

namespace Private {
//namespace start
bool deal_unicode_code_point(wchar_t &d, const std::wstring &s, size_t  &index);

bool cp_to_utf8(unsigned int cp,wchar_t & u8);

bool wstring_to_double(const std::wstring & s,double & d);

bool deal_escape_char(const std::wstring &s,std::wstring & d)
{
    decltype (s.length()) i=0;
    wchar_t c;
    d.clear();
    wchar_t code_point=0;
    while (i<s.length()) {
        c=s[i++];
        if(c==L'\\'){
            c=s[i++];
            switch (c) {
            case L'\"':
                d+=L'\"';
                break;
            case L'\\':
                d+=L'\\';
                break;
            case L'/':
                d+=L'/';
                break;
            case L'b':
                d+=L'\b';
                break;
            case L'f':
                d+=L'\f';
                break;
            case L'n':
                d+=L'\n';
                break;
            case L'r':
                d+=L'\r';
                break;
            case L't':
                d+=L'\t';
                break;
            case L'u':{
                if(deal_unicode_code_point(code_point,s,i)){
                    d+=code_point;
                }
                else
                    return false;
                break;
            }
            default:
                return false;
            }
        }
        else
            d+=c;
//        c=s[i++];
    }
    return true;
}

bool deal_unicode_code_point(wchar_t &d, const std::wstring & s, size_t &index)
{
    if(s.length()-index<4)
        return false;
    unsigned int high=0,low=0;
    wchar_t c{};
    for(int i=0;i<4;i++){
        c=s[index++];
        high*=16;
        if(c>=L'A'&&c<=L'F'){
            high+=c-L'A'+10;
        }else if(c>=L'a'&&c<=L'f'){
            high+=c-L'a'+10;
        }else if(c>=L'0'&&c<=L'9'){
            high+=c-L'0';
        }else
            return false;
    }
    d=high;
    if(high>=0xD800 && high<=0xDBFF){
        if(s.length()-index<6)
            return false;
        c=s[index++];
        if(c==L'\\'){
            c=s[index++];
            if(c==L'u'){
                for(int i=0;i<4;i++){
                    c=s[index++];
                    low*=16;
                    if(c>=L'A'&&c<=L'F'){
                        low+=c-L'A'+10;
                    }else if(c>=L'a'&&c<=L'f'){
                        low+=c-L'a'+10;
                    }else if(c>=L'0'&&c<=L'9'){
                        low+=c-L'0';
                    }else
                        return false;
                }
                if(low>=0xdc00&&low<=0xdfff){
                    //https://en.wikipedia.org/wiki/Universal_Character_Set_characters#Surrogates
                    d=0x10000+(high-0xd800)*0x400+(low-0xdc00);
                    return true;
                }
                return false;
            }
            return false;
        }
        return false;
    }
    return true;
}

//用不着
bool cp_to_utf8(unsigned int cp,wchar_t & u8)
{
    if(cp<=0x007f){
        char bb[]={char(cp)};
        mbtowc(&u8,bb,1);
    }else if(cp<=0x07ff){
        char bb[]={
            (char)(0xc0+(cp>>6)),
            (char)(0x80+(cp&0x3f))
        };
        mbtowc(&u8,bb,2);
    }else if(cp<=0xffff){
        char bb[]={
            (char)(0xe0+(cp>>12)),
            (char)(0x80+((cp>>6)&0x3f)),
            (char)(0x80+(cp&0x3f))
        };
        mbtowc(&u8,bb,3);
    }else if(cp<=0x10ffff){
        char bb[]={
            (char)(0xf0+(cp>>18)),
            (char)(0x80+((cp>>12)&0x3f)),
            (char)(0x80+((cp>>6)&0x3f)),
            (char)(0x80+(cp&0x3f))
        };
        mbtowc(&u8,bb,4);
    }else
        return false;
    return true;
}

//namespace end
}

lsf::Lexer::Lexer(std::shared_ptr<lsf::BuffBase> input):input_(input)
{
    const wchar_t * l[]={
        L"true",
        L"false",
        L"null"
    };
    for(auto key_word:l)
        symbol_.insert(key_word);
}

Token &Lexer::next_token()
{
    //  可以不抛异常，但定义一个不是任何Token类型的ErrorToken类型，这样，语法解析的时候
    //  就会对这个错误做特别处理。因为它不是任何词法单元，所以不会被认为是语法错误
    //  从而在错误消息中区别到底词法还是语法部分出了问题
    if(!run()){        
        has_error_=true;
        throw LexerError("词法分析出错\n");
    }
    return current_token_;
}

bool Lexer::run()
{
    auto c=input_->next_char();
    if(c==BuffBase::Eof_w){
        current_token_=Token{Type::END};
        return true;
    }
    if(c==L'/'){//注释
        current_token_.type_=Type::Comment;
        return try_comment(c);

    }else if(c==L'\"'){//字符串
        current_token_.type_=Type::String;
        auto & s=current_token_.value_;
        s.clear();
        c=input_->next_char();
        while (c!=L'\"') {
            if(c>=L'\u0020'&&c<=0x10FFFF){
                s+=c;
                c=input_->next_char();
            }else{//包含eof
                return false;
            }
        }
        std::wstring d{};
        if(!Private::deal_escape_char(s,d)){
            input_->discard_token();
            return false;
        }
        current_token_.value_=d;
        input_->discard_token();
        return true;
    }else if(c==L'{'){
        current_token_=Token{Type::LBRACE,L"{"};
        input_->discard_token();
        goto T;
    }
    else if(c==L'}'){
        current_token_=Token{Type::RBRACE,L"}"};
         input_->discard_token();
        goto T;
    }
    else if(c==L'['){
        current_token_=Token{Type::LSQUARE,L"["};
         input_->discard_token();
        goto T;
    }
    else if(c==L']'){
        current_token_=Token{Type::RSQUARE,L"]"};
         input_->discard_token();
        goto T;
    }
    else if(c==L':'){
        current_token_=Token{Type::COLON,L":"};
         input_->discard_token();
        goto T;
    }
    else if(c==L','){
        current_token_=Token{Type::COMMA,L","};
         input_->discard_token();
        goto T;
    }
    else if(c==L'\u0020'||c==L'\u000a'||c==L'\u000d'||c==L'\u0009'){
        current_token_.type_=Type::WHITESPACE;
        auto & s=current_token_.value_;
        s.clear();
        do{
            s+=c;
            c=input_->next_char();
        }while (c==L'\u0020'||c==L'\u000a'||c==L'\u000d'||c==L'\u0009');
        input_->rollback_char();
        input_->discard_token();
        goto T;
    }else if((c>=L'0'&&c<=L'9')||c==L'-'){
        current_token_.type_=Type::Number;
        return try_number(c);
    }else if(c>=L'\u0020'&&c<=0x10FFFF){
        current_token_.type_=Type::KeyWord;
        auto & s=current_token_.value_;
        s.clear();
        if(c==L'_'){
            s+=c;
            c=input_->next_char();
        }
        while ((c>=L'a'&&c<=L'z')||(c>=L'A'&&c<=L'Z')||c==L'_') {
            s+=c;
            c=input_->next_char();
        }
        input_->rollback_char();
#if __cplusplus >= 202002L
        auto haskey=symbol_.contains(s);
#else
        auto haskey=symbol_.end()!=symbol_.find(s);
#endif
        if(haskey){
            if(s==L"null")
                current_token_.type_=Type::Null;
            input_->discard_token();
            goto T;
        }
        else{
            input_->rollback_char(s.size());
            goto F;
        }
    }else
        goto F;

F:  return false;
T:  return true;
}

Token &Lexer::get_token()
{
    return current_token_;
}

const Token &Lexer::get_error()
{
    assert(has_error_==true);
    return current_token_;
}

bool Lexer::try_number(wchar_t c)
{
    auto end=0;
    if(c==L'-'){
        c=input_->next_char();
//        end++;
    }
    if(c>=L'0'&&c<=L'9'){
        if(c==L'0'){
            c=input_->next_char();
//            end++;
        }
        else{
            do{
                c=input_->next_char();
//                end++;
            }while (c>=L'0'&&c<=L'9');
        }
        //可接受状态
        end=1;
        if(c==L'.'){
            c=input_->next_char();
            end++;
            if(c>=L'0'&&c<=L'9'){
                do{
                    c=input_->next_char();
                    end++;
                }while(c>=L'0'&&c<=L'9');
                //可接受状态
                end=1;
                if(c==L'e'||c==L'E'){
                    c=input_->next_char();
                    end++;
                    if(c==L'+'||c==L'-'){
                        c=input_->next_char();
                        end++;
                    }
                    if(c>=L'0'&&c<=L'9'){
                        do{
                            c=input_->next_char();
                            end++;
                        }while(c>=L'0'&&c<=L'9');
                        input_->rollback_char();
                        current_token_.value_=input_->get_token();
                        return true;
                    }else
                        goto T;
                }else{
                    goto T;
                }
            }else
                goto T;
        }else
            goto T;
    }else
        goto F;

F:  return false;
T:  input_->rollback_char(end);
    current_token_.value_=input_->get_token();
    return true;
}

bool Lexer::try_comment(wchar_t c)
{
    auto & s=current_token_.value_;
    s.clear();
    s+=c;
    c=input_->next_char();
    if(c==L'*'){
        s+=c;
        c=input_->next_char();
        while (true) {
            if(c==MBuff::Eof_w)
                goto F;
            if(c==L'*'){
                s+=c;
                c=input_->next_char();
                if(c==L'/')
                    break;
            }
            s+=c;
            c=input_->next_char();
        }
        s+=c;
        input_->discard_token();
        return true;
    }else if(c==L'/'){
        s+=c;
        c=input_->next_char();
        while (true) {
            if(c==MBuff::Eof_w)
                goto F;
            if(c==L'\n'||c==L'\t'||c==L'\r')
                break;;
            s+=c;
            c=input_->next_char();
        }

        input_->rollback_char();
        input_->discard_token();
        return true;
    }else
        ;
F:  return false;
}

}
