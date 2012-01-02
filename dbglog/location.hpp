#ifndef shared_dbglog_location_hpp_included_
#define shared_dbglog_location_hpp_included_

#include <cstddef>

namespace dbglog {

struct location {
    const char *file;
    const char *func;
    size_t line;

    location(const char *file, const char *func, size_t line)
        : file(file), func(func), line(line)
    {}
};

} // namespace dbglog

#endif // shared_dbglog_location_hpp_included_

