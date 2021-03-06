#ifndef COMMON_H
#    define COMMON_H

#    define _CRTDBG_MAP_ALLOC
#    include <crtdbg.h>
#    include <stdlib.h>

extern "C" {
#    include "libavutil/error.h"
}
static char errBuf[AV_ERROR_MAX_STRING_SIZE] = {0};
#    define av_err2str(errnum) av_make_error_string(errBuf, AV_ERROR_MAX_STRING_SIZE, errnum)

#    define int2str(X) (#    X)    //将数字直转成字符串
#endif                             // COMMON_H

#pragma execution_character_set("utf-8")
