#include<cassert>
#include "inner_imp.h"
#include "lexer.h"
#include <limits>

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
//            using std::wcout;
//            wcout<<str.substr(0,i-pattern_len+1)<<L"("<<
//                  pattern<<L")"<<str.substr(i+1)<<std::endl;
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
//    for(int i=0;i<pattern_len;i++)
//        std::cout<<pi[i]<<" ";
//    std::cout<<std::endl;
}

FilterBuff::FilterBuff(std::unique_ptr<BuffBase> buff):b_(std::move(buff)),history_(1,1),p_begin_{1,1}
{
}

FilterBuff::~FilterBuff()
{

}

wchar_t FilterBuff::next_char()
{
    auto c=b_->next_char();
    number_++;
    if(c==L'\n'){
        //    \n \n  换行符
        //  5  2  1     数量
        //  0  1  2     数组索引
        //除第一次外，每次遇到换行就从数组下一位开始从1计数，直到遇到换行符，然后重复过程，数组的大小减一就是当前token的换行符个数
        //在回滚的时候从后往前，依次减掉回滚的个数，然后遇到换行符就把stat_.line_减一
        //所以上述图示表示,前5个字符不是换行符，第6个字符是换行符，第7个字符不是换行符，第8个是换行符
        //好吧 我描述的不太好，仔细想想就明白了
        history_.push_back(0);
    }
    history_.back()++;
    return c;
}

///这个函数实现有毒,我跪了
void FilterBuff::rollback_char(std::size_t len)
{
    if(number_<=len){
        this->rollback_all_chars();
        return;
    }
    number_-=len;
    b_->rollback_char(len);
    auto n=history_.size();
    long llen=len;
    assert(len <= std::numeric_limits<decltype(llen)>::max());
    do{
        n--;
        llen-=history_[n];  //不会出现llen=0&&n=0,这种情况属于上面的if的责任
    }while(llen>=0);
    //history_[n]就是stat_.column_curr_
    history_[n]=-llen;
    //实际上不用判断，直接操作，但一般来说，回滚只是一两个字符，
    //所以当n仅仅减少1时，直接跳过,不需要操作
    if(n<history_.size()-1){
        history_.resize(n+1);
    }
}

void FilterBuff::rollback_all_chars()
{
    b_->rollback_all_chars();
    number_=0;

    history_.resize(1);
    history_[0]=p_begin_.column_;
}

///和get_token()功能几乎相同,但丢弃缓冲区内的字符(隐含着当前处理完成,准备分析下个token)
void FilterBuff::discard_token()
{
    b_->discard_token();
}

///获取当前缓冲区的字符(假定它们被分析,且已经被认为是token了)
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
    number_=0;
    auto old_size=history_.size();
    //初始化history_就是初始化p_current_
    history_[0]=history_.back();
    history_.resize(1);

    p_begin_.column_=history_.back();
    p_begin_.line_+=old_size-1;
}

///这应该成为成员函数吗?
bool FilterBuff::test_and_skipBOM()
{
    wchar_t head[3]={};
    for (std::size_t i=0;i<3;i++){
        head[i]=b_->next_char();
        if(head[i]==MBuff::Eof_w)
            return true;
    }
    if(wcsncmp(head,L"\xEF\xBB\xBF",3)==0){
        b_->discard_token();
        return true;
    }
    b_->rollback_char(3);
    return false;
}

///*************************///
Location FunnyTokenGen::token_position() const
{
    return tk_begin_;
}

void FunnyTokenGen::next_()
{
    buff_->record_location();
    tk_begin_=buff_->begin_location();
    lexer_->next_token();
}

Token &FunnyTokenGen::current_()
{
    return lexer_->get_token();
}

namespace Inner {

Chunk::~Chunk()
{

}

void Chunk::init(std::size_t block_size, unsigned char n)
{
    assert(n>0&&block_size>0);
    //溢出检查
    assert(block_size*n/n==block_size);

    mem_=new unsigned char[block_size*n];
    list_head_=0;
    available_n_=n;

    //[0,n-1]共n个块,循环n次.n最大为255
    for(decltype (n) i=0;i<n;i++){
        *(mem_+i*block_size)=i+1;
    }
}

void Chunk::release()
{
    delete [] mem_;
}

void *Chunk::allocate(std::size_t block_size)
{
    assert(available_n_>0);
    assert(block_size>0);

    auto r=mem_+list_head_*block_size;
    list_head_=*r;
    available_n_--;
    return r;
}

void Chunk::deallocate(void *p, std::size_t block_size)
{
    auto ucp=static_cast<unsigned char *>(p);
    assert((ucp-mem_)%block_size==0);

    *ucp=list_head_;
    list_head_=static_cast<decltype (list_head_)>((ucp-mem_)/block_size);

    available_n_++;
}

std::size_t Chunk::available_size() const
{
    return available_n_;
}

bool Chunk::contain(void *p, std::size_t chunck_length) const
{
    auto where=static_cast<unsigned char *>(p);
    return p>=mem_&&p<mem_+chunck_length;
}

/***************FixedAllocator*******************/
FixedAllocator::FixedAllocator(std::size_t block_size):block_size_(block_size)
{

}

FixedAllocator::~FixedAllocator()
{
    release();
}

std::size_t FixedAllocator::block_size() const
{
    return block_size_;
}

void *FixedAllocator::allocate()
{
    if(pool_.size()>0){
        if(allocate_<pool_.size()&&pool_[allocate_].available_size()>0){
            return pool_[allocate_].allocate(block_size_);
        }else if(deallocate_!=allocate_&&pool_[deallocate_].available_size()>0){
            allocate_=deallocate_;
            return pool_[deallocate_].allocate(block_size_);
        }else
            ;//we need to find a free chunk
    }

    bool found=false;
    for(decltype (pool_)::size_type i=0;i<pool_.size();i++){
        if(pool_[allocate_].available_size()>0){
            allocate_=i;
            found=true;
            break;
        }
    }
    if(found){
        return pool_[allocate_].allocate(block_size_);
    }else{//we need allocate one another chunk
        if(add_chunck())//allocate_被add_chunck()修改了,所以作为index是安全的
            return pool_[allocate_].allocate(block_size_);
    }
    return nullptr;
}

///如果p不指向当前分配器中的内存,则不做任何事
void FixedAllocator::deallocate(void *p)
{
    assert(pool_.size()>0);

    std::size_t byte_sizes=block_size_*blocks_num_;

    if(pool_[deallocate_].contain(p,byte_sizes)){
        pool_[deallocate_].deallocate(p,block_size_);
    }else if(pool_[allocate_].contain(p,byte_sizes)){
        pool_[allocate_].deallocate(p,block_size_);
        deallocate_=allocate_;
    }else{
        decltype (deallocate_) left=deallocate_,right=deallocate_;
        decltype (pool_.size()) num=0;

        while (num<pool_.size()) {
            if(left>0){
                if(pool_[left].contain(p,byte_sizes)){
                    pool_[left].deallocate(p,block_size_);
                    deallocate_=left;
                    return;
                }else
                    left--;
                num++;
            }
            if(right<pool_.size()){
                if(pool_[right].contain(p,byte_sizes)){
                    pool_[right].deallocate(p,block_size_);
                    deallocate_=right;
                    return;
                }else
                    right++;
                num++;
            }
        }
    }
    if(pool_[deallocate_].available_size()==blocks_num_){
        std::swap(pool_[deallocate_],pool_.back());
        pool_.back().release();
        pool_.pop_back();
        deallocate_=0;
        allocate_--;
    }
}

void FixedAllocator::release()
{
    for(auto i=pool_.begin();i!=pool_.end();i++){
        i->release();
    }
}

bool FixedAllocator::add_chunck()
{
    Chunk chunk{};
    chunk.init(block_size_,blocks_num_);
    try {
        pool_.push_back(std::move(chunk));
    }  catch (...) {
        chunk.release();
        return false;
    }
    allocate_=pool_.size()-1;
    return true;
}

/*****************MyAllocator******************/
void * MyAllocator::allocate(std::size_t bytes)
{
    assert(bytes<=MaxObjSize);
    auto node=tree_.find(bytes);
    if(tree_.get_null()==node){
        node=tree_.insert(bytes,FixedAllocator(bytes));
        if(node==tree_.get_null())
            return nullptr;
    }
    return node->value_.allocate();
}

void MyAllocator::deallocate(void *p, std::size_t bytes)
{
    auto node=tree_.find(bytes);
    assert(node!=tree_.get_null());
    node->value_.deallocate(p);
}

}//namespace Inner end

}
