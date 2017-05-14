/**********************************************************************************************************************
File: user_app2.c                                                                

Description:
Runs the LED display functions and control.  
Provides the API to create new display lists.

------------------------------------------------------------------------------------------------------------------------
API:

Public functions:
void LedDisplayStartList(void)
Clears the existing USER list so it is ready to be programmed again.  All data is lost.

bool LedDisplayAddCommand(LedDisplayListNameType eListName_, LedCommandType* pCommandInfo_)
Adds a new LED commmand node.  When a new node is added, the size of the list is incremented
and the list end time is checked to see if the new end time is greater and needs updating.
Check the return value to ensure the command was added successfully.
e.g.
LedCommandType eExampleCommand;
// Initialize an "ON" command that will turn on RED at 1000ms 
eExampleCommand.eLED = RED;
eExampleCommand.bOn = TRUE;
eExampleCommand.u32Time = 1000;
if(!LedDisplayAddCommand(UserApp2_sUserLedCommandList, &eExampleCommand))
{
  DebugPrintf("Add command failed\n\r");
}

bool LedDisplayPrintListLine(u8 u8ListItem_)
Outputs a string with the color, start time and end time of each LED command in the USER list.
The USER list must be in the correct format where each on/off pair is grouped together.
The function will return FALSE when a command at u8ListItem_ does not exist.  So to
print an entire list, you could do this:

u8 u8ListEntry = 0;
while( LedDisplayPrintListLine(u8ListEntry++) );


Protected System functions:
void UserApp2Initialize(void)
Runs required initialzation for the task.  Should only be called once in main init section.

void UserApp2RunActiveState(void)
Runs current task state.  Should only be called once in main loop.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp2Flags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */



/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp2_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp2_StateMachine;            /* The state machine function pointer */

static LedDisplayListHeadType UserApp2_sDemoLedCommandList;   
static LedDisplayListHeadType UserApp2_sUserLedCommandList;     
static u32 UserApp2_u32SystemTime = 0;

static LedDisplayListHeadType* UserApp2_psActiveList;
static bool UserApp2_bSystemRunning = TRUE;
static bool UserApp2_bSystemDark = FALSE;


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------
Function: LedDisplayStartList

Description:
Clears the current USER list and ensures that all allocated memory is freed. 
Since the list is being cleared, the LED display task must be stopped if it is currently
displaying the USER list.  Alternately, the DEMO list could be started, but it 
makes more sense to turn off the display at this time (the user can restart the DEMO
list if they want to).

Requires:
  - UserApp2_sUserLedCommandList is initialized and can be any length including 0.

Promises:
  - UserApp2_sUserLedCommandList is an empty list.
*/
void LedDisplayStartList(void)
{
  LedDisplayListNodeType* psNodeParser;
  LedDisplayListNodeType* psNodeToKill;
  
  /* If the user list is active, stop the program, all LEDs off and clear the '<' char */
  if(UserApp2_psActiveList == &UserApp2_sUserLedCommandList)
  {
    UserApp2_StateMachine = UserApp2SM_Idle;
    AllLedsOff();
    LCDClearChars(LCD_USR_ON_CHAR_ADDRESS, 1);
  }
  
  /* If a list has already been created, the memory must be freed */
  if(UserApp2_sUserLedCommandList.u8ListSize != 0)
  {
    /* Free all the list space */
    psNodeParser = UserApp2_sUserLedCommandList.psFirstCommand;
    
    while(psNodeParser != NULL)
    {
      /* Kill each node in the list without losing any */
      psNodeToKill = psNodeParser;
      psNodeParser = psNodeParser->psNextNode;
      free(psNodeToKill);
    }
    
    /* Initialize the head node and reset system time */
    UserApp2_sUserLedCommandList.psFirstCommand = NULL;
    UserApp2_sUserLedCommandList.u8ListSize = 0;
    UserApp2_sUserLedCommandList.u32ListEndTime = 0;
    UserApp2_u32SystemTime = 0;
  }
  
} /* end LedDisplayStartList */


/*--------------------------------------------------------------------------------------------------------------------
Function: LedDisplayAddCommand

Description:
Adds a new LED commmand node.  When a new node is added, the size of the list must be incremented
and the list end time needs to be checked to see if the new end time is greater.

Requires:
  - eListName_ determine if the command is for the DEMO or USER list
  - pCommandInfo_ points to the command node information

Promises:
  - As long as malloc is ok and the requested list is valid, a new list node is added
  - The list's head node ListSize and ListEndTime parameters are updated
  - Returns TRUE if the command is successfully added
  - Returns FALSE on any errors
*/
bool LedDisplayAddCommand(LedDisplayListNameType eListName_, LedCommandType* pCommandInfo_)
{
  LedDisplayListNodeType* psNewNode;
  LedDisplayListNodeType* psParser;
  LedDisplayListHeadType* psDisplayList;
  
  /* Allocate the memory */
  psNewNode = malloc(sizeof(LedDisplayListNodeType));
  if(psNewNode == NULL)
  {
    DebugPrintf("\n\r*** Add node failed malloc***\n\r");
    return FALSE;
  }

  /* Select the list to modify */
  if(eListName_ == DEMO_LIST)
  {
    psDisplayList = &UserApp2_sDemoLedCommandList;
  }
  else if(eListName_ == USER_LIST)
  {
    psDisplayList = &UserApp2_sUserLedCommandList;
  }
  else
  {
    DebugPrintf("\n\r***Invalid list***\n\r");
    return FALSE;
  }
  
  
  /* Copy in the LED and ON/OFF info */
  psNewNode->psNextNode = NULL;
  psNewNode->eCommand.eLED = pCommandInfo_->eLED;
  psNewNode->eCommand.bOn = pCommandInfo_->bOn;
  
  /* If the LED is on, then it needs to start at LED_PWM_0, otherwise it starts at LED_PWM_100.
  The ON time is always the actual start time.
  The OFF time is adjusted back by LED_FADE_TIME to allow the fade-out time. */
  if(pCommandInfo_->bOn)
  {
    psNewNode->eCommand.eCurrentRate = LED_PWM_0;
    psNewNode->eCommand.u32Time = pCommandInfo_->u32Time;
  }
  else
  {
    psNewNode->eCommand.eCurrentRate = LED_PWM_100;
    /* Check that the time adjustment won't be negative */
    if(pCommandInfo_->u32Time >= LED_TOTAL_FADE_TIME)
    {
      psNewNode->eCommand.u32Time = (pCommandInfo_->u32Time) - LED_TOTAL_FADE_TIME;
    }
    else
    {
      psNewNode->eCommand.u32Time  = 0;
    }
  }
  
  /* Locate the place in the list for the new node */
  if(psDisplayList->u8ListSize == 0)
  {
    psDisplayList->psFirstCommand = psNewNode;
  }
  else
  {
    /* Find the last node in the list */
    psParser = psDisplayList->psFirstCommand;
    while(psParser->psNextNode != NULL)
    {
      psParser = psParser->psNextNode;
    }
    psParser->psNextNode = psNewNode;
    psParser = psNewNode;
  }
  
  /* Update the size and max time of the list */
  psDisplayList->u8ListSize++;
  if(pCommandInfo_->u32Time > psDisplayList->u32ListEndTime)
  {
    psDisplayList->u32ListEndTime = pCommandInfo_->u32Time;
  }
  
  return TRUE;
  
} /* end LedDisplayAddCommand() */

    
/*--------------------------------------------------------------------------------------------------------------------
Function: LedDisplayPrintListLine

Description:
Outputs a string with the color, start time and end time of each LED command in the USER list.

Requires:
  - List entries are always paired as a start time and end time
  - u8ListItem_ is the command number to print; if u8ListItem is 8, then items 16 and 17 will
    be printed since those are the 8th pair of on/off commands.

Promises:
  - If the command number is valid, queues the command display by reading the command
at number and the command at number + 1 in the form:

LED  ON TIME   OFF TIME
-----------------------    
_L___XXXXXX____XXXXXX \n\r
0123456789ABCDEF012345 6 7

ON TIME value starts at index 0x5
OFF TIME value starts at index 0xF

We add a space after OFF TIME because a 6-digit off time will return a NULL here.  It is
easiest to just overwrite a space to this character.
*/
#define ONTIME_INDEX  (u8)0x05
#define OFFTIME_INDEX (u8)0xF

bool LedDisplayPrintListLine(u8 u8ListItem_)
{
  LedDisplayListNodeType* psListParser;
  u8 au8LedNames[] = {'W','P','B','C','G','Y','O','R'};
  u8 au8DisplayString[] = " L                   \n\r";
  u8 u8ListIndex = 0;
  u8 u8NumChars;
  u32 u32CurrentTime;
  
  /* Find the list entry */
  psListParser = UserApp2_sUserLedCommandList.psFirstCommand;
  while( (psListParser != NULL) && 
         (u8ListIndex != (u8ListItem_ * 2)) ) 
  {
    psListParser = psListParser->psNextNode; 
    u8ListIndex++;
  }
  
  /* Check if the pointer is valid or not */
  if( psListParser == NULL )
  {
    return FALSE;
  }
  
  /* Make sure the next node is there, too */
  if ( psListParser->psNextNode == NULL )
  {
    return FALSE;
  }

  /* Load each part of the display array starting with the LED name */
  au8DisplayString[1] = au8LedNames[(u8)(psListParser->eCommand.eLED)];
  
  /* ON time is at the current node; Correct the time if a fade is in progress. */
  u32CurrentTime = psListParser->eCommand.u32Time;
  if(psListParser->eCommand.eCurrentRate != LED_PWM_0)
  {
    u32CurrentTime -= ( (u32)(psListParser->eCommand.eCurrentRate) / LED_FADE_STEP) * LED_FADE_TIME;
  }
  
  /* Convert to ASCII and clear the NULL that comes back with the string from NumberToAscii() */
  u8NumChars = NumberToAscii( u32CurrentTime, &au8DisplayString[ONTIME_INDEX] );
  au8DisplayString[ONTIME_INDEX + u8NumChars] = ' ';  
  
  /* Advance to next node which must be the OFF time and load the time string.
  Correct the time if a fade is in progress. */
  psListParser = psListParser->psNextNode; 
  u32CurrentTime = psListParser->eCommand.u32Time;
  if(psListParser->eCommand.eCurrentRate != LED_PWM_100)
  {
    u32CurrentTime -= (u32)( (LED_PWM_100 - (psListParser->eCommand.eCurrentRate)) / LED_FADE_STEP) * LED_FADE_TIME;
  }

  /* The time must also be corrected by LED_TOTAL_FADE_TIME so it matches what the user entered.
  Clear the NULL that comes back with the string from NumberToAscii() */
  u8NumChars = NumberToAscii( u32CurrentTime + LED_TOTAL_FADE_TIME, &au8DisplayString[OFFTIME_INDEX] );
  au8DisplayString[OFFTIME_INDEX + u8NumChars] = ' ';  
  
  /* The string is ready, so send it and return TRUE since the requested item exists */
  DebugPrintf(au8DisplayString);
  return TRUE;
  
} /* end LedDisplayPrintListLine() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: UserApp2Initialize

Description:
Initializes the State Machine and its variables.   The Demo list is loaded.

Requires:
  -

Promises:
  - aeDemoList is loaded and ready for display
*/
void UserApp2Initialize(void)
{
  u8 au8UserApp1Start2[] = "LED display task started\n\r";
  
  /* Initialize the list variables */
  UserApp2_sDemoLedCommandList.psFirstCommand = NULL;
  UserApp2_sDemoLedCommandList.u8ListSize = 0;
  UserApp2_sDemoLedCommandList.u32ListEndTime = 0;
  
  UserApp2_sUserLedCommandList.psFirstCommand = NULL;
  UserApp2_sUserLedCommandList.u8ListSize = 0;
  UserApp2_sUserLedCommandList.u32ListEndTime = 0;

#ifndef DEMO_KNIGHT_RIDER
  /* Set up the standard hard-coded display array */
  LedCommandType aeDemoList[] =
  { 
    {WHITE, 0, TRUE, LED_PWM_0},
    {WHITE, 2000, FALSE, LED_PWM_100},
    {PURPLE, 0, TRUE, LED_PWM_0},
    {PURPLE, 2000, FALSE, LED_PWM_100},
    {BLUE, 2000, TRUE, LED_PWM_0},
    {BLUE, 4000, FALSE, LED_PWM_100},
    {CYAN, 3000, TRUE, LED_PWM_0},
    {CYAN, 5000, FALSE, LED_PWM_100},
    {RED, 0, TRUE, LED_PWM_0},
    {RED, 2000, FALSE, LED_PWM_100},
    {ORANGE, 1000, TRUE, LED_PWM_0},
    {ORANGE, 3000, FALSE, LED_PWM_100},
    {YELLOW, 2000, TRUE, LED_PWM_0},
    {YELLOW, 4000, FALSE, LED_PWM_100},
    {GREEN, 3000, TRUE, LED_PWM_0},
    {GREEN, 5000, FALSE, LED_PWM_100},
    {RED, 6000, TRUE, LED_PWM_0},
    {RED, 9000, FALSE, LED_PWM_100},
    {ORANGE, 6100, TRUE, LED_PWM_0},
    {ORANGE, 9000, FALSE, LED_PWM_100},
    {YELLOW, 6200, TRUE, LED_PWM_0},
    {YELLOW, 9000, FALSE, LED_PWM_100},
    {GREEN, 6300, TRUE, LED_PWM_0},
    {GREEN, 9000, FALSE, LED_PWM_100},
    {CYAN, 6400, TRUE, LED_PWM_0},
    {CYAN, 9000, FALSE, LED_PWM_100},
    {BLUE, 6500, TRUE, LED_PWM_0},
    {BLUE, 9000, FALSE, LED_PWM_100},
    {PURPLE, 6600, TRUE, LED_PWM_0},
    {PURPLE, 9000, FALSE, LED_PWM_100},
    {WHITE, 6700, TRUE, LED_PWM_0},
    {WHITE, 9000, FALSE, LED_PWM_100}
#if 0
    /* The ON times */
    {WHITE, 0, TRUE, LED_PWM_0},
    {PURPLE, 0, TRUE, LED_PWM_0},
    {BLUE, 2000, TRUE, LED_PWM_0},
    {CYAN, 3000, TRUE, LED_PWM_0},
    {RED, 0, TRUE, LED_PWM_0},
    {ORANGE, 1000, TRUE, LED_PWM_0},
    {YELLOW, 2000, TRUE, LED_PWM_0},
    {GREEN, 3000, TRUE, LED_PWM_0},
    {RED, 6000, TRUE, LED_PWM_0},
    {ORANGE, 6100, TRUE, LED_PWM_0},
    {YELLOW, 6200, TRUE, LED_PWM_0},
    {GREEN, 6300, TRUE, LED_PWM_0},
    {CYAN, 6400, TRUE, LED_PWM_0},
    {BLUE, 6500, TRUE, LED_PWM_0},
    {PURPLE, 6600, TRUE, LED_PWM_0},
    {WHITE, 6700, TRUE, LED_PWM_0},
    /* The OFF times */
    {WHITE, 2000, FALSE, LED_PWM_100},
    {PURPLE, 2000, FALSE, LED_PWM_100},
    {BLUE, 4000, FALSE, LED_PWM_100},
    {CYAN, 5000, FALSE, LED_PWM_100},
    {RED, 2000, FALSE, LED_PWM_100},
    {ORANGE, 3000, FALSE, LED_PWM_100},
    {YELLOW, 4000, FALSE, LED_PWM_100},
    {GREEN, 5000, FALSE, LED_PWM_100},
    {RED, 9000, FALSE, LED_PWM_100},
    {ORANGE, 9000, FALSE, LED_PWM_100},
    {YELLOW, 9000, FALSE, LED_PWM_100},
    {GREEN, 9000, FALSE, LED_PWM_100},
    {CYAN, 9000, FALSE, LED_PWM_100},
    {BLUE, 9000, FALSE, LED_PWM_100},
    {PURPLE, 9000, FALSE, LED_PWM_100},
    {WHITE, 9000, FALSE, LED_PWM_100},
#endif
  };

#else /* DEMO_KNIGHT_RIDER */
  /* This doesn't work quite properly due to the system design -- a limitation imposed by
  the way the alorithm was designed an implemented. */
  LedCommandType aeDemoList[] =
  {
    {RED, 0, TRUE, LED_PWM_0},
    {ORANGE, 100, TRUE, LED_PWM_0},
    {YELLOW, 200, TRUE, LED_PWM_0},
    {GREEN, 300, TRUE, LED_PWM_0},
    {CYAN, 400, TRUE, LED_PWM_0},
    {BLUE, 500, TRUE, LED_PWM_0},
    {PURPLE, 600, TRUE, LED_PWM_0},
    {WHITE, 700, TRUE, LED_PWM_0},
    
    {WHITE, 900, TRUE, LED_PWM_0},
    {PURPLE, 1000, TRUE, LED_PWM_0},
    {BLUE, 1200, TRUE, LED_PWM_0},
    {CYAN, 1300, TRUE, LED_PWM_0},
    {GREEN, 1400, TRUE, LED_PWM_0},
    {YELLOW, 1500, TRUE, LED_PWM_0},
    {ORANGE, 1600, TRUE, LED_PWM_0},
    {RED, 1700, TRUE, LED_PWM_0},
    
    {RED, 300, FALSE, LED_PWM_100},
    {ORANGE, 400, FALSE, LED_PWM_100},
    {YELLOW, 500, FALSE, LED_PWM_100},
    {GREEN, 600, FALSE, LED_PWM_100},
    {CYAN, 700, FALSE, LED_PWM_100},
    {BLUE, 800, FALSE, LED_PWM_100},
    {PURPLE, 900, FALSE, LED_PWM_100},
    {WHITE, 1000, FALSE, LED_PWM_100},

    {WHITE, 1300, FALSE, LED_PWM_100},
    {PURPLE, 1400, FALSE, LED_PWM_100},
    {BLUE, 1500, FALSE, LED_PWM_100},
    {CYAN, 1600, FALSE, LED_PWM_100},
    {GREEN, 1700, FALSE, LED_PWM_100},
    {YELLOW, 1800, FALSE, LED_PWM_100},
    {ORANGE, 1900, FALSE, LED_PWM_100},
    {RED, 2000, TRUE, LED_PWM_0},
  };
#endif /* USER_KNIGHT_RIDER */
  
  /* Build the command array */
  for(u8 i = 0; i < (sizeof(aeDemoList) / sizeof(LedCommandType)); i++)
  {
    LedDisplayAddCommand(DEMO_LIST, &aeDemoList[i]);
  }

  
#ifdef USER_LIST_EXAMPLE
  LedCommandType aeUserList[] =
  { /* The ON times */
    {WHITE, 0, TRUE, LED_PWM_0},
    {RED, 500, TRUE, LED_PWM_0},
    {YELLOW, 0, TRUE, LED_PWM_0},
    {YELLOW, 1000, TRUE, LED_PWM_0},
    /* The OFF times */    
    {WHITE, 2000, FALSE, LED_PWM_100},
    {RED, 2000, FALSE, LED_PWM_100},
    {YELLOW, 1000, FALSE, LED_PWM_100},
    {YELLOW, 2000, FALSE, LED_PWM_100}
  };

  /* Build the command array */
  for(u8 i = 0; i < (sizeof(aeUserList) / sizeof(LedCommandType)); i++)
  {
    LedDisplayAddCommand(USER_LIST, &aeUserList[i]);
  }
#endif  
  

  UserApp2_psActiveList = &UserApp2_sDemoLedCommandList;

  /* Load the LCD and complete the initialization */
  LoadLcdScreen();
  DebugPrintf(au8UserApp1Start2);
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    UserApp2_StateMachine = UserApp2SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp2_StateMachine = UserApp2SM_FailedInit;
  }

} /* end UserApp2Initialize() */

  
/*----------------------------------------------------------------------------------------------------------------------
Function UserApp2RunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void UserApp2RunActiveState(void)
{
  UserApp2_StateMachine();

} /* end UserApp2RunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function LoadLcdScreen()

Description:
Clears the LCD screen and writes the default text.
The display is ON without a cursor.

Requires:
  - None

Promises:
- The LCD is updated to the main screen:
EiE Competition
DMO   USR    ||    X
0123456789ABCDEF0123  (reference character addresses)
*/
void LoadLcdScreen(void)
{
                             /*0123456789ABCDEF0123*/
  u8 au8UserApp2LcdStart1[] = "EiE Competition";
  u8 au8UserApp2LcdStart2[] = "DMO   USR    ||    X";

  LCDCommand(LCD_CLEAR_CMD);
  LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
  
  LCDMessage(LINE1_START_ADDR, au8UserApp2LcdStart1);
  LCDMessage(LINE2_START_ADDR, au8UserApp2LcdStart2);
  
} /* end LoadLcdScreen() */
  

/*----------------------------------------------------------------------------------------------------------------------
Function AllLedsOff()

Description:
Sets all LEDs to LED_PWM_0. 

Requires:
  - None

Promises:
  - All discrete LEDs are set to PWM mode with LED_PWM_0 (OFF)
*/
void AllLedsOff(void)
{
  LedPWM(RED, LED_PWM_0);
  LedPWM(ORANGE, LED_PWM_0);
  LedPWM(YELLOW, LED_PWM_0);
  LedPWM(GREEN, LED_PWM_0);
  LedPWM(CYAN, LED_PWM_0);
  LedPWM(BLUE, LED_PWM_0);
  LedPWM(PURPLE, LED_PWM_0);
  LedPWM(WHITE, LED_PWM_0);
  
} /* end AllLedsOff() */


/*----------------------------------------------------------------------------------------------------------------------
Function ResetListFades()

Description:
Resets all of the fade times for a list.

Requires:
  - psTargetList_

Promises:
  - All of the current fade times are reset back to their initial states.
*/
void ResetListFades(LedDisplayListNodeType* psTargetList_)
{
  u32 u32Adjustment;
  LedDisplayListNodeType* psListParser;
  
  psListParser = psTargetList_;
  
  /* Check each element to see if it was in the middle of a fade.  If so, reset
  the command information so the command will be correct */
  while(psListParser != NULL)
  {
    /* Fade on commands should start at LED_PWM_0 */
    if( (psListParser->eCommand.bOn == TRUE) && 
        (psListParser->eCommand.eCurrentRate != LED_PWM_0) )
    {
      /* Correct the time and rate back for this LED.  
      The correction is the eCurrentRate divided by LED_FADE_STEP 
      and then multiplied by LED_FADE_TIME 
      e.g. eCurrentRate = LED_PWM_60, LED_FADE_STEP = 2, LED_FADE_TIME = 20 
      So correction is (12/2) x 20 = 120ms 
      Because of the way the steps are added, the integer division should always work. */
      u32Adjustment = ( (u32)(psListParser->eCommand.eCurrentRate) / LED_FADE_STEP) * LED_FADE_TIME;
      psListParser->eCommand.u32Time -= u32Adjustment;
      psListParser->eCommand.eCurrentRate = LED_PWM_0;
    }
           
    /* Fade off commands should start at LED_PWM_100 */
    if( (psListParser->eCommand.bOn == FALSE) && 
        (psListParser->eCommand.eCurrentRate != LED_PWM_100) )
    {
      /* Correct the time and rate back for this LED.  Since it is decreasing from LED_PWM_100, we must
      subtract the eCurrentRate from LED_PWM_100 */
      u32Adjustment = (u32)( (LED_PWM_100 - (psListParser->eCommand.eCurrentRate)) / LED_FADE_STEP) * LED_FADE_TIME;
      psListParser->eCommand.u32Time -= u32Adjustment;
      psListParser->eCommand.eCurrentRate = LED_PWM_100;
    }

    /* Go to the next entry in the list */
    psListParser = psListParser->psNextNode;
  }
  
} /* end ResetListFades */


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for first button to be pressed to start the program
BUTTON0 starts the DEMO code.
BUTTON1 starts the USER code.
*/
static void UserApp2SM_Idle(void)
{
  /* BUTTON0 selects the Demo List which is always available */
  if(WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);
   
    /* Turn on the < indicator and set the active list to DEMO */
    LCDMessage(LCD_DMO_ON_CHAR_ADDRESS, "<");
    UserApp2_psActiveList = &UserApp2_sDemoLedCommandList;
    UserApp2_u32SystemTime = 0;
    UserApp2_StateMachine = UserApp2SM_RunCommandList;
  } /* end BUTTON0 code */

  /* BUTTON1 selects the User List which may or may not be available */
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
   
    /* If the user list is empty, display a message for 2 seconds to tell the user this.
    Otherwise, start the USER code */
    if(UserApp2_sUserLedCommandList.u8ListSize == 0)
    {
      /* Update the LCD with the message and then go to a wait state */
      LCDCommand(LCD_CLEAR_CMD);
      LCDMessage(LINE1_START_ADDR, "USER list is empty");
      UserApp2_StateMachine = UserApp2SM_Delay;
    }
    else
    {
      /* Turn on the < indicator and set the active list to USER */
      LCDMessage(LCD_USR_ON_CHAR_ADDRESS, "<");
      UserApp2_psActiveList = &UserApp2_sUserLedCommandList;
      UserApp2_u32SystemTime = 0;
      UserApp2_StateMachine = UserApp2SM_RunCommandList;
    }
  } /* end BUTTON1 code */

} /* end UserApp2SM_Idle() */
    

/*-------------------------------------------------------------------------------------------------------------------*/
/* The main program that runs the LEDs and LCD user interface */
static void UserApp2SM_RunCommandList(void)
{
  LedDisplayListNodeType* psListParser;

  /**** BUTTON MANAGEMENT ****/

  /* BUTTON0 starts the DEMO LED pattern */
  if(WasButtonPressed(BUTTON0))
  {
    ButtonAcknowledge(BUTTON0);
    
    /* Reset the list control variables and system time */
    UserApp2_psActiveList = &UserApp2_sDemoLedCommandList;
    UserApp2_u32SystemTime = 0;
    AllLedsOff();
    
    /* Make sure all the fade times are reset */
    ResetListFades(UserApp2_psActiveList->psFirstCommand);
    
    /* Update the LCD to indicate which list is being shown */
    LCDMessage(LCD_DMO_ON_CHAR_ADDRESS, "<");
    LCDClearChars(LCD_USR_ON_CHAR_ADDRESS, 1);

  } /* end of BUTTON0 handler */

  
  /* BUTTON1 starts the USER LED pattern */
  if(WasButtonPressed(BUTTON1))
  {
    ButtonAcknowledge(BUTTON1);
    
    /* Reset the list control variables and system time */
    UserApp2_psActiveList = &UserApp2_sUserLedCommandList;
    UserApp2_u32SystemTime = 0;
    AllLedsOff();
    
    /* Make sure all the fade times are reset */
    ResetListFades(UserApp2_psActiveList->psFirstCommand);

    /* If the user list is empty, display a message for 2 seconds to tell the user this.
    Otherwise, start the USER code */
    if(UserApp2_sUserLedCommandList.u8ListSize == 0)
    {
      /* Update the LCD with the message and then go to a wait state */
      LCDCommand(LCD_CLEAR_CMD);
      LCDMessage(LINE1_START_ADDR, "USER list is empty");
      UserApp2_StateMachine = UserApp2SM_Delay;
    }
    else
    {
      /* Update the LCD to indicate which list is being shown */
      LCDMessage(LCD_USR_ON_CHAR_ADDRESS, "<");
      LCDClearChars(LCD_DMO_ON_CHAR_ADDRESS, 1);
    }
   
  } /* end of BUTTON1 handler */

  /* BUTTON2 pauses the active program */
  if(WasButtonPressed(BUTTON2))
  {
    ButtonAcknowledge(BUTTON2);
    
    /* If the system is running, stop it and change the icon to > */
    if(UserApp2_bSystemRunning)
    {
      UserApp2_bSystemRunning = FALSE;
      LCDMessage(LCD_PAUSE_CHAR_ADDRESS, "> ");
    }
    /* Otherwise start it running again and set the icon back to || */
    else
    {
      UserApp2_bSystemRunning = TRUE;
      LCDMessage(LCD_PAUSE_CHAR_ADDRESS, "||");
    }

  } /* end of BUTTON2 handler */


  /* BUTTON3 toggles "Dark mode" where all LEDs remain off but the active LED program continues to run */
  if(WasButtonPressed(BUTTON3))
  {
    UserApp2_bSystemDark = (bool)(!UserApp2_bSystemDark);
    ButtonAcknowledge(BUTTON3);
    
  } /* end of BUTTON3 handler */


  /**** LED LIST PARSING ****/
  
  /* Handle an empty list */
  if( (UserApp2_psActiveList->u8ListSize == 0) ||
      (UserApp2_bSystemDark == TRUE) )
  {
    AllLedsOff();
  }
  else
  {
    /* Check for any LED that must change */
    psListParser = UserApp2_psActiveList->psFirstCommand;
    while(psListParser != NULL)
    {
  
#if 0 /* Debug code */
      if(UserApp2_u32SystemTime == 8800)
      {
        DebugPrintf("Should be fading off now\n\r");
      }
#endif
      
      if(psListParser->eCommand.u32Time == UserApp2_u32SystemTime)
      {
        /* Manage an LED turning ON: fading is handled by setting the next level of
           brightness and changing the stored time in the node until the LED is 100% on */
        if( (psListParser->eCommand.bOn == TRUE) )
        { 
          if(psListParser->eCommand.eCurrentRate != LED_PWM_100)
          {
            psListParser->eCommand.eCurrentRate += LED_FADE_STEP;
            psListParser->eCommand.u32Time += LED_FADE_TIME;

           /* Set the current rate */
           LedPWM(psListParser->eCommand.eLED, psListParser->eCommand.eCurrentRate);
          }
          else
          {
             psListParser->eCommand.u32Time -= LED_TOTAL_FADE_TIME;
             psListParser->eCommand.eCurrentRate = LED_PWM_0;
          }
        }

        /* Manage an LED turning OFF: fading is handled by setting the next level of
           brightness and changing the stored time in the node until the LED is 100% off */
        if( (psListParser->eCommand.bOn == FALSE))
        {
          /* Adjust the fade rate until the LED is off */
          if(psListParser->eCommand.eCurrentRate != LED_PWM_0)
          {
            psListParser->eCommand.eCurrentRate -= LED_FADE_STEP;
            psListParser->eCommand.u32Time += LED_FADE_TIME;

            /* Set the current rate */
           LedPWM(psListParser->eCommand.eLED, psListParser->eCommand.eCurrentRate);
          }
          /* Once the LED is off, then reset the start time */
          else
          {
            psListParser->eCommand.u32Time -= LED_TOTAL_FADE_TIME;
            psListParser->eCommand.eCurrentRate = LED_PWM_100;
          }
        }
      }

      psListParser = psListParser->psNextNode;
    }
  } /* end if(psListParser->eCommand.u32Time == UserApp2_u32SystemTime) */

  /* Advance the system time */
  if(UserApp2_bSystemRunning)
  {
    UserApp2_u32SystemTime++;
   
    /* Check if we're at the end of the list */
    if(UserApp2_u32SystemTime > UserApp2_psActiveList->u32ListEndTime)
    {
      UserApp2_u32SystemTime = 0;
    }
  }

} /* end UserApp2SM_RunCommandList() */
 

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait state on the way back to Idle */
static void UserApp2SM_Delay(void)
{
  static u32 u32Delay = 2000;
  
  u32Delay--;
  if(u32Delay == 0)
  {
    u32Delay = 2000;
    UserApp2_StateMachine = UserApp2SM_Idle;
    LoadLcdScreen();
  }
  
} /* end UserApp2SM_LcdMessageDisplay() */


#if 0
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp2SM_Error(void)          
{
  
} /* end UserApp2SM_Error() */
#endif


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void UserApp2SM_FailedInit(void)          
{
    
} /* end UserApp2SM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
