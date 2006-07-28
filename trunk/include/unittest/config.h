#ifndef __ARCHITECTURE_DEPENDENT_INTEGER_TYPES_H__
#define __ARCHITECTURE_DEPENDENT_INTEGER_TYPES_H__

#ifndef NULL
#define NULL (void*)0
#endif

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef long long int64_t;
typedef int int32_t;
typedef short int16_t;
//typedef char int8_t;

typedef void * 	  native_ptr_t; 
typedef int32_t  	  intptr_t;
typedef uint32_t 	 uintptr_t;

#endif
