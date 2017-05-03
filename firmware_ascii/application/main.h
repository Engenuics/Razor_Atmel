/***********************************************************************************************************************
File: main.h

Description:
Header file for main.c.
*******************************************************************************/

#ifndef __MAIN_H
#define __MAIN_H

/***********************************************************************************************************************
* Firmware Version - Printed FIRMWARE_MAIN_REV.FIRMWARE_SUB_REV1 FIRMWARE_SUB_REV2
* See releasenotes.txt for firmware details.
***********************************************************************************************************************/
#define FIRMWARE_MAIN_REV               '0'
#define FIRMWARE_SUB_REV1               '0'
#define FIRMWARE_SUB_REV2               '1'


/***********************************************************************************************************************
* Constant Definitions
***********************************************************************************************************************/
/* G_u32ApplicationFlags definitions are in configuration.h */

/* G_u32SystemFlags */
#define _SYSTEM_CLOCK_NO_STOP_DEBUG     (u32)0x00000001        /* DEBUG module preventing STOP mode */
#define _SYSTEM_CLOCK_NO_STOP_LEDS      (u32)0x00000002        /* LED module preventing STOP mode */
#define _SYSTEM_CLOCK_NO_STOP_USER      (u32)0x00000004        /* User interaction preventing STOP mode */
#define _SYSTEM_CLOCK_OSC_FAIL          (u32)0x00000008        /* Flag if oscillator start-up fails */
#define _SYSTEM_CLOCK_PLL_NO_LOCK       (u32)0x00000010        /* Flag if PLL0 does not lock on startup */
#define _SYSTEM_TIME_WARNING            (u32)0x00000020        /* Flag if a 1ms violation has occurred */

#define _SYSTEM_STARTUP_NO_ANT          (u32)0x01000000        /* Flag if button hold detected at startup */

#define _SYSTEM_SLEEPING                (u32)0x40000000        /* Set into sleep mode to go back to sleep if woken before 1ms period */
#define _SYSTEM_INITIALIZING            (u32)0x80000000        /* Set when system is in initialization phase */

#define  SYSTEM_CLOCK_ALL_FLAGS         (u32)0x0000001F        /* Value to set all System Clock flags */


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/
void Timer1CallBack(void);


#endif /* __MAIN_H */