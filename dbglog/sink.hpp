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
        : shared_mask_(false), mask_(~mask.get()), name_(name)
    {}

    virtual ~Sink() {}

    inline bool check_level(level l) const {
        return !(mask_ & l) || (l == fatal);
    }

    virtual void write(const std::string &line) = 0;

    const std::string& name() const { return name_; }

    void set_mask(const mask &m) {  mask_ = ~m.get();  }

    void set_mask(unsigned int m) { mask_ = ~m; }

    unsigned int get_mask() const { return ~mask_; }

    std::string get_mask_string() const {
        return mask(~mask_).as_string();
    }

    bool shared_mask() const { return shared_mask_; }

    void shared_mask(bool v) { shared_mask_ = v; }

private:
    bool shared_mask_;
    unsigned int mask_;
    const std::string name_;
};

} // namespace dbglog

#endif // shared_dbglog_sink_hpp_included_
