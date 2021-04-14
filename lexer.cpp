#include "lexer.h"
#include <tuple>
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
    while (!input_->is_eof()) {
        if(c==L'\"'){
            std::wstring s{};
            while (c!=L'\"') {
                if(input_->get_char_count()>=MBuff::BuffLen)
                    s+=input_->current_chars();
                c=input_->next_char();
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

        if(c==L'0'){

        }else if(c>L'0'&&c<=L'9'){

        }

    }
    return Token{Type::END};
}

bool Lexer::get_string()
{

}

}
