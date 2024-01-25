module;
#include <string>

export module lsf:tree_builder;

namespace lsf
{
    //TODO 提供是否是对象成员的上下文信息,这样,可以创建合适的节点,不必在所有节点都携带key属性
    class ParserResultBuilder
    {
    public:
        virtual ~ParserResultBuilder() = default;

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
