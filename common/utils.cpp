#include "utils.h"
#include <string>
#include <sys/time.h>

namespace fantuan
{
int64_t now()
{
    timeval tv;
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000000 + tv.tv_usec;
}

std::string timeToString(int64_t time)
{
    char buf[64] = {0};
    time_t sec = (time_t)(time / 1000000);
    tm t;
    gmtime_r(&sec, &t);
    int millisec = (int)(time % 1000000) / 1000;
    snprintf(buf, sizeof(buf), "%4d%02d%02d %02d:%02d:%02d.%03d",
             t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
             t.tm_hour, t.tm_min, t.tm_sec, millisec);
    return buf;
}
}