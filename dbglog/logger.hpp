#ifndef shared_dbglog_logger_hpp_included_
#define shared_dbglog_logger_hpp_included_

#include <string>
#include <iostream>
#include <atomic>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

#include "level.hpp"
#include "location.hpp"
#include "mask.hpp"
#include "detail/log_helpers.hpp"

#include "logfile.hpp"
#include "sink.hpp"

namespace dbglog {

class logger : public logger_file {
public:
    logger(unsigned int mask)
        : logger_file(), mask_(~mask), show_threads_(true), show_pid_(true)
        , time_precision_(0), use_console_(true)
    {
    }

    ~logger() {}

    // NB: not threads safe; this must be done before any new thread is created!
    void addSink(const Sink::pointer &sink) {
        sinks_.push_back(sink);
        if (sink->shared_mask()) { sink->set_mask(get_mask()); }
    }

    bool log(level l, const std::string &message
             , const location &loc)
    {
        return prefix_log(l, empty_, message, loc);
    }

    bool prefix_log(level l, const std::string &prefix
                    , const std::string &message
                    , const location &loc)
    {
        if (!check_level(l)) {
            return false;
        }

        std::ostringstream os;
        line_prefix(os, l);
        if (!prefix.empty()){
            os << prefix << ' ';
        }
        os << message << ' ' << loc << '\n';

        const auto line(os.str());

        if (check_level_(l)) {
            write(line);
        }

        for (auto &sink : sinks_) {
            if (sink->check_level(l)) { sink->write(line); }
        }

        return true;
    }

    inline bool check_level(level l) const {
        if (!(mask_ & l) || (l == fatal)) {
            return true;
        }

        for (const auto &sink : sinks_) {
            if (sink->check_level(l)) {
                return true;
            }
        }
        return false;
    }

    inline bool check_level(level l, std::atomic<bool> &guard) const {
        bool exp_val = false;
        bool new_val = true;
        if (!std::atomic_compare_exchange_strong(&guard, &exp_val, new_val)) {
            return false;
        }
        return check_level(l);
    }

    void log_thread(bool value = true) {
        show_threads_ = value;
    }

    void log_pid(bool value = true) {
        show_pid_ = value;
    }

    void log_console(bool value = true) {
        use_console_ = value;
    }

    void set_mask(const mask &m) {
        mask_ = ~m.get();
        for (const auto &sink : sinks_) {
            if (sink->shared_mask()) { sink->set_mask(m); }
        }
    }

    void set_mask(unsigned int m) {
        mask_ = ~m;

        for (const auto &sink : sinks_) {
            if (sink->shared_mask()) { sink->set_mask(m); }
        }
    }

    unsigned int get_mask() const {
        return ~mask_;
    }

    std::string get_mask_string() const {
        return mask(~mask_).as_string();
    }

    unsigned short log_time_precision() const {
        return time_precision_;
    }

    void log_time_precision(unsigned short time_precision) {
        time_precision_ = time_precision;
    }

private:
    inline bool check_level_(level l) const {
        return !(mask_ & l) || (l == fatal);
    }

    void line_prefix(std::ostream &os, level l) {
        detail::timebuffer now;
        os << detail::format_time(now, time_precision_) << ' '
           << detail::level2string(l);

        if (show_pid_) {
            os << " [" << getpid();
            if (show_threads_) {
                os << '(' << detail::thread_id::get() << ')';
            }
            os << ']';
        } else if (show_threads_) {
            os << " [(" << detail::thread_id::get() << ")]";
        }

        os << ": ";
    }

    void write(const std::string &line) {
        if (use_console_) {
            std::cerr.write(line.data(), line.size());
        }

        logger_file::write_file(line);
    }

    unsigned int mask_; //!< Log mask
    bool show_threads_; //!< Output thread ID (after PID)
    bool show_pid_; //!< Output PID of current process
    unsigned short time_precision_;

    bool use_console_; //!< Log to console (stderr)

    Sink::list sinks_;

    static const std::string empty_;
};

class module {
public:
    module(logger &sink)
        : name_(), log_name_(), sink_(&sink)
    {
    }

    module(const std::string &name, logger &sink)
        : name_(name), log_name_("[" + name + "]"), sink_(&sink)
    {
    }

    module(const std::string &name, const module &other)
        : name_(other.name_ + "/" + name)
        , log_name_("[" + other.name_ + "/" + name + "]")
        , sink_(other.sink_)
    {
    }

    bool check_level(level l) const {
        return sink_->check_level(l);
    }

    bool check_level(level l, std::atomic<bool> &guard) const {
        return sink_->check_level(l, guard);
    }

    bool log(level l, const std::string &message
             , const location &loc)
    {
        return sink_->prefix_log(l, log_name_, message, loc);
    }

private:
    std::string name_;
    std::string log_name_;
    logger *sink_;
};

} // namespace dbglog

#endif // shared_dbglog_logger_hpp_included_

