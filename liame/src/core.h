#pragma once
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>

#include "log.h"

typedef uint8_t u8;
typedef int8_t s8;

typedef int64_t s64;
typedef uint64_t u64;
typedef double f64;

typedef int32_t s32;
typedef uint32_t u32;
typedef float f32;

#define BIT(x) (1 << (x))

#ifdef WINDOWS
	#define __FILENAME__ (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
	#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#endif

#define EDITOR_START() \
{ \
	init_logger(); \
}