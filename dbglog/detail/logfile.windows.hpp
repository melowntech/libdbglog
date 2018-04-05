/**
 * Copyright (c) 2017 Melown Technologies SE
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * *  Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef dbglog_logfile_posix_hpp_included_
#define dbglog_logfile_posix_hpp_included_

#include <io.h>

#include <set>
#include <cstdio>
#include <string>

#include <boost/noncopyable.hpp>
#include <boost/thread.hpp>

#include <cerrno>

// implement TEMP_FAILURE_RETRY if not present on platform (via C++11 lambda)
#ifndef TEMP_FAILURE_RETRY
#define TEMP_FAILURE_RETRY(operation) [&]()->int {       \
        for (;;) { int e(operation);                     \
            if ((-1 == e) && (EINTR == errno)) continue; \
            return e;                                    \
        }                                                \
    }()
#endif

namespace dbglog {

namespace detail {
    const int DefaultMode(_S_IWRITE | _S_IREAD);
}

class logger_file : boost::noncopyable {
public:
    logger_file()
        : use_file_(false), fd_(-1)
    {
        if (::_sopen_s(&fd_, "NUL", _O_WRONLY, _SH_DENYNO
                       , detail::DefaultMode))
        {
            throw std::runtime_error
                ("Cannot open NUL for log file. Oops.");
        }
    }

    ~logger_file() {
        if (-1 == fd_) {
            return;
        }

        ::_close(fd_);

        for (auto fd : ties_) {
            ::_close(fd);
        }
    }

    bool log_file(const std::string &filename
                  , int mode = detail::DefaultMode)
    {
        boost::mutex::scoped_lock guard(m_);
        if (filename.empty()) {
            if (!open_file("NUL", fd_, mode)) {
                return false;
            }
            use_file_ = false;
            retie(); // back to NUL to allow log file to be closed
        } else {
            if (!open_file(filename, fd_, mode)) {
                return false;
            }
            use_file_ = true;
            retie(); // point ties to freshly open log file
        }

        filename_ = filename;
        return true;
    }

    bool log_file_truncate() {
        boost::mutex::scoped_lock guard(m_);
        if (filename_.empty()) { return false; }

        std::cerr << "Cannot truncate file on Windows." << std::endl;
        return false;
    }

    bool tie(int fd, bool remember=true) {
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
        if (remember) {
            ties_.insert(fd);
        }
        return true;
    }

    bool untie(int fd, const std::string &path = "NUL"
               , int mode = detail::DefaultMode)
    {
        boost::mutex::scoped_lock guard(m_);

        // check for existence
        if (ties_.find(fd) == ties_.end()) {
            // not tied -> error
            return false;
        }

        // point fd to something else
        if (!open_file(path, fd, mode)) {
            return false;
        }

        // forget fd
        ties_.erase(fd);
        return true;
    }

    bool log_file_owner(int owner, int group) {
        return false;
    }

    bool closeOnExec(bool value) { return false; }

protected:
    bool write_file(const std::string &line) {
        return write_file(line.data(), line.size());
    }

    bool write_file(const char *data, std::size_t left) {
        if (!use_file_) {
            return false;
        }

        while (left) {
            auto written(TEMP_FAILURE_RETRY(::_write(fd_, data, int(left))));
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
    bool open_file(const std::string &filename, int dest, int mode) {
        class file_closer {
        public:
            file_closer(const char *filename, int mode)
                : fd_(-1)
            {
                ::_sopen_s(&fd_, filename, _O_WRONLY | _O_CREAT | _O_APPEND
                           , _SH_DENYNO, mode);
            }

            ~file_closer() {
                if (-1 == fd_) {
                    return;
                }

                ::_close(fd_);
            }

            operator int() {
                return fd_;
            }

        private:
            int fd_;
        } f(filename.c_str(), mode);

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
            int res(::_dup2(oldfd, newfd));
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

#endif // dbglog_logfile_posix_hpp_included_
