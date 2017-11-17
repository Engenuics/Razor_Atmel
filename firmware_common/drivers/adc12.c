/*!**********************************************************************************************************************
@file adc12.c 
@brief 12-bit ADC driver and API

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
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- Adc12ChannelType {ADC12_CH0 ... ADC12_CH7}

PUBLIC FUNCTIONS
- void Adc12AssignCallback(Adc12ChannelType eAdcChannel_, fnCode_u16_type pfUserCallback_)
- bool Adc12StartConversion(Adc12ChannelType eAdcChannel_)

PROTECTED FUNCTIONS
- void Adc12Initialize(void)
- void Adc12RunActiveState(void)
- void ADCC0_IrqHandler(void)

**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxAdc12"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32Adc12Flags;                          /*!< @brief Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                 /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                  /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;             /*!< @brief From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Adc12_xx" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Adc12_pfnStateMachine;              /*!< @brief The state machine function pointer */
//static u32 Adc12_u32Timeout;                         /*!< @brief Timeout counter used across states */

static bool Adc12_bAdcAvailable;                       /*!< @brief Binary semaphore to control access to the ADC12 peripheral */

static Adc12ChannelType Adc12_aeChannels[] = ADC_CHANNEL_ARRAY;  /*!< @brief Available channels defined in configuration.h */
static fnCode_u16_type Adc12_apfCallbacks[8];          /*!< @brief ADC12 ISR callback function pointers */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!---------------------------------------------------------------------------------------------------------------------
@fn void Adc12AssignCallback(Adc12ChannelType eAdcChannel_, fnCode_u16_type pfUserCallback_)

@brief Assigns callback for the client application.  

This is how the ADC result for any channel is accessed.  The callback function 
must have one u16 parameter where the result is passed.  Define the function that 
will be used for the callback, then assign this during user task initialization.

Different callbacks may be assigned for each channel. 

*** To mitigate the chance of indefinitely holding control of
the ADC resource, new conversions shall not be started in this callback. ***

Example:

void UserApp_AdcCallback(u16 u16Result_);

void UserApp1Initialize(void)
{
 Adc12AssignCallback(ADC12_BLADE_AN0, UserApp_AdcCallback);
}


Requires:
@param eAdcChannel_ is the channel to which the callback will be assigned
@param pfUserCallback_ is the function address (name) for the user's callback

Promises:
- Adc12_fpCallbackCh<eAdcChannel_> ADC global value loaded with pfUserCallback_

*/
void Adc12AssignCallback(Adc12ChannelType eAdcChannel_, fnCode_u16_type pfUserCallback_)
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
    Adc12_apfCallbacks[eAdcChannel_] = pfUserCallback_;
  }
  else
  {
    DebugPrintf("Invalid channel\n\r");
  }
  
} /* end Adc12AssignCallback() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn bool Adc12StartConversion(Adc12ChannelType eAdcChannel_)

@brief Checks if the ADC is available and starts the conversion on the selected channel.

Returns TRUE if the conversion is started; returns FALSE if the ADC is not available.

Example:

bool bConversionStarted = FALSE;

bConversionStarted = Adc12StartConversion(ADC12_CH2);


Requires:
@param eAdcChannel_ the ADC12 channel to disable
@param Adc12_bAdcAvailable indicates if the ADC is available for a conversion

Promises:

If Adc12_bAdcAvailable is TRUE:
- Adc12_bAdcAvailable changed to FALSE
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
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void Adc12Initialize(void)

@brief Runs required initialization for the task.  

Should only be called once in main init section.


Requires:
- NONE

Promises:
- The ADC-12 peripheral is configured
- ADC interrupt is enabled

@param Adc12_pfnStateMachine set to Idle

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
  for(u8 i = 0; i < (sizeof(Adc12_apfCallbacks) / sizeof(fnCode_u16_type)); i++)
  {
    Adc12_apfCallbacks[i] = Adc12DefaultCallback;
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
    Adc12_pfnStateMachine = Adc12SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    Adc12_pfnStateMachine = Adc12SM_Error;
  }

} /* end Adc12Initialize() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void Adc12RunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void Adc12RunActiveState(void)
{
  Adc12_pfnStateMachine();

} /* end Adc12RunActiveState */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void ADCC0_IrqHandler(void)

@brief Parses the ADC12 interrupts and handles them appropriately. 

Note that all ADC12 interrupts are ORed and will trigger this handler, therefore 
any expected interrupt that is enabled must be parsed out and handled.  There is 
no obviously available explanation for why this handler is called ADCC0_IrqHandler 
instead of ADC12B_IrqHandler

Requires:
- Only one channel can be converting at a time, so only one interrupt flag
  will be set.

Promises:
- Adc12_bAdcAvailable = TRUE

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
      Adc12_apfCallbacks[Adc12_aeChannels[i]](u16Adc12Result);
      
      /* Disable the channel and exit the loop since only one channel can be set */
      AT91C_BASE_ADC12B->ADC12B_CHDR = (1 << Adc12_aeChannels[i]);
      break;
    }
  }
  
  /* Give the Semaphore back, clear the ADC pending flag and exit */
  Adc12_bAdcAvailable = TRUE;
  NVIC->ICPR[0] = (1 << AT91C_ID_ADC12B);
  
} /* end ADCC0_IrqHandler() */


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn Adc12DefaultCallback()

@brief An empty function that the unset Adc Callbacks point to.  

Expected that the user will set their own.

Requires:
@param u16Result_ Required for the ISR to pass the result to but not used

Promises:
- NONE
  
*/
void Adc12DefaultCallback(u16 u16Result_)
{
  /* This is an empty function */
  
} /* end Adc12DefaultCallback() */


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void Adc12SM_Idle(void)

@brief Wait for a message to be queued 
*/
static void Adc12SM_Idle(void)
{
    
} /* end Adc12SM_Idle() */
     

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void Adc12SM_Error(void)          

@brief Handle an error 
*/
static void Adc12SM_Error(void)          
{
  
} /* end Adc12SM_Error() */





/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
