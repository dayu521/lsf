module;
#include <utility> //std::move
#include <cassert>
#include <cwchar>

#include <string>
// #include <string_view>
#include <stdexcept>
#include <clocale>
#include <iostream>

module lsf:json_src.imp;
import :json_src;
import :mbuff;

namespace lsf
{
    LocaleGuard::LocaleGuard()
    {
        // setlocale(LC_ALL,"zh_CN.UTF-8");
        // std::locale("").name().c_str()

        // 设置全局c++环境,所有之后std::locale()的实例都是此locale的副本，
        // 同时设置本地c环境为用户偏好的locale，默认c环境的name好像是"C"
        // std::locale::global(std::locale(""));
#ifdef MSVC_SPECIAL
        old_locale_ = "C";
#else
        old_locale_ = std::setlocale(LC_CTYPE, nullptr);
#endif // MSVC_SPECIAL
        if (std::setlocale(LC_ALL, std::locale("").name().c_str()) == nullptr)
        {
            throw new std::runtime_error("setlocale() failed!");
        }
    }
    LocaleGuard::~LocaleGuard()
    {
        std::setlocale(LC_CTYPE, old_locale_);
    }

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
        const char *strp = src_.data() + next_p_;

        // BUG 这里转换时需要设置区域信息,否则中文有问题.但肯定不能在这块设置
        // 一方面这里不合适,另一方面不知道何时恢复to_cstring
        // 如果之前没有设置,这块是最后的机会了

        // 还有一种解决是提前自己转换好,这样区域就可以直接恢复
        LocaleGuard lg;
        auto state = std::mbstate_t();
        auto size = mbsrtowcs(buff, &strp, buff_size, &state);
        if (size == static_cast<std::size_t>(-1))
        {
            return -1;
        }
        //https://zh.cppreference.com/w/cpp/string/multibyte/mbsrtowcs
        next_p_ += strp-(src_.data()+next_p_);
        return size;
    }

    void StrSource::after_read()
    {
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
    void FileSource::after_read()
    {
    }
} // namespace lsf

namespace lsf
{
    std::string to_cstring(const std::wstring &s, std::string::value_type buff[], std::size_t len)
    {
        auto &cc = buff;
        std::string r{};
        auto cstr = s.c_str();
        std::mbstate_t state{};
        auto n = wcsrtombs(cc, &cstr, len, &state);

        while (n != static_cast<std::size_t>(-1))
        {
            r.append(cc, n);
            if (cstr == nullptr)
                return r;
            n = wcsrtombs(cc, &cstr, len, &state);
        }
        // 转换错误
        // https://en.cppreference.com/w/cpp/string/multibyte/wcsrtombs
        assert(false);
        return r;
    }
} // namespace lsf
