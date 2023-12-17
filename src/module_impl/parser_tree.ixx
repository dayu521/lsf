module;
#include <string>

module lsf:parser_tree;

import :inner_imp;

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
    
    //TODO 需要使用concept进行约束
    template <typename T>
    struct TreeNode
    {
        TreeNode<T> *left_child_{nullptr}; /// 左孩子
        TreeNode<T> *right_bro_{nullptr};  /// 右兄弟

        std::string key_;              /// 作为对象成员的key
        NodeC ele_type_{NodeC::Error}; /// json类型标识

        void operator delete(void *ptr, std::size_t sz);
        void *operator new(std::size_t count);

        T *get_this()
        {
            return static_cast<T *>(this);
        }
    };

    template <typename T>
    void TreeNode<T>::operator delete(void *ptr, std::size_t sz)
    {
        if (sz > Inner::MyAllocator::MaxObjSize)
            ::operator delete(ptr);
        else
        {
            Inner::get_singleton<Inner::MyAllocator>().deallocate(ptr, sz);
        }
    }

    template <typename T>
    void *TreeNode<T>::operator new(std::size_t count)
    {
        if (count > Inner::MyAllocator::MaxObjSize)
            return ::operator new(count);
        auto p = Inner::get_singleton<Inner::MyAllocator>().allocate(count);
        if (p == nullptr)
            //        throw std::bad_alloc("MyAllocator allocate failed");
            throw std::bad_alloc();
        return p;
    }

    class ParserResultBuilder
    {
    public:
        virtual ~ParserResultBuilder() =default;

    public:
        virtual void before_build() = 0;
        virtual void after_build() = 0;
        virtual void build_obj() = 0;
        virtual void build_arr() = 0;
        virtual void build_string(std::wstring str) = 0;
        virtual void build_number(std::wstring str) = 0;
        virtual void build_keyword(std::wstring str) = 0;
        virtual void build_Null(std::wstring str) = 0;
        virtual void set_memberkey(std::wstring key) = 0;
        virtual void build_null_mbr() = 0;
        virtual void can_start_iteration() = 0;
        virtual void move_next() = 0;
        virtual void finish_iteration() = 0;
    };

} // namespace lsf
