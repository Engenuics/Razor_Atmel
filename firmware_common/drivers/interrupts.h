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


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/




#endif /* __INTERRUPTS_H */
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
