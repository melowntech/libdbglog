#ifndef shared_dbglog_detail_log_helpers_hpp_included_
#define shared_dbglog_detail_log_helpers_hpp_included_

#include <dbglog/level.hpp>
#include <dbglog/location.hpp>

#include <time.h>
#include <sys/time.h>

#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

#include <atomic>

namespace dbglog { namespace detail {

inline const char* level2string(dbglog::level l)
{
    switch (l) {
    case debug: return "DD";

    case info1: return "I1";
    case info2: return "I2";
    case info3: return "I3";
    case info4: return "I4";

    case warn1: return "W1";
    case warn2: return "W2";
    case warn3: return "W3";
    case warn4: return "W4";

    case err1: return "E1";
    case err2: return "E2";
    case err3: return "E3";
    case err4: return "E4";

    case fatal: return "FF";

    default: return "??";
    }
}

typedef char timebuffer[50];

inline char* format_time(timebuffer &b)
{
    timeval now;
    gettimeofday(&now, 0x0);
    tm now_bd;
    localtime_r(&now.tv_sec, &now_bd);
    strftime(b, sizeof(b) - 1, "%Y-%m-%d %T", &now_bd);
    return b;
}

class thread_id {
public:
    static const std::string& get() {
        if (!holder_.get()) {
            holder_.reset(new std::string(boost::lexical_cast<std::string>
                                          (generator_++)));
        }
        return *holder_;
    }

    static void set(const std::string &value) {
        if (!holder_.get()) {
            holder_.reset(new std::string(value));
        } else {
            *holder_ = value;
        }
    }

private:
    static boost::thread_specific_ptr<std::string> holder_;
    static std::atomic_uint_fast64_t generator_;
};

} // namespace detail

inline std::ostream& operator<<(std::ostream &os, const dbglog::location &l) {
    return os << '{' << l.file << ':' << l.func << "():" << l.line << '}';
}

} // namespace dbglog

#endif // shared_dbglog_detail_log_helpers_hpp_included_
