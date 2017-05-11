/*!**********************************************************************************************************************
@file main.c                                                                
@brief Main system file for the EiE firmware.  
***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32SystemFlags = 0;                     /*!< Global system flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Main_" and be declared as static.
***********************************************************************************************************************/


/*!**********************************************************************************************************************
@fn void main(void)
@brief Main program where all tasks are initialized and executed.

Main has two sections:

1. Initialization which is run once on power-up or reset.  All drivers and 
applications are setup here without timing contraints but must complete 
execution regardless of success or failure of starting the application. 

2. Super loop which runs infinitely giving processor time to each application.  
The total loop time should not exceed 1ms of execution time counting all 
application execution.  SystemSleep() will execute to complete the remaining 
time in the 1ms period.

***********************************************************************************************************************/

void main(void)
{
  G_u32SystemFlags |= _SYSTEM_INITIALIZING;

  /* Low level initialization */
  WatchDogSetup(); /* During development, does not reset processor if timeout */
  GpioSetup();
  ClockSetup();
  
  /* Application Initialization */
  UserApp1Initialize();
  
  /* Exit initialization */
  G_u32SystemFlags &= ~_SYSTEM_INITIALIZING;
    
  /* Super loop */  
  while(1)
  {
    WATCHDOG_BONE();
    UserApp1RunActiveState();

    /* System sleep */
    do
    {
      SystemSleep();
    } while(G_u32SystemFlags & _SYSTEM_SLEEPING);
   
  } /* end while(1) main super loop */
  
} /* end main() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
