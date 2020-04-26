#ifndef _H_UTILS
#define _H_UTILS

#include <string>
#include <stdio.h>

namespace fantuan
{
#define SAFE_DELETE(ptr)	if (ptr) {delete (ptr); (ptr) = NULL;}
#define ERROR(ptr, ...)     printf((ptr), __VA_ARGS__)
#define DEBUG(ptr, ... )    printf((ptr), __VA_ARGS__)
#define TRACE(ptr, ...)     //printf((ptr), __VA_ARGS__)
#define ALERT(ptr, ...)     printf((ptr), __VA_ARGS__)

int64_t now();
std::string timeToString(int64_t);
}

#endif // _H_UTILS