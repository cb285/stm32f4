#ifndef _COMPILER_CONFIG_H
#define _COMPILER_CONFIG_H

#include <stdint.h>
#include <stddef.h>

#define __I volatile const // defines 'read only' permissions
#define __O volatile       // defines 'write only' permissions
#define __IO volatile      // defines 'read / write' permissions

#define ASM asm
#define VOLATILE volatile
#define INLINE inline
#define ALIGNED __attribute__ ((aligned))

#endif //_COMPILER_CONFIG_H
