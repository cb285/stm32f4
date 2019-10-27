#include "debug.h"
#include <stdio.h> // vsprintf
#include <string.h> // sprintf
#include <stdarg.h> // va list
#include <stdbool.h>
#include "drivers/usart.h"

#define _MAX_STR 1024
#define _ENDL "\r\n"

static bool _initialized = false;
static usart_t _usart;

static void _Print(const char* str)
{
    Usart_WriteString(_usart, str);
}

void Debug_Create(void) {
    if(_initialized)
	return;
    
    usart_options_t usart_opts;
    usart_opts.base = USART2;
    usart_opts.baudrate = 500000;
    usart_opts.mode = USART__MODE__USART;
    usart_opts.data_len = USART__DATA_LEN__8BITS;
    usart_opts.stop_bits = USART__STOP_BITS__1;
    usart_opts.tx_port = GPIOA;
    usart_opts.tx_pin = 2;
    usart_opts.tx_mode = PIN__MODE__AF7;
    usart_opts.rx_port = GPIOA;
    usart_opts.rx_pin = 3;
    usart_opts.rx_mode = PIN__MODE__AF7;

    _usart = Usart_Create(&usart_opts);
    
    _initialized = true;

    _Print("\r\n");
}

void Debug_Log_(const char* file, int line, const char* func, debug_level_t level, const char *format, ...)
{	
    // don't print if not initialized
    if(!_initialized)
	return;
	
    // don't print if isn't under current level
    if(DEBUG_LEVEL < level)
	return;
    
    char entry_str[_MAX_STR]; // string with format arguments filled
    char print_str[_MAX_STR]; // debug info prepended to entry_str
    va_list args;
    
    // intialize va list
    va_start(args, format);
    
    // create entry string with arguments filled
    vsprintf(entry_str, format, args);
    
    // de-initialize va list
    va_end(args);
    
    // create string to actually print
    sprintf(print_str, "(%s, %s, %d): %s" _ENDL, file, func, line, entry_str);
    
    _Print(print_str);
}
