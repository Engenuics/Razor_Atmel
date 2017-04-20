/**********************************************************************************************************************
File: timer.c                                                                

Description:
Provide easy access to setting up and running the Timer Counter (TC) Peripherals.

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:
void TimerSet(TimerChannelType eTimerChannel_, u16 u16TimerValue_)
e.g. 
TimerSet(TIMER_CHANNEL1, 0xFFFF);

void TimerStart(TimerChannelType eTimerChannel_)
e.g.
TimeStart(TIMER_CHANNEL1);

void TimerStop(TimerChannelType eTimerChannel_)
e.g.
TimerStop(TIMER_CHANNEL1);

u16 TimerGetTime(TimerChannelType eTimerChannel_)
e.g.
u16 u16Timer1CurrentValue;
u16Timer1CurrentValue = TimerGetTime(TIMER_CHANNEL1);

void TimerAssignCallback(TimerChannelType eTimerChannel_, fnCode_type fpUserCallback_)
e.g.
TimerAssignCallback(TIMER_CHANNEL1, UserAppCallback);
where UserAppCallBack is defined in a user task:
void UserAppCallBack(void)
Note: any callback should execute as quickly as possible since it runs during
the timer interrupt handler.


Protected System functions:
void TimerInitialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void TimerRunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32TimerFlags;                          /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Timer_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Timer_StateMachine;            /* The state machine function pointer */
static fnCode_type fpTimer1Callback;              /* Timer1 ISR callback function pointer */

//static u32 Timer_u32Timeout;                      /* Timeout counter used across states */

static u32 Timer_u32Timer1Counter = 0;            /* Track instances of The TC1 interrupt handler */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: TimerSet

Description
Sets the timer tick period (interrupt rate).

Requires:
  - eTimerChannel_ holds a valid channel
  - u16TimerValue_ x in ticks

Promises:
  - Updates TC_RC value with u16TimerValue_
*/
void TimerSet(TimerChannelType eTimerChannel_, u16 u16TimerValue_)
{
  /* Build the offset to the selected peripheral */
  u32 u32TimerBaseAddress = (u32)AT91C_BASE_TC0;
  u32TimerBaseAddress += (u32)eTimerChannel_;

  /* Load the new timer value */
  (AT91_CAST(AT91PS_TC)u32TimerBaseAddress)->TC_RC = (u32)(u16TimerValue_ & 0x0000FFFF);

} /* end TimerSet() */


/*----------------------------------------------------------------------------------------------------------------------
Function: TimerStart

Description
Starts the designated Timer.

Requires:
  - eTimerChannel_ is the timer to start

Promises:
  - Specified channel on Timer is set to run; if already running it remains running
  - Does NOT reset the timer value
*/
void TimerStart(TimerChannelType eTimerChannel_)
{
  /* Build the offset to the selected peripheral */
  u32 u32TimerBaseAddress = (u32)AT91C_BASE_TC0;
  u32TimerBaseAddress += (u32)eTimerChannel_;

  /* Ensure clock is enabled and triggered */
  (AT91_CAST(AT91PS_TC)u32TimerBaseAddress)->TC_CCR |= (AT91C_TC_CLKEN | AT91C_TC_SWTRG);
  
} /* end TimerStart() */


/*----------------------------------------------------------------------------------------------------------------------
Function: TimerStop

Description
Stops the designated Timer.

Requires:
  - eTimerChannel_ is the timer to stop

Promises:
  - Specified timer is stopped; if already stopped it remains stopped
  - Does NOT reset the timer value
*/
void TimerStop(TimerChannelType eTimerChannel_)
{
  /* Build the offset to the selected peripheral */
  u32 u32TimerBaseAddress = (u32)AT91C_BASE_TC0;
  u32TimerBaseAddress += (u32)eTimerChannel_;
  
  /* Ensure clock is disabled */
  (AT91_CAST(AT91PS_TC)u32TimerBaseAddress)->TC_CCR |= AT91C_TC_CLKDIS;
  
} /* end TimerStop */


/*----------------------------------------------------------------------------------------------------------------------
Function: TimerGetTime

Description
Returns the current count.

Requires:
  - eTimerChannel_ is the timer to query

Promises:
  - Current 16 bit timer value is returned
*/
u16 TimerGetTime(TimerChannelType eTimerChannel_)
{
 
  /* Build the offset to the selected peripheral */
  u32 u32TimerBaseAddress = (u32)AT91C_BASE_TC0;
  u32TimerBaseAddress += (u32)eTimerChannel_;
  
  /* Read and format the timer count */
  return ((u16)( (AT91_CAST(AT91PS_TC)u32TimerBaseAddress)->TC_CV & 0x0000FFFF));
  
} /* end TimerGetTime */


/*----------------------------------------------------------------------------------------------------------------------
Function: TimerAssignCallback

Description
Allows user to specify a custom callback function for when the Timer interrupt occurs.

Requires:
  - eTimerChannel_ is the channel to which the callback will be assigned
  - fpUserCallback_ is the function address (name) for the user's callback

Promises:
  - fpTimerxCallback loaded with fpUserCallback_
*/
void TimerAssignCallback(TimerChannelType eTimerChannel_, fnCode_type fpUserCallback_)
{
  switch(eTimerChannel_)
  {
    case TIMER_CHANNEL0:
    {
      break;
    }
    case TIMER_CHANNEL1:
    {
      fpTimer1Callback = fpUserCallback_;
      break;
    }
    case TIMER_CHANNEL2:
    {
      break;
    }
    default:
    {
      DebugPrintf("Invalid channel\n\r");
    }
  } /* end switch(eTimerChannel_) */
  
} /* end TimerAssignCallback */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: TimerInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - Timer 1 is configured per timer.h INIT settings
*/
void TimerInitialize(void)
{
  u8 au8TimerStarted[] = "Timer1 initialized\n\r";

  /* Channel 0 settings not configured at this time */

  /* Load the block configuration register */
  AT91C_BASE_TCB1->TCB_BMR = TCB_BMR_INIT;
 
  /* Load Channel 1 settings and set the default callback */
  AT91C_BASE_TC1->TC_CMR = TC1_CMR_INIT;
  AT91C_BASE_TC1->TC_RC  = TC1_RC_INIT;
  AT91C_BASE_TC1->TC_IER = TC1_IER_INIT;
  AT91C_BASE_TC1->TC_IDR = TC1_IDR_INIT;
  AT91C_BASE_TC1->TC_CCR = TC1_CCR_INIT;
 
  fpTimer1Callback = TimerDefaultCallback;

  /* Channel 2 settings not configured at this time */
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    /* Enable required interrupts */
    NVIC_ClearPendingIRQ(IRQn_TC1);
    NVIC_EnableIRQ(IRQn_TC1);
    Timer_StateMachine = TimerSM_Idle;
    DebugPrintf(au8TimerStarted);
    
    /* Flag that the Timer task is ready */
    G_u32ApplicationFlags |= _APPLICATION_FLAGS_TIMER;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    Timer_StateMachine = TimerSM_FailedInit;
  }

} /* end TimerInitialize() */

  
/*----------------------------------------------------------------------------------------------------------------------
Function TimerRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void TimerRunActiveState(void)
{
  Timer_StateMachine();

} /* end TimerRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
  
/*----------------------------------------------------------------------------------------------------------------------
Function TimerDefaultCallback()

Description:
An empty function that the Timer Callback points to.  Expected that the 
user will set their own.

Requires:
  - 

Promises:
  - 
*/
inline void TimerDefaultCallback(void)
{
} /* End TimerDefaultCallback() */


/*----------------------------------------------------------------------------------------------------------------------
ISR: TC1_IrqHandler

Description:
Parses the TC1 interrupts and handles them appropriately.  Note that all TC1
interrupts are ORed and will trigger this handler, therefore any expected interrupt 
that is enabled must be parsed out and handled.

Requires:
  - 

Promises:
  - If Channel1 RC: Timer Channel 1 is reset
*/
void TC1_IrqHandler(void)
{
  /* Check for RC compare interrupt - reading TC_SR clears the bit if set */
  if(AT91C_BASE_TC1->TC_SR & AT91C_TC_CPCS)
  {
    Timer_u32Timer1Counter++;
    fpTimer1Callback();
  }

  /* Clear the TC0 pending flag and exit */
  NVIC->ICPR[0] = (1 << IRQn_TC1);
  
} /* end TC1_IrqHandler() */


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void TimerSM_Idle(void)
{
   
} /* end TimerSM_Idle() */
     

#if 0
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void TimerSM_Error(void)          
{
  
} /* end TimerSM_Error() */
#endif

/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void TimerSM_FailedInit(void)          
{
    
} /* end TimerSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
