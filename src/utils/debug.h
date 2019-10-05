#ifndef _DEBUG_H
#define _DEBUG_H

#include <inttypes.h> // printf type specifiers

// debug level setting
#define DEBUG_LEVEL DEBUG__LEVEL_ALL

// rename function to include filename, line, and caller function
#define Debug_Entry(level, ...) Debug_Entry_Traced(__FILE__, __LINE__, __FUNCTION__, level, __VA_ARGS__)

typedef enum {
	DEBUG__LEVEL_NONE,  // nothing
	DEBUG__LEVEL_ERROR, // errors only
	DEBUG__LEVEL_WARN,  // warnings and errors
	DEBUG__LEVEL_INFO,  // information, warnings, and errors
	DEBUG__LEVEL_ALL    // everything
} debug_level_t;

void Debug_Entry_Traced(const char* file, int line, const char* func, debug_level_t level, const char *format, ...);

#endif // _DEBUG_H
