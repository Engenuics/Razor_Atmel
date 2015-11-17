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
#ifdef MPGL1
  /* All discrete LEDs to off */
  LedOff(WHITE);
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
  
  /* Backlight to white */  
  LedOn(LCD_RED);
  LedOn(LCD_GREEN);
  LedOn(LCD_BLUE);
#endif /* MPGL1 */

#ifdef MPGL2
  /* All discrete LEDs to off */
  LedOff(RED0);
  LedOff(RED1);
  LedOff(RED2);
  LedOff(RED3);
  LedOff(GREEN0);
  LedOff(GREEN1);
  LedOff(GREEN2);
  LedOff(GREEN3);
  LedOff(BLUE0);
  LedOff(BLUE1);
  LedOff(BLUE2);
  LedOff(BLUE3);
  
  /* Backlight to white */  
  LedOn(LCD_BL);
#endif /* MPGL2 */
  
  /* If good initialization, set state to Idle */
  if( 1 /* Add condition for good init */)
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
/* Update counter and display on LEDs. */
static void UserAppSM_Idle(void)
{
  static u16 u16BlinkCount = 0;
  static u8 u8Counter = 0;
  static u8 u8ColorIndex = 0;
  
#if MPGL2
  u16BlinkCount++;
  if(u16BlinkCount == 500)
  {
    u16BlinkCount = 0;
    
    /* Update the counter and roll at 16 */
    u8Counter++;
    if(u8Counter == 16)
    {
      u8Counter = 0;
      
      LedOff((LedNumberType)(RED3 + (4 * u8ColorIndex)));
      LedOff((LedNumberType)(RED2 + (LedNumberType)(4 * u8ColorIndex)));
      LedOff((LedNumberType)(RED1 + (LedNumberType)(4 * u8ColorIndex)));
      LedOff((LedNumberType)(RED0 + (LedNumberType)(4 * u8ColorIndex)));
      
      u8ColorIndex++;
      if(u8ColorIndex == 3)
      {
        u8ColorIndex = 0;
      }
    } /* end if(u8Counter == 16) */
    
    /* Parse the current count to set the LEDs.  From leds.h we see the enum for red, green and blue
    are seperated by 4 so use this with u8ColorIndex to */
    
    if(u8Counter & 0x01)
    {
      LedOn(RED3 + (4 * u8ColorIndex));
    }
    else
    {
      LedOff(RED3 + (4 * u8ColorIndex));
    }

    if(u8Counter & 0x02)
    {
      LedOn(RED2 + (4 * u8ColorIndex));
    }
    else
    {
      LedOff(RED2 + (4 * u8ColorIndex));
    }

    if(u8Counter & 0x04)
    {
      LedOn(RED1 + (4 * u8ColorIndex));
    }
    else
    {
      LedOff(RED1 + (4 * u8ColorIndex));
    }

    if(u8Counter & 0x08)
    {
      LedOn(RED0 + (4 * u8ColorIndex));
    }
    else
    {
      LedOff(RED0 + (4 * u8ColorIndex));
    }
    
  } /* end if(u16BlinkCount == 500) */
#endif /* MPGL2 */  

#if MPGL1
  u16BlinkCount++;
  if(u16BlinkCount == 500)
  {
    u16BlinkCount = 0;
    
    /* Update the counter and roll at 16 */
    u8Counter++;
    if(u8Counter == 16)
    {
      u8Counter = 0;
      
      /* Manage the backlight color */
      u8ColorIndex++;
      if(u8ColorIndex == 7)
      {
        u8ColorIndex = 0;
      }
      
      /* Set the backlight color: white (all), purple (blue + red), blue, cyan (blue + green),
      green, yellow (green + red), red */
      switch(u8ColorIndex)
      {
        case 0: /* white */
          LedOn(LCD_RED);
          LedOn(LCD_GREEN);
          LedOn(LCD_BLUE);
          break;

        case 1: /* purple */
          LedOn(LCD_RED);
          LedOff(LCD_GREEN);
          LedOn(LCD_BLUE);
          break;
          
        case 2: /* blue */
          LedOff(LCD_RED);
          LedOff(LCD_GREEN);
          LedOn(LCD_BLUE);
          break;
          
        case 3: /* cyan */
          LedOff(LCD_RED);
          LedOn(LCD_GREEN);
          LedOn(LCD_BLUE);
          break;
          
        case 4: /* green */
          LedOff(LCD_RED);
          LedOn(LCD_GREEN);
          LedOff(LCD_BLUE);
          break;
          
        case 5: /* yellow */
          LedOn(LCD_RED);
          LedOn(LCD_GREEN);
          LedOff(LCD_BLUE);
          break;
          
        case 6: /* red */
          LedOn(LCD_RED);
          LedOff(LCD_GREEN);
          LedOff(LCD_BLUE);
          break;
          
        default: /* off */
          LedOff(LCD_RED);
          LedOff(LCD_GREEN);
          LedOff(LCD_BLUE);
          break;
      } /* end switch */
    } /* end if(u8Counter == 16) */

    /* Parse the current count to set the LEDs.  RED is bit 0, ORANGE is bit 1,
    YELLOW is bit 2, GREEN is bit 3. */
    
    if(u8Counter & 0x01)
    {
      LedOn(RED);
    }
    else
    {
      LedOff(RED);
    }

    if(u8Counter & 0x02)
    {
      LedOn(ORANGE);
    }
    else
    {
      LedOff(ORANGE);
    }

    if(u8Counter & 0x04)
    {
      LedOn(YELLOW);
    }
    else
    {
      LedOff(YELLOW);
    }

    if(u8Counter & 0x08)
    {
      LedOn(GREEN);
    }
    else
    {
      LedOff(GREEN);
    }
    
  } /* end if(u16BlinkCount == 500) */
#endif /* MPGL1 */
  
} /* end UserAppSM_Idle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserAppSM_Error(void)          
{
  UserApp_StateMachine = UserAppSM_Idle;
  
} /* end UserAppSM_Error() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserAppSM_FailedInit(void)          
{
    
} /* end UserAppSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
