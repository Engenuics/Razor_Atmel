/**********************************************************************************************************************
File: user_app1.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
 1. Copy both user_app1.c and user_app1.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "user_app1" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
10. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a user_app1.c file template 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void UserApp1Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp1RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern const u8 aau8FullScreen1[LCD_IMAGE_ROW_SIZE_64PX][LCD_IMAGE_COL_BYTES_128PX];
extern const u8 aau8FullScreen2[LCD_IMAGE_ROW_SIZE_64PX][LCD_IMAGE_COL_BYTES_128PX];
extern const u8 aau8FullScreen3[LCD_IMAGE_ROW_SIZE_64PX][LCD_IMAGE_COL_BYTES_128PX];

/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */

static PixelBlockType UserApp1_sEngenuicsImage;


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
Function: UserApp1Initialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void UserApp1Initialize(void)
{
 
  /* Show static image */
  //LcdClearPixels(&G_sLcdClearWholeScreen);
  
  UserApp1_sEngenuicsImage.u16RowStart = 0;
  UserApp1_sEngenuicsImage.u16ColumnStart = 0;
  UserApp1_sEngenuicsImage.u16RowSize = LCD_IMAGE_ROW_SIZE_64PX;
  UserApp1_sEngenuicsImage.u16ColumnSize = LCD_IMAGE_COL_SIZE_128PX;
  LcdLoadBitmap(&aau8FullScreen1[0][0], &UserApp1_sEngenuicsImage);
 
  UserApp1_u32Timeout = G_u32SystemTime1ms;
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp1_StateMachine = UserApp1SM_Idle1;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */

  
/*----------------------------------------------------------------------------------------------------------------------
Function UserApp1RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp1RunActiveState(void)
{
  UserApp1_StateMachine();

} /* end UserApp1RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for first image display */
static void UserApp1SM_Idle1(void)
{
  if(IsTimeUp(&UserApp1_u32Timeout, U32_IMAGE_DELAY_TIME_MS))
  {
    LcdLoadBitmap(&aau8FullScreen2[0][0], &UserApp1_sEngenuicsImage);
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_Idle2;
  }
  
} /* end UserApp1SM_Idle1() */
    
/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for image display */
static void UserApp1SM_Idle2(void)
{
  if(IsTimeUp(&UserApp1_u32Timeout, U32_IMAGE_DELAY_TIME_MS))
  {
    LcdLoadBitmap(&aau8FullScreen3[0][0], &UserApp1_sEngenuicsImage);
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_Idle3;
  }
  
} /* end UserApp1SM_Idle2() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for image display */
static void UserApp1SM_Idle3(void)
{
  if(IsTimeUp(&UserApp1_u32Timeout, U32_IMAGE_DELAY_TIME_MS))
  {
    LcdLoadBitmap(&aau8FullScreen1[0][0], &UserApp1_sEngenuicsImage);
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_Idle1;
  }
  
} /* end UserApp1SM_Idle3() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
