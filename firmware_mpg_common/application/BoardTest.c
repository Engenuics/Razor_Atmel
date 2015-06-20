/**********************************************************************************************************************
File: template.c                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this template:
 1. Copy both template.c and template.h to the Application directory
 2. Rename the files yournewtaskname.c and yournewtaskname.h
 3. Add yournewtaskname.c and yournewtaskname.h to the Application Include and Source groups in the IAR project
 4. Use ctrl-h (make sure "Match Case" is checked) to find and replace all instances of "template" with "yournewtaskname"
 5. Use ctrl-h to find and replace all instances of "Template" with "YourNewTaskName"
 6. Use ctrl-h to find and replace all instances of "TEMPLATE" with "YOUR_NEW_TASK_NAME"
 7. Add a call to YourNewTaskNameInitialize() in the init section of main
 8. Add a call to YourNewTaskNameRunActiveState() in the Super Loop section of main
 9. Add a text description to aau8AppShortNames in SystemStatusReport for your task
10. Update yournewtaskname.h per the instructions at the top of yournewtaskname.h
11. Delete this text (between the dashed lines) and update the Description below to describe your task
----------------------------------------------------------------------------------------------------------------------

Description:
This is a template .c file new source code 

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:


Protected System functions:
void TemplateInitialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void TemplateRunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32TemplateFlags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Template_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Template_StateMachine;            /* The state machine function pointer */
static u32 Template_u32Timeout;                      /* Timeout counter used across states */


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
Function: TemplateInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void TemplateInitialize(void)
{

  /* If good initialization, set flag and set state to Idle */
  if( 1 /* Add condition for good init */)
  {
    G_u32ApplicationFlags |= _APPLICATION_FLAGS_TEMPLATE;
    Template_StateMachine = TemplateIdle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    Template_StateMachine = TemplateFailedInit;
  }

} /* end TemplateInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function TemplateRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void TemplateRunActiveState(void)
{
  Template_StateMachine();

} /* end TemplateRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void TemplateIdle(void)
{
    
} /* end TemplateIdle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void TemplateError(void)          
{
  Template_StateMachine = TemplateIdle;
  
} /* end TemplateError() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void TemplateFailedInit(void)          
{
    
} /* end TemplateFailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
