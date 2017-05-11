/*!*********************************************************************************************************************
@file interrupts.h                                                               
@brief Interrupt declarations for use with EIE development board firmware.
***********************************************************************************************************************/

#ifndef __INTERRUPTS_H
#define __INTERRUPTS_H

/***********************************************************************************************************************
Type Definitions
***********************************************************************************************************************/

/*! 
@enum IRQn
@brief Exception table items to map ATMEL defined symbols to CMSIS values.
*/
typedef enum IRQn
{
/******  Cortex-M3 Processor Exceptions Numbers ***************************************************/
  NonMaskableInt_IRQn         = -14,         /*!< @brief 2 Non Maskable Interrupt                             */
  MemoryManagement_IRQn       = -12,         /*!< @brief 4 Cortex-M3 Memory Management Interrupt              */
  BusFault_IRQn               = -11,         /*!< @brief 5 Cortex-M3 Bus Fault Interrupt                      */
  UsageFault_IRQn             = -10,         /*!< @brief 6 Cortex-M3 Usage Fault Interrupt                    */
  SVCall_IRQn                 = -5,          /*!< @brief 11 Cortex-M3 SV Call Interrupt                       */
  DebugMonitor_IRQn           = -4,          /*!< @brief 12 Cortex-M3 Debug Monitor Interrupt                 */
  PendSV_IRQn                 = -2,          /*!< @brief 14 Cortex-M3 Pend SV Interrupt                       */
  SysTick_IRQn                = -1,          /*!< @brief 15 Cortex-M3 System Tick Interrupt                   */

/******  AT91SAM3U4 specific Interrupt Numbers ****************************************************/
 IRQn_SUPC                = AT91C_ID_SUPC ,  /*!< @brief SUPPLY CONTROLLER */
 IRQn_RSTC                = AT91C_ID_RSTC ,  /*!< @brief RESET CONTROLLER */
 IRQn_RTC                 = AT91C_ID_RTC  ,  /*!< @brief REAL TIME CLOCK */
 IRQn_RTT                 = AT91C_ID_RTT  ,  /*!< @brief REAL TIME TIMER */
 IRQn_WDG                 = AT91C_ID_WDG  ,  /*!< @brief WATCHDOG TIMER */
 IRQn_PMC                 = AT91C_ID_PMC  ,  /*!< @brief PMC */
 IRQn_EFC0                = AT91C_ID_EFC0 ,  /*!< @brief EFC0 */
 IRQn_EFC1                = AT91C_ID_EFC1 ,  /*!< @brief @brief EFC1 */
 IRQn_DBGU                = AT91C_ID_DBGU ,  /*!< @brief DBGU (dedicated UART) */
 IRQn_HSMC4               = AT91C_ID_HSMC4,  /*!< @brief HSMC4 */
 IRQn_PIOA                = AT91C_ID_PIOA ,  /*!< @brief Parallel IO Controller A */
 IRQn_PIOB                = AT91C_ID_PIOB ,  /*!< @brief Parallel IO Controller B */
 IRQn_PIOC                = AT91C_ID_PIOC ,  /*!< @brief Parallel IO Controller C */
 IRQn_US0                 = AT91C_ID_US0  ,  /*!< @brief USART 0 */
 IRQn_US1                 = AT91C_ID_US1  ,  /*!< @brief USART 1 */
 IRQn_US2                 = AT91C_ID_US2  ,  /*!< @brief USART 2 */
 IRQn_US3                 = AT91C_ID_US3  ,  /*!< @brief USART 3 */
 IRQn_MCI0                = AT91C_ID_MCI0 ,  /*!< @brief Multimedia Card Interface */
 IRQn_TWI0                = AT91C_ID_TWI0 ,  /*!< @brief TWI 0 */
 IRQn_TWI1                = AT91C_ID_TWI1 ,  /*!< @brief TWI 1 */
 IRQn_SPI0                = AT91C_ID_SPI0 ,  /*!< @brief Serial Peripheral Interface */
 IRQn_SSC0                = AT91C_ID_SSC0 ,  /*!< @brief Serial Synchronous Controller 0 */
 IRQn_TC0                 = AT91C_ID_TC0  ,  /*!< @brief @brief Timer Counter 0 */
 IRQn_TC1                 = AT91C_ID_TC1  ,  /*!< @brief Timer Counter 1 */
 IRQn_TC2                 = AT91C_ID_TC2  ,  /*!< @brief Timer Counter 2 */
 IRQn_PWMC                = AT91C_ID_PWMC ,  /*!< @brief Pulse Width Modulation Controller */
 IRQn_ADCC0               = AT91C_ID_ADC12B, /*!< @brief ADC controller0 */
 IRQn_ADCC1               = AT91C_ID_ADC  ,  /*!< @brief ADC controller1 */
 IRQn_HDMA                = AT91C_ID_HDMA ,  /*!< @brief HDMA */
 IRQn_UDPHS               = AT91C_ID_UDPHS   /*!< @brief USB Device High Speed */
} IRQn_Type;


/***********************************************************************************************************************
Constants / Definitions
***********************************************************************************************************************/
/*! Total number of IRQ Channels -> listed in AT91SAM3U4.h around line 6686 */
#define SAM3U2_INTERRUPT_SOURCES            (u8)(30)


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
void InterruptSetup(void);
void HardFault_Handler(void);
void SysTick_Handler(void);
void PIOA_IrqHandler(void);
void PIOB_IrqHandler(void);


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*!*********************************************************************************************************************
@brief Priority registers allocate all interrupts to the available
priority levels.

Interrupt Premption Priorities (Highest -> Lowest)

Interrupt priorities start at 0 (highest priority) and go to
15 (lowest priority).  However, these correspond to Exception priorities 16 
through 31 since there are Core exceptions that are always of higher priority.
The interrupt number is processor specific and can be found around line 6650 in 
the AT91SAM3U4 header file and as "Peripheral Identifiers" on page 42 of the 10-Feb-12 datasheet.
Interrupt number / peripheral identifier has nothing to do with the corresponding interrupt priority.
Interrupt priorities are set by loading a priority slot with an interrupt number.

All unused interrupt sources will be set to priority 31.
*/
#define PRIORITY_REGISTERS          (u8)8



#define IPR0_INIT (u32)0xF0F000F0
/*!< Bit Set Description
    31 [1] ( 3) // REAL TIME TIMER priority 15
    30 [1] "
    29 [1] "
    28 [1] "

    27 [0] Unimplemented
    26 [0] "
    25 [0] "
    24 [0] "

    23 [1] ( 2) // REAL TIME CLOCK priority 15
    22 [1] "
    21 [1] "
    20 [1] "

    19 [0] Unimplemented
    18 [0] "
    17 [0] "
    16 [0] "

    15 [0] ( 1) // RESET CONTROLLER priority 0
    14 [0] "
    13 [0] "
    12 [0] "

    11 [0] Unimplemented
    10 [0] "
    09 [0] "
    08 [0] "

    07 [1] ( 0) // SUPPLY CONTROLLER priority 15
    06 [1] "
    05 [1] "
    04 [1] "

    03 [0] Unimplemented
    02 [0] "
    01 [0] "
    00 [0] "
*/

#define IPR1_INIT (u32)0xF0F0F000
/*!< Bit Set Description
    31 [1] ( 7) // EFC1 priority 15
    30 [1] "
    29 [1] "
    28 [1] "

    27 [0] Unimplemented
    26 [0] "
    25 [0] "
    24 [0] "

    23 [1] ( 6) // EFC0 priority 15
    22 [1] "
    21 [1] "
    20 [1] "

    19 [0] Unimplemented
    18 [0] "
    17 [0] "
    16 [0] "

    15 [1] ( 5) // PMC priority 15
    14 [1] "
    13 [1] "
    12 [1] "

    11 [0] Unimplemented
    10 [0] "
    09 [0] "
    08 [0] "

    07 [0] ( 4) // WATCHDOG TIMER priority 0
    06 [0] "
    05 [0] "
    04 [0] "

    03 [0] Unimplemented
    02 [0] "
    01 [0] "
    00 [0] "
*/

#define IPR2_INIT (u32)0x5050F0F0
/*!< Bit Set Description
    31 [0] (11) // Parallel IO Controller B priority 5
    30 [1] "
    29 [0] "
    28 [1] "

    27 [0] Unimplemented
    26 [0] "
    25 [0] "
    24 [0] "

    23 [0] (10) // Parallel IO Controller A priority 5
    22 [1] "
    21 [0] "
    20 [1] "

    19 [0] Unimplemented
    18 [0] "
    17 [0] "
    16 [0] "

    15 [1] ( 9) // HSMC4 priority 15
    14 [1] "
    13 [1] "
    12 [1] "

    11 [0] Unimplemented
    10 [0] "
    09 [0] "
    08 [0] "

    07 [1] ( 8) // DBGU priority 15
    06 [1] "
    05 [1] "
    04 [1] "

    03 [0] Unimplemented
    02 [0] "
    01 [0] "
    00 [0] "
*/

#define IPR3_INIT (u32)0xF0303050
/*!< Bit Set Description
    31 [1] (15) // USART 2 priority 15
    30 [1] "
    29 [1] "
    28 [1] "

    27 [0] Unimplemented
    26 [0] "
    25 [0] "
    24 [0] "

    23 [0] (14) // USART 1 priority 3
    22 [0] "
    21 [1] "
    20 [1] "

    19 [0] Unimplemented
    18 [0] "
    17 [0] "
    16 [0] "

    15 [0] (13) // USART 0 priority 3
    14 [0] "
    13 [1] "
    12 [1] "

    11 [0] Unimplemented
    10 [0] "
    09 [0] "
    08 [0] "

    07 [0] (12) // Parallel IO Controller C priority 5
    06 [1] "
    05 [0] "
    04 [1] "

    03 [0] Unimplemented
    02 [0] "
    01 [0] "
    00 [0] "
*/

#define IPR4_INIT (u32)0xF020F0F0
/*!< Bit Set Description
    31 [1] (19) // TWI 1 priority 15
    30 [1] "
    29 [1] "
    28 [1] "

    27 [0] Unimplemented
    26 [0] "
    25 [0] "
    24 [0] "

    23 [0] (18) // TWI 0 priority 2
    22 [0] "
    21 [1] "
    20 [0] "

    19 [0] Unimplemented
    18 [0] "
    17 [0] "
    16 [0] "

    15 [1] (17) // Multimedia Card Interface priority 15
    14 [1] "
    13 [1] "
    12 [1] "

    11 [0] Unimplemented
    10 [0] "
    09 [0] "
    08 [0] "

    07 [1] (16) // USART 3 priority 15
    06 [1] "
    05 [1] "
    04 [1] "

    03 [0] Unimplemented
    02 [0] "
    01 [0] "
    00 [0] "
*/

#define IPR5_INIT (u32)0x4000F020
/*!< Bit Set Description
    31 [0] (23) // Timer Counter 1 priority 4
    30 [1] "
    29 [0] "
    28 [0] "

    27 [0] Unimplemented
    26 [0] "
    25 [0] "
    24 [0] "

    23 [0] (22) // Timer Counter 0 priority 0
    22 [0] "
    21 [0] "
    20 [0] "

    19 [0] Unimplemented
    18 [0] "
    17 [0] "
    16 [0] "

    15 [1] (21) // Serial Synchronous Controller 0 priority 15
    14 [1] "
    13 [1] "
    12 [1] "

    11 [0] Unimplemented
    10 [0] "
    09 [0] "
    08 [0] "

    07 [0] (20) // Serial Peripheral Interface priority 2
    06 [0] "
    05 [1] "
    04 [0] "

    03 [0] Unimplemented
    02 [0] "
    01 [0] "
    00 [0] "
*/

#define IPR6_INIT (u32)0xF0F0F040
/*!< Bit Set Description
    31 [1] (27) // 10-bit ADC Controller (ADC) priority 15
    30 [1] "
    29 [1] "
    28 [1] "

    27 [0] Unimplemented
    26 [0] "
    25 [0] "
    24 [0] "

    23 [1] (26) // 12-bit ADC Controller (ADC12B) priority 15
    22 [1] "
    21 [1] "
    20 [1] "

    19 [0] Unimplemented
    18 [0] "
    17 [0] "
    16 [0] "

    15 [1] (25) // Pulse Width Modulation Controller priority 15
    14 [1] "
    13 [1] "
    12 [1] "

    11 [0] Unimplemented
    10 [0] "
    09 [0] "
    08 [0] "

    07 [0] (24) // Timer Counter 2 priority 4
    06 [1] "
    05 [0] "
    04 [0] "

    03 [0] Unimplemented
    02 [0] "
    01 [0] "
    00 [0] "
*/

#define IPR7_INIT (u32)0x000010F0
/*!< Bit Set Description
    31 - 16 [0] Unimplemented

    15 [0] (29) // USB Device High Speed priority 1
    14 [0] "
    13 [0] "
    12 [1] "

    11 [0] Unimplemented
    10 [0] "
    09 [0] "
    08 [0] "

    07 [1] (28) // HDMA priority 15
    06 [1] "
    05 [1] "
    04 [1] "

    03 [0] Unimplemented
    02 [0] "
    01 [0] "
    00 [0] "
*/

/*! @endcond */





#endif /* __INTERRUPTS_H */
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
