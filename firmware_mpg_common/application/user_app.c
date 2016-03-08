/**********************************************************************************************************************
File: user_app.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app as a template:
 1. Copy both user_app.c and user_app.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserAppInitialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserAppRunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserAppFlags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp_StateMachine;            /* The state machine function pointer */
static u32 UserApp_u32Timeout;                      /* Timeout counter used across states */


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
Function: UserAppInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserAppInitialize(void)
{
  /* Start with red LED on 100%, green and blue off */
#ifdef MPG1
  LedPWM(LCD_RED, LED_PWM_100);
  LedPWM(LCD_GREEN, LED_PWM_0);
  LedPWM(LCD_BLUE, LED_PWM_0);
#endif /* MPG 1 */
  
#ifdef MPG2
  LedPWM(RED0,   LED_PWM_100);
  LedPWM(GREEN0, LED_PWM_0);
  LedPWM(BLUE0,  LED_PWM_0);

  LedPWM(RED1,   LED_PWM_100);
  LedPWM(GREEN1, LED_PWM_0);
  LedPWM(BLUE1,  LED_PWM_0);

  LedPWM(RED2,   LED_PWM_100);
  LedPWM(GREEN2, LED_PWM_0);
  LedPWM(BLUE2,  LED_PWM_0);

  LedPWM(RED3,   LED_PWM_100);
  LedPWM(GREEN3, LED_PWM_0);
  LedPWM(BLUE3,  LED_PWM_0);
#endif /* MPG2 */
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp_StateMachine = UserAppSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp_StateMachine = UserAppSM_FailedInit;
  }

} /* end UserAppInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserAppRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserAppRunActiveState(void)
{
  UserApp_StateMachine();

} /* end UserAppRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void UserAppSM_Idle(void)
{
#ifdef MPG1
  static LedNumberType aeCurrentLed[]  = {LCD_GREEN, LCD_RED, LCD_BLUE, LCD_GREEN, LCD_RED, LCD_BLUE};
#endif /* MPG 1 */

#ifdef MPG2
  static LedNumberType aeCurrentLed[]  = {GREEN0, RED0, BLUE0, GREEN0, RED0, BLUE0};
  static LedNumberType aeCurrentLed1[] = {GREEN1, RED1, BLUE1, GREEN1, RED1, BLUE1};
  static LedNumberType aeCurrentLed2[] = {GREEN2, RED2, BLUE2, GREEN2, RED2, BLUE2};
  static LedNumberType aeCurrentLed3[] = {GREEN3, RED3, BLUE3, GREEN3, RED3, BLUE3};
#endif /* MPG2 */
  
  static bool abLedRateIncreasing[]    = {TRUE,      FALSE,   TRUE,     FALSE,     TRUE,    FALSE};
  static u8 u8CurrentLedIndex  = 0;
  static u8 u8LedCurrentLevel  = 0;
  static u8 u8DutyCycleCounter = 0;
  static u16 u16Counter = COLOR_CYCLE_TIME;
  static bool bCyclingOn = TRUE;

  /* Advance the cycle only if bCyclingOn */
  if(bCyclingOn)
  {
    u16Counter--;
  }

  /* Check for update color every COLOR_CYCLE_TIME ms */  
  if(u16Counter == 0)
  {
		u16Counter = COLOR_CYCLE_TIME;
    
    /* Update the current level based on which way it's headed */
    if( abLedRateIncreasing[u8CurrentLedIndex] )
    {
      u8LedCurrentLevel++;
    }
    else
    {
      u8LedCurrentLevel--;
    }

    /* Change direction once we're at the end */
    if(u8DutyCycleCounter++ == 20)
    {
      u8DutyCycleCounter = 0;
      
      /* Watch for the indexing variable to reset */
      u8CurrentLedIndex++;
      if(u8CurrentLedIndex == sizeof(aeCurrentLed))
      {
        u8CurrentLedIndex = 0;
      }
      
      /* Set the current level based on what direction we're now going */
      u8LedCurrentLevel = 20;
      if(abLedRateIncreasing[u8CurrentLedIndex])
      {
         u8LedCurrentLevel = 0;
      }
    } 

    /* Update the value to the current LED */
    LedPWM( (LedNumberType)aeCurrentLed[u8CurrentLedIndex], (LedRateType)u8LedCurrentLevel );
#ifdef MPG2
    LedPWM( (LedNumberType)aeCurrentLed1[u8CurrentLedIndex], (LedRateType)u8LedCurrentLevel );
    LedPWM( (LedNumberType)aeCurrentLed2[u8CurrentLedIndex], (LedRateType)u8LedCurrentLevel );
    LedPWM( (LedNumberType)aeCurrentLed3[u8CurrentLedIndex], (LedRateType)u8LedCurrentLevel );
#endif /* MPG2 */
  } 

  /* Toggle cycling on and off */
  if( WasButtonPressed(BUTTON0) )
  {
    ButtonAcknowledge(BUTTON0);
    bCyclingOn = (bool)!bCyclingOn;
  }
  
} /* end UserAppSM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_Error(void)          
{
  
} /* end UserAppSM_Error() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserAppSM_FailedInit(void)          
{
    
} /* end UserAppSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
