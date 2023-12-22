module;
#include <utility> //std::move
#include <cassert>
#include <cwchar>

#include <string>
// #include <string_view>
#include <stdexcept>

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
        if (next_p_ >= src_.size())
        {
            return 0;
        }
        assert(buff != nullptr);
        auto state = std::mbstate_t();
        const char *strp = src_.data() + next_p_;
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
