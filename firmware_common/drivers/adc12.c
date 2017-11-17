/**********************************************************************************************************************
File: adc12.c                                                                

Description:
Driver function to give access to the 12-bit ADC on the EiE development boards.  
The ADC hardware is the same for the EiE 1 and EiE 2 development board Blade connectors.
The EiE1 board has an additional on-board potentiometer for testing purporses.

This driver currently only provides setup and single result read access from any
channel on the ADC at a time.  Any averaging or special operations should be handled by the
application using the driver.  This driver is set up as a state machine for future
feature additions.



------------------------------------------------------------------------------------------------------------------------
API:

TYPES

PUBLIC FUNCTIONS


PROTECTED FUNCTIONS
void Adc12Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void Adc12RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32Adc12Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Adc12_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Adc12_StateMachine;                /* The state machine function pointer */
//static u32 Adc12_u32Timeout;                        /* Timeout counter used across states */

static Adc12ChannelType Adc12_aeChannels[] = ADC_CHANNEL_ARRAY;  /* Available channels defined in configuration.h */
static fnCode_u16_type Adc12_afCallbacks[8];          /* ADC12 ISR callback function pointers */

static bool Adc12_bAdcAvailable;                      /* Binary semaphore to control access to the ADC12 peripheral */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: Adc12Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void Adc12Initialize(void)
{
  
  /* Check initialization and set first state */
  if( 1 )
  {
    Adc12_StateMachine = Adc12SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    Adc12_StateMachine = Adc12SM_FailedInit;
  }

} /* end Adc12Initialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function Adc12RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void Adc12RunActiveState(void)
{
  Adc12_StateMachine();

} /* end Adc12RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void Adc12SM_Idle(void)
{
    
} /* end Adc12SM_Idle() */
     

#if 0
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void Adc12SM_Error(void)          
{
  
} /* end Adc12SM_Error() */
#endif


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void Adc12SM_FailedInit(void)          
{
    
} /* end Adc12SM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
