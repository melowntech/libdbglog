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

#include "time.hpp"

#include <sys/time.h>
#include <cstdio>
#include <ctime>

namespace dbglog { namespace detail {

char* format_time(timebuffer &b, unsigned short precision)
{
    timeval now;
    gettimeofday(&now, 0x0);
    tm now_bd;
    localtime_r(&now.tv_sec, &now_bd);

    auto left(sizeof(b) - 1);
    auto written(strftime(b, left, "%Y-%m-%d %T", &now_bd));
    // NB: written == 0 if buffer was too short; should not happen
    auto end(b + written);
    left -= written;

    // append sub-second fraction; snprintf writes up to left characters
    // including final NUL
    switch (precision) {
    case 0: break;

    case 1:
        snprintf(end, left, ".%01u"
                 , static_cast<unsigned int>(now.tv_usec / 100000));
        break;

    case 2:
        snprintf(end, left, ".%02u"
                 , static_cast<unsigned int>(now.tv_usec / 10000));
        break;

    case 3:
        snprintf(end, left, ".%03u"
                 , static_cast<unsigned int>(now.tv_usec / 1000));
        break;

    case 4:
        snprintf(end, left, ".%04u"
                 , static_cast<unsigned int>(now.tv_usec / 100));
        break;

    case 5:
        snprintf(end, left, ".%05u"
                 , static_cast<unsigned int>(now.tv_usec / 10));
        break;

    default:
        // 6 and more
        snprintf(end, left, ".%06u"
                 , static_cast<unsigned int>(now.tv_usec));
        break;
    }

    // enforce final NUL even if snprintf truncated output (should not happen)
    end[left] = '\0';

    return b;
}

} } // namespace dbglog::detail
