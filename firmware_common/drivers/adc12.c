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

The first sample tends to read 20-30 bits high.  If no sample is taken for a few minutes,
the next first sample will also read high.  This implies a long time constant in the hold
time, but the timing parameters that have been set all line up with the electrical
characteristics and source impedence considerations.  So this is a mystery for
now -- suggest the first sample is thrown out, or average it out with at least 16 samples
per displayed result which will reduce the error down to 1 or 2 LSBs.  

------------------------------------------------------------------------------------------------------------------------
API:

TYPES
Adc12ChannelType {ADC12_CH0...ADC12_CH7}

PUBLIC FUNCTIONS
void Adc12AssignCallback(Adc12ChannelType eAdcChannel_, fnCode_u16_type fpUserCallback_)
Assigns callback for the client application.  This is how the ADC result for any channel
is accessed.  The callback function must have one u16 parameter where the result is passed.
Different callbacks may be assigned for each channel. 

e.g. to read AN0 from the blade connector (which is channel 2 on the ADC):
void UserApp_AdcCallback(u16 u16Result_);
...
Adc12AssignCallback(ADC12_CH2, UserApp_AdcCallback);


bool Adc12StartConversion(Adc12ChannelType eAdcChannel_)
Checks if the ADC is available and starts the conversion on the selected channel.
Returns TRUE if the conversion is started; returns FALSE if the ADC is not available.
e.g.
bool bConversionStarted = FALSE;
bConversionStarted = Adc12StartConversion(ADC12_CH2);


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
  bool bChannelValid = FALSE;

  /* Check to ensure the requested channel exists */
  for(u8 i = 0; i < (sizeof(Adc12_aeChannels) / sizeof (Adc12ChannelType)); i++)
  {
    if(Adc12_aeChannels[i] == eAdcChannel_)
    {
      bChannelValid = TRUE;
    }
  }
  
  /* If the channel is valid, then assign the new callback function */
  if(bChannelValid)
  {
    Adc12_afCallbacks[eAdcChannel_] = fpUserCallback_;
  }
  else
  {
    DebugPrintf("Invalid channel\n\r");
  }
  
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

If Adc12_bAdcAvailable is FALSE:
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
  
  /* Set all the callbacks to default */
  for(u8 i = 0; i < (sizeof(Adc12_afCallbacks) / sizeof(fnCode_u16_type)); i++)
  {
    Adc12_afCallbacks[i] = Adc12DefaultCallback;
  }
  
  /* Mark the ADC semaphore as available */
  Adc12_bAdcAvailable = TRUE;
  
  /* Check initialization and set first state */
  if( 1 )
  {
    /* Enable required interrupts */
    NVIC_ClearPendingIRQ(IRQn_ADCC0);
    NVIC_EnableIRQ(IRQn_ADCC0);
    
    /* Write message, set "good" flag and select Idle state */
    DebugPrintf(au8Adc12Started);
    G_u32ApplicationFlags |= _APPLICATION_FLAGS_ADC;
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
  - Only one channel can be converting at a time, so only one interrupt flag
    will be set.

Promises:
  - 
*/
void ADCC0_IrqHandler(void)
{
  u16 u16Adc12Result;
  /* WARNING: if you step through this handler with the ADC12B registers
  debugging, the debugger reads ADC12B_SR and clears the EOC flag bits */
  

  /* Check through all the available channels */
  for(u8 i = 0; i < (sizeof(Adc12_aeChannels) / sizeof(Adc12ChannelType)); i++)
  {
    if(AT91C_BASE_ADC12B->ADC12B_SR & (1 << Adc12_aeChannels[i]))
    {
      /* Read the channel's result register (clears EOC bit / interrupt) and send to callback */
      u16Adc12Result = AT91C_BASE_ADC12B->ADC12B_CDR[Adc12_aeChannels[i]];
      Adc12_afCallbacks[Adc12_aeChannels[i]](u16Adc12Result);
      
      /* Disable the channel and exit the loop since only one channel can be set */
      AT91C_BASE_ADC12B->ADC12B_CHDR = (1 << Adc12_aeChannels[i]);
      break;
    }
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
