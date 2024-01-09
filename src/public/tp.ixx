module;
#include <cstddef>
#include <concepts>

export module lsf:tp;

namespace lsf
{
    namespace is_concept
    {

        template <typename T>
        concept around_read = requires(T input) {
            {
                input.before_read()
            } -> std::same_as<void>;
            {
                input.after_read()
            } -> std::same_as<void>;
        };

        template <typename T>
        concept read = requires(T input, wchar_t *buff, std::size_t buff_size) {
            {
                input.read(buff, buff_size)
            } -> std::same_as<long>;
        };

        template <typename T>
        concept eof = requires(T input) {
            {
                input.eof()
            } -> std::same_as<bool>;
        };
    } // namespace is_concept

    template <typename T>
    concept InputSource = is_concept::around_read<T> &&
                          is_concept::read<T>;

} // namespace lsf
