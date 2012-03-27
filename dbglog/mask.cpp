#include <dbglog/dbglog.hpp>
#include <dbglog/mask.hpp>
#include <dbglog/level.hpp>
#include <dbglog/detail/log_helpers.hpp>

#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_as.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>

#include <stdexcept>

namespace dbglog {

std::string mask::as_string() const
{
    unsigned int m(mask_);
    if (level::none == m) {
        return "NONE";
    } else if (level::all == m) {
            return "ALL";
    }

    return std::string
        (detail::mask2string(m, level::debug)
         + detail::mask2string(m, level::info1)
         + detail::mask2string(m, level::warn1)
         + detail::mask2string(m, level::err1));
}

void mask::from_string(const std::string &str)
{
    using boost::spirit::qi::string;
    using boost::spirit::qi::char_;
    using boost::spirit::qi::parse;
    using boost::spirit::qi::lexeme;
    using boost::spirit::qi::_1;
    using boost::spirit::ascii::space;
    using boost::spirit::as_string;
    using boost::phoenix::ref;

    unsigned int m(0);

    auto default_(string("DEFAULT")
                  [boost::phoenix::ref(m) = dbglog::default_]);
    auto none(string("NONE")[boost::phoenix::ref(m) = dbglog::none]);
    auto all(string("ALL")[boost::phoenix::ref(m) = dbglog::all]);

    auto debug(char_('D')[boost::phoenix::ref(m) |= dbglog::debug]);

    auto info1(string("I1")[boost::phoenix::ref(m) |= dbglog::info1]);
    auto info2(string("I2")[boost::phoenix::ref(m) |= dbglog::info2]);
    auto info3(string("I3")[boost::phoenix::ref(m) |= dbglog::info3]);
    auto info4(string("I4")[boost::phoenix::ref(m) |= dbglog::info4]);

    auto warn1(string("W1")[boost::phoenix::ref(m) |= dbglog::warn1]);
    auto warn2(string("W2")[boost::phoenix::ref(m) |= dbglog::warn2]);
    auto warn3(string("W3")[boost::phoenix::ref(m) |= dbglog::warn3]);
    auto warn4(string("W4")[boost::phoenix::ref(m) |= dbglog::warn4]);

    auto err1(string("E1")[boost::phoenix::ref(m) |= dbglog::err1]);
    auto err2(string("E2")[boost::phoenix::ref(m) |= dbglog::err2]);
    auto err3(string("E3")[boost::phoenix::ref(m) |= dbglog::err3]);
    auto err4(string("E4")[boost::phoenix::ref(m) |= dbglog::err4]);

    auto grammar(default_ | all | none
                 | +(debug | info1 | info2 | info3 | info4
                     | warn1 | warn2 | warn3 | warn4
                     | err1 | err2 | err3 | err4));

    auto first(str.begin());
    auto last(str.end());
    if (!parse(first, last, grammar) || (first != last)) {
        LOG(err1) << "Not a mask definition: <" << str << '>';
        throw std::runtime_error("Bad mask syntax.");
    }

    mask_ = m;
}

} // namespace dbglog
