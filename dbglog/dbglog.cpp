#include <dbglog/detail/log_helpers.hpp>
#include <dbglog/detail/logger.hpp>

namespace dbglog {

const std::string logger::empty_;

namespace detail {

boost::thread_specific_ptr<std::string> thread_id::holder_;
std::atomic_uint_fast64_t thread_id::generator_(0);

logger deflog(info3 | warn2 | err2 | fatal);

} // namespace detail

void thread_id(const std::string &id)
{
    detail::thread_id::set(id);
}

void thread_id()
{
    detail::thread_id::get();
}

} // namespace dbglog
