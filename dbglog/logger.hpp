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

#include "sink.hpp"

namespace dbglog {

class logger : boost::noncopyable {
public:
    logger(unsigned int mask)
        : mask_(~mask), show_threads_(true), show_pid_(true)
        , time_precision_(0), use_console_(true), use_file_(false)
        , fd_(::open("/dev/null", O_WRONLY))
    {
        if (-1 == fd_) {
            throw std::runtime_error
                ("Cannot open /dev/null for log file. Fatal");
        }
    }

    ~logger() {
        if (-1 == fd_) {
            return;
        }

        TEMP_FAILURE_RETRY(::close(fd_));
    }

    // NB: not threads safe; this must be done before any new thread is created!
    void addSink(const Sink::pointer &sink) {
        sinks_.push_back(sink);
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

    bool log_file(const std::string &filename) {
        boost::mutex::scoped_lock guard(m_);
        if (filename.empty()) {
            if (!open_file("/dev/null")) {
                return false;
            }
            use_file_ = false;
        } else {
            if (!open_file(filename)) {
                return false;
            }
            use_file_ = true;
        }

        filename_ = filename;
        return true;
    }

    void set_mask(const mask &m) {
        mask_ = ~m.get();
    }

    void set_mask(unsigned int m) {
        mask_ = ~m;
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

    bool tie(int fd) {
        // TODO: add untie function + remember fd to be re-tied on file reopen
        if (-1 == dup2(fd_, fd)) {
            std::cerr << "Error dupplicating fd(" << fd_ << ") to fd("
                      << fd << "): " << errno << std::endl;
            return false;
        }
        return true;
    }

private:
    inline bool check_level_(level l) const {
        return !(mask_ & l) || (l == fatal);
    }

    bool open_file(const std::string &filename) {
        class file_closer {
        public:
            file_closer(int fd) : fd_(fd) {}
            ~file_closer() {
                if (-1 == fd_) {
                    return;
                }

                TEMP_FAILURE_RETRY(::close(fd_));
            }

            operator int() {
                return fd_;
            }

        private:
            int fd_;
        } f(::open(filename.c_str(), O_WRONLY | O_CREAT | O_APPEND
                   , S_IRUSR | S_IWUSR));

        if (-1 == f) {
            std::cerr << "Error opening log file <" << filename
                      << ">: " << errno << std::endl;
            return false;
        }

        if (-1 == dup2(f, fd_)) {
            std::cerr << "Error dupplicating fd(" << f << ") to fd("
                      << fd_ << "): " << errno << std::endl;
            return false;
        }

        return true;
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

        if (use_file_) {
            const char *data(line.data());
            size_t left(line.size());
            while (left) {
                ssize_t written(TEMP_FAILURE_RETRY(::write(fd_, data, left)));
                if (-1 == written) {
                    std::cerr << "Error writing to log file: "
                              << errno << std::endl;
                    return;
                }
                left -= written;
                data += written;
            }
        }
    }

    unsigned int mask_; //!< Log mask
    bool show_threads_; //!< Output thread ID (after PID)
    bool show_pid_; //!< Output PID of current process
    unsigned short time_precision_;

    bool use_console_; //!< Log to console (stderr)

    bool use_file_; //!< Log to configured file
    std::string filename_; //!< log file filename
    int fd_; //!< fd associated with output file

    boost::mutex m_;

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

