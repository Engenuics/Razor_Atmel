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
/* GGu32SystemFlags */
#define _SYSTEM_CLOCK_NO_STOP_DEBUG     0x00000001        /* DEBUG module preventing STOP mode */
#define _SYSTEM_CLOCK_NO_STOP_LEDS      0x00000002        /* LED module preventing STOP mode */
#define _SYSTEM_CLOCK_NO_STOP_USER      0x00000004        /* User interaction preventing STOP mode */
#define _SYSTEM_CLOCK_OSC_FAIL          0x00000008        /* Flag if oscillator start-up fails */
#define _SYSTEM_CLOCK_PLL_NO_LOCK       0x00000010        /* Flag if PLL0 does not lock on startup */

#define _SYSTEM_INITIALIZING            0x80000000        /* Set when system is in initialization phase */

#define  SYSTEM_CLOCK_ALL_FLAGS         0x0000001F        /* Value to set all System Clock flags */

/* G_u32ApplicationFlags */
#define _APPLICATION_FLAGS_LED          0x00000001        /* LedStateMachine */
#define _APPLICATION_FLAGS_BUTTON       0x00000002        /* ButtonStateMachine */
#define _APPLICATION_FLAGS_DEBUG        0x00000004        /* DebugStateMachine */
//#define _APPLICATION_FLAGS_DATAFLASH    0x00000008        /* DataflashStateMachine */
//#define _APPLICATION_FLAGS_SDCARD       0x00000010        /* SdCardStateMachine */

#define NUMBER_APPLICATIONS             (u8)3             /* Total number of applications */


#endif /* __MAIN_H */