/**********************************************************************************************************************
File: interrupts.c                                                               

Description:
Interrupt definitions for use with LED sign controller firmware.
***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword)  */
extern volatile u32 G_u32SystemTime1ms;                            /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                             /* From board-specific source file */

extern volatile u32 G_u32SystemFlags;                              /* From main.c       */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variables names shall start with "ISR_" and be declared as static.
***********************************************************************************************************************/


/**********************************************************************************************************************
Interrupt Service Routine Definitions
***********************************************************************************************************************/

/*----------------------------------------------------------------------------------------------------------------------
Function: InterruptSetup

Description:
Sets up interrupt priorities in the NVIC and enables required interrupts.
Note that other interrupt sources may be enabled outside of this function.

Requires:
  - All peripherals for which interrupts are enabled here should be configured

Promises:
  - Interrupt priorities are set 
*/
void InterruptSetup(void)
{
  u32 au32PriorityConfig[PRIORITY_REGISTERS] = {IPR0_INIT, IPR1_INIT, IPR2_INIT, IPR3_INIT, IPR4_INIT,
                                                IPR5_INIT, IPR6_INIT, IPR7_INIT};
  
  /* Set interrupt priorities */
  for(u8 i = 0; i < PRIORITY_REGISTERS; i++)
  {
    ((u32*)(AT91C_BASE_NVIC->NVIC_IPR))[i] = au32PriorityConfig[i];
  }
  
  /* Disable all interrupts and ensure pending bits are clear */
  for(u8 i = 0; i < SAM3U2_INTERRUPT_SOURCES; i++)
  {
    NVIC_DisableIRQ( (IRQn_Type)i );
    NVIC_ClearPendingIRQ( (IRQn_Type) i);
  } 
    
} /* end InterruptSetup(void) */


/*----------------------------------------------------------------------------------------------------------------------
ISR: HardFault_Handler

Description:
A non-maskable (always available) core interrupt that occurs when something extraordinary
event.  In many cases, this is referencing an invalid address, but can be other events
of various levels of mystery.  

Requires:
  -

Promises:
  - Red LED is on, all others off
  - Code held
*/
void HardFault_Handler(void)
{
  while(1);  /* !!!!! update to log and/or report error and/or restart */
  
} /* end HardFault_Handler() */


/*----------------------------------------------------------------------------------------------------------------------
ISR: SysTick_Handler

Description:
Updates the global ms timer.  This interrupt is always enabled and running in 
the system and is essential for system timing and sleep wakeup.
This ISR should be as fast as possible!

Requires:

Promises:
  - G_u32SystemTime1ms counter is incremented by 1
  - System tick interrupt pending flag is cleared
*/
void SysTick_Handler(void)
{
  static u16 u16SecondCounter = 1000;
  
  /* Update the 1ms system timer and clear sleep flag */
  G_u32SystemTime1ms++;
  G_u32SystemFlags &= ~_SYSTEM_SLEEPING;

  /* Update the 1 second timer if required */
  if(--u16SecondCounter == 0)
  {
    u16SecondCounter = 1000;
    G_u32SystemTime1s++;
  }
    
} /* end SysTickHandler(void) */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
