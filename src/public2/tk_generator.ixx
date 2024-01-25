module;

export module lsf:tk_generator;

import :constant;

namespace lsf
{
    struct GenToken
    {
        virtual ~GenToken() {}
        virtual void next_() = 0;
        virtual Token &current_() = 0;
        virtual void shutdown(){}
    };
} // namespace lsf
