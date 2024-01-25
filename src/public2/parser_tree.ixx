module;
#include <string>
// #include <concepts>

export module lsf:parser_tree;

namespace lsf
{
    enum class NodeC : char
    {
        Obj,
        Arr,
        String,
        Number,
        Keyword,
        Null,
        Error
    };

    // TODO 需要使用concept进行约束
    template <typename T>
    // requires std::derived_from<T,TreeNode<T>>
    struct TreeNode
    {
        TreeNode<T> *left_child_{nullptr}; /// 左孩子
        TreeNode<T> *right_bro_{nullptr};  /// 右兄弟

        NodeC ele_type_{NodeC::Error}; /// json类型标识

        void operator delete(void *ptr, std::size_t sz);
        void *operator new(std::size_t count);

        T *get_this()
        {
            return static_cast<T *>(this);
        }
    };

    void *new_bridge(std::size_t count);
    void delete_bridge(void *ptr, std::size_t sz);

    template <typename T>
    void TreeNode<T>::operator delete(void *ptr, std::size_t sz)
    {
        delete_bridge(ptr,sz);
    }

    template <typename T>
    void *TreeNode<T>::operator new(std::size_t count)
    {
        return new_bridge(count);
    }

} // namespace lsf
