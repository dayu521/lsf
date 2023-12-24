module;
// #include <cstddef>
#include <string>
// #include <string_view>
#include <fstream>

export module lsf:json_src;

export namespace lsf
{
    // 在涉及到宽字符转换之前,创建此变量
    class LocaleGuard
    {
    public:
        LocaleGuard();
        ~LocaleGuard();

    private:
        const char *old_locale_;
    };

    class StrSource
    {
    public:
        void before_read();
        long read(wchar_t *buff, std::size_t buff_size);

    public:
        StrSource(std::string str);
        ~StrSource() = default;

    private:
        long next_p_{};
        std::string src_{};
    };

    class FileSource
    {
    public:
        void before_read();
        long read(wchar_t *buff, std::size_t buff_size);

    public:
        FileSource(const std::string &file_name);
        ~FileSource() = default;

    private:
        std::wifstream f_{}; // 析构函数自动关闭
    };
} // namespace lsf

namespace lsf
{
    std::string to_cstring(const std::wstring &s, std::string::value_type buff[], std::size_t len);

    template <std::size_t N = 64>
    std::string to_cstring(const std::wstring &s)
    {
        std::string::value_type cc[N];
        return to_cstring(s, cc,std::extent_v<decltype(cc)>);
    }
} // namespace lsf