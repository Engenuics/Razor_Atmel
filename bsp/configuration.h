/**********************************************************************************************************************
File: configuration.h      

Description:
Main configuration header file for project.  This file bridges many of the generic features of the 
firmware to the specific features of the design. The definitions should be updated
to match the target hardware.  

Bookmarks:
!!!!! External module peripheral assignments
@@@@@ GPIO board-specific parameters
##### Communication peripheral board-specific parameters


***********************************************************************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/**********************************************************************************************************************
Runtime switches
***********************************************************************************************************************/
#define MPGL1             1         /* Use to activate MPG Level 1 specific code */
//#define MPGL2             1         /* Use to activate MPG Level 2 specific code */

#define DEBUG_MODE        1         /* Define to enable certain debugging code */


/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "AT91SAM3U4.h"
#include "exceptions.h"
#include "interrupts.h"
#include "core_cm3.h"
#include "main.h"
#include "typedefs.h"
#include "utilities.h"

/* Board support package header files */
#ifdef MPGL1
#include "mpgl1-ehdw-02.h"
#endif

#ifdef MPGL2
/* Board support package header files */
#include "mpgl2-ehdw-01.h"
#endif

/* Driver header files */
#include "buttons.h"
#include "debug.h"
#include "leds.h" 
#include "messaging.h"
#include "sam3u_uart.h"

//#include "got.c"

/* Application header files */
#include "mpgl1_audio_test.h"


/**********************************************************************************************************************
!!!!! External device peripheral assignments
***********************************************************************************************************************/
/* %UART% Configuration */
/* Debug UART Peripheral Allocation (USART0) */
#define DEBUG_UART                  USART0
#define USART0_US_CR_INIT           DEBUG_US_CR_INIT
#define USART0_US_MR_INIT           DEBUG_US_MR_INIT
#define USART0_US_IER_INIT          DEBUG_US_IER_INIT
#define USART0_US_IDR_INIT          DEBUG_US_IDR_INIT
#define USART0_US_BRGR_INIT         DEBUG_US_BRGR_INIT

#define DEBUG_UART_IRQHandler       USART0_IRQHandler
#define DEBUG_UART_PERIPHERAL       AT91C_ID_US0



/***********************************************************************************************************************
@@@@@ GPIO board-specific parameters
***********************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------
%LED% LED Configuration                                                                                                  
------------------------------------------------------------------------------------------------------------------------
Update the values below for the LEDs on the board.  Any name can be used for the LED definitions.
Open the LED source.c and edit Led_au32BitPositions and Leds_asLedArray with the correct values for the LEDs in the system.  
*/

#ifdef MPGL1
#define TOTAL_LEDS            (u8)11        /* Total number of LEDs in the system */
#endif /* MPGL1 */

#ifdef MPGL2
#define TOTAL_LEDS            (u8)5         /* Total number of LEDs in the system */
#endif /* MPGL2 */


/*----------------------------------------------------------------------------------------------------------------------
%BUTTON% Button Configuration                                                                                                  
------------------------------------------------------------------------------------------------------------------------
Update the values below for the buttons on the board.  Any name can be used for the BUTTON definitions.
Open buttons.c and edit the GPIO definitions sections with the bit numbers for the buttons.
The order of the definitions below must match the order of the definitions provided in buttons.c. 

***The driver currently only supports a single GPIO port, so just use BUTTON0 for now.***
*/

#ifdef MPGL1
#define TOTAL_BUTTONS         (u8)4       /* Total number of Buttons in the system */

#define BUTTON0               (u32)0
#define BUTTON1               (u32)1
#define BUTTON2               (u32)2
#define BUTTON3               (u32)3

/* All buttons on each port must be ORed together here: set to 0 if no buttons on the port */
#define GPIOA_BUTTONS         (u32)( PA_17_BUTTON0 )
#define GPIOB_BUTTONS         (u32)( PB_00_BUTTON1 | PB_01_BUTTON2 | PB_02_BUTTON3 )
#endif /* MPGL1 */

#ifdef MPGL2
#define TOTAL_BUTTONS         (u8)2       /* Total number of Buttons in the system */

#define BUTTON0               (u32)0
#define BUTTON1               (u32)1

/* All buttons on each port must be ORed together here: set to 0 if no buttons on the port */
#define GPIOA_BUTTONS         (u32)( PA_17_BUTTON0 )
#define GPIOB_BUTTONS         (u32)( PB_00_BUTTON1 )
#endif /* MPGL2 */

#endif /* __CONFIG_H */


/***********************************************************************************************************************
##### Communication peripheral board-specific parameters
***********************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------
Debug UART Setup

Debug is used for the terminal (serial: 38.4k, 8-N-1) debugging interface.
*/
/* USART Control Register - Page 734 */
#define DEBUG_US_CR_INIT (u32)0x00000050
/*
    31 - 20 [0] Reserved

    19 [0] RTSDIS/RCS no release/force RTS to 1
    18 [0] RTSEN/FCS no drive/force RTS to 0
    17 [0] DTRDIS no drive DTR to 1
    16 [0] DTREN no drive DTR to 0

    15 [0] RETTO no restart timeout
    14 [0] RSTNACK N/A
    13 [0] RSTIT N/A
    12 [0] SENDA N/A

    11 [0] STTTO no start time-out
    10 [0] STPBRK no stop break
    09 [0] STTBRK no transmit break
    08 [0] RSTSTA status bits not reset

    07 [0] TXDIS transmitter not disabled
    06 [1] TXEN transmitter enabled
    05 [0] RXDIS receiver not disabled
    04 [1] RXEN receiver enabled

    03 [0] RSTTX not reset
    02 [0] RSTRX not reset
    01 [0] Reserved
    00 [0] "
*/

/* USART Mode Register - page 737 */
#define DEBUG_US_MR_INIT (u32)0x004008C0
/*
    31 [0] ONEBIT start frame delimiter is COMMAND or DATA SYNC
    30 [0] MODSYNC Manchester start bit N/A
    29 [0] MAN Machester encoding disabled
    28 [0] FILTER no filter on Rx line

    27 [0] Reserved
    26 [0] MAX_ITERATION (ISO7816 mode only)
    25 [0] "
    24 [0] "

    23 [0] INVDATA data is not inverted
    22 [1] VAR_SYNC sync field is updated on char to US_THR
    21 [0] DSNACK delicious! NACK is sent on ISO line immeidately on parity error
    20 [0] INACK NACK generated (N/A for async)

    19 [0] OVER 16x oversampling
    18 [0] CLKO USART does not drive the SCK pin
    17 [0] MODE9 CHRL defines char length
    16 [0] MSBF/CPOL LSB first

    15 [0] CHMODE normal mode
    14 [0] "
    13 [0] NBSTOP 1 stop bit
    12 [0] "

    11 [1] PAR no parity
    10 [0] "
    09 [0] "
    08 [0] SYNC/CPHA asynchronous

    07 [1] CHRL 8 bits
    06 [1] "
    05 [0] USCLKS MCK
    04 [0] "

    03 [0] USART_MODE normal
    02 [0] "
    01 [0] "
    00 [0] "
*/


/* USART Interrupt Enable Register - Page 741 */
#define DEBUG_US_IER_INIT (u32)0x00000001
/*
    31 [0] Reserved
    30 [0] "
    29 [0] "
    28 [0] "

    27 [0] "
    26 [0] "
    25 [0] "
    24 [0] MANE Manchester Error interrupt not enabled

    23 [0] Reserved
    22 [0] "
    21 [0] "
    20 [0] "

    19 [0] CTSIC Clear to Send Change interrupt not enabled
    18 [0] DCDIC Data Carrier Detect Change interrupt not enabled
    17 [0] DSRIC Data Set Ready Change interrupt not enabled
    16 [0] RIIC Ring Inidicator Change interrupt not enabled

    15 [0] Reserved
    14 [0] "
    13 [0] NACK Non Ack interrupt not enabled
    12 [0] RXBUFF Reception Buffer Full (PDC) interrupt not enabled

    11 [0] TXBUFE Transmission Buffer Empty (PDC) interrupt not enabled
    10 [0] ITER/UNRE Max number of Repetitions Reached interrupt not enabled
    09 [0] TXEMPTY Transmitter Empty interrupt not enabled (yet)
    08 [0] TIMEOUT Receiver Time-out interrupt not enabled

    07 [0] PARE Parity Error interrupt not enabled
    06 [0] FRAME Framing Error interrupt not enabled
    05 [0] OVRE Overrun Error interrupt not enabled
    04 [0] ENDTX End of Transmitter Transfer (PDC) interrupt enabled

    03 [0] ENDRX End of Receiver Transfer (PDC) interrupt enabled
    02 [0] RXBRK Break Received interrupt not enabled
    01 [0] TXRDY Transmitter Ready interrupt not enabled
    00 [1] RXRDY Receiver Ready interrupt enabled
*/

/* USART Interrupt Disable Register - Page 743 */
#define DEBUG_US_IDR_INIT (u32)~DEBUG_US_IER_INIT

/* USART Baud Rate Generator Register - Page 752
BAUD = MCK / (8(2-OVER)(CD + FP / 8))
=> CD = (MCK / (8(2-OVER)BAUD)) - (FP / 8)
MCK = 48MHz
OVER = 0 (16-bit oversampling)
BAUD desired = 38400 bps
=> CD = 78.125 - (FP / 8)
Set FP = 1, CD = 78
*/
#define DEBUG_US_BRGR_INIT (u32)0x0001004E
/*
    31-20 [0] Reserved

    19 [0] Reserved
    18 [0] FP = 1
    17 [0] "
    16 [1] "

    15 [0] CD = 78 = 0x4E
    14 [0] "
    13 [0] "
    12 [0] "

    11 [0] "
    10 [0] "
    09 [0] "
    08 [0] "

    07 [0] "
    06 [1] "
    05 [0] "
    04 [0] "

    03 [1] "
    02 [1] "
    01 [1] "
    00 [0] "
*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

