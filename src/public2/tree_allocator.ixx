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

    void *new_bridge(std::size_t count);
    void delete_bridge(void *ptr, std::size_t sz);

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

} // namespace Inner end