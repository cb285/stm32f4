#ifndef _COMPILER_CONFIG_H
#define _COMPILER_CONFIG_H

#include <stdint.h>
#include <stddef.h>

#define ASM asm
#define VOLATILE volatile
#define INLINE inline
#define ALIGNED __attribute__ ((aligned))
#define PACKED __attribute__ ((packed))

#define __ASM            __asm
#define __STATIC_INLINE  static inline

#define __I volatile const  // defines 'read only' permissions
#define __O volatile        // defines 'write only' permissions
#define __IO volatile       // defines 'read / write' permissions
#define __IM volatile const // defines 'read only' structure member permissions
#define __IOM volatile      // defines 'read / write' structure member permissions
#define __OM volatile       // defines 'write only' structure member permissions

#endif //_COMPILER_CONFIG_H
