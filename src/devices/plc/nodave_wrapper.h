// Note: nodave.h includes windows.h and defines some macros, as well as sets #pragma pack(1)
#ifndef BCCWIN
#define BCCWIN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#ifndef UNICODE
#define UNICODE
#endif
#ifndef STRICT
#define STRICT
#endif

#pragma pack(push)
#include "nodave.h"
#pragma pack(pop)

#undef interface
#undef ACK