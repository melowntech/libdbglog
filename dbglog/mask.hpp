#ifndef shared_dbglog_mask_hpp_included_
#define shared_dbglog_mask_hpp_included_

#include <string>
#include <iosfwd>

#include "level.hpp"

namespace dbglog {

class mask {
public:
    mask(unsigned int m = default_) : mask_(m) {}

    mask(const std::string &m) { from_string(m); }

    std::string as_string() const;

    void from_string(const std::string &str);

    unsigned int get() const { return mask_; }

    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>&
    operator<<(std::basic_ostream<CharT, Traits> &os, const mask &m)
    {
        return os << m.as_string();
    }

    template<typename CharT, typename Traits>
    friend std::basic_istream<CharT, Traits>&
    operator>>(std::basic_istream<CharT, Traits> &is, mask &m)
    {
        std::string s;
        is >> s;
        m.from_string(s);
        return is;
    }

private:
    unsigned int mask_;
};

} // namespace dbglog

#endif // shared_dbglog_mask_hpp_included_
