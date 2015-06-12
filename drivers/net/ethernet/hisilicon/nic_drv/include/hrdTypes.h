#ifndef __HRD_TYPES_H__
#define __HRD_TYPES_H__

#if 0
typedef unsigned char                                       UINT8;
typedef unsigned short                                      UINT16;
typedef unsigned int                                        UINT32;
typedef signed char                                         INT8;
typedef signed short                                        INT16;
typedef signed int                                          INT32;
typedef float                                               FLOAT;
typedef double                                              DOUBLE;
typedef char                                                CHAR;
typedef unsigned long long                                  UINT64;
typedef signed long long                                    INT64;
typedef unsigned int                                        UINTPTR;
typedef signed int                                          INTPTR;


#ifndef OK
#define OK      0
#endif

#ifndef ERROR
#define ERROR	(-1)
#endif


#ifndef HRD_STATUS
#define HRD_STATUS int
#endif

#ifndef NULL              /* pointer to nothing */
   #define NULL ((void *) 0)
#endif

#ifndef TRUE              /* Boolean true */
   #define TRUE (1)
#endif

#ifndef FALSE              /* Boolean false */
   #define FALSE (0)
#endif
#else
#include "hrd_typedef.h"
#endif


#endif
