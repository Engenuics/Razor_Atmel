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
volatile u32 G_u32ApplicationFlags = 0;                /* Global applications flags: set when application is successfully initialized. Bit defs in configuration.h */


/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Main_" and be declared as static.
***********************************************************************************************************************/


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
  WatchDogSetup(); /* During development, set to not reset processor if timeout */
  ClockSetup();
  GpioSetup();
  InterruptSetup();
  SysTickSetup();

  /* Driver initialization */
  MessagingInitialize();
  UartInitialize();
  DebugInitialize();

  /* Debug messages through DebugPrintf() are available from here */

  SspInitialize();
  TWIInitialize();
  
  LcdInitialize();
  LedInitialize();
  ButtonInitialize();
   
  CapTouchInitialize();
  AntInitialize();
  
  /* Application initialization */
  BoardTestInitialize();
  UserAppInitialize();
  
  /* Exit initialization */
  SystemStatusReport();
  G_u32SystemFlags &= ~_SYSTEM_INITIALIZING;
  
#if 0  /* LED Color Testing */
  LedPWM(BLUE0, LED_PWM_100);
  LedPWM(RED0, LED_PWM_100);
  LedPWM(GREEN0, LED_PWM_100);
  
  LedPWM(BLUE1, LED_PWM_100);
  LedPWM(RED1, LED_PWM_100);
  LedPWM(GREEN1, LED_PWM_100);

  LedPWM(BLUE2, LED_PWM_100);
  LedPWM(RED2, LED_PWM_100);
  LedPWM(GREEN2, LED_PWM_100);

  LedPWM(BLUE3, LED_PWM_80);
  LedPWM(RED3, LED_PWM_100);
  LedPWM(GREEN3, LED_PWM_100);
#endif

#if 0  /* LED Color Testing / screen display hold */
  LedOff(BLUE0);
  LedOff(RED0);
  LedOn(GREEN0);
  
  LedOn(BLUE1);
  LedOn(RED1);
  LedOff(GREEN1);

  LedOff(BLUE2);
  LedOn(RED2);
  LedOff(GREEN2);

  LedOn(BLUE3);
  LedOff(RED3);
  LedOff(GREEN3);
  //LedUpdate();

  while(1);
#endif
  
  /* Super loop */  
  while(1)
  {
    WATCHDOG_BONE();
    
    /* Drivers */
    LedUpdate();
    ButtonRunActiveState();
    UartRunActiveState();
    SspRunActiveState();
    TWIRunActiveState();
    CapTouchRunActiveState(); /* This function violates 1ms loop timing every 25ms */ 
    MessagingRunActiveState();
    DebugRunActiveState();
    LcdRunActiveState();
    AntRunActiveState();

    /* Applications */
    BoardTestRunActiveState();
    UserAppRunActiveState();
        
    /* System sleep*/
    HEARTBEAT_OFF();
    SystemSleep();
    HEARTBEAT_ON();
    
  } /* end while(1) main super loop */
  
} /* end main() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
