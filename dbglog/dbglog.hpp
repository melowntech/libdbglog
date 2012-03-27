#ifndef shared_dbglog_dbglog_hpp_included_
#define shared_dbglog_dbglog_hpp_included_

#include <dbglog/logger.hpp>
#include <dbglog/stream.hpp>
#include <dbglog/config.hpp>
#include <dbglog/mask.hpp>

namespace dbglog {
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

    void thread_id(const std::string &id);

    void thread_id();

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
    DBGLOG_CONCATENATE(DBGLOG_EXPAND_, DBGLOG_NARG(__VA_ARGS__)(__VA_ARGS__))

#endif // shared_dbglog_dbglog_hpp_included_

