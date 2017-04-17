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

extern u32 G_u32AntApiCurrentMessageTimeStamp;                           /* From ant_api.c */
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;           /* From ant_api.c */
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES]; /* From ant_api.c */
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;               /* From ant_api.c */


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

  LCDCommand(LCD_CLEAR_CMD);
  LCDMessage(LINE1_START_ADDR, "ANT MULTICHAN DEMO");
  LCDMessage(LINE2_START_ADDR, "CH0   CH1   CH2  CH3");
  UserApp1_StateMachine = UserApp1SM_Idle;


  /* If good initialization, set state to Idle */
  if( 1 )
  {
    DebugPrintf("User app ready\n\r");

  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_StateMachine = UserApp1SM_FailedInit;
    DebugPrintf("User app setup failed\n\r");
  }
  
} /* end UserAppInitialize() */


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
/* Monitor BUTTON0 - BUTTON3 to assign or unassign channels */
static void UserApp1SM_Idle(void)
{
  AntAssignChannelInfoType sChannelInfo;
  u8 au8DataContent[] = {"ANT_DATA CHX: xx-xx-xx-xx-xx-xx-xx-xx xx-xx-xx-xx xx dBm\n\r"};
  u8 au8TickContent[] = {"ANT_TICK CHX: xx-xx-xx-xx-xx-xx-xx-xx\n\r"};
  u8 u8MessageIndex;
  #define MESSAGE_INDEX_CHANNEL         (u8)11
  #define MESSAGE_INDEX_PAYLOAD_START   (u8)14
  
  /* Look for any new messages from ANT in the application message buffer */
  if( AntReadAppMessageBuffer() )
  {
    if( G_eAntApiCurrentMessageClass == ANT_TICK)
    {
      /* Get the channel number */
      au8TickContent[MESSAGE_INDEX_CHANNEL] = HexToASCIICharUpper(G_sAntApiCurrentMessageExtData.u8Channel);

      /* Read all the data bytes and convert to ASCII for the display string */
      u8MessageIndex = MESSAGE_INDEX_PAYLOAD_START;
      for(u8 i = 0; i < ANT_DATA_BYTES; i++)
      {
       au8TickContent[u8MessageIndex]     = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] / 16);
       au8TickContent[u8MessageIndex + 1] = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] % 16);
       u8MessageIndex += 3;
      }
      
      DebugPrintf(au8TickContent);
    }
    
    if( G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      /* Get the channel number */
      au8DataContent[MESSAGE_INDEX_CHANNEL] = HexToASCIICharUpper(G_sAntApiCurrentMessageExtData.u8Channel);

      /* Read all the data bytes and convert to ASCII for the display string */
      u8MessageIndex = MESSAGE_INDEX_PAYLOAD_START;
      for(u8 i = 0; i < ANT_DATA_BYTES; i++)
      {
        au8DataContent[u8MessageIndex]     = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] / 16);
        au8DataContent[u8MessageIndex + 1] = HexToASCIICharUpper(G_au8AntApiCurrentMessageBytes[i] % 16);
        u8MessageIndex += 3;
      }

      /* Add the extended data bytes; u8MessageIndex is already at the next location */
    
      /* Device ID (2 bytes) */
      au8DataContent[u8MessageIndex++] = HexToASCIICharUpper( (G_sAntApiCurrentMessageExtData.u16DeviceID >>  4) & 0x000F);
      au8DataContent[u8MessageIndex]   = HexToASCIICharUpper( (G_sAntApiCurrentMessageExtData.u16DeviceID >>  0) & 0x000F);
      u8MessageIndex += 2;
      au8DataContent[u8MessageIndex++] = HexToASCIICharUpper( (G_sAntApiCurrentMessageExtData.u16DeviceID >> 12) & 0x000F);
      au8DataContent[u8MessageIndex]   = HexToASCIICharUpper( (G_sAntApiCurrentMessageExtData.u16DeviceID >>  8) & 0x000F);

      /* Device Type */
      u8MessageIndex += 2;
      au8DataContent[u8MessageIndex++] = HexToASCIICharUpper( (G_sAntApiCurrentMessageExtData.u8DeviceType >> 4) & 0x0F);
      au8DataContent[u8MessageIndex]   = HexToASCIICharUpper( (G_sAntApiCurrentMessageExtData.u8DeviceType >> 0) & 0x0F);

      /* Transmission Type */
      u8MessageIndex += 2;
      au8DataContent[u8MessageIndex++] = HexToASCIICharUpper( (G_sAntApiCurrentMessageExtData.u8TransType >> 4) & 0x0F);
      au8DataContent[u8MessageIndex]   = HexToASCIICharUpper( (G_sAntApiCurrentMessageExtData.u8TransType >> 0) & 0x0F);
 
      /* RSSI value */
      u8MessageIndex += 2;
      au8DataContent[u8MessageIndex++] = HexToASCIICharUpper( (G_sAntApiCurrentMessageExtData.s8RSSI >> 4) & 0x0F);
      au8DataContent[u8MessageIndex++] = HexToASCIICharUpper( (G_sAntApiCurrentMessageExtData.s8RSSI >> 0) & 0x0F);
    
      DebugPrintf(au8DataContent);
    }
  } /* end if( AntReadAppMessageBuffer() ) */
  
  /* BUTTON0 sets up CHANNEL0 communication if the channel is currently not configured. 
  If the channel is already configured, the channel is unassigned.
  CHANNEL 0 is SLAVE SCANNING CHANNEL with Device ID 0xffff */
  if(WasButtonPressed(BUTTON0))
  {
    if(AntRadioStatusChannel(ANT_CHANNEL_0) == ANT_UNCONFIGURED)
    {
      ButtonAcknowledge(BUTTON0);

      /* Setup channel 0 */
      sChannelInfo.AntChannel = ANT_CHANNEL_0;
      sChannelInfo.AntChannelType = CHANNEL_TYPE_SLAVE;
      sChannelInfo.AntChannelPeriodHi = ANT_CHANNEL_PERIOD_HI_DEFAULT;
      sChannelInfo.AntChannelPeriodLo = ANT_CHANNEL_PERIOD_LO_DEFAULT;
      
      sChannelInfo.AntDeviceIdHi = 0;
      sChannelInfo.AntDeviceIdLo = 0;
      sChannelInfo.AntDeviceType = 0;
      sChannelInfo.AntTransmissionType = 0;
      
      sChannelInfo.AntFrequency = ANT_FREQUENCY_DEFAULT;
      sChannelInfo.AntTxPower = ANT_TX_POWER_DEFAULT;
      
      sChannelInfo.AntNetwork = ANT_NETWORK_DEFAULT;
      for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
      {
        sChannelInfo.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
      }
      
      AntAssignChannel(&sChannelInfo);
    }
    else if(AntRadioStatusChannel(ANT_CHANNEL_0) == ANT_CONFIGURED)
    {
      ButtonAcknowledge(BUTTON0);
      AntOpenScanningChannel();
    }
    else if(AntRadioStatusChannel(ANT_CHANNEL_0) == ANT_OPEN)
    {
      ButtonAcknowledge(BUTTON0);
      AntCloseChannelNumber(ANT_CHANNEL_0);
    }

    /* Otherwise, don't do anything since ANT must be opening or closing */
    
  } /* end BUTTON0 */
    
  /* BUTTON1 sets up CHANNEL1 communication if the channel is currently not configured. 
  If the channel is already configured, the channel is unassigned.
  CHANNEL 1 is Device ID 0x1111 */
  if(WasButtonPressed(BUTTON1))
  {
    if(AntRadioStatusChannel(ANT_CHANNEL_1) == ANT_UNCONFIGURED)
    {
      ButtonAcknowledge(BUTTON1);

      /* Setup channel 1 */
      sChannelInfo.AntChannel = ANT_CHANNEL_1;
      sChannelInfo.AntChannelType = CHANNEL_TYPE_SLAVE;
      sChannelInfo.AntChannelPeriodHi = ANT_CHANNEL_PERIOD_HI_DEFAULT;
      sChannelInfo.AntChannelPeriodLo = ANT_CHANNEL_PERIOD_LO_DEFAULT;
      
      sChannelInfo.AntDeviceIdHi = 0x11;
      sChannelInfo.AntDeviceIdLo = 0x11;
      sChannelInfo.AntDeviceType = ANT_DEVICE_TYPE_DEFAULT;
      sChannelInfo.AntTransmissionType = ANT_TRANSMISSION_TYPE_DEFAULT;
      
      sChannelInfo.AntFrequency = ANT_FREQUENCY_DEFAULT;
      sChannelInfo.AntTxPower = ANT_TX_POWER_DEFAULT;
      
      sChannelInfo.AntNetwork = ANT_NETWORK_DEFAULT;
      for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
      {
        sChannelInfo.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
      }
      
      AntAssignChannel(&sChannelInfo);
    }
    else if(AntRadioStatusChannel(ANT_CHANNEL_1) == ANT_CONFIGURED)
    {
      ButtonAcknowledge(BUTTON1);
      AntOpenChannelNumber(ANT_CHANNEL_1);
    }
    else if(AntRadioStatusChannel(ANT_CHANNEL_1) == ANT_OPEN)
    {
      ButtonAcknowledge(BUTTON1);
      AntCloseChannelNumber(ANT_CHANNEL_1);
    }

    /* Otherwise, don't do anything since ANT must be opening or closing */
    
  } /* end BUTTON1 */


  /* BUTTON2 sets up CHANNEL2 communication if the channel is currently not configured. 
  If the channel is already configured, the channel is unassigned.
  CHANNEL 2 is a MASTER with Device ID 0x2222 */
  if(WasButtonPressed(BUTTON2))
  {
    if(AntRadioStatusChannel(ANT_CHANNEL_2) == ANT_UNCONFIGURED)
    {
      ButtonAcknowledge(BUTTON2);

      /* Setup channel 2 */
      sChannelInfo.AntChannel = ANT_CHANNEL_2;
      sChannelInfo.AntChannelType = CHANNEL_TYPE_SLAVE;
      sChannelInfo.AntChannelPeriodHi = ANT_CHANNEL_PERIOD_HI_DEFAULT;
      sChannelInfo.AntChannelPeriodLo = ANT_CHANNEL_PERIOD_LO_DEFAULT;
      
      sChannelInfo.AntDeviceIdHi = 0x22;
      sChannelInfo.AntDeviceIdLo = 0x22;
      sChannelInfo.AntDeviceType = ANT_DEVICE_TYPE_DEFAULT;
      sChannelInfo.AntTransmissionType = ANT_TRANSMISSION_TYPE_DEFAULT;
      
      sChannelInfo.AntFrequency = ANT_FREQUENCY_DEFAULT;
      sChannelInfo.AntTxPower = ANT_TX_POWER_DEFAULT;
      
      sChannelInfo.AntNetwork = ANT_NETWORK_DEFAULT;
      for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
      {
        sChannelInfo.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
      }
      
      AntAssignChannel(&sChannelInfo);
    }
    else if(AntRadioStatusChannel(ANT_CHANNEL_2) == ANT_CONFIGURED)
    {
      ButtonAcknowledge(BUTTON2);
      AntOpenChannelNumber(ANT_CHANNEL_2);
    }
    else if(AntRadioStatusChannel(ANT_CHANNEL_2) == ANT_OPEN)
    {
      ButtonAcknowledge(BUTTON2);
      AntCloseChannelNumber(ANT_CHANNEL_2);
    }

    /* Otherwise, don't do anything since ANT must be opening or closing */
    
  } /* end BUTTON2 */

  /* BUTTON3 sets up CHANNEL3 communication if the channel is currently not configured. 
  If the channel is already configured, the channel is unassigned.
  CHANNEL 3 is Device ID 0x3333 */
  if(WasButtonPressed(BUTTON3))
  {
    if(AntRadioStatusChannel(ANT_CHANNEL_3) == ANT_UNCONFIGURED)
    {
      ButtonAcknowledge(BUTTON3);

      /* Setup channel 3 */
      sChannelInfo.AntChannel = ANT_CHANNEL_3;
      sChannelInfo.AntChannelType = CHANNEL_TYPE_MASTER;
      sChannelInfo.AntChannelPeriodHi = ANT_CHANNEL_PERIOD_HI_DEFAULT;
      sChannelInfo.AntChannelPeriodLo = ANT_CHANNEL_PERIOD_LO_DEFAULT;
      
      sChannelInfo.AntDeviceIdHi = 0x33;
      sChannelInfo.AntDeviceIdLo = 0x33;
      sChannelInfo.AntDeviceType = ANT_DEVICE_TYPE_DEFAULT;
      sChannelInfo.AntTransmissionType = ANT_TRANSMISSION_TYPE_DEFAULT;
      
      sChannelInfo.AntFrequency = ANT_FREQUENCY_DEFAULT;
      sChannelInfo.AntTxPower = ANT_TX_POWER_DEFAULT;
      
      sChannelInfo.AntNetwork = ANT_NETWORK_DEFAULT;
      for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
      {
        sChannelInfo.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
      }
      
      AntAssignChannel(&sChannelInfo);
    }
    else if(AntRadioStatusChannel(ANT_CHANNEL_3) == ANT_CONFIGURED)
    {
      ButtonAcknowledge(BUTTON3);
      AntOpenChannelNumber(ANT_CHANNEL_3);
    }
    else if(AntRadioStatusChannel(ANT_CHANNEL_3) == ANT_OPEN)
    {
      ButtonAcknowledge(BUTTON3);
      AntCloseChannelNumber(ANT_CHANNEL_3);
    }

    /* Otherwise, don't do anything since ANT must be opening or closing */
    
  } /* end BUTTON3 */

  
} /* end UserApp1SM_Idle() */

    
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
