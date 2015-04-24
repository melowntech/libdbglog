#ifndef shared_dbglog_level_hpp_included_
#define shared_dbglog_level_hpp_included_

namespace dbglog {

enum level {
    all =        0xfffffu
    , none =     0x00000u

    , debug =    0x0000fu

    , info1 =    0x000f0u
    , info2 =    0x00070u
    , info3 =    0x00030u
    , info4 =    0x00010u

    , warn1 =    0x00f00u
    , warn2 =    0x00700u
    , warn3 =    0x00300u
    , warn4 =    0x00100u

    , err1 =     0x0f000u
    , err2 =     0x07000u
    , err3 =     0x03000u
    , err4 =     0x01000u

    , fatal =    0xf0000u

    , default_ =  info3 | warn2 | err2
    , verbose = info2 | warn2 | err2
};

} // namespace dbglog

#endif // shared_dbglog_level_hpp_included_

