#ifndef _H_UTILS
#define _H_UTILS

namespace fantuan
{
#define SAFE_DELETE(ptr)	if (ptr) {delete (ptr); (ptr) = NULL;}
#define PRINTF(ptr, ...)    PRINTF((ptr), __VA_ARGS__)
}

#endif // _H_UTILS