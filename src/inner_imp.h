#ifndef INNER_IMP_H
#define INNER_IMP_H

#include<limits>

#include"mbuff.h"
#include"jsonparser.h"
#include"constant.h"

namespace lsf {

//本来以为用得着kmp算法，但没想到没用
class KMP
{
public:
    KMP(const std::wstring &p_);
    KMP(const KMP &)=delete ;
    int match(const std::wstring &s) const;
    ~KMP();
private:
    void piFunc();
private:
    std::wstring pattern{};
    int pattern_len{0};
    std::vector<int> pi{};
};

class FilterBuff:public BuffBase
{
public:
    virtual wchar_t next_char();
    virtual void rollback_char(std::size_t len=1);
    virtual void rollback_all_chars();
    virtual void discard_token();
    virtual std::wstring get_token();
public:
    FilterBuff(std::unique_ptr<BuffBase> buff );
    ~FilterBuff();
    ///调用前需要已经调用了this->discard_token()或this->get_token()
    Location begin_location();
    void record_location();
    bool test_and_skipBOM();
private:
    std::unique_ptr<BuffBase> b_;
    std::vector<std::size_t> history_{};
    //begin position
    Location p_begin_{};
    //current position
//    Location p_current_{};
    std::size_t number_{0};
};

class Lexer;

class FunnyTokenGen : public lsf::GenToken
{
public:
    FunnyTokenGen(std::shared_ptr<lsf::Lexer> l,std::shared_ptr<FilterBuff> b):lexer_(l),buff_(b),tk_begin_(b->begin_location()){}
    ///token 开始的位置,相对于文件的行与列
    Location token_position()const;
    // GenToken interface
public:
    virtual void next_() override;
    virtual lsf::Token &current_() override;
private:
    std::shared_ptr<lsf::Lexer> lexer_{};
    std::shared_ptr<FilterBuff> buff_{};
    Location tk_begin_{};
};

namespace Inner {

///Loki分配器

class Chunk final
{
public:
    Chunk()=default;
//    Chunk(const Chunk &)=delete;
//    Chunk(Chunk &&)=default;
//    Chunk &  operator= ( Chunk && ) = default;
    ~Chunk();

    void init(std::size_t block_size, unsigned char n );
    void release();

    void * allocate(std::size_t block_size);
    void deallocate(void * p,std::size_t block_size);

    std::size_t available_size()const;
    bool contain(void * p,std::size_t chunck_length)const;
private:
    unsigned char * mem_{};
    ///链表头的index
    unsigned char list_head_{};
    ///当前可用的empty块
    unsigned char available_n_{};
};

inline constexpr unsigned char BlockNum=std::numeric_limits<unsigned char>::max();

class FixedAllocator final
{
public:
    FixedAllocator(std::size_t block_size);
    ~FixedAllocator();
    FixedAllocator(const FixedAllocator &)=delete;

    std::size_t block_size()const;

    void * allocate();
    void deallocate(void * p);
private:
    std::size_t block_size_{};
    unsigned char blocks_num_=BlockNum;
    std::vector<Chunk> pool_{};

    std::size_t allocate_=0;
    std::size_t deallocate_=0;
private:
    void release();
    bool add_chunck();
};

}//namespace Inner end

}

#endif // IMP_H
