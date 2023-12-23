module;
#include <utility> //std::move
#include <cassert>
#include <cwchar>

#include <string>
// #include <string_view>
#include <stdexcept>
#include <clocale>

module lsf:json_src.imp;
import :json_src;
import :mbuff;

namespace lsf
{
    StrSource::StrSource(std::string str) : src_(std::move(str))
    {
    }

    void StrSource::before_read()
    {
    }
    long StrSource::read(wchar_t *buff, std::size_t buff_size)
    {
        auto old = std::setlocale(LC_ALL, nullptr);
        if (next_p_ >= src_.size())
        {
            return 0;
        }
        assert(buff != nullptr);
        auto state = std::mbstate_t();
        const char *strp = src_.data() + next_p_;

        // BUG 这里转换时需要设置区域信息,否则中文有问题.但肯定不能在这块设置
        // 一方面这里不合适,另一方面不知道何时恢复
        //  auto fx=std::locale("").name().c_str();
        // auto lx=std::setlocale(LC_ALL, std::locale("").name().c_str());

        // 还有一种解决是提前自己转换好,这样区域就可以直接恢复
        auto size = mbsrtowcs(buff, &strp, buff_size, &state);

        if (size == static_cast<std::size_t>(-1))
        {
            return -1;
        }
        return size;
    }

    FileSource::FileSource(const std::string &file_name) : f_(file_name)
    {
        f_.imbue(std::locale(""));
        if (!f_.is_open())
            throw std::runtime_error("failed to open file:" + file_name);
    }

    void FileSource::before_read()
    {
    }
    long FileSource::read(wchar_t *buff, std::size_t buff_size)
    {
        f_.read(buff, buff_size);
        if (f_.bad())
            throw std::runtime_error("read file failed!");
        return f_.gcount();
    }
} // namespace lsf
