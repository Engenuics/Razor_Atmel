/**********************************************************************************************************************
File: user_app1.h                                                                

----------------------------------------------------------------------------------------------------------------------
To start a new task using this user_app1 as a template:
1. Follow the instructions at the top of user_app1.c
2. Use ctrl-h to find and replace all instances of "user_app1" with "yournewtaskname"
3. Use ctrl-h to find and replace all instances of "UserApp1" with "YourNewTaskName"
4. Use ctrl-h to find and replace all instances of "USER_APP1" with "YOUR_NEW_TASK_NAME"
5. Add #include yournewtaskname.h" to configuration.h
6. Add/update any special configurations required in configuration.h (e.g. peripheral assignment and setup values)
7. Delete this text (between the dashed lines)
----------------------------------------------------------------------------------------------------------------------

Description:
Header file for user_app1.c

**********************************************************************************************************************/

#ifndef __USER_APP1_H
#define __USER_APP1_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
#define   INPUT_MAX_CHARS             (u8)16  /* The maximum number of characters in a command string */
#define   INPUT_MAX_TIME_DIGITS       (u8)6   /* The maximum number of digits in a command string time entry */
#define   INPUT_MAX_COMMANDS          (u8)(MAX_LIST_SIZE / 2) /* Maximum number of commands that may be entered */

#define   INPUT_ERROR_NONE            (u8)0   /* No error */
#define   INPUT_ERROR_LED             (u8)1   /* An invalid character was entered for the LED in the user command string */
#define   INPUT_ERROR_FORMAT          (u8)2   /* The LED command does not follow the L-S-E format */
#define   INPUT_ERROR_START_INVALID   (u8)3   /* The start time is not a valid number */
#define   INPUT_ERROR_END_INVALID     (u8)4   /* The end time is not a valid number */


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void UserApp1Initialize(void);
void UserApp1RunActiveState(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
//static void PrintMenu(void);
//static void PrintInstructions(void);
static bool CheckCommandString(u8* pu8CommandToCheck_);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void UserApp1SM_Start(void);    
static void UserApp1SM_Idle(void);    

static void UserApp1SM_EnterProgram(void);
static void UserApp1SM_DisplayMenu(void);

static void UserApp1SM_Error(void);         
static void UserApp1SM_FailedInit(void);        


#endif /* __USER_APP1_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
