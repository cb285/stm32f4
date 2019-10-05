#include "debug.h"
#include <stdio.h> // vsprintf
#include <string.h> // sprintf
#include <stdarg.h> // va list
#include <stdbool.h>

// initialized flag
static bool _initialized = false;

static void _Print(const char* str)
{
	//printf(DEBUG: %s\r\n", str);
}

void Debug_Entry_Traced(const char* file, int line, const char* func, debug_level_t level, const char *format, ...)
{	
	// don't print entry if not initialized
	if(!_initialized)
		return;
	
	// don't print entry if isn't under current level
	if(DEBUG_LEVEL < level)
		return;
	
	char entry_str[100]; // string with format arguments filled
	char print_str[100]; // debug info prepended to entry_str
    va_list args;

	// intialize va list
    va_start(args, format);

	// create entry string with arguments filled
	vsprintf(entry_str, format, args);

	// de-initialize va list
	va_end(args);
	
	// create string to actually print
	sprintf(print_str, "DEBUG (file:%s func:%s line:%d): %s", file, func, line, entry_str);
	
    _Print(print_str);
}
