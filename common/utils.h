#ifndef _H_UTILS
#define _H_UTILS

namespace fantuan
{
#define SAFE_DELETE(ptr)	if (ptr) {delete (ptr); (ptr) = NULL;}
#define ERROR(ptr, ...)     printf((ptr), __VA_ARGS__)
#define DEBUG(ptr, ... )    printf((ptr), __VA_ARGS__)
#define TRACE(ptr, ...)     //printf((ptr), __VA_ARGS__)
#define ALERT(ptr, ...)     printf((ptr), __VA_ARGS__)
}

#endif // _H_UTILS