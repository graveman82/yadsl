#ifndef YADSL_BASETYPES_H
#define YADSL_BASETYPES_H

/** @file BaseTypes.h

Base types used by the library: integers, floats etc.
*/



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

namespace yadsl
{

typedef uint32_t uint;
enum { /** @brief Недопустмое значение идентификатора. */ kNoId = 0xffffffff  };
enum { /** @brief Недопустимое значение индекса. */ kNoIndex = 0xffffffff };

} // end of yadsl

#endif // YADSL_BASETYPES_H

