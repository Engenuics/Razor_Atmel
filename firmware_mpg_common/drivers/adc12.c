/**********************************************************************************************************************
File: adc12.c                                                                

Description:
This is a adc12.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
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
static fnCode_type Adc12_StateMachine;            /* The state machine function pointer */
//static u32 Adc12_u32Timeout;                      /* Timeout counter used across states */
static fnCode_u16_type Adc12_fpCallbackCh1;           /* ADC12 ISR callback function pointer */
static fnCode_u16_type Adc12_fpCallbackCh2;           /* ADC12 ISR callback function pointer */
static fnCode_u16_type Adc12_fpCallbackCh3;           /* ADC12 ISR callback function pointer */

static bool Adc12_bAdcAvailable;                  /* Binary semaphore to control access to the ADC12 peripheral */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

#if 0
/*----------------------------------------------------------------------------------------------------------------------
Function: Adc12EnableChannel

Description
Enables the specified channel including the associated interrupt.

Requires:
  - eAdcChannel_ is the ADC12 channel to enable

Promises:
  - ADC12B_CHER bit for eAdcChannel_ is set
  - ADC12B_IER bit for eAdcChannel_is set
*/
void Adc12EnableChannel(Adc12ChannelType eAdcChannel_)
{
    AT91C_BASE_ADC12B->ADC12B_CHER = (1 << eAdcChannel_);
} /* end Adc12EnableChannel() */

    
/*----------------------------------------------------------------------------------------------------------------------
Function: Adc12DisableChannel

Description
Disables the specified channel including the associated interrupt.

Requires:
  - eAdcChannel_ is the ADC12 channel to disable

Promises:
  - ADC12B_CHDR bit for eAdcChannel_ is set
  - ADC12B_IDR bit for eAdcChannel_is set
*/
void Adc12DisableChannel(Adc12ChannelType eAdcChannel_)
{
} /* end Adc12DisableChannel() */
#endif


/*----------------------------------------------------------------------------------------------------------------------
Function: Adc12AssignCallback

Description
Allows user to specify a custom callback function for when the ADC12 interrupt occurs.

Requires:
  - eAdcChannel_ is the channel to which the callback will be assigned
  - fpUserCallback_ is the function address (name) for the user's callback

Promises:
  - Adc12_fpCallbackCh<eAdcChannel_> loaded with fpUserCallback_
*/
void Adc12AssignCallback(Adc12ChannelType eAdcChannel_, fnCode_u16_type fpUserCallback_)
{
  switch(eAdcChannel_)
  {
    case ADC12_CH1:
    {
      Adc12_fpCallbackCh1 = fpUserCallback_;
      break;
    }
    case ADC12_CH2:
    {
      Adc12_fpCallbackCh2 = fpUserCallback_;
      break;
    }
    case ADC12_CH3:
    {
      Adc12_fpCallbackCh3 = fpUserCallback_;
      break;
    }
    default:
    {
      DebugPrintf("Invalid channel\n\r");
    }
  } /* end switch(eAdcChannel_) */

} /* end Adc12AssignCallback() */


/*----------------------------------------------------------------------------------------------------------------------
Function: Adc12StartConversion

Description
Starts the conversion on the selected channel if the ADC is ready.
If the ADC is busy, the function returns FALSE so the calling application knows
to wait and try later.

Requires:
  - eAdcChannel_ is the ADC12 channel to disable
  - Adc12_bAdcAvailable indicates if the ADC is available for a conversion

Promises:
If Adc12_bAdcAvailable is TRUE:
  - Adc12_bAdcAvailable changed to false
  - ADC12B_CHER bit for eAdcChannel_ is set
  - ADC12B_IER bit for eAdcChannel_is set
  - Returns TRUE

If Adc12_bAdcAvailable is TRUE:
  - Returns FALSE
*/
bool Adc12StartConversion(Adc12ChannelType eAdcChannel_)
{
  if(Adc12_bAdcAvailable)
  {
    /* Take the semaphore so we have the ADC resource.  Since this is a binary semaphore 
    that is only cleared in the ISR, it is safe to do this with interrupts enabled */
    Adc12_bAdcAvailable = FALSE;
   
    /* Enable the channel and its interrupt */
    AT91C_BASE_ADC12B->ADC12B_CHER = (1 << eAdcChannel_);
    AT91C_BASE_ADC12B->ADC12B_IER  = (1 << eAdcChannel_);
  
    /* Start the conversion and exit */
    AT91C_BASE_ADC12B->ADC12B_CR |= AT91C_ADC12B_CR_START;
    return TRUE;
  }
  
  /* The ADC is not available */
  return FALSE;

} /* end Adc12StartConversion() */


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
  u8 au8Adc12Started[] = "ADC12 task initialized\n\r";

  /* Initialize peripheral registers. ADC starts totally disabled. */
  AT91C_BASE_ADC12B->ADC12B_MR   = ADC12B_MR_INIT;
  AT91C_BASE_ADC12B->ADC12B_CHDR = ADC12B_CHDR_INIT;
  AT91C_BASE_ADC12B->ADC12B_ACR  = ADC12B_ACR_INIT;
  AT91C_BASE_ADC12B->ADC12B_EMR  = ADC12B_EMR_INIT;
  AT91C_BASE_ADC12B->ADC12B_IDR  = ADC12B_IDR_INIT;
  
  /* Set all the callbacks to default and set the ADC available */
  Adc12_fpCallbackCh1 = Adc12DefaultCallback;
  Adc12_fpCallbackCh2 = Adc12DefaultCallback;
  Adc12_fpCallbackCh3 = Adc12DefaultCallback;
  Adc12_bAdcAvailable = TRUE;
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    /* Enable required interrupts */
    NVIC_ClearPendingIRQ(IRQn_ADCC0);
    NVIC_EnableIRQ(IRQn_ADCC0);
    DebugPrintf(au8Adc12Started);
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

/*----------------------------------------------------------------------------------------------------------------------
Function Adc12DefaultCallback()

Description:
An empty function that the unset Adc Callbacks point to.  Expected that the 
user will set their own.

Requires:
  - 

Promises:
  - 
*/
void Adc12DefaultCallback(u16 u16Result_)
{
} /* End Adc12DefaultCallback() */


/*----------------------------------------------------------------------------------------------------------------------
ISR: ADCC0_IrqHandler

Description:
Parses the ADC12 interrupts and handles them appropriately.  Note that all ADC12
interrupts are ORed and will trigger this handler, therefore any expected interrupt 
that is enabled must be parsed out and handled.  There is no obviously available
explanation for why this handler is called ADCC0_IrqHandler instead of ADC12B_IrqHandler

Requires:
  - 

Promises:
  - 
*/
void ADCC0_IrqHandler(void)
{
  u16 u16Adc12Result;
  /* WARNING: if you step through this handler with the ADC12B registers
  debugging, the debugger reads ADC12B_SR and clears the EOC flag bits */
    
  /* Check for CH1  */
  if(AT91C_BASE_ADC12B->ADC12B_SR & (1 << ADC12_CH1))
  {
    /* Read the channel's result register (clears EOC bit / interrupt) and send to callback */
    u16Adc12Result = AT91C_BASE_ADC12B->ADC12B_CDR[ADC12_CH1];
    Adc12_fpCallbackCh1(u16Adc12Result);
    
    /* Disable the channel */
    AT91C_BASE_ADC12B->ADC12B_CHDR = (1 << ADC12_CH1);
  }

  /* Give the Semaphore back, clear the ADC pending flag and exit */
  Adc12_bAdcAvailable = TRUE;
  NVIC->ICPR[0] = (1 << AT91C_ID_ADC12B);
  
} /* end ADCC0_IrqHandler() */


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
