#include "lexer.h"

namespace lsf {

namespace Private {
//namespace start
bool deal_unicode_code_point(unsigned int &d, const std::wstring &s, unsigned long &index);

bool cp_to_utf8(unsigned int cp,wchar_t & u8);

bool wstring_to_double(const std::wstring & s,double & d);

bool deal_escape_char(const std::wstring &s,std::wstring & d)
{
    decltype (s.length()) i=0;
    wchar_t c;
    d.clear();
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
                wchar_t ec = L'\0';
                unsigned int code_point=0;
                if(deal_unicode_code_point(code_point,s,i)){
                    //必定是合法的utf码点，因此下面的转换一定成功
//                    if(cp_to_utf8(code_point,ec))
//                        d+=ec;
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

bool deal_unicode_code_point(unsigned int & d, const std::wstring & s, unsigned long &index)
{
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

void Lexer::set_file(const std::string &name)
{
    input_->open(name);
    input_->init();
}

bool Lexer::run()
{
    auto c=input_->next_char();
    if(input_->is_eof()){
        current_token_=Token{Type::END};
        return true;
    }
    if(c==L'/'){//注释
        return try_comment(c);

    }else if(c==L'\"'){//字符串
        std::wstring s{};
        c=input_->next_char();
        while (c!=L'\"') {
            if(c>=L'\u0020'&&c<=0x10FFFF){
                s+=c;
                c=input_->next_char();

            }else{               
                return false;
            }
        }
        input_->discard_token();
        std::wstring d{};
        Private::deal_escape_char(s,d);
        if(symbol_.contains(d))
            current_token_=Token{Type::KeyWord,d};
        current_token_=Token({Type::String,d});
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
        std::wstring s{};
        do{
            s+=c;
            c=input_->next_char();
        }while (c==L'\u0020'||c==L'\u000a'||c==L'\u000d'||c==L'\u0009');
        input_->roll_back_char();
        input_->discard_token();
        current_token_=Token{Type::WHITESPACE,s};
        goto T;
    }else if((c>=L'0'&&c<=L'9')||c==L'-'){
        return try_number(c);
    }else
        goto F;

F:  return false;
T:  return true;
}

Token &Lexer::get_token()
{
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
                        input_->roll_back_char();
                        current_token_=Token{Type::Number,input_->current_token()};
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
T:  input_->roll_back_char(end);
    current_token_=Token{Type::Number,input_->current_chars()};
    input_->discard_token();
    return true;
}

bool Lexer::try_comment(wchar_t c)
{
    std::wstring s{};
    s+=c;
    c=input_->next_char();
    if(c==L'*'){
        s+=c;
        c=input_->next_char();
        while (c!=MBuff::Eof) {
            while (c!=L'*'&&c!=MBuff::Eof) {
                s+=c;
                c=input_->next_char();
            }
            if(c==MBuff::Eof)
                break;
            do {
                s+=c;
                c=input_->next_char();
            } while (c==L'*');
            if(c==L'/')
                break;
            c=input_->next_char();
        }
        if(c==MBuff::Eof)
            return false;
        s+=c;
        current_token_=Token{Type::Comment,s};
        input_->discard_token();
        return true;
    }else if(c==L'/'){
        s+=c;
        c=input_->next_char();
        while (c!=L'\n'&&c!=L'\t'&&c!=L'\r') {
            if(c==MBuff::Eof)
                return false;
            s+=c;
            c=input_->next_char();
        }
        s+=c;
        current_token_=Token{Type::Comment,s};
        input_->discard_token();
        return true;
    }else{
//        input_->discard_token();
        return false;
    }
}

}
