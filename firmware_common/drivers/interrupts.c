/*!*********************************************************************************************************************
@file interrupts.c                                                               
@brief Definitions for main system interrupts.

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- IRQn_Type IRQ (see the peripheral interrupt number table)

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- void InterruptSetup(void)

Redefinition of WEAK defines in exceptions.c:
- void HardFault_Handler(void);
- void SysTick_Handler(void);
- void PIOA_IrqHandler(void);
- void PIOB_IrqHandler(void);

***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxISR"
***********************************************************************************************************************/
/* New variables */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword)  */
extern volatile u32 G_u32SystemTime1ms;                /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                 /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                  /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;             /*!< @brief From main.c */

extern volatile bool G_abButtonDebounceActive[TOTAL_BUTTONS];      /*!<@brief  From buttons.c    */
extern volatile u32 G_au32ButtonDebounceTimeStart[TOTAL_BUTTONS];  /*!<@brief  From buttons.c    */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variables names shall start with "ISR_" and be declared as static.
***********************************************************************************************************************/


/**********************************************************************************************************************
Interrupt Service Routine Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn void InterruptSetup(void)

@brief Sets up interrupt priorities in the NVIC and enables required interrupts.

Note that other interrupt sources may be enabled outside of this function.
As this should be the first interrupt-related function that is called in
the system, we can conclude that clearing all the pending flags should
work since no peripheral interrupt sources should be connected yet.  However
this is not a requirement.


Requires:
- IRQn_Type enum is sequentially orderred interrupt values starting at 0

Promises:
- Interrupt priorities are set 
- All NVIC pending flags are cleared

*/
void InterruptSetup(void)
{
  u32 au32PriorityConfig[PRIORITY_REGISTERS] = {IPR0_INIT, IPR1_INIT, IPR2_INIT, 
                                                IPR3_INIT, IPR4_INIT, IPR5_INIT,
                                                IPR6_INIT, IPR7_INIT};
  
  /* Disable all interrupts and ensure pending bits are clear */
  for(u8 i = 0; i < SAM3U2_INTERRUPT_SOURCES; i++)
  {
    NVIC_DisableIRQ( (IRQn_Type)i );
    NVIC_ClearPendingIRQ( (IRQn_Type) i);
  } 

  /* Set interrupt priorities */
  for(u8 i = 0; i < PRIORITY_REGISTERS; i++)
  {
    ((u32*)(AT91C_BASE_NVIC->NVIC_IPR))[i] = au32PriorityConfig[i];
  }
      
} /* end InterruptSetup(void) */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void HardFault_Handler(void)
 
@brief A non-maskable (always available) core interrupt that occurs when 
something extraordinary occurs.  

In many cases, this is referencing an invalid address, but can be other 
events of various levels of mystery.  

Requires:
-

Promises:
- Red LED is on, all others off
- Code is held here for debug purposes

*/
void HardFault_Handler(void)
{
#ifdef EIE1
  LedOff(WHITE);
  LedOff(CYAN);
  LedOff(PURPLE);
  LedOff(ORANGE);
  LedOff(BLUE);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOn(RED);
#endif /* EIE1 */

  
#ifdef MPGL2
#ifdef MPGL2_R01  
  LedOff(BLUE);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOn(RED);
#else
  LedOff(BLUE0);
  LedOff(BLUE1);
  LedOff(BLUE2);
  LedOff(BLUE3);
  LedOff(GREEN0);
  LedOff(GREEN1);
  LedOff(GREEN2);
  LedOff(GREEN3);
  LedOff(RED1);
  LedOff(RED2);
  LedOff(RED3);
  
  LedOn(RED0);
#endif /* MPGL2_R01 */
#endif /* MPGL2 */

  while(1);  /* !!!!! update to log and/or report error and/or restart */
  
} /* end HardFault_Handler() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void SysTick_Handler(void)

@brief Updates the global ms timer.  

This interrupt is always enabled and running in 
the system and is essential for system timing and sleep wakeup.
This ISR should be as fast as possible!

Requires:
- NONE

Promises:
- System tick interrupt pending flag is cleared
- G_u32SystemFlags _SYSTEM_SLEEPING cleared

@param G_u32SystemTime1ms counter is incremented by 1

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


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void PIOA_IrqHandler(void)

@brief Parses the PORTA GPIO interrupts and handles them appropriately.  

Note that all PORTA GPIO interrupts are ORed and will trigger this handler, 
therefore any expected interrupt that is enabled must be parsed out and handled.

Requires:
- The button IO bits match the interrupt flag locations

Promises:
- Buttons: sets the active button's debouncing flag, clears the interrupt
  and initializes the button's debounce timer.

*/
void PIOA_IrqHandler(void)
{
  u32 u32GPIOInterruptSources;
  u32 u32ButtonInterrupts;
  u32 u32CurrentButtonLocation;

  /* Grab a snapshot of the current PORTA status flags (clears all flags) */
  u32GPIOInterruptSources  = AT91C_BASE_PIOA->PIO_ISR;

  /******** DO NOT set a breakpoint before this line of the ISR because the debugger
  will "read" PIO_ISR and clear the flags. ********/
  
  /* Examine button interrupts */
  u32ButtonInterrupts = u32GPIOInterruptSources & GPIOA_BUTTONS;
  
  /* Check if any port A buttons interrupted */
  if(u32ButtonInterrupts)
  {
    /* Parse through all the buttons to find those that have interrupted */
    for(u8 i = 0; i < TOTAL_BUTTONS; i++)
    {
      /* Get the bit position of the current button and mask against set interrupts */  
      u32CurrentButtonLocation = GetButtonBitLocation(i, BUTTON_PORTA);
      if(u32ButtonInterrupts & u32CurrentButtonLocation)
      {
        /* Button has interrupted: disable the button's interrupt and start the button's debounce timer */ 
        AT91C_BASE_PIOA->PIO_IDR |= u32CurrentButtonLocation;

        /* Initialize the button's debouncing information */
        G_abButtonDebounceActive[i] = TRUE;
        G_au32ButtonDebounceTimeStart[i] = G_u32SystemTime1ms;
      }
    }
  } /* end button interrupt checking */
  
  /* Clear the PIOA pending flag and exit */
  NVIC_ClearPendingIRQ(IRQn_PIOA);
  
} /* end PIOA_IrqHandler() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void PIOB_IrqHandler(void)

@brief Parses the PORTB GPIO interrupts and handles them appropriately.  

Note that all PORTB GPIO interrupts are ORed and will trigger this handler, 
therefore any expected interrupt that is enabled must be parsed out and handled.

Requires:
- The button IO bits match the interrupt flag locations

Promises:
- Buttons: sets the active button's debouncing flag, clears the interrupt
  and initializes the button's debounce timer.

*/
void PIOB_IrqHandler(void)
{
  u32 u32GPIOInterruptSources;
  u32 u32ButtonInterrupts;
  u32 u32CurrentButtonLocation;

  /* Grab a snapshot of the current PORTB status flags (clears all flags) */
  u32GPIOInterruptSources  = AT91C_BASE_PIOB->PIO_ISR;

  /******** DO NOT set a breakpoint before this line of the ISR because the debugger
  will "read" PIO_ISR and clear the flags. ********/
  
  /* Parse interrupts */
  u32ButtonInterrupts = u32GPIOInterruptSources & GPIOB_BUTTONS;
  
  /* Check if any port B buttons interrupted */
  if(u32ButtonInterrupts)
  {
    /* Parse through all the buttons to find those that have interrupted */
    for(u8 i = 0; i < TOTAL_BUTTONS; i++)
    {
      /* Get the bit position of the current button and mask against set interrupts */  
      u32CurrentButtonLocation = GetButtonBitLocation(i, BUTTON_PORTB);
      if(u32ButtonInterrupts & u32CurrentButtonLocation)
      {
        /* Button has interrupted: disable the button's interrupt and start the button's debounce timer */ 
        AT91C_BASE_PIOB->PIO_IDR |= u32CurrentButtonLocation;

        /* Initialize the button's debouncing information */
        G_abButtonDebounceActive[i] = TRUE;
        G_au32ButtonDebounceTimeStart[i] = G_u32SystemTime1ms;
      }
    }
  } /* end button interrupt checking */

  /* Clear the PIOB pending flag and exit */
  NVIC_ClearPendingIRQ(IRQn_PIOB);
  
} /* end PIOB_IrqHandler() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
