module;
#include <string>
// #include <fstream>
#include <array>
#include <cassert>
#include <memory>
#include <utility>
#include <stdexcept>

#ifndef BUFFER_ARRAY_SIZE
#define BUFFER_ARRAY_SIZE 1024
#endif

export module lsf:mbuff;

import :tp;

namespace lsf
{

    class BuffBase
    {
    public:
        BuffBase() {}
        virtual ~BuffBase() {}
        virtual wchar_t next_char() = 0;
        virtual void rollback_char(std::size_t len = 1) = 0;
        virtual void rollback_all_chars() = 0;
        virtual void discard_token() = 0;
        virtual std::wstring get_token() = 0;
        static constexpr auto Eof_w = WEOF;
        static constexpr std::size_t BuffLen = BUFFER_ARRAY_SIZE;
    };

    template <InputSource T>
    class MBuff final : public BuffBase
    {

    public:
        virtual wchar_t next_char();
        virtual void rollback_char(std::size_t len = 1);
        virtual void rollback_all_chars();
        virtual void discard_token();
        virtual std::wstring get_token();

    public:
        MBuff();
        ~MBuff();
        MBuff(std::unique_ptr<T> input_stream);
        MBuff(const MBuff &) = delete;

        std::wstring current_chars();

        wchar_t current_char() const;
        bool is_eof() const;

        void init();

    private:
        int fence_{0};
        int lexeme_begin_{0};
        int forward_{-1};

        // 2*BuffLen
        std::array<wchar_t, 2 * BuffLen> buff_{};

        std::unique_ptr<T> input_stream_;

        enum class State
        {
            S0,
            S1,
            S2,
            S3
        };
        // fence,forward分别代表fence和forward
        //   init state            S0
        // fence forward |         S1
        // fence | forward         S2
        // forward | fence         S3
        State state_{State::S0};

    private:
        void read(int begin, int length = BuffLen);
    };
}

namespace lsf
{

    namespace Private
    {

    }

    template <InputSource T>
    MBuff<T>::MBuff() : state_(State::S0)
    {
    }

    template <InputSource T>
    MBuff<T>::~MBuff()
    {
    }

    template <InputSource T>
    MBuff<T>::MBuff(std::unique_ptr<T> input_stream) : input_stream_(std::move(input_stream)),
                                                       state_(State::S0)
    {
        input_stream_->before_read();

        // 跳过utf-8 bom头
        wchar_t head[3] = {};
        for (std::size_t i = 0; i < 3; i++)
        {
            head[i] = MBuff::next_char();
            if (head[i] == BuffBase::Eof_w)
                ;
        }
        if (wcsncmp(head, L"\xEF\xBB\xBF", 3) == 0)
        {
            MBuff::discard_token();
            
        }
        MBuff::rollback_char(3);
    
    }

    /// 因为当缓冲区重新填充时，当前已分析的这些字符被新读取的字符覆盖,且lexeme_begin_也表示的是原先分析的token的开始处,
    /// 因此需要在合适的时候调用current_token或discard来消耗当前已分析的字符.
    /// 否则current_token返回的是被污染的token
    template <InputSource T>
    wchar_t MBuff<T>::next_char()
    {
        // 无法达到2*BuffLen，因为在这之前已经被转移到下一状态了，从而forward_和lexeme_begin_被重置了
        // exceed the max length of token,should not peek next char
        //    assert(forward_-lexeme_begin_<2*BuffLen-1);
        switch (state_)
        {
        case State::S0:
        {
            lexeme_begin_ = forward_ = fence_ = 0;
            read(forward_);
            state_ = State::S1;
        }
        break;
        case State::S1:
        case State::S3:
        {
            forward_++;
            if (forward_ == BuffLen)
            {
                fence_ = 0;
                read(forward_);
                state_ = State::S2;
            }
        }
        break;
        case State::S2:
        {
            forward_++;
            if (forward_ == 2 * BuffLen)
            {
                forward_ = 0;
                fence_ = BuffLen;
                // 因此,在S3中所有索引都是负的,从而任何时刻
                // lexeme_begin_和forward_都是相同符号
                lexeme_begin_ = lexeme_begin_ - 2 * BuffLen;
                read(forward_);
                state_ = State::S3;
            }
        }
        break;
        }
        return buff_[(forward_ + BuffLen * 2) % (BuffLen * 2)];
    }

    template <InputSource T>
    wchar_t MBuff<T>::current_char() const
    {
        // 体会这种思想
        return buff_[(forward_ + BuffLen * 2) % (BuffLen * 2)];
    }

    template <InputSource T>
    std::wstring MBuff<T>::current_chars()
    {
        assert(state_ != State::S0); // there is not currently token,current S0
        if (state_ == State::S3)
        {
            std::wstring s(forward_ - lexeme_begin_ + 1, wchar_t());
            if (lexeme_begin_ >= 0 || forward_ < 0)
                s.assign(buff_.data() + (lexeme_begin_ + BuffLen * 2) % (BuffLen * 2), (-lexeme_begin_) + forward_ + 1);
            else
            {
                s.assign(buff_.data() + lexeme_begin_ + BuffLen * 2, (-lexeme_begin_));
                s.append(buff_.data(), forward_ + 1);
            }
            return s;
        }
        else
            return std::wstring(buff_.data() + lexeme_begin_, (-lexeme_begin_) + forward_ + 1);
    }

    template <InputSource T>
    std::wstring MBuff<T>::get_token()
    {
        assert(state_ != State::S0);
        auto s = current_chars();
        lexeme_begin_ = forward_ + 1;
        return s;
    }

    template <InputSource T>
    void MBuff<T>::discard_token()
    {
        // can not discard,we are in S0
        assert(state_ != State::S0);
        lexeme_begin_ = forward_ + 1;
    }

    /// 回滚不超过1个token
    /// 溢出未考虑，留给调用者自己决定
    template <InputSource T>
    void MBuff<T>::rollback_char(std::size_t len)
    {
        // roolback fail
        assert(state_ != State::S0);

        if (len > forward_ - (lexeme_begin_ - 1))
            forward_ = lexeme_begin_ - 1;
        else
            forward_ -= len;
    }

    template <InputSource T>
    void MBuff<T>::rollback_all_chars()
    {
        forward_ = lexeme_begin_ - 1;
    }

    template <InputSource T>
    bool MBuff<T>::is_eof() const
    {
        return buff_[forward_] == Eof_w;
    }

    template <InputSource T>
    void MBuff<T>::init()
    {
        state_ = State::S0;
        forward_ = -1;
        lexeme_begin_ = fence_ = 0;
    }

    template <InputSource T>
    void MBuff<T>::read(int begin, int length)
    {
        auto pb = &buff_[0] + begin;
        auto size = input_stream_->read(pb, length);
        if (size < 0)
            throw std::runtime_error("read file failed!");
        auto c = size;
        if (c < length){
            pb[c] = Eof_w;
            input_stream_->after_read();
        }
    }

}
