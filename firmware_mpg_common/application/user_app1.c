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
Provides a Tera-Term driven system to display, read and write an LED command list.

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
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern u8 G_au8DebugScanfBuffer[DEBUG_SCANF_BUFFER_SIZE]; /* From debug.c */
extern u8 G_u8DebugScanfCharCount;                        /* From debug.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */

static u32 UserApp1_u32UserListCount;


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
  u8 au8UserApp1Start1[] = "LED program task started\n\r";
  
  DebugPrintf(au8UserApp1Start1);
  
    /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp1_StateMachine = UserApp1SM_Start;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_FailedInit;
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
/*--------------------------------------------------------------------------------------------------------------------
Function: PrintInstructions

Description:
Print out the instructions

Requires:
  -

Promises:
  - 
*/
void PrintInstructions(void)
{
  u8 au8Instruction0[] = "\n\rProgram LEDs at the prompt\n\r";
  u8 au8Instruction1[] = "Format: LED-START_TIME-END_TIME (e.g. R-1000-3000) then press <Enter>\n\r";
  u8 au8Instruction2[] = "Press <Enter> after each line\n\r";
  u8 au8Instruction3[] = "Press <Enter> on blank line to end program\n\r";
  u8 au8Entry[] = ": ";

  DebugPrintf(au8Instruction0);
  DebugPrintf(au8Instruction1);
  DebugPrintf(au8Instruction2);
  DebugPrintf(au8Instruction3);

} /* end PrintInstructions() */


/*--------------------------------------------------------------------------------------------------------------------
Function: PrintMenu

Description:
Print out the menu

Requires:
  -

Promises:
  - 
*/
void PrintMenu(void)
{
  u8 au8Menu1[] = "******************************************************\n\r";
  u8 au8Menu2[] = "LED Programming Interface\n\r";
  u8 au8Menu3[] = "Press 1 to program LED sequence\n\r";
  u8 au8Menu4[] = "Press 2 to show current program\n\r";

  DebugPrintf(au8Menu1);
  DebugPrintf(au8Menu2);
  DebugPrintf(au8Menu3);
  DebugPrintf(au8Menu4);
  DebugPrintf(au8Menu1);

} /* end PrintMenu() */

/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for input*/
static void UserApp1SM_Start(void)
{
  PrintMenu();
  UserApp1_StateMachine = UserApp1SM_Idle;
  
} /* end UserApp1SM_Start() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for input*/
static void UserApp1SM_Idle(void)
{
  u8 au8Buffer[2] = "  ";
  
  /* Wait for user input to appear.  Must be either 1 or 2 */
  if(G_u8DebugScanfCharCount == 2)
  {
    DebugScanf(au8Buffer);
    if(au8Buffer[1] == ASCII_CARRIAGE_RETURN)
    {
      if(au8Buffer[0] == '1')
      { 
        LedDisplayStartList();
        UserApp1_u32UserListCount = 0;
        DebugPrintf("Enter LED-START_TIME-END_TIME and press enter\n\rLED colors: R, O, Y, G, C, B, P, W\n\rPress enter on blank line to end\n\r");
        DebugPrintNumber(UserApp1_u32UserListCount + 1);
        DebugPrintf(": ");
        UserApp1_StateMachine = UserApp1SM_EnterProgram; 
      }
    }
  }
  
} /* end UserApp1SM_Idle() */
     
         
/*-------------------------------------------------------------------------------------------------------------------*/
/* Enter a list */
static void UserApp1SM_EnterProgram(void)
{
  u8 u8CurrentChar;
  u8 au8CommandString[] = "X-XXXXXX-XXXXXX";
  
  /* Watch for characters coming in and check them as they arrive */
  if(DebugScanf(&u8CurrentChar))
  {
    
  }
} /* end UserApp1SM_EnterProgram() */
         
         
#if 0
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */
#endif


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserApp1SM_FailedInit(void)          
{
    
} /* end UserApp1SM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
