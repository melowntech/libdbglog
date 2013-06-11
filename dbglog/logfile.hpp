#ifndef shared_dbglog_logger_file_hpp_included_
#define shared_dbglog_logger_file_hpp_included_

#include <string>
#include <iostream>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>

namespace dbglog {

class logger_file : boost::noncopyable {
public:
    logger_file()
        : use_file_(false), fd_(::open("/dev/null", O_WRONLY))
    {
        if (-1 == fd_) {
            throw std::runtime_error
                ("Cannot open /dev/null for log file. Fatal");
        }
    }

    ~logger_file() {
        if (-1 == fd_) {
            return;
        }

        ::close(fd_);
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

    bool tie(int fd) {
        // TODO: add untie function + remember fd to be re-tied on file reopen
        if (-1 == dup2(fd_, fd)) {
            std::cerr << "Error dupplicating fd(" << fd_ << ") to fd("
                      << fd << "): " << errno << std::endl;
            return false;
        }
        return true;
    }

protected:
    bool write_file(const std::string &line) {
        return write_file(line.data(), line.size());
    }

    bool write_file(const char *data, size_t left) {
        if (!use_file_) {
            return false;
        }

        while (left) {
            ssize_t written(TEMP_FAILURE_RETRY(::write(fd_, data, left)));
            if (-1 == written) {
                std::cerr << "Error writing to log file: "
                          << errno << std::endl;
                break;
            }
            left -= written;
            data += written;
        }
        return true;
    }

    bool use_file() const { return use_file_; }

private:
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

    bool use_file_; //!< Log to configured file
    std::string filename_; //!< log file filename
    int fd_; //!< fd associated with output file

    boost::mutex m_;
};

} // namespace dbglog

#endif // shared_dbglog_logger_file_hpp_included_
