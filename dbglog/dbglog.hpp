#ifndef shared_dbglog_dbglog_hpp_included_
#define shared_dbglog_dbglog_hpp_included_

#include <dbglog/logger.hpp>
#include <dbglog/stream.hpp>
#include <dbglog/config.hpp>
#include <dbglog/mask.hpp>

namespace dbglog {
    const unsigned short millis(3);
    const unsigned short micros(6);

    inline module make_module() {
        return module(detail::deflog);
    }

    inline module make_module(const std::string &name)
    {
        return module(name, detail::deflog);
    }

    inline void set_mask(unsigned int mask)
    {
        return detail::deflog.set_mask(mask);
    }

    inline void set_mask(const mask &m)
    {
        return detail::deflog.set_mask(m);
    }

    inline void set_mask(const std::string &m)
    {
        return detail::deflog.set_mask(mask(m));
    }

    inline unsigned int get_mask()
    {
        return detail::deflog.get_mask();
    }

    inline std::string get_mask_string()
    {
        return detail::deflog.get_mask_string();
    }

    inline void log_thread(bool value = true)
    {
        return detail::deflog.log_thread(value);
    }

    inline void log_pid(bool value = true)
    {
        return detail::deflog.log_pid(value);
    }

    inline void log_console(bool value = true)
    {
        return detail::deflog.log_console(value);
    }

    inline bool log_file(const std::string &filename)
    {
        return detail::deflog.log_file(filename);
    }

    inline bool log_file_truncate()
    {
        return detail::deflog.log_file_truncate();
    }

    inline bool log_file_owner(long uid, long gid)
    {
        return detail::deflog.log_file_owner(uid, gid);
    }

    void thread_id(const std::string &id);

    std::string thread_id();

    inline void log_time_precision(unsigned short precision) {
        detail::deflog.log_time_precision(precision);
    }

    inline unsigned short log_time_precision() {
        return detail::deflog.log_time_precision();
    }

    inline void add_sink(const Sink::pointer &sink) {
        detail::deflog.addSink(sink);
    }

    inline bool tie(int fd) {
        return detail::deflog.tie(fd);
    }

    inline bool untie(int fd) {
        return detail::deflog.untie(fd);
    }
} // namespace dbglog

/** Main log facility.
 *  Usage:
 *      Log with dbglog::info1 level to default logger
 *          LOG(info1) << "text";
 *
 *      Log with dbglog::debug level to logger L
 *      LOG(info1, L) << "text";
 *
 *  L must be instance of a class with this interface:
 *
 *  class LoggerConcept {
 *      void log(dbglog::level l, const std::string &message
 *               , const dbglog::location &loc);
 *
 *      bool check_level(dbglog::level l);
 *  };
 */
#define LOG(...) \
    DBGLOG_CONCATENATE(DBGLOG_EXPAND_, DBGLOG_NARG(__VA_ARGS__) \
                       (__VA_ARGS__))

/** One shot log facility.
 *  Same as LOG but logs almost once during program lifetime.
 */
#define LOGONCE(...) \
    DBGLOG_CONCATENATE(DBGLOG_ONCE_EXPAND_, DBGLOG_NARG(__VA_ARGS__) \
                       (__VA_ARGS__))

/** Log'n'throw convenience logger.
 *
 *  Same as LOG but throws exception of given type (2nd or 3rd) initialized with
 *  log line (only logged content, no time, location etc. is appended)
 */
#define LOGTHROW(...) \
    DBGLOG_CONCATENATE(DBGLOG_THROW_EXPAND_, DBGLOG_NARG(__VA_ARGS__) \
                       (__VA_ARGS__))

#endif // shared_dbglog_dbglog_hpp_included_
