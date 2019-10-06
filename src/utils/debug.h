#ifndef _DEBUG_H
#define _DEBUG_H

#include <inttypes.h> // printf type specifiers

// debug level setting
#define DEBUG_LEVEL DEBUG__LEVEL__ALL

// rename function to include filename, line, and caller function
#define Debug_Log(level, ...) Debug_Log_(__FILE__, __LINE__, __FUNCTION__, level, __VA_ARGS__)

typedef enum {
	DEBUG__LEVEL__NONE,  // nothing
	DEBUG__LEVEL__ERROR, // errors only
	DEBUG__LEVEL__WARN,  // warnings and errors
	DEBUG__LEVEL__INFO,  // information, warnings, and errors
	DEBUG__LEVEL__ALL    // everything
} debug_level_t;

void Debug_Create(void);
void Debug_Log_(const char* file, int line, const char* func, debug_level_t level, const char *format, ...);

#endif // _DEBUG_H
