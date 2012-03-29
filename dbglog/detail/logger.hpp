#ifndef shared_dbglog_detail_logger_hpp_included_
#define shared_dbglog_detail_logger_hpp_included_

#include <dbglog/logger.hpp>

namespace dbglog { namespace detail {

extern logger deflog;

template <typename SinkType>
inline bool check_level(level l, const SinkType &sink
                        , std::atomic<bool> &once_guard)
{
    return sink.check_level(l, once_guard);
}

template <typename SinkType>
inline bool check_level(level l, const SinkType &sink)
{
    return sink.check_level(l);
}

inline bool check_level(level l)
{
    return detail::deflog.check_level(l);
}

inline bool check_level(level l, std::atomic<bool> &once_guard)
{
    return detail::deflog.check_level(l, once_guard);
}

} } // namespace dbglog::detail

#endif // shared_dbglog_detail_logger_hpp_included_
