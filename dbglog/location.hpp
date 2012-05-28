#ifndef shared_dbglog_location_hpp_included_
#define shared_dbglog_location_hpp_included_

#include <cstddef>
#include <cstring>

namespace dbglog {

struct location {
    const char *file;
    const char *func;
    size_t line;

    location(const char *file, const char *func, size_t line
             , bool trimFile = false)
        : file(trimFile ? trim(file) : file), func(func), line(line)
    {}

private:
    static const char *trim(const char *file) {
        const char *end = strrchr(file, '/');
        return end ? end + 1 : file;
    }
};

} // namespace dbglog

#endif // shared_dbglog_location_hpp_included_

