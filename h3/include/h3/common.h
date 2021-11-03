#ifndef _H3_COMMON_H_
#define _H3_COMMON_H_

#define H3_USE_IMGUI_ASSERT

#include <stdarg.h>
#include <stdint.h>

#ifdef __cplusplus
#define H3_CAPI             extern "C"
#define H3_CAPI_BEGIN_BLOCK extern "C" { 
#define H3_CAPI_END_BLOCK   }
#else
#define H3_CAPI
#define H3_CAPI_BEGIN_BLOCK
#define H3_CAPI_END_BLOCK

#include <stdbool.h>
#endif

H3_CAPI_BEGIN_BLOCK

typedef void* H3Handle;

#ifndef NDEBUG
void H3_ASSERT(bool b, const char* msg);
void H3_ASSERT_CONSOLE(bool b, const char* msg);
#else
# define H3_ASSERT(B, MSG)         ((void)(0))
# define H3_ASSERT_CONSOLE(B, MSG) ((void)(0))
#endif

H3_CAPI_END_BLOCK

#endif /* _H3_COMMON_H_ */
