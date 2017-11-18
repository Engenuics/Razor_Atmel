/*!*********************************************************************************************************************
@file user_app1.c                                                                
@brief User's tasks / applications are written here.  This description
should be replaced by something specific to the task.

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

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- void UserApp1Initialize(void)
- void UserApp1RunActiveState(void)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                          /*!< @brief Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                   /*!<@brief From main.c */
extern volatile u32 G_u32SystemTime1s;                    /*!<@brief From main.c */
extern volatile u32 G_u32SystemFlags;                     /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;                /*!< @brief From main.c */

// Globals for passing data from the ANT application to the API
extern u32 G_u32AntApiCurrentMessageTimeStamp;                            // From ant_api.c
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;            // From ant_api.c
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];  // From ant_api.c
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;                // From ant_api.c


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_StateMachine;                 /*!< @brief The state machine function pointer */
static u32 UserApp1_u32Timeout;                           /*!< @brief Timeout counter used across states */

static AntAssignChannelInfoType UserApp1_sAntChannelInfo;


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*!--------------------------------------------------------------------------------------------------------------------
@fn void UserApp1Initialize(void)

@brief
Initializes the State Machine and its variables.

Should only be called once in main init section.

Requires:
- NONE

Promises:
- NONE

*/
void UserApp1Initialize(void)
{
  u8 au8AntNetworkKey[] = AU8_ANT_PLUS_NETWORK_KEY;
  
  /* All LEDs off with purple BL */
  LedOff(WHITE);
  LedOff(PURPLE);
  LedOff(BLUE);
  LedOff(CYAN);
  LedOff(GREEN);
  LedOff(YELLOW);
  LedOff(ORANGE);
  LedOff(RED);
  LedOff(LCD_GREEN);
  LedOn(LCD_BLUE);
  LedOn(LCD_RED);

  /* Welcome the user */
  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, "**ANT+ HR MONITOR**");
  
  /* Cute little startup animation... */
  for(u8 i = 0; i < 20; i++)
  {
    /* Print a star, then wait */
    LCDMessage( (LINE2_START_ADDR + i), "*");
    for(u32 j = 0; j < 100000; j++);
  }
  for(u32 i = 0; i < 1000000; i++);
  
  /* Load values to the ANT channel setup struct for an ANT+ HRM receiver */
  UserApp1_sAntChannelInfo.AntChannel = E_USERAPP1_ANT_CHANNEL;
  UserApp1_sAntChannelInfo.AntChannelType = CHANNEL_TYPE_SLAVE;
  UserApp1_sAntChannelInfo.AntChannelPeriodHi = ANT_CHANNEL_PERIOD_HI_DEFAULT;
  UserApp1_sAntChannelInfo.AntChannelPeriodLo = ANT_CHANNEL_PERIOD_LO_DEFAULT;
  
  UserApp1_sAntChannelInfo.AntDeviceIdHi = U8_ANT_WILDCARD;
  UserApp1_sAntChannelInfo.AntDeviceIdLo = U8_ANT_WILDCARD;
  UserApp1_sAntChannelInfo.AntDeviceType = U8_DEVICE_TYPE_ANTPLUS_HRM;
  UserApp1_sAntChannelInfo.AntTransmissionType = U8_ANT_WILDCARD;
  
  UserApp1_sAntChannelInfo.AntFrequency = U8_ANT_FREQUENCY_ANTPLUS;
  UserApp1_sAntChannelInfo.AntTxPower = ANT_TX_POWER_DEFAULT;
  
  UserApp1_sAntChannelInfo.AntNetwork = ANT_NETWORK_DEFAULT;
  for(u8 i = 0; i < 8; i++)
  {
    UserApp1_sAntChannelInfo.AntNetworkKey[i] = au8AntNetworkKey[i];
  }
  
  /* Queue channel assignment */
  if( AntAssignChannel(&UserApp1_sAntChannelInfo) )
  {
    LedBlink(YELLOW, LED_2HZ);
    DebugPrintf("Assigning channel\n\r");
    
    UserApp1_u32Timeout = G_u32SystemTime1ms;
    UserApp1_StateMachine = UserApp1SM_WaitAssign;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */

  
/*!----------------------------------------------------------------------------------------------------------------------
@fn void UserApp1RunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

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


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/*!-------------------------------------------------------------------------------------------------------------------
@fn static void UserApp1SM_WaitAssign(void)

@brief Wait for Ant channel to become ANT_CONFIGURED.

YELLOW LED should be blinking in this state.
LCD BL should be LCD_BLUE + LCD_RED
*/
static void UserApp1SM_WaitAssign(void)
{
  if(AntRadioStatusChannel(E_USERAPP1_ANT_CHANNEL) == ANT_CONFIGURED)
  {
    DebugPrintf("ANT channel assigned and ready\n\r");
    LedOn(YELLOW);
    
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, "Activate HRM then");
    LCDMessage(LINE2_START_ADDR, "press BTN0 to pair");
    
    UserApp1_StateMachine = UserApp1SM_Idle;
  }
  
  while( !IsTimeUp(&UserApp1_u32Timeout, U32_ASSIGN_TIMEOUT) )
  {
    DebugPrintf("WaitAssign Timeout\n\r");
    LedOff(YELLOW);
    LedOff(LCD_BLUE);
    LedOn(LCD_RED);

    UserApp1_StateMachine = UserApp1SM_Error;
  }

} /* end UserApp1SM_WaitAssign() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void UserApp1SM_Idle(void)

@brief Wait for user to press BUTTON0 to connect to an HRM 
*/
static void UserApp1SM_Idle(void)
{

} /* end UserApp1SM_Idle() */
    

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void UserApp1SM_Error(void)

@brief Handle an error here.  For now, the task is just held in this state. 
*/
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
