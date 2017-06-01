/*!**********************************************************************************************************************
@file eief1-pcb-01.h                                                                
@brief This file provides header information for the eief1-pcb-01 board.
***********************************************************************************************************************/

#ifndef __EIEF1
#define __EIEF1

/***********************************************************************************************************************
Type Definitions
***********************************************************************************************************************/


/***********************************************************************************************************************
* Constants
***********************************************************************************************************************/



/***********************************************************************************************************************
* Macros
***********************************************************************************************************************/


/***********************************************************************************************************************
* Function Declarations
***********************************************************************************************************************/

/*------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
!!!!! GPIO pin names
***********************************************************************************************************************/
/* Hardware Definition for PCB EIEF1-PCB-01 */



/*! @cond DOXYGEN_EXCLUDE */
/***********************************************************************************************************************
Perihperal Setup Initializations

Bookmarks:
@@@@@ Clock, Power Control, Systick and Watchdog setup values

***********************************************************************************************************************/

/***********************************************************************************************************************
@@@@@ Clock, Systick and Power Control setup values
***********************************************************************************************************************/
#define PMC_PCER_INIT (u32)0x27FCED73
/*
    31 [0] Reserved
    30 [0] "
    29 [1] AT91C_ID_UDPHS  USB Device High Speed clock enabled
    28 [0] AT91C_ID_HDMA   HDMA

    27 [0] AT91C_ID_ADC    10-bit ADC Controller (ADC) not enabled
    26 [1] AT91C_ID_ADC12B 12-bit ADC Controller (ADC12B) clock enabled
    25 [1] AT91C_ID_PWMC   Pulse Width Modulation Controller clock enabled
    24 [1] AT91C_ID_TC2    Timer Counter 2 clock enabled

    23 [1] AT91C_ID_TC1    Timer Counter 1 clock enabled
    22 [1] AT91C_ID_TC0    Timer Counter 0 clock enabled
    21 [1] AT91C_ID_SSC0   Serial Synchronous Controller 0 clock enabled
    20 [1] AT91C_ID_SPI0   Serial Peripheral Interface clock enabled

    19 [1] AT91C_ID_TWI1   TWI 1 clock enabled
    18 [1] AT91C_ID_TWI0   TWI 0 clock enabled
    17 [0] AT91C_ID_MCI0   Multimedia Card Interface not enabled
    16 [0] AT91C_ID_US3    USART 3 not enabled

    15 [1] AT91C_ID_US2    USART 2 clock enabled
    14 [1] AT91C_ID_US1    USART 1 clock enabled
    13 [1] AT91C_ID_US0    USART 0 clock enabled
    12 [0] AT91C_ID_PIOC   Parallel IO Controller C  not enabled

    11 [1] AT91C_ID_PIOB   Parallel IO Controller B clock enabled
    10 [1] AT91C_ID_PIOA   Parallel IO Controller A clock enabled
    09 [0] AT91C_ID_HSMC4  HSMC4 not enabled
    08 [1] AT91C_ID_DBGU   DBGU (standalone UART) clock enabled

    07 [0] AT91C_ID_EFC1   EFC1 not enabled
    06 [1] AT91C_ID_EFC0   EFC0 clock enabled
    05 [1] AT91C_ID_PMC    PMC clock enabled
    04 [1] AT91C_ID_WDG    WATCHDOG TIMER clock enabled

    03 [0] AT91C_ID_RTT    REAL TIME TIMER not enabled
    02 [0] AT91C_ID_RTC    REAL TIME CLOCK not enabled
    01 [1] AT91C_ID_RSTC   RESET CONTROLLER clock enabled
    00 [1] AT91C_ID_SUPC   SUPPLY CONTROLLER clock enabled
*/


#define MOR_KEY      (u32)(0x37 << 16)
#define PMC_MOR_INIT (u32)0x0037F019
/*
    31 [0] Reserved
    30 [0] "
    29 [0] "
    28 [0] "

    27 [0] "
    26 [0] "
    25 [0] CFDEN clock failure detector disabled
    24 [0] MOSCSEL internal main oscillator selected for now

    23 [0] KEY 0x37
    22 [0] "
    21 [1] "
    20 [1] "

    19 [0] "
    18 [1] "
    17 [1] "
    16 [1] "

    15 [1] MOSCXTST main crystal start-up time (based on 8 x the slow clock frequency)
    14 [1] "
    13 [1] "
    12 [1] "

    11 [0] "
    10 [0] "
    09 [0] "
    08 [0] "

    07 [0] Reserved
    06 [0] MOSCRCF 8MHz
    05 [0] "
    04 [1] "

    03 [1] MOSCRCEN main on-chip RC osc is on for now
    02 [0] WAITMODE disabled
    01 [0] MOSCXTBY main crystal osc is not bypassed
    00 [1] MOSCXTEN main crystal osc is enabled
*/


#define PMC_MCKR_INIT (u32)(0x00002011)
/*
    31 [0] Reserved
    30 [0] "
    29 [0] "
    28 [0] "

    27 [0] "
    26 [0] "
    25 [0] "
    24 [0] "

    23 [0] "
    22 [0] "
    21 [0] "
    20 [0] "

    19 [0] "
    18 [0] "
    17 [0] "
    16 [0] "

    15 [0] "
    14 [0] "
    13 [1] UPLLDIV UPLL divided by 2 
    12 [0] Reserved 

    11 [0] "
    10 [0] "
    09 [0] "
    08 [0] "

    07 [0] "
    06 [0] PRES processor clock prescaler selected clock divided by 2
    05 [0] 
    04 [1] 

    03 [0] 
    02 [0] 
    01 [0] CSS Master clock selection MAIN_CLK for now
    00 [1] 
*/

#define PMC_MCKR_PLLA (u32)(0x00002012)


/* The PLL DIVA and MULA coefficients set the PLLA output frequency.
The PLLA input frequency must be between 8 and 16 MHz so with 12MHz clock, DIVA must be 1.
Since we want PLLACK at 96 MHz:
(96 MHZ * DIVA / 12 MHz) - 1 = MULA = 7
*/

#define PMC_PLAAR_INIT (u32)0x20073F01
/* Bit Set Description
    31 [0] Reserved
    30 [0] "
    29 [1] Always 1
    28 [0] "

    27 [0] "
    26 [0] MULA
    25 [0] "
    24 [0] "


    23 [0] "
    22 [0] "
    21 [0] "
    20 [0] "

    19 [0] "
    18 [1] "
    17 [1] "
    16 [1] "

    15 [0] Reserved
    14 [0] "
    13 [1] PLLACOUNT
    12 [1] "

    11 [1] "
    10 [1] "
    09 [1] "
    08 [1] "

    07 [0] DIVA
    06 [0] "
    05 [0] "
    04 [0] "

    03 [0] "
    02 [0] "
    01 [0] "
    00 [1] "
*/


/* Watch Dog Values
The watchdog oscillator is on the internal 32k RC with a 128 prescaler = 3.9ms / tick.  
For a minimum 5 second watchdog timeout, the watchdog
counter must be set at 1280. */

#define WDT_CR_FEED  (u32)0xA5000001
/*
    31 [1] Key
    30 [0] " 
    29 [1] "
    28 [0] "

    27 [0] "
    26 [1] "
    25 [0] "
    24 [1] "

    23 - 04 [0] Reserved

    03 [0] Reserved
    02 [0] "
    01 [0] "
    00 [1] WDRSTT Restart watchdog
*/


#define WDT_MR_INIT      (u32)0x1FFF0500
/*
    31 [0] Reserved
    30 [0] "
    29 [0] WDIDLEHLT Watchdog runs when system is idle
    28 [1] WDDBGHLT Watchdog stops in debug state

    27 [1] WDD Watchdog delta value (allow resets any time)
    26 [1] "
    25 [1] "
    24 [1] "

    23 [1] "
    22 [1] "
    21 [1] "
    20 [1] "

    19 [1] "
    18 [1] "
    17 [1] "
    16 [1] "

    15 [0] WDDIS Watchdog is enabled
    14 [0] WDRPROC watchdog reset processor off
    13 [0] WDRSTEN Watchdog reset enable off
    12 [0] WDFIEN Watchdog fault interrupt enable off

    11 [0] WDV Watchdog counter value: 0x500 = 1280 x (128 x 1/32768) = 5 seconds
    10 [1] "
    09 [0] "
    08 [1] "

    07 [0] "
    06 [0] "
    05 [0] "
    04 [0] "

    03 [0] "
    02 [0] "
    01 [0] "
    00 [0] "
*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

#define TEMPLATE_INIT (u32)0x00000000
/*
    31 [0] 
    30 [0] 
    29 [0] 
    28 [0] 

    27 [0] 
    26 [0] 
    25 [0] 
    24 [0] 

    23 [0] 
    22 [0] 
    21 [0] 
    20 [0] 

    19 [0] 
    18 [0] 
    17 [0] 
    16 [0] 

    15 [0] 
    14 [0] 
    13 [0] 
    12 [0] 

    11 [0] 
    10 [0] 
    09 [0] 
    08 [0] 

    07 [0] 
    06 [0] 
    05 [0] 
    04 [0] 

    03 [0] 
    02 [0] 
    01 [0] 
    00 [0] 
*/



#endif /* __EIEF1 */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

