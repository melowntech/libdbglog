#include <boost/test/unit_test.hpp>

#include <iostream>
#include <stdexcept>

#include <dbglog/dbglog.hpp>
#include <dbglog/mask.hpp>

void once() {
    LOGONCE(info4) << "once";
    LOGONCE(info4, dbglog::detail::deflog) << "once: another line";
}

BOOST_AUTO_TEST_CASE(dbglog_once)
{
    once();
    once();
    once();
}


BOOST_AUTO_TEST_CASE(dbglog_mask)
{
    std::string def;
    while (std::cin >> def) {
        try {
            dbglog::mask mask(def);
            std::cout << def <<  " -> " << mask
                      << "(" << mask.get() << ")" << std::endl;
            dbglog::set_mask(mask);
        } catch (const std::runtime_error &e) {
            std::cout << def <<  " -> error: " << e.what() << std::endl;
        }
    }
}
