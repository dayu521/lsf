module;
// #include <cstddef>
#include <string>
// #include <string_view>
#include <fstream>

export module lsf:json_src;

export namespace lsf
{
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
        FileSource(const std::string & file_name);
        ~FileSource() = default;

    private:
        std::wifstream f_{}; //析构函数自动关闭
    };
} // namespace lsf
