
#ifndef _EMBEDDEDTYPES_H_
#define _EMBEDDEDTYPES_H_

#include "stdint.h"
/************************************************************************************
*
*       TYPE DEFINITIONS
*
************************************************************************************/
#define ROM_PATCH_TRIGGER(func)

#if defined(__GNUC__)
#define PACKED_STRUCT struct __attribute__ ((__packed__))
#define PACKED_UNION  union __attribute__ ((__packed__))
#elif defined(__IAR_SYSTEMS_ICC__)
#define PACKED_STRUCT __packed struct
#define PACKED_UNION __packed union
#else
#define PACKED_STRUCT struct
#define PACKED_UNION union
#endif


//typedef signed    char  int8_t;
//typedef unsigned  char  uint8_t;
//typedef signed    short int16_t;
//typedef unsigned  short uint16_t;
//typedef signed    long  int32_t;
//typedef unsigned  long  uint32_t;
//typedef signed    long long int64_t;
//typedef unsigned  long long uint64_t;

typedef signed    char  intn8_t;
typedef unsigned  char  uintn8_t;
typedef signed    short intn16_t;
typedef unsigned  short uintn16_t;
typedef signed    long  intn32_t;
typedef unsigned  long  uintn32_t;
typedef signed    long long intn64_t;
typedef unsigned  long long uintn64_t;

typedef volatile signed char vint8_t;
typedef volatile unsigned char vuint8_t;
typedef volatile signed short vint16_t;
typedef volatile unsigned short vuint16_t;
typedef volatile signed long vint32_t;
typedef volatile unsigned long vuint32_t;

/* boolean types */
typedef uint8_t   bool_t;
typedef uintn8_t  booln_t;
/* used for indexing into an array in the most efficient manner for the platform */
typedef uint8_t   index_t;

#ifndef TRUE 
#define TRUE 1
#endif

#ifndef FALSE 
#define FALSE 0
#endif



#endif


