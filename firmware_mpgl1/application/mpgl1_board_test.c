/**********************************************************************************************************************
File: mpgl1_board_test.c                                                                

Description:
Application to completely test the MPGL1 Atmel development board, mpgl1-ehdw-02.
**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32BoardTestFlags;                       /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern u8 G_au8MessageOK[];                           /* From utilities.c */
extern u8 G_au8MessageFAIL[];                         /* From utilities.c */

extern u32 G_u32AntFlags;                             /* From ant.c */
extern AntSetupDataType G_stAntSetupData;             /* From ant.c */

extern u32 G_u32AntApiCurrentDataTimeStamp;                      /* From ant_api.c */
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;   /* From ant_api.c */
extern u8 G_au8AntApiCurrentData[ANT_APPLICATION_MESSAGE_BYTES]; /* From ant_api.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "BoardTest_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type BoardTest_StateMachine;            /* The state machine function pointer */

static u32 BoardTest_u32Timeout;                      /* Timeout counter used across states */


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: BoardTestInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void BoardTestInitialize(void)
{
  u8 au8BoardTestStartupMsg[] = "Board test task started\n\r";

  /* Start with all LEDs on */
  LedOn(WHITE);
  LedOn(PURPLE);
  LedOn(BLUE);
  LedOn(CYAN);
  LedOn(GREEN);
  LedOn(YELLOW);
  LedOn(ORANGE);
  LedOn(RED);
  LedOn(LCD_BLUE);
  LedOn(LCD_GREEN);
  LedOn(LCD_RED);

  /* Configure the ANT radio */
  G_stAntSetupData.AntChannel          = ANT_CHANNEL_BOARDTEST;
  G_stAntSetupData.AntChannelType      = ANT_CHANNEL_TYPE_BOARDTEST;
  G_stAntSetupData.AntNetwork          = ANT_NETWORK_BOARDTEST;
  G_stAntSetupData.AntSerialLo         = ANT_SERIAL_LO_BOARDTEST;
  G_stAntSetupData.AntSerialHi         = ANT_SERIAL_HI_BOARDTEST;
  G_stAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_BOARDTEST;
  G_stAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_BOARDTEST;
  G_stAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_BOARDTEST;
  G_stAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_BOARDTEST;
  G_stAntSetupData.AntFrequency        = ANT_FREQUENCY_BOARDTEST;
  G_stAntSetupData.AntTxPower          = ANT_TX_POWER_BOARDTEST;
  
  /* Send all the channel config; 0 the message counter after so we clear the messsage from init */
  AntChannelConfig(ANT_MASTER);
  
  /* Init complete: print message, set function pointer and application flag */
  BoardTest_u32Timeout = G_u32SystemTime1ms;
  DebugPrintf(au8BoardTestStartupMsg);
  BoardTest_StateMachine = BoardTestSM_Idle;

} /* end BoardTestInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function BoardTestRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void BoardTestRunActiveState(void)
{
  BoardTest_StateMachine();

} /* end BoardTestRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
State Machine Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
void BoardTestSM_Idle(void)
{
  static bool bButton0Test = FALSE;
  static u8 u8Button2Test = 0;
  static u8 u8Button3Test = 0;
  static u8 au8TestMessage[] = {0, 0, 0, 0, 0, 0, 0, 0};
  static u8 au8DataMessage[] = "ANT data: ";
  u8 au8DataContent[26];
  AntChannelStatusType eAntCurrentState;

                          /* "01234567890123456789" */
  static const u8 au8Eng[] = "ENGENUICS RAZOR     ";
  static const u8 au8MPG[] = "ASCII DEV BOARD     ";
  u8 au8Temp1[21];
  u8 au8Temp2[21];
  
  static u8 u8ResetIndex = 0;
  static u8 u8Index = 0;
  static u32 u32LcdTimer;

  /* BUTTON0 toggles LEDs */
  if( WasButtonPressed(BUTTON0) )
  {
    ButtonAcknowledge(BUTTON0);
    
    /* If test is active, deactivate it, put all LEDs back on */
    if(bButton0Test)
    {
      bButton0Test = FALSE;

      LedOn(WHITE);
      LedOn(PURPLE);
      LedOn(BLUE);
      LedOn(CYAN);
      LedOn(GREEN);
      LedOn(YELLOW);
      LedOn(ORANGE);
      LedOn(RED);
    }
    /* Else activate it: turn all LEDs off */
    else
    {
      bButton0Test = TRUE;

      LedOff(WHITE);
      LedOff(PURPLE);
      LedOff(BLUE);
      LedOff(CYAN);
      LedOff(GREEN);
      LedOff(YELLOW);
      LedOff(ORANGE);
      LedOff(RED);
    }
  } /* End of BUTTON 0 test */

/* BUTTON1 toggles the radio and buzzer test.  When the button is pressed,
  an open channel request is made.  The system monitors _ANT_FLAGS_CHANNEL_OPEN
  to control wether or not the buzzer is on. */
  
  /* Toggle the beeper and ANT radio on BUTTON1 */
  if( WasButtonPressed(BUTTON1) )
  {
    ButtonAcknowledge(BUTTON1);
    eAntCurrentState = AntRadioStatus();

    if(eAntCurrentState == ANT_CLOSED )
    {
       AntOpenChannel();
    }

    if(eAntCurrentState == ANT_OPEN)
    {
       AntCloseChannel();
    }
  }
 
 
#if 0
  /* Monitor the CHANNEL_OPEN flag to decide whether or not audio should be on */
  if( (AntRadioStatus() == ANT_OPEN ) && !(BoardTest_u32Flags & _AUDIO_ANT_ON) )
  {
    PWMAudioOn(BUZZER1);
    BoardTest_u32Flags |= _AUDIO_ANT_ON;
  }
  
  if( AntRadioStatus() == ANT_CLOSED )
  {
    PWMAudioOff(BUZZER1);
    BoardTest_u32Flags &= ~_AUDIO_ANT_ON;
  }
#endif
  
  /* Process ANT Application messages */  
        
  if( AntReadData() )
  {
     /* New data message: check what it is */
    if(G_eAntApiCurrentMessageClass == ANT_DATA)
    {
      /* We got some data: print it */
      for(u8 i = 0; i < ANT_DATA_BYTES; i++)
      {
        au8DataContent[3 * i]     = HexToASCIICharUpper(G_au8AntApiCurrentData[i] / 16);
        au8DataContent[3 * i + 1] = HexToASCIICharUpper(G_au8AntApiCurrentData[i] % 16);
        au8DataContent[3 * i + 2] = '-';
      }
      au8DataContent[23] = '\n';
      au8DataContent[24] = '\r';
      au8DataContent[25] = '\0';
      
      DebugPrintf(au8DataMessage);
      DebugPrintf(au8DataContent);
    }
    else if(G_eAntApiCurrentMessageClass == ANT_TICK)
    {

     /* Update and queue the new message data */
      au8TestMessage[7]++;
      if(au8TestMessage[7] == 0)
      {
        au8TestMessage[6]++;
        if(au8TestMessage[6] == 0)
        {
          au8TestMessage[5]++;
        }
      }
      AntQueueBroadcastMessage(au8TestMessage);
    }
  }

  /* BUTTON2 toggles LCD backlights */
  if( WasButtonPressed(BUTTON2) )
  {
    ButtonAcknowledge(BUTTON2);
    
    /* If test is active, deactivate it, put all LEDs back on */
    switch(u8Button2Test)
    {
      case 0:
        u8Button2Test = 1;

        LedOff(LCD_RED);
        LedOff(LCD_GREEN);
        LedOff(LCD_BLUE);
        break;

      case 1:
        u8Button2Test = 2;

        LedOn(LCD_RED);
        LedOff(LCD_GREEN);
        LedOff(LCD_BLUE);
        break;

      case 2:
        u8Button2Test = 3;

        LedOff(LCD_RED);
        LedOn(LCD_GREEN);
        LedOff(LCD_BLUE);
        break;

      case 3:
        u8Button2Test = 4;

        LedOff(LCD_RED);
        LedOff(LCD_GREEN);
        LedOn(LCD_BLUE);
        break;

      case 4:
        u8Button2Test = 0;

        LedOn(LCD_RED);
        LedOn(LCD_GREEN);
        LedOn(LCD_BLUE);
        break;
        
      default:
        break;
    }
  } /* End of BUTTON 2 test */

  /* BUTTON3 toggles buzzer test */
  if( WasButtonPressed(BUTTON3) )
  {
    ButtonAcknowledge(BUTTON3);
    
    /* If test is active, deactivate it, put all LEDs back on */
    switch(u8Button3Test)
    {
      case 0:
        u8Button3Test = 1;

        PWMAudioSetFrequency(BUZZER2, 1000);
        PWMAudioOff(BUZZER1);
        PWMAudioOn(BUZZER2);
        break;

      case 1:
        u8Button3Test = 2;

        PWMAudioSetFrequency(BUZZER1, 500);
        PWMAudioOn(BUZZER1);
        PWMAudioOff(BUZZER2);
        break;

      case 2:
        u8Button3Test = 0;

        PWMAudioOff(BUZZER1);
        PWMAudioOff(BUZZER2);
        break;
       
      default:
        break;
    }
  } /* End of BUTTON 3 test */

  
  /* LCD scrolling message */
  if(IsTimeUp(&u32LcdTimer, 200))
  {
    u32LcdTimer = G_u32SystemTime1ms;
    au8Temp1[20] = NULL;
    au8Temp2[20] = NULL;
    u8Index = u8ResetIndex;
    for(u8 i = 0; i < 20; i++)
    { 
      if( u8Index == 20)
      {
        u8Index = 0; 
      }
      au8Temp1[u8Index] = au8Eng[i];
      au8Temp2[u8Index] = au8MPG[i];
      u8Index++;
    }
    
    LCDMessage(LINE1_START_ADDR, au8Temp1);
    LCDMessage(LINE2_START_ADDR, au8Temp2);
    
    if(u8ResetIndex == 0)
    {
      u8ResetIndex = 21;
    }

    u8ResetIndex--;

  }
  
} /* end BoardTestSM_Idle() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
void BoardTestSM_Error(void)          
{
  BoardTest_StateMachine = BoardTestSM_Idle;
  
} /* end BoardTestSM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
