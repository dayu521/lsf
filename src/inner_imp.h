#ifndef INNER_IMP_H
#define INNER_IMP_H

#include<limits>
#include<queue>
#include<cassert>

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
    FixedAllocator()=default;
    ~FixedAllocator();
//    FixedAllocator(const FixedAllocator &)=delete;

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

///BinarySearchTree
template<typename T,typename V>
class BST
{
public:
    struct Node
    {
        Node * left_=nullptr;
        Node * right_=nullptr;
        T key_;
        V value_;
    };

public:
    BST()
    {
        root_=null_=new Node{};
        null_->left_=null_->right_=null_;
    }
    ~BST(){release();}

    Node * insert(const T & key,V v);
    bool remove(const T & key);
    Node * & find(const T & key);
    Node * & find_min(Node * &root)
    {
        auto * it=&root;
        while ((*it)->left_!=null_) {
            it=&((*it)->left_);
        }
        return *it;
    }
    Node * & find_max(Node * &root)
    {
        auto * it=&root;
        while ((*it)->right_!=null_) {
            it=&((*it)->right_);
        }
        return *it;
    }
    const Node * get_null()const
    {
        return null_;
    }
private:
    bool insert(Node * & root, std::pair<const T &,V &> pair);
    bool remove(Node * & root, const T & key);
    void release();
private:
    Node * root_=nullptr;
    Node * null_=nullptr;
};

template<typename T, typename V>
typename BST<T, V>::Node * BST<T, V>::insert(const T &key, V v)
{
    auto & target_node=find(key);
    if(target_node!=null_)
        return null_;
    target_node=new Node{null_,null_,key,std::move(v)};
    return target_node;
}

template<typename T, typename V>
bool BST<T, V>::remove(const T &key)
{
    auto & target_node=find(key);
    if(target_node==null_)
        return false;
    if(target_node->left_!=null_&&target_node->right_!=null_){
        auto & min=find_min(target_node->right_);
        auto old=target_node;
        target_node=min;
        min=min->right_;
        target_node->left_=old->left_;
        target_node->right_=old->right_;
        delete old;
        return true;
    }else{
        auto old=target_node;
        if(target_node->left_==null_){
            target_node=target_node->right_;
        }else
            target_node=target_node->left_;
        delete old;
        return true;
    }
}

template<typename T, typename V>
void BST<T, V>::release()
{
    std::queue<Node *> q{};
    if(root_!=null_)
        q.push(root_);
    while (q.size()>0) {
        auto c=q.front();
        q.pop();
        if(c->left_!=null_){
            q.push(c->left_);
        }
        if(c->right_!=null_){
            q.push(c->right_);
        }
        delete c;
    }
    delete null_;
    root_=null_=nullptr;
}

template<typename T, typename V>
typename BST<T,V>::Node * & BST<T, V>::find(const T &key)
{
    auto * iterator=&root_;
    while (*iterator!=null_) {
        if(key>(*iterator)->key_){
            iterator=&((*iterator)->right_);
        }else if(key<(*iterator)->key_){
            iterator=&((*iterator)->left_);
        }else
            break;
    }
    return *iterator;
}

template<typename T>
class SingletonHolder
{
public:
    ~SingletonHolder()
    {
        assert(!destroyed_);
        p_ = nullptr;
        destroyed_ = true;
    }
    static T & instance();
private:
    template<typename U>
    friend  U & get_singleton();

    SingletonHolder()
    {
        assert(p_==nullptr&&destroyed_==false);
    }
private:
    inline thread_local static T * p_=nullptr;
    inline thread_local static bool destroyed_=false;
};

template<typename T>
T &SingletonHolder<T>::instance()
{
    if (!p_)
    {
        if(destroyed_){
            throw std::runtime_error("This singleton case has been destroyed");
        }
        thread_local static T instance{};
        p_=&instance;
    }
    return *p_;
}

template<typename T>
T & get_singleton()
{
    static thread_local SingletonHolder<T> sh{};
    return SingletonHolder<T>::instance();
}

class MyAllocator
{
public:
    void * allocate(std::size_t bytes);
    void deallocate(void * p, std::size_t bytes);

    constexpr static std::size_t MaxObjSize=128;
private:
    BST<std::size_t,FixedAllocator> tree_{};
};

}//namespace Inner end

}

#endif // IMP_H
