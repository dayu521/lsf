module;
#include <limits>
#include <queue>
#include <stack>
#include <functional>
#include <cassert>
#include <string>
#include <memory>

export module lsf:inner_imp;

import :constant;
import :mbuff;
import :lexer;
// import :jsonparser;
// import :analyze;

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
        ~FilterBuff();
        /// 调用前需要已经调用了this->discard_token()或this->get_token()
        Location begin_location();
        void record_location();
        bool test_and_skipBOM();

    private:
        std::unique_ptr<BuffBase> b_;
        std::vector<std::size_t> history_{};
        // begin position
        Location p_begin_{};
        // current position
        //    Location p_current_{};
        std::size_t number_{0};
    };

    class Lexer;

    struct GenToken
    {
        virtual ~GenToken() {}
        virtual void next_() = 0;
        virtual Token &current_() = 0;
    };

    class FunnyTokenGen : public lsf::GenToken
    {
    public:
        FunnyTokenGen(std::shared_ptr<lsf::Lexer> l, std::shared_ptr<FilterBuff> b) : lexer_(l), buff_(b), tk_begin_(b->begin_location()) {}
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

    namespace Inner
    {

        /// Loki分配器

        class Chunk final
        {
        public:
            ~Chunk();

            void init(std::size_t block_size, unsigned char n);
            void release();

            void *allocate(std::size_t block_size);
            void deallocate(void *p, std::size_t block_size);

            std::size_t available_size() const;
            bool contain(void *p, std::size_t chunck_length) const;

        private:
            unsigned char *mem_{};
            /// 链表头的index
            unsigned char list_head_{};
            /// 当前可用的empty块
            unsigned char available_n_{};
        };

        inline constexpr unsigned char BlockNum = std::numeric_limits<unsigned char>::max();

        class FixedAllocator final
        {
        public:
            FixedAllocator(std::size_t block_size);
            FixedAllocator() = default;
            ~FixedAllocator();
            FixedAllocator(const FixedAllocator &) = delete;
            FixedAllocator(FixedAllocator &&) = default;

            std::size_t block_size() const;

            void *allocate();
            void deallocate(void *p);

        private:
            std::size_t block_size_{};
            unsigned char blocks_num_ = BlockNum;
            std::vector<Chunk> pool_{};

            std::size_t allocate_ = 0;
            std::size_t deallocate_ = 0;

        private:
            void release();
            bool add_chunck();
        };

        /// BinarySearchTree
        template <typename T, typename V>
        class BST
        {
        public:
            struct Node
            {
                Node *left_ = nullptr;
                Node *right_ = nullptr;
                T key_;
                V value_;
            };

        public:
            BST()
            {
                root_ = null_ = new Node{};
                null_->left_ = null_->right_ = null_;
            }
            ~BST() { release(); }

            Node *insert(const T &key, V v);
            bool remove(const T &key);
            Node *&find(const T &key);
            Node *&find_min(Node *&root)
            {
                auto *it = &root;
                while ((*it)->left_ != null_)
                {
                    it = &((*it)->left_);
                }
                return *it;
            }
            Node *&find_max(Node *&root)
            {
                auto *it = &root;
                while ((*it)->right_ != null_)
                {
                    it = &((*it)->right_);
                }
                return *it;
            }
            const Node *get_null() const
            {
                return null_;
            }

            template <typename Fun>
            void set_fun(Fun &&f) { fun_ = f; }

            void inorder_traversal();
            void preorder_traversal();
            void postorder_traversal();

            void inorder_recursion()
            {
                assert(fun_);
                inorder_recursion(root_);
            }
            void preorder_recursion()
            {
                assert(fun_);
                preorder_recursion(root_);
            }
            void postorder_recursion()
            {
                assert(fun_);
                postorder_recursion(root_);
            }

        private:
            void inorder_recursion(Node *root);
            void preorder_recursion(Node *root);
            void postorder_recursion(Node *root);
            void release();

        private:
            Node *root_ = nullptr;
            Node *null_ = nullptr;
            std::function<void(const T &key, const V &value)> fun_{};
        };

        template <typename T, typename V>
        typename BST<T, V>::Node *BST<T, V>::insert(const T &key, V v)
        {
            auto &target_node = find(key);
            if (target_node != null_)
                return null_;
            target_node = new Node{null_, null_, key, std::move(v)};
            return target_node;
        }

        template <typename T, typename V>
        bool BST<T, V>::remove(const T &key)
        {
            auto &target_node = find(key);
            if (target_node == null_)
                return false;
            if (target_node->left_ != null_ && target_node->right_ != null_)
            {
                auto &min = find_min(target_node->right_);
                auto old = target_node;
                target_node = min;
                min = min->right_;
                target_node->left_ = old->left_;
                target_node->right_ = old->right_;
                delete old;
                return true;
            }
            else
            {
                auto old = target_node;
                if (target_node->left_ == null_)
                {
                    target_node = target_node->right_;
                }
                else
                    target_node = target_node->left_;
                delete old;
                return true;
            }
        }

        template <typename T, typename V>
        void BST<T, V>::release()
        {
            std::queue<Node *> q{};
            if (root_ != null_)
                q.push(root_);
            while (q.size() > 0)
            {
                auto c = q.front();
                q.pop();
                if (c->left_ != null_)
                {
                    q.push(c->left_);
                }
                if (c->right_ != null_)
                {
                    q.push(c->right_);
                }
                delete c;
            }
            delete null_;
            root_ = null_ = nullptr;
        }

        template <typename T, typename V>
        typename BST<T, V>::Node *&BST<T, V>::find(const T &key)
        {
            auto *iterator = &root_;
            while (*iterator != null_)
            {
                if (key > (*iterator)->key_)
                {
                    iterator = &((*iterator)->right_);
                }
                else if (key < (*iterator)->key_)
                {
                    iterator = &((*iterator)->left_);
                }
                else
                    break;
            }
            return *iterator;
        }

        template <typename T, typename V>
        void BST<T, V>::inorder_traversal()
        {
            assert(fun_);
            std::stack<Node *> st{};
            st.push(root_);

            while (true)
            {
                auto c = st.top();
                while (c != null_)
                {
                    st.push(c->left_);
                    c = st.top();
                }
                st.pop();
                if (st.empty())
                    break;
                c = st.top();

                //        std::cout<<c->key_<<':'<<c->value_<<std::endl;
                fun_(c->key_, c->value_);
                st.pop();
                st.push(c->right_);
            }
        }

        template <typename T, typename V>
        void BST<T, V>::preorder_traversal()
        {
            assert(fun_);
            std::stack<Node *> st{};
            st.push(root_);

            while (true)
            {
                auto c = st.top();
                while (c != null_)
                {
                    //            std::cout<<c->key_<<':'<<c->value_<<std::endl;
                    fun_(c->key_, c->value_);
                    st.push(c->left_);
                    c = st.top();
                }
                st.pop();
                if (st.empty())
                    break;
                c = st.top();

                st.pop();
                st.push(c->right_);
            }
        }

        template <typename T, typename V>
        void BST<T, V>::postorder_traversal()
        {
            assert(fun_);
            std::stack<Node *> st{};
            st.push(root_);

            while (true)
            {
                auto c = st.top();
                while (c != null_)
                {
                    st.push(c->left_);
                    c = st.top();
                }
                st.pop();
                if (st.empty())
                    break;
                c = st.top();

                if (c == null_)
                {
                    st.pop();
                    c = st.top();
                    st.top() = null_;
                    //            std::cout<<c->key_<<':'<<c->value_<<std::endl;
                    fun_(c->key_, c->value_);
                }
                else
                {
                    st.push(null_);
                    st.push(c->right_);
                }
            }
        }

        template <typename T, typename V>
        void BST<T, V>::inorder_recursion(Node *root)
        {
            if (root != null_)
            {
                inorder_recursion(root->left_);
                //        std::cout<<root->key_<<':'<<root->value_<<std::endl;
                fun_(root->key_, root->value_);
                inorder_recursion(root->right_);
            }
        }

        template <typename T, typename V>
        void BST<T, V>::preorder_recursion(Node *root)
        {
            if (root != null_)
            {
                //        std::cout<<root->key_<<':'<<root->value_<<std::endl;
                fun_(root->key_, root->value_);
                preorder_recursion(root->left_);
                preorder_recursion(root->right_);
            }
        }

        template <typename T, typename V>
        void BST<T, V>::postorder_recursion(Node *root)
        {
            if (root != null_)
            {
                postorder_recursion(root->left_);
                postorder_recursion(root->right_);
                //        std::cout<<root->key_<<':'<<root->value_<<std::endl;
                fun_(root->key_, root->value_);
            }
        }

        template <typename T>
        class SingletonHolder
        {
        public:
            ~SingletonHolder()
            {
                assert(!destroyed_);
                p_ = nullptr;
                destroyed_ = true;
            }
            static T &instance();

        private:
            template <typename U>
            friend U &get_singleton();

            SingletonHolder()
            {
                assert(obj_count_ == 0);
                obj_count_++;
            }

        private:
            inline thread_local static T *p_ = nullptr;
            inline thread_local static bool destroyed_ = false;
            inline thread_local static int obj_count_ = 0;
        };

        template <typename T>
        T &SingletonHolder<T>::instance()
        {
            if (!p_)
            {
                if (destroyed_)
                {
                    throw std::runtime_error("This singleton case has been destroyed");
                }
                thread_local static T instance{};
                p_ = &instance;
            }
            return *p_;
        }

        template <typename T>
        T &get_singleton()
        {
            static thread_local SingletonHolder<T> sh{};
            return SingletonHolder<T>::instance();
        }

        class MyAllocator
        {
        public:
            MyAllocator() = default;
            MyAllocator(const MyAllocator &) = delete;
            void *allocate(std::size_t bytes);
            void deallocate(void *p, std::size_t bytes);

            constexpr static std::size_t MaxObjSize = 128;

        private:
            BST<std::size_t, FixedAllocator> tree_{};
        };

    } // namespace Inner end

}

namespace lsf
{
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
} // namespace lsf
