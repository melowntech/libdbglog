#ifndef shared_dbglog_logger_file_hpp_included_
#define shared_dbglog_logger_file_hpp_included_

#include <set>
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
                ("Cannot open /dev/null for log file. Oops.");
        }
    }

    ~logger_file() {
        if (-1 == fd_) {
            return;
        }

        ::close(fd_);

        for (auto fd : ties_) {
            ::close(fd);
        }
    }

    bool log_file(const std::string &filename) {
        boost::mutex::scoped_lock guard(m_);
        if (filename.empty()) {
            if (!open_file("/dev/null", fd_)) {
                return false;
            }
            use_file_ = false;
            retie(); // back to /dev/null to allow log file to be closed
        } else {
            if (!open_file(filename, fd_)) {
                return false;
            }
            use_file_ = true;
            retie(); // point ties to freshly open log file
        }

        filename_ = filename;
        return true;
    }

    bool tie(int fd) {
        boost::mutex::scoped_lock guard(m_);

        // check for duplicate
        if (ties_.find(fd) != ties_.end()) {
            // already tied -> fine
            return true;
        }

        // TODO: add untie function + remember fd to be re-tied on file reopen
        if (-1 == safeDup2(fd_, fd)) {
            std::cerr << "Error dupplicating fd(" << fd_ << ") to fd("
                      << fd << "): " << errno << std::endl;
            return false;
        }

        // remember fd
        ties_.insert(fd);
        return true;
    }

    bool untie(int fd, const std::string &path = "/dev/null") {
        boost::mutex::scoped_lock guard(m_);

        // check for existence
        if (ties_.find(fd) == ties_.end()) {
            // not tied -> error
            return false;
        }

        // point fd to something else
        if (!open_file(path, fd)) {
            return false;
        }

        // forget fd
        ties_.erase(fd);
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
    bool open_file(const std::string &filename, int dest) {
        class file_closer {
        public:
            file_closer(int fd) : fd_(fd) {}
            ~file_closer() {
                if (-1 == fd_) {
                    return;
                }

                ::close(fd_);
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

        if (-1 == safeDup2(f, dest)) {
            std::cerr << "Error dupplicating fd(" << f << ") to fd("
                      << dest << "): " << errno << std::endl;
            return false;
        }

        return true;
    }

    void retie() {
        // retie all tied file descriptors
        for (auto fd : ties_) {
            if (-1 == safeDup2(fd_, fd)) {
                std::cerr << "Error dupplicating fd(" << fd_ << ") to fd("
                          << fd << "): " << errno << std::endl;
            }
        }
    }

    int safeDup2(int oldfd, int newfd) {
        for (;;) {
            int res(::dup2(oldfd, newfd));
            if (res != -1) {
                // OK
                return res;
            }

            // some error
            switch (res) {
            case EBUSY:
                // race condition between open and dup -> try again
            case EINTR:
                // interrupted -> try again
                continue;
            }

            // fatal error
            return res;
        }
    }

    bool use_file_; //!< Log to configured file
    std::string filename_; //!< log file filename
    int fd_; //!< fd associated with output file

    boost::mutex m_;
    std::set<int> ties_;
};

} // namespace dbglog

#endif // shared_dbglog_logger_file_hpp_included_
