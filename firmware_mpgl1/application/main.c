/***********************************************************************************************************************
File: main.c                                                                

Description:
Container for the MPG firmware.  
***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32SystemFlags = 0;                     /* Global system flags */
volatile u32 G_u32ApplicationFlags = 0;                /* Global applications flags: set when application is successfully initialized */

/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Main_" and be declared as static.
***********************************************************************************************************************/
static volatile u32 Main_u32Num = 0;
static volatile u8  Main_u8NumDone = 0;
static volatile bool Main_bKeepGoing = TRUE;


/***********************************************************************************************************************
Main Program
Main has two sections:

1. Initialization which is run once on power-up or reset.  All drivers and applications are setup here without timing
contraints but must complete execution regardless of success or failure of starting the application. 

2. Super loop which runs infinitely giving processor time to each application.  The total loop time should not exceed
1ms of execution time counting all application execution.  SystemSleep() will execute to complete the remaining time in
the 1ms period.
***********************************************************************************************************************/

void main(void)
{
  G_u32SystemFlags |= _SYSTEM_INITIALIZING;

  /* Low level initialization */
  WatchDogSetup(); /* During development, does not reset processor if timeout */
  GpioSetup();
  ClockSetup();
  InterruptSetup();
  SysTickSetup();

  /* Driver initialization */
  MessagingInitialize();
  UartInitialize();
  DebugInitialize();

  /* Debug messages through DebugPrintf() are available from here */

  TimerInitialize();  
  SspInitialize();
  TWIInitialize();

  LcdInitialize();
  LedInitialize();
  ButtonInitialize();
  AntInitialize();
  SdCardInitialize();

  /* Application initialization */
    UserAppInitialize();
  
  /* Exit initialization */
  SystemStatusReport();
  G_u32SystemFlags &= ~_SYSTEM_INITIALIZING;
  
  TimerStart(TIMER_CHANNEL1);
  
  /* Super loop */  
  while(1)
  {
    WATCHDOG_BONE();
    
    /* Drivers */
    LedUpdate();
    ButtonRunActiveState();
    UartRunActiveState();
    TimerRunActiveState(); 
    SspRunActiveState();
    TWIRunActiveState();
    MessagingRunActiveState();
    DebugRunActiveState();
    LcdRunActiveState();
    AntRunActiveState();
    SdCardRunActiveState();

    /* Applications */
    UserAppRunActiveState();
    
    /* System sleep*/
    HEARTBEAT_OFF();
    SystemSleep();
    HEARTBEAT_ON();
    
  } /* end while(1) main super loop */
  
} /* end main() */


/*--------------------------------------------------------------------------------------------------------------------
Function: TimerCallBack

Description:
Timer1 ISR-accessed function to increment the counter.
  
Requires:
  - No pre-reqs allowed in a call back!

Promises:
  - 
*/
void Timer1CallBack(void)
{
  Main_u32Num++;
  
  if(Main_bKeepGoing) 
  {
    /* Show when a certain number of interrupts have occurred */
    if(Main_u32Num > 46875) 
    { 
      Main_u32Num = 0;
      DebugPrintf("46875\n\r");
      
      /* After 6 times, turn off the timer */
      Main_u8NumDone++;
      if(Main_u8NumDone == 6)  
      {
        TimerStop(TIMER_CHANNEL1);
        Main_bKeepGoing = FALSE;
        LedOff(BLUE);
      }
    }
  }
} /* end Timer1CallBack() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
