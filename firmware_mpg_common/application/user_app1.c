/**********************************************************************************************************************
File: user_app1.c                                                                

Description:
Provides a Tera-Term driven system to display, read and write an LED command list.

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:
None.

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
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;            /* The state machine function pointer */
//static u32 UserApp1_u32Timeout;                      /* Timeout counter used across states */


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
Function: CheckCommandString

Description:
Parses through a command string to verify the following:
- The first character is a valid LED (R, r, O, o, Y, y, G, g, C, c, B, b, P, p, W, w)
- The second character is '-'
- Up to 6 next characters are digits 0-9 followed by '-' (ON time)
- Up to 6 next characters are digits 0-9 followed by <CR> (OFF time)

The function does NOT check if the ON and/or OFF time make sense.

Requires:
  - pu8CommandToCheck_ points to the command string to verify
  - the last character at *pu8CommandToCheck_ is <CR>

Promises:
  - Returns TRUE if the command is ok
  - Returns FALSE if the command is invalid.  An error message is printed with the reason.
*/
static bool CheckCommandString(u8* pu8CommandToCheck_)
{
  u8* pu8Parser;
  u8 au8ValidLeds[] = {'R', 'r', 'O', 'o', 'Y', 'y', 'G', 'g', 'C', 'c', 'B', 'b', 'P', 'p', 'W', 'w'};
  u8 u8ErrorCode;
  u8 u8Temp;
  
  /* First check for a valid LED character */
  pu8Parser = pu8CommandToCheck_;
  u8ErrorCode = INPUT_ERROR_LED;
  for(u8 i = 0; i < sizeof(au8ValidLeds); i++)
  {
    if(*pu8Parser == au8ValidLeds[i])
    {
      u8ErrorCode = INPUT_ERROR_NONE;
    }
  }
  
  /* Continue if a valid LED was found */
  if(u8ErrorCode == INPUT_ERROR_NONE)
  {
    /* Move to next char and assume an error */
    pu8Parser++;
    u8ErrorCode = INPUT_ERROR_FORMAT;
    if(*pu8Parser == '-')
    {
      /* Advance to the next character and make sure
      it is not a '-' */
      pu8Parser++;
      u8ErrorCode = INPUT_ERROR_START_INVALID;
      u8Temp = 0;
      if(*pu8Parser != '-')
      {
        /* Parse through the number to make sure only digits appear before '-' */
        while( *pu8Parser >= '0' && *pu8Parser <= '9' &&
               u8Temp <= INPUT_MAX_TIME_DIGITS)
        {
          pu8Parser++;
          u8Temp++;
          if(*pu8Parser == '-')
          {
            u8ErrorCode = INPUT_ERROR_NONE;
            break;
          }
        }
      }
      
      /* If the number is valid and ends with '-' then u8ErrorCode is INPUT_ERROR_NONE */
      if(u8ErrorCode == INPUT_ERROR_NONE)
      {
        /* pu8Parser is pointing at the '-' before start of the END number.
        Advance it and make sure it is not <CR> and then check the digits */
        u8ErrorCode = INPUT_ERROR_END_INVALID;
        pu8Parser++;
        u8Temp = 0;
        if(*pu8Parser != ASCII_CARRIAGE_RETURN)
        {
          /* Parse through the number to make sure only digits appear before <CR> */
          while( *pu8Parser >= '0' && *pu8Parser <= '9' &&
                 u8Temp <= INPUT_MAX_TIME_DIGITS)
          {
            u8Temp++;
            pu8Parser++;
            if(*pu8Parser == ASCII_CARRIAGE_RETURN)
            {
              /* If we get here, we have a good command, so return TRUE */
              return TRUE;
            }
          }
        } 
      } /* end of check for END time and <CR> */
    } /* end of check for START time and second '-' */
  } /* end of check for first '-' */
    
  /* If we're here we have an error so check the error code and print a message */
  DebugPrintf("\n\rInvalid command: ");
  switch(u8ErrorCode)
  {
   case INPUT_ERROR_LED:
   {
     DebugPrintf("LED name must be R, O, Y, G, C, B, P, or W\n\r");
     break;
   }
   
   case INPUT_ERROR_FORMAT:
   {
     DebugPrintf("incorrect format.  Please use L-ONTIME-OFFTIME\n\r");
     break;
   }
   
   case INPUT_ERROR_START_INVALID:
   {
     DebugPrintf("bad START value.  Must be 0 to 999999\n\r");
     break;
   }
   
   case INPUT_ERROR_END_INVALID:
   {
     DebugPrintf("bad END value.  Must be 0 to 999999\n\r");
     break;
   }
   
   default:
   {
     DebugPrintf("unknown error\n\r");
     break;
   }
  } /* end switch */
  
  return FALSE;
  
} /* end CheckCommandString() */


/*--------------------------------------------------------------------------------------------------------------------
Function: CreateNewListCommands

Description:
Creates the ON and OFF list command pair based on a verified command string.

Requires:
  - pu8CommandToAdd_ points to the start of a VERIFIED command string

Promises:
  - If successful, two commands are added to the USER list (ON and OFF) and function returns TRUE
  - Otherwise return FALSE
*/
static bool CreateNewListCommands(u8* pu8CommandToAdd_)
{
  LedCommandType sCommandOn;
  LedCommandType sCommandOff;
  LedNumberType eLed;
  u8 au8TempString[INPUT_MAX_TIME_DIGITS + 1];
  u8 u8Index;
  u8 u8IndexOffset;

  /* We rely on the fact that we have already verified the format of the command. */
  switch(*pu8CommandToAdd_)
  {
    case 'R':
      /* fall through */
    case 'r':
    {
      eLed = RED;
      break;
    }
    case 'O':
      /* fall through */
    case 'o':
    {
      eLed = ORANGE;
      break;
    }
    case 'Y':
      /* fall through */
    case 'y':
    {
      eLed = YELLOW;
      break;
    }
    case 'G':
      /* fall through */
    case 'g':
    {
      eLed = GREEN;
      break;
    }
    case 'C':
      /* fall through */
    case 'c':
    {
      eLed = CYAN;
      break;
    }
    case 'B':
      /* fall through */
    case 'b':
    {
      eLed = BLUE;
      break;
    }
    case 'P':
      /* fall through */
    case 'p':
    {
      eLed = PURPLE;
      break;
    }
    case 'W':
      /* fall through */
    case 'w':
    {
      eLed = WHITE;
      break;
    }
    default:
    {
      DebugPrintf("\n\rError: unexpected LED color\n\r");
      eLed = RED;
      break;
    }
  } /* end switch */

  /* Load the LED name to both the ON and OFF nodes */ 
  sCommandOn.bOn = TRUE;
  sCommandOn.eLED = eLed;
  sCommandOff.eLED = eLed;
  sCommandOff.bOn = FALSE;

  /* Get the ON time. The ON time starts at au8CommandString[2] */
  for(u8Index = 0; *(pu8CommandToAdd_ + u8Index + 2) != '-'; u8Index++)
  {
    /* Copy the string into its own array */
    au8TempString[u8Index] = *(pu8CommandToAdd_ + u8Index + 2);
  }
  /* u8Index is at the end of the number in au8TempString so add a NULL there and then
  convert the string to a number */
  au8TempString[u8Index] = '\0';
  sCommandOn.u32Time = (u32)(atoi( (const char*)au8TempString ) & 0x0000FFFF);
                 
  /* Get the Off time.
  u8Index + 2 is at the '-' after the ON number.  So if we add 1 more and save this to u8IndexOffset
  we can index pu8CommandToAdd_ in the same loop structure as above */
  u8IndexOffset = u8Index + 3;
  for(u8Index = 0; *(pu8CommandToAdd_ + u8Index + u8IndexOffset) != ASCII_CARRIAGE_RETURN; u8Index++)
  {
    /* Copy the string into its own array */
    au8TempString[u8Index] = *(pu8CommandToAdd_ + u8Index + u8IndexOffset);
  }
  /* u8Index is at the end of the number in au8TempString so add a NULL there and then
  convert the string to a number */
  au8TempString[u8Index] = '\0';
  sCommandOff.u32Time = (u32)(atoi( (const char*)au8TempString ) & 0x0000FFFF);
                              
  /* Add the commands to the USER list.  These should ALWAYS be added in pairs or the system will break */
  if(LedDisplayAddCommand(USER_LIST, &sCommandOn))
  {
    /* Once the ON command is added successfully, add the OFF command */
    if(LedDisplayAddCommand(USER_LIST, &sCommandOff))
    {
      /* Both commands have been added successfully, so we can return TRUE */
      return TRUE;
    }
  }
  
  /* Something failed, so return FALSE */
  return FALSE;
    
} /* end CreateNewListCommands */


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Print the user menu and advance.  This doesn't have to be a state, but the start-up sequence may become more
complicated later so it's coded as a state. */
static void UserApp1SM_Start(void)
{
  u8 au8Menu1[] = "******************************************************\n\r";

  DebugPrintf(au8Menu1);
  DebugPrintf("LED Programming Interface\n\rPress 1 to program LED command sequence\n\rPress 2 to show current USER program\n\r");
  DebugPrintf(au8Menu1);

  UserApp1_StateMachine = UserApp1SM_Idle;
  
} /* end UserApp1SM_Start() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for input */
static void UserApp1SM_Idle(void)
{
  u8 u8InputChar;
  
  /* Wait for user input to appear.  Responds immediately to any input since only 1 character is valid. */
  if(G_u8DebugScanfCharCount == 1)
  {
    DebugScanf(&u8InputChar);

    /* Check for menu command 1 which starts the command-entry state */
    if(u8InputChar == '1')
    {
      LedDisplayStartList();
      DebugPrintf("\n\n\rEnter commands as LED-ONTIME-OFFTIME and press Enter\n\rTime is in milliseconds, max 100 commands\n\rLED colors: R, O, Y, G, C, B, P, W\n\rExample: R-100-200 (Red on at 100ms and off at 200ms)\n\rPress Enter on blank line to end\n\r");
      DebugPrintf("1: ");
      UserApp1_StateMachine = UserApp1SM_EnterProgram; 
    }
    
    /* Check for menu command 2 which displays the user program */
    else if(u8InputChar == '2')
    { 
      DebugPrintf("\n\n\rCurrent USER program:\n\n\rLED  ON TIME   OFF TIME\n\r-----------------------\n\r");
      UserApp1_StateMachine = UserApp1SM_DisplayMenu; 
    }

    /* All other commands are invalid */
    else 
    { 
      DebugPrintf("\n\rInvalid entry.  Please choose either 1 or 2.\n\r");
    }

  } /* end if(G_u8DebugScanfCharCount == 1) */
  
} /* end UserApp1SM_Idle() */
     
         
/*-------------------------------------------------------------------------------------------------------------------*/
/* Enter a command list.  We will assume that characters cannot come in faster
than one at a time since this is a human interface. This could be improved
later by adding a check to ensure DebugScanf only returns 1.

We will also choose to leave all character checking until the user presses Enter.
Pressing Enter on a blank line shows how many commands were entered and the new
command list, then returns to the main menu.

If a command string is correct, both the ON and OFF command nodes are created.
TWO NODES MUST BE CREATED FOR EVERY COMMAND.  THE ON NODE IS ADDED FIRST TO THE LIST.
*/
static void UserApp1SM_EnterProgram(void)
{
  static u8 u8CharCount = 0;
  static u8 u8CommandCount = 0;
  static u8 au8CommandString[16];
  
  u8 u8CurrentChar;
    
  /* Watch for characters coming in and check them as they arrive */
  if(DebugScanf(&u8CurrentChar))
  {
    /* Check for <CR> on blank line which means the user is done */
    if( (u8CharCount == 0) && (u8CurrentChar == ASCII_CARRIAGE_RETURN) )
    {
      /* Tell the user, reset variables, and set up to print the list */
      DebugPrintf("\n\rCommand entry complete.\n\rCommands entered: ");
      DebugPrintNumber(u8CommandCount);
      DebugPrintf("\n\n\rNew USER program:\n\n\rLED  ON TIME   OFF TIME\n\r-----------------------\n\r");
      
      u8CharCount = 0;
      u8CommandCount = 0;
      UserApp1_StateMachine = UserApp1SM_DisplayMenu; 
    }
    else
    {
      /* Add the char to the string command string, increment the character 
      counter and check for CR */
      au8CommandString[u8CharCount] = u8CurrentChar;
      u8CharCount++;
      
      if(u8CurrentChar == ASCII_CARRIAGE_RETURN)
      {
        /* If CR is found, then check if the command format is correct */
        if(CheckCommandString(au8CommandString))
        {
          /* Good command, so we can safely add it to the list */
          CreateNewListCommands(au8CommandString);
            
          /* Update the variables to take the next command and print the next input line header */
          u8CharCount = 0;
          u8CommandCount++;

          /* Check to see if the list is full */
          if(u8CommandCount == INPUT_MAX_COMMANDS)
          {
            /* Report the list is full, reset variables and go to print list */
            DebugPrintf("\n\rCommand list is full.\n\rNew USER program:\n\n\rLED  ON TIME   OFF TIME\n\r-----------------------\n\r");
            u8CommandCount = 0;
            UserApp1_StateMachine = UserApp1SM_DisplayMenu; 
          }
          else
          {
            /* The list is not full, so get ready for the next command */
            DebugLineFeed();
            DebugPrintNumber( (u32)(u8CommandCount + 1) & 0x000000FF);
            DebugPrintf(": ");
          }
        } /* end if(CheckCommandString(au8CommandString)) */
        else
        {
          /* We have an invalid command, so reset the variables and print out
          the same line header. */
          u8CharCount = 0;
          DebugLineFeed();
          DebugPrintNumber( (u32)(u8CommandCount + 1) & 0x000000FF);
          DebugPrintf(": ");
        }
      } /* end if(u8CurrentChar == ASCII_CARRIAGE_RETURN) */
    } /* end of adding / checking the new character */
  } /* end of a new character */
  
} /* end UserApp1SM_EnterProgram() */
         

/*-------------------------------------------------------------------------------------------------------------------*/
/* Display a user list.  This state slows down the rate at which messages are sent to the Debug UART
to ensure the list does not overflow. 
Each line of the display list is 24 characters:
LED  ON_TIME   OFF_TIME\n\r
-----------------------
_L___XXXXXXX___XXXXXXX\n\r

24 bytes x 8bits/byte = 192bits
At 115200bits/second, each line will take 1.7ms to print.
Therefore, use a delay of two loop cycles between sending each line and this should give the system
enough time to send each line regardless of how many lines are in the list.
*/
static void UserApp1SM_DisplayMenu(void)
{
  static u8 u8CurrentListItem = 0;
  static u8 u8DelayCounter = 0;
  
  /* Send one line at a time */
  if(u8DelayCounter == 0)
  {
    /* Reset the delay counter: increase this number if not enough time is being allowed */
    u8DelayCounter = 2;
    
    /* Request the line is printed. LedDisplayPrintListLine will return 0 if an invalid line is requested 
    which means we are finished displaying the list (or the list was empty) */
    if( LedDisplayPrintListLine(u8CurrentListItem) )
    {
      u8CurrentListItem++;
    }
    else
    {
      /* Handle an empty list */
      if(u8CurrentListItem == 0)
      {
        DebugPrintf(" **USER LIST EMPTY**\n\r");
      }
      
      /* Print a closing line and restart the task */
      DebugPrintf("-----------------------\n\n\r");
      UserApp1_StateMachine = UserApp1SM_Start;
      u8CurrentListItem = 0;
      u8DelayCounter = 0;
    }
  }
  else
  {
    u8DelayCounter--;
  }
  
} /* end UserApp1SM_DisplayMenu() */
         
            
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
