module;
#include <cassert>
#include <string>
#include <memory>
#include <vector>
#include <limits>

export module lsf:inner_imp;

import :constant;
import :mbuff;
import :lexer;
import :tk_generator;

namespace lsf
{

    // 本来以为用得着kmp算法，但没想到没用
    class KMP
    {
    public:
        KMP(const std::wstring &p_);
        KMP(const KMP &) = delete;
        int match(const std::wstring &s) const;
        ~KMP();

    private:
        void piFunc();

    private:
        std::wstring pattern{};
        int pattern_len{0};
        std::vector<int> pi{};
    };

    class FilterBuff : public BuffBase
    {
    public:
        virtual wchar_t next_char();
        virtual void rollback_char(std::size_t len = 1);
        virtual void rollback_all_chars();
        virtual void discard_token();
        virtual std::wstring get_token();

    public:
        FilterBuff(std::unique_ptr<BuffBase> buff);
        FilterBuff() : FilterBuff(std::unique_ptr<BuffBase>(nullptr)) {}
        void set_buff_base(std::unique_ptr<BuffBase> buff);
        ~FilterBuff();
        /// 调用前需要已经调用了this->discard_token()或this->get_token()
        Location begin_location();
        void record_location();
        void init();

    private:
        std::unique_ptr<BuffBase> b_;
        std::vector<std::size_t> history_{};
        // begin position
        Location p_begin_{};
        // current position
        //    Location p_current_{};
        std::size_t number_{0};
    };

    class FunnyTokenGen : public lsf::GenToken
    {
    public:
        FunnyTokenGen(std::shared_ptr<lsf::Lexer> l, std::shared_ptr<FilterBuff> b) : lexer_(l), buff_(b), tk_begin_(b->begin_location()) {}

        // FunnyTokenGen(std::shared_ptr<lsf::Lexer> l):FunnyTokenGen(l,std::shared_ptr<FilterBuff>()){}
        /// token 开始的位置,相对于文件的行与列
        Location token_position() const;
        // GenToken interface
    public:
        virtual void next_() override;
        virtual lsf::Token &current_() override;

    private:
        std::shared_ptr<lsf::Lexer> lexer_{};
        std::shared_ptr<FilterBuff> buff_{};
        Location tk_begin_{};
    };

}

namespace lsf
{
    KMP::KMP(const std::wstring &p_) : pattern(p_)
    {
        pattern_len = pattern.size();
        pi.resize(pattern_len);
        piFunc();
    }

    int KMP::match(const std::wstring &str) const
    {
        int k = pi[0];
        for (std::size_t i = 0; i < str.size(); i++)
        {
            while (k > 0 && pattern[k] != str[i])
                k = pi[k - 1];
            if (pattern[k] == str[i])
                k++;
            if (k == pattern_len)
            {
                //            using std::wcout;
                //            wcout<<str.substr(0,i-pattern_len+1)<<L"("<<
                //                  pattern<<L")"<<str.substr(i+1)<<std::endl;
                //            k=pi[k];
                //            break;
                k = pi[0];
            }
        }
        return str.size() - k;
    }

    KMP::~KMP()
    {
    }

    void KMP::piFunc()
    {
        pi[0] = 0;
        int k = pi[0];
        for (int i = 1; i < pattern_len; i++)
        {
            while (k > 0 && pattern[k] != pattern[i])
            {
                k = pi[k - 1];
            }
            if (pattern[k] == pattern[i])
            {
                k++;
            }
            pi[i] = k;
        }
        //    for(int i=0;i<pattern_len;i++)
        //        std::cout<<pi[i]<<" ";
        //    std::cout<<std::endl;
    }

    FilterBuff::FilterBuff(std::unique_ptr<BuffBase> buff) : b_(std::move(buff)), history_(1, 1), p_begin_{1, 1}
    {
    }

    void FilterBuff::set_buff_base(std::unique_ptr<BuffBase> buff)
    {
        b_ = std::move(buff);
    }

    FilterBuff::~FilterBuff()
    {
    }

    wchar_t FilterBuff::next_char()
    {
        auto c = b_->next_char();
        number_++;
        if (c == L'\n')
        {
            //    \n \n  换行符
            //  5  2  1     数量
            //  0  1  2     数组索引
            // 除第一次外，每次遇到换行就从数组下一位开始从1计数，直到遇到换行符，然后重复过程，数组的大小减一就是当前token的换行符个数
            // 在回滚的时候从后往前，依次减掉回滚的个数，然后遇到换行符就把stat_.line_减一
            // 所以上述图示表示,前5个字符不是换行符，第6个字符是换行符，第7个字符不是换行符，第8个是换行符
            // 好吧 我描述的不太好，仔细想想就明白了
            history_.push_back(0);
        }
        history_.back()++;
        return c;
    }

    /// 这个函数实现有毒,我跪了
    void FilterBuff::rollback_char(std::size_t len)
    {
        if (number_ <= len)
        {
            this->rollback_all_chars();
            return;
        }
        number_ -= len;
        b_->rollback_char(len);
        auto n = history_.size();
        long llen = len;
        assert(len <= std::numeric_limits<decltype(llen)>::max());
        do
        {
            n--;
            llen -= history_[n]; // 不会出现llen=0&&n=0,这种情况属于上面的if的责任
        } while (llen >= 0);
        // history_[n]就是stat_.column_curr_
        history_[n] = -llen;
        // 实际上不用判断，直接操作，但一般来说，回滚只是一两个字符，
        // 所以当n仅仅减少1时，直接跳过,不需要操作
        if (n < history_.size() - 1)
        {
            history_.resize(n + 1);
        }
    }

    void FilterBuff::rollback_all_chars()
    {
        b_->rollback_all_chars();
        number_ = 0;

        history_.resize(1);
        history_[0] = p_begin_.column_;
    }

    /// 和get_token()功能几乎相同,但丢弃缓冲区内的字符(隐含着当前处理完成,准备分析下个token)
    void FilterBuff::discard_token()
    {
        b_->discard_token();
    }

    /// 获取当前缓冲区的字符(假定它们被分析,且已经被认为是token了)
    std::wstring FilterBuff::get_token()
    {
        return b_->get_token();
    }

    Location FilterBuff::begin_location()
    {
        return p_begin_;
    }

    void FilterBuff::record_location()
    {
        number_ = 0;
        auto old_size = history_.size();
        // 初始化history_就是初始化p_current_
        history_[0] = history_.back();
        history_.resize(1);

        p_begin_.column_ = history_.back();
        p_begin_.line_ += old_size - 1;
    }

    ///*************************///
    Location FunnyTokenGen::token_position() const
    {
        return tk_begin_;
    }

    void FunnyTokenGen::next_()
    {
        buff_->record_location();
        tk_begin_ = buff_->begin_location();
        lexer_->next_token();
    }

    Token &FunnyTokenGen::current_()
    {
        return lexer_->get_token();
    }
} // namespace lsf
