module;
#include <cassert>
#include <stdexcept>

module lsf:tree_allocator.imp;

import :tree_allocator;

namespace lsf::Inner
{
    void *new_bridge(std::size_t count)
    {
        if (count > Inner::MyAllocator::MaxObjSize)
            return ::operator new(count);
        auto p = Inner::get_singleton<Inner::MyAllocator>().allocate(count);
        if (p == nullptr)
            //        throw std::bad_alloc("MyAllocator allocate failed");
            throw std::bad_alloc();
        return p;
    }
    void delete_bridge(void *ptr, std::size_t sz)
    {
        if (sz > Inner::MyAllocator::MaxObjSize)
            ::operator delete(ptr);
        else
        {
            Inner::get_singleton<Inner::MyAllocator>().deallocate(ptr, sz);
        }
    }

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