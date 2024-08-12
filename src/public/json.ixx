module;
#include <memory>
#include <string>
#include <optional>

export module lsf;

export import :util;
export import :json_src;

import :analyze;
import :mbuff;
import :tp;

// TODO 修改接口,此接口很迷惑

// 流,解析器,解析树,json字符串构建器,struct_to_jsonstr,解析树到struct,解析树到jsonstr

namespace lsf
{
    struct impl;

    export class Json
    {
    public:
        template <InputSource T>
        [[nodiscard]] std::optional<std::shared_ptr<TreeBuilder>> run(std::unique_ptr<T> input);

        template <InputSource T>
        void set_input(std::unique_ptr<T> input);

    public:
        Json();
        Json(const Json &) = delete;
        Json(Json &&) = default;
        // https://en.cppreference.com/w/cpp/memory/unique_ptr
        ~Json();

        std::optional<std::shared_ptr<TreeBuilder>> run();

        [[nodiscard]] bool weak_type_check(std::shared_ptr<TreeBuilder> builder);

        std::string get_errors() const;

    private:
        std::unique_ptr<impl> impl_;
    };

    void set_buff_base(std::unique_ptr<impl> &j, std::unique_ptr<BuffBase> b);

    template <InputSource T>
    void Json::set_input(std::unique_ptr<T> input)
    {
        set_buff_base(this->impl_, std::make_unique<MBuff<T>>(std::move(input)));
    }

    template <InputSource T>
    std::optional<std::shared_ptr<TreeBuilder>> Json::run(std::unique_ptr<T> input)
    {
        set_input(std::move(input));
        return run();
    }

    export void json_to_string(std::shared_ptr<TreeBuilder> builder, SerializeBuilder &sb);

    export template <typename S>
    void struct_to_jsonstr(const S &obj, SerializeBuilder &builder)
    {
        serialize(obj, builder);
    }

    export template <typename S>
    void json_to_struct(std::shared_ptr<TreeBuilder> builder, S &s)
    {
        // LocaleGuard lg;
        // deserialize(s, std::get<0>(builder->get_ast()));

        LocaleGuard lg;
        lsf::ReadJsonStr rje(std::get<0>(builder->get_ast()));
        lsf::parse_cpp_type(s, rje);
    }

    export template <typename S>
    void json_to_struct_ignore_absence(std::shared_ptr<TreeBuilder> builder, S &s)
    {
        LocaleGuard lg;
        lsf::ReadJsonStrExt rje(builder->get_ast());
        lsf::parse_cpp_type(s, rje);
    }

} // namespace lsf