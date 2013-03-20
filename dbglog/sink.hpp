#ifndef shared_dbglog_sink_hpp_included_
#define shared_dbglog_sink_hpp_included_

#include <string>
#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <dbglog/level.hpp>

namespace dbglog {

class Sink : boost::noncopyable {
public:
    typedef boost::shared_ptr<Sink> pointer;

    typedef std::vector<pointer> list;

    Sink(const mask &mask, const std::string &name)
        : mask_(~mask.get()), name_(name)
    {}

    virtual ~Sink() {}

    inline bool check_level(level l) const {
        return !(mask_ & l) || (l == fatal);
    }

    virtual void write(const std::string &line) = 0;

    const std::string& name() const { return name_; }

private:
    unsigned int mask_;
    const std::string name_;
};

} // namespace dbglog

#endif // shared_dbglog_sink_hpp_included_
