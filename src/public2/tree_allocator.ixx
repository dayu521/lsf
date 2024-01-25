module;
// #include <cstddef> //std::size_t
#include <utility> //std::move
#include <limits>
#include <vector>
#include <functional>
#include <queue>
#include <stack>
#include <cassert>
#include <stdexcept>

export module lsf:tree_allocator;

namespace lsf::Inner
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

namespace lsf::Inner
{

    Chunk::~Chunk()
    {
    }

    void Chunk::init(std::size_t block_size, unsigned char n)
    {
        assert(n > 0 && block_size > 0);
        // 溢出检查
        assert(block_size * n / n == block_size);

        mem_ = new unsigned char[block_size * n];
        list_head_ = 0;
        available_n_ = n;

        //[0,n-1]共n个块,循环n次.n最大为255
        for (decltype(n) i = 0; i < n; i++)
        {
            *(mem_ + i * block_size) = i + 1;
        }
    }

    void Chunk::release()
    {
        delete[] mem_;
    }

    void *Chunk::allocate(std::size_t block_size)
    {
        assert(available_n_ > 0);
        assert(block_size > 0);

        auto r = mem_ + list_head_ * block_size;
        list_head_ = *r;
        available_n_--;
        return r;
    }

    void Chunk::deallocate(void *p, std::size_t block_size)
    {
        auto ucp = static_cast<unsigned char *>(p);
        assert((ucp - mem_) % block_size == 0);

        *ucp = list_head_;
        list_head_ = static_cast<decltype(list_head_)>((ucp - mem_) / block_size);

        available_n_++;
    }

    std::size_t Chunk::available_size() const
    {
        return available_n_;
    }

    bool Chunk::contain(void *p, std::size_t chunck_length) const
    {
        auto where = static_cast<unsigned char *>(p);
        return p >= mem_ && p < mem_ + chunck_length;
    }

    /***************FixedAllocator*******************/
    FixedAllocator::FixedAllocator(std::size_t block_size) : block_size_(block_size)
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
        if (pool_.size() > 0)
        {
            if (allocate_ < pool_.size() && pool_[allocate_].available_size() > 0)
            {
                return pool_[allocate_].allocate(block_size_);
            }
            else if (deallocate_ != allocate_ && pool_[deallocate_].available_size() > 0)
            {
                allocate_ = deallocate_;
                return pool_[deallocate_].allocate(block_size_);
            }
            else
                ; // we need to find a free chunk
        }

        bool found = false;
        for (decltype(pool_)::size_type i = 0; i < pool_.size(); i++)
        {
            if (pool_[allocate_].available_size() > 0)
            {
                allocate_ = i;
                found = true;
                break;
            }
        }
        if (found)
        {
            return pool_[allocate_].allocate(block_size_);
        }
        else
        {                     // we need allocate one another chunk
            if (add_chunck()) // allocate_被add_chunck()修改了,所以作为index是安全的
                return pool_[allocate_].allocate(block_size_);
        }
        return nullptr;
    }

    /// 如果p不指向当前分配器中的内存,则不做任何事
    void FixedAllocator::deallocate(void *p)
    {
        assert(pool_.size() > 0);

        std::size_t byte_sizes = block_size_ * blocks_num_;

        if (pool_[deallocate_].contain(p, byte_sizes))
        {
            pool_[deallocate_].deallocate(p, block_size_);
        }
        else if (pool_[allocate_].contain(p, byte_sizes))
        {
            pool_[allocate_].deallocate(p, block_size_);
            deallocate_ = allocate_;
        }
        else
        {
            decltype(deallocate_) left = deallocate_, right = deallocate_;
            decltype(pool_.size()) num = 0;

            while (num < pool_.size())
            {
                if (left > 0)
                {
                    if (pool_[left].contain(p, byte_sizes))
                    {
                        pool_[left].deallocate(p, block_size_);
                        deallocate_ = left;
                        return;
                    }
                    else
                        left--;
                    num++;
                }
                if (right < pool_.size())
                {
                    if (pool_[right].contain(p, byte_sizes))
                    {
                        pool_[right].deallocate(p, block_size_);
                        deallocate_ = right;
                        return;
                    }
                    else
                        right++;
                    num++;
                }
            }
        }
        if (pool_[deallocate_].available_size() == blocks_num_)
        {
            std::swap(pool_[deallocate_], pool_.back());
            pool_.back().release();
            pool_.pop_back();
            deallocate_ = 0;
            allocate_--;
        }
    }

    void FixedAllocator::release()
    {
        for (auto i = pool_.begin(); i != pool_.end(); i++)
        {
            i->release();
        }
    }

    bool FixedAllocator::add_chunck()
    {
        Chunk chunk{};
        chunk.init(block_size_, blocks_num_);
        try
        {
            pool_.push_back(std::move(chunk));
        }
        catch (...)
        {
            chunk.release();
            return false;
        }
        allocate_ = pool_.size() - 1;
        return true;
    }

    /*****************MyAllocator******************/
    void *MyAllocator::allocate(std::size_t bytes)
    {
        assert(bytes <= MaxObjSize);
        auto node = tree_.find(bytes);
        if (tree_.get_null() == node)
        {
            node = tree_.insert(bytes, FixedAllocator(bytes));
            if (node == tree_.get_null())
                return nullptr;
        }
        return node->value_.allocate();
    }

    void MyAllocator::deallocate(void *p, std::size_t bytes)
    {
        auto node = tree_.find(bytes);
        assert(node != tree_.get_null());
        node->value_.deallocate(p);
    }

} // namespace Inner end