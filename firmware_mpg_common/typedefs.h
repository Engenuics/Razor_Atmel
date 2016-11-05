/*******************************************************************************
* File: typedefs.h                                                               
* Description:
* Type definitions for use with the system.
*******************************************************************************/

#ifndef __TYPEDEFS_H
#define __TYPEDEFS_H


/* CHAR/SHORT/LONG types here for legacy code compatibility */
typedef char CHAR;              /* Signed 8-bits */
typedef unsigned char UCHAR;    /* Unsigned 8-bits */
typedef short SHORT;            /* Signed 16-bits */
typedef unsigned short USHORT;  /* Unsigned 16-bits */
typedef long LONG;              /* Signed 32-bits */
typedef unsigned long ULONG;    /* Unsigned 32-bits */
typedef unsigned char BOOL;     /* Boolean */


/* Standard Peripheral Library old types (maintained for legacy purpose) */
typedef long s32;
typedef short s16;
typedef signed char  s8;

typedef const long sc32;  /*!< Read Only */
typedef const short sc16;  /*!< Read Only */
typedef const char sc8;   /*!< Read Only */

typedef ULONG  u32;
typedef USHORT u16;
typedef UCHAR  u8;

typedef const ULONG uc32;  /*!< Read Only */
typedef const USHORT uc16;  /*!< Read Only */
typedef const USHORT uc8;   /*!< Read Only */

typedef void(*fnCode_type)(void);
typedef void(*fnCode_u16_type)(u16 x);


#ifndef __cplusplus
typedef enum {FALSE = 0, TRUE = !FALSE} bool;
#endif

typedef enum {RESET = 0, SET = !RESET} FlagStatus, ITStatus;

typedef enum {DISABLE = 0, ENABLE = !DISABLE} FunctionalState;
#define IS_FUNCTIONAL_STATE(STATE) (((STATE) == DISABLE) || ((STATE) == ENABLE))

typedef enum {ERROR = 0, SUCCESS = !ERROR} ErrorStatus;

#define BIT0    ((u8)0x01)
#define BIT1    ((u8)0x02)
#define BIT2    ((u8)0x04)
#define BIT3    ((u8)0x08)
#define BIT4    ((u8)0x10)
#define BIT5    ((u8)0x20)
#define BIT6    ((u8)0x40)
#define BIT7    ((u8)0x80)
#define BIT8    ((u16)0x0100)
#define BIT9    ((u16)0x0200)
#define BIT10   ((u16)0x0400)
#define BIT11   ((u16)0x0800)
#define BIT12   ((u16)0x1000)
#define BIT13   ((u16)0x2000)
#define BIT14   ((u16)0x4000)
#define BIT15   ((u16)0x8000)
#define BIT16   ((u32)0x00010000)
#define BIT17   ((u32)0x00020000)
#define BIT18   ((u32)0x00040000)
#define BIT19   ((u32)0x00080000)
#define BIT20   ((u32)0x00100000)
#define BIT21   ((u32)0x00200000)
#define BIT22   ((u32)0x00400000)
#define BIT23   ((u32)0x00800000)
#define BIT24   ((u32)0x01000000)
#define BIT25   ((u32)0x02000000)
#define BIT26   ((u32)0x04000000)
#define BIT27   ((u32)0x08000000)
#define BIT28   ((u32)0x10000000)
#define BIT29   ((u32)0x20000000)
#define BIT30   ((u32)0x40000000)
#define BIT31   ((u32)0x80000000)


#endif /* __TYPEDEFS_H */

