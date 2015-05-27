#include "./logfile.hpp"

namespace dbglog {

bool logger_file::closeOnExec(bool value)
{
    // no log file -> fine
    if (fd_ < 0) { return true; }

    int flags(::fcntl(fd_, F_GETFD, 0));
    if (flags == -1) { return false; }
    if (value) {
        flags |= FD_CLOEXEC;
    } else {
        flags &= ~FD_CLOEXEC;
    }

    if (::fcntl(fd_, F_SETFD, flags) == -1) {
        return false;
    }
    return true;
}

} // namespace dbglog
