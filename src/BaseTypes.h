#ifndef YADSL_BASETYPES_H
#define YADSL_BASETYPES_H

namespace yadsl
{

#ifdef _MSC_VER
typedef __int8				int8_t;
typedef __int16				int16_t;
typedef __int32				int32_t;
typedef __int64				int64_t;
typedef unsigned __int8		uint8_t;
typedef unsigned __int16	uint16_t;
typedef unsigned __int32	uint32_t;
typedef unsigned __int64	uint64_t;
typedef unsigned __int64	uint64_t;
#elif defined(__GNUC__)
#include <stdint.h>
#else
#error unsupported compiler
#endif

typedef uint32_t uint;

} // end of yadsl

#define YADSL_UNUSED_FUNC_PARAM(x) (void)(x)

#endif // YADSL_BASETYPES_H

