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
#ifdef MPG1
  LedOff(WHITE);
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
#endif /* MPG 1 */
  
#ifdef MPG2
  LedOff(RED0);
  LedOff(BLUE0);
  LedOff(GREEN0);

  LedOff(RED1);
  LedOff(BLUE1);
  LedOff(GREEN1);

  LedOff(RED2);
  LedOff(BLUE2);
  LedOff(GREEN2);

  LedOff(RED3);
  LedOff(BLUE3);
  LedOff(GREEN3);
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
  static LedRateType aeBlinkRate[] = {LED_1HZ, LED_2HZ, LED_4HZ, LED_8HZ};
  static u8 u8BlinkRateIndex = 0;
  static bool bLedBlink = FALSE;
  
#ifdef MPG1
  /* BUTTON0 functionality */
  if( IsButtonPressed(BUTTON0) )
  {
    /* The button is currently pressed, so make sure the LED is on */
    LedOn(WHITE);
  }
  else
  {
    /* The button is not pressed, so make sure the LED is off */
    LedOff(WHITE);
  }
  
  if( IsButtonHeld(BUTTON0, BUTTON0_HOLD_TIME) )
  {
    LedOn(GREEN);
  }
  else
  {
    LedOff(GREEN);
  }

  /* BUTTON1 functionality */
  if( IsButtonPressed(BUTTON1) )
  {
    LedOn(PURPLE);
  }
  else
  {
    LedOff(PURPLE);
  }

  if( WasButtonPressed(BUTTON1) )
  {
    /* Be sure to acknowledge the button press */
    ButtonAcknowledge(BUTTON1);

    /* If the LED is already blinking, toggle it off */
    if(bLedBlink)
    {
      bLedBlink = FALSE;
      LedOff(YELLOW);
    }
    /* else start blinking the LED at the current rate */
    else
    {
      bLedBlink = TRUE;
      LedBlink(YELLOW, aeBlinkRate[u8BlinkRateIndex]);
    }
  }
 
  /* BUTTON2 functionality */
  if( IsButtonPressed(BUTTON2) )
  {
    LedOn(BLUE);
  }
  else
  {
    LedOff(BLUE);
  }

  /* Check to see if we need to update the blink rate */
  if( WasButtonPressed(BUTTON2) )
  {
    /* Be sure to acknowledge the button press */
    ButtonAcknowledge(BUTTON2);

    /* Update the blink rate and handle overflow only if the LED is currently blinking */
    if(bLedBlink)
    {
      u8BlinkRateIndex++;
      if(u8BlinkRateIndex == 4)
      {
        u8BlinkRateIndex = 0;
      }
      
      /* Request the rate udpate */
      LedBlink(YELLOW, aeBlinkRate[u8BlinkRateIndex]);
    }
  }

  /* BUTTON3 functionality */
  if( IsButtonHeld(BUTTON3, 2000) )
  {
    LedOn(CYAN);
  }
  else
  {
    LedOff(CYAN);
  }
#endif /* MPG1 */

#ifdef MPG2
  if( IsButtonPressed(BUTTON0) )
  {
    /* The button is currently pressed, so make sure the LED is on */
    LedOn(BLUE0 );
  }
  else
  {
    /* The button is not pressed, so make sure the LED is off */
    LedOff(BLUE0 );
  }

  /* Check to see if we need to update the blink rate */
  if( WasButtonPressed(BUTTON0) )
  {
    /* Be sure to acknowledge the button press */
    ButtonAcknowledge(BUTTON0);

    /* Update the blink rate and handle overflow only if the LED is currently blinking */
    if(bLedBlink)
    {
      u8BlinkRateIndex++;
      if(u8BlinkRateIndex == 4)
      {
        u8BlinkRateIndex = 0;
      }
      
      /* Request the rate udpate */
      LedBlink(GREEN3, aeBlinkRate[u8BlinkRateIndex]);
    }
  }
  
  if( IsButtonPressed(BUTTON1) )
  {
    /* The button is currently pressed, so make sure the LED is on */
    LedOn(RED1);
  }
  else
  {
    /* The button is not pressed, so make sure the LED is off */
    LedOff(RED1);
  }

  /* Toggle blinking on/off of the GREEN3 LED when BUTTON1 has been pressed.  
  ButtonAcknowledge must be called after WasButtonPressed() */
  if( WasButtonPressed(BUTTON1) )
  {
    /* Be sure to acknowledge the button press */
    ButtonAcknowledge(BUTTON1);

    /* If the LED is already blinking, toggle it off */
    if(bLedBlink)
    {
      bLedBlink = FALSE;
      LedOff(GREEN3);
    }
    else
    {
     /* start blinking the LED at the current rate */
      bLedBlink = TRUE;
      LedBlink(GREEN3, aeBlinkRate[u8BlinkRateIndex]);
    }
  }

  /* Turn on the RED2 and GREEN2 LEDs after BUTTON1 has been held for 2 seconds */
  if( IsButtonHeld(BUTTON1, 2000) )
  {
    LedOn(RED2);
    LedOn(GREEN2);
  }
  else
  {
    LedOff(RED2);
    LedOff(GREEN2);
  }
#endif /* MPG2 */  

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
