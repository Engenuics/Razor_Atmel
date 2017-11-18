/*!
@file exceptions.h 
@brief Atmel-supplied header file for exceptions.c
*/

/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support 
 * ----------------------------------------------------------------------------
 * Copyright (c) 2008, Atmel Corporation
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/*
** This file contains the default exception handlers
** and exception table.
*/

//------------------------------------------------------------------------------
//         Types
//------------------------------------------------------------------------------

/*! 
@enum IntFunc
@brief Function pointer defintion for interrupt handler functions.
*/
typedef void( *IntFunc )( void );

/// Weak attribute
#if defined ( __ICCARM__ )
	#define WEAK __weak
#elif defined (  __GNUC__  )
	#define WEAK __attribute__((weak))
#else
	#error "Unsupported compiler."
#endif


//------------------------------------------------------------------------------
//         Global functions
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
//         Exception Handlers
//------------------------------------------------------------------------------

void NMI_Handler( void );
void HardFault_Handler( void );
void MemManage_Handler( void );
void BusFault_Handler( void );
void UsageFault_Handler( void );
void SVC_Handler( void );
void DebugMon_Handler( void );
void PendSV_Handler( void );
void SysTick_Handler( void );
void IrqHandlerNotUsed(void);


// System Controller
void SYS_IrqHandler(void);
// SUPPLY CONTROLLER
void SUPC_IrqHandler(void);
// RESET CONTROLLER
void RSTC_IrqHandler(void);
// REAL TIME CLOCK
void RTC_IrqHandler(void);
// REAL TIME TIMER
void RTT_IrqHandler(void);
// WATCHDOG TIMER
void WDT_IrqHandler(void);
// PMC
void PMC_IrqHandler(void);
// EFC0
void EFC0_IrqHandler(void);
// EFC1
void EFC1_IrqHandler(void);
// DBGU
void DBGU_IrqHandler(void);
// HSMC4
void HSMC4_IrqHandler(void);
// Parallel IO Controller A
void PIOA_IrqHandler(void);
// Parallel IO Controller B
void PIOB_IrqHandler(void);
// Parallel IO Controller C
void PIOC_IrqHandler(void);
// USART 0
void USART0_IrqHandler(void);
// USART 1
void USART1_IrqHandler(void);
// USART 2
void USART2_IrqHandler(void);
// USART 3
void USART3_IrqHandler(void);
// Multimedia Card Interface
void MCI0_IrqHandler(void);
// TWI 0
void TWI0_IrqHandler(void);
// TWI 1
void TWI1_IrqHandler(void);
// Serial Peripheral Interface 0
void SPI0_IrqHandler(void);
// Serial Synchronous Controller 0
void SSC0_IrqHandler(void);
// Timer Counter 0
void TC0_IrqHandler(void);
// Timer Counter 1
void TC1_IrqHandler(void);
// Timer Counter 2
void TC2_IrqHandler(void);
// PWM Controller
void PWM_IrqHandler(void);
// ADC controller0
void ADCC0_IrqHandler(void);
// ADC controller1
void ADCC1_IrqHandler(void);
// HDMA
void HDMA_IrqHandler(void);
// USB Device High Speed UDP_HS
void UDPD_IrqHandler(void);

