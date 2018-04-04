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

#ifndef dbglog_detail_time_posix_hpp_included_
#define dbglog_detail_time_posix_hpp_included_

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>

#include <cstdio>

namespace dbglog { namespace detail {

inline char* format_time(timebuffer &b, unsigned short precision = 0)
{
    SYSTEMTIME now;
    ::GetLocalTime(&now);
    auto left(sizeof(b) - 1);
    auto written(snprintf(b, left
                          , "%4d-%2d-%2d %2d:%2d:%2d"
                          , now.wYear, now.wMonth, now.wDay
                          , now.wHour, now.wMinute, now.wSecond));
    // append
    switch (precision) {
    case 0: break;

    case 1:
        snprintf(b + written, left - written, ".%01u"
                 , static_cast<unsigned int>(now.wMilliseconds / 100));
        break;

    case 2:
        snprintf(b + written, left - written, ".%02u"
                 , static_cast<unsigned int>(now.wMilliseconds / 10));
        break;

    default:
        // 3 and more
        snprintf(b + written, left - written, ".%03u"
                 , static_cast<unsigned int>(now.wMilliseconds));
        break;
    }

    return b;
}

} } // namespace dbglog::detail

#endif // dbglog_detail_time_posix_hpp_included_
