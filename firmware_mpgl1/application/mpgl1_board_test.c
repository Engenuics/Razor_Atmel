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

extern u8 G_au8MessageOK[];                            /* From utilities.c */
extern u8 G_au8MessageFAIL[];                          /* From utilities.c */

extern u32 G_u32AntFlags;                              /* From ant.c */

extern u32 G_u32AntApiCurrentMessageTimeStamp;                           /* From ant_api.c */
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;           /* From ant_api.c */
extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES]; /* From ant_api.c */
extern AntExtendedDataType G_sAntApiCurrentMessageExtData;               /* From ant_api.c  */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "BoardTest_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type BoardTest_StateMachine;               /* The state machine function pointer */

static u32 BoardTest_u32Timeout;                         /* Timeout counter used across states */

static  AntAssignChannelInfoType BoardTest_sChannelInfo; /* ANT channel configuration */


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

  /* Load the ANT setup parameters */
  /* Configure the ANT radio */
  BoardTest_sChannelInfo.AntChannel          = ANT_CHANNEL_BOARDTEST;
  BoardTest_sChannelInfo.AntChannelType      = ANT_CHANNEL_TYPE_BOARDTEST;
  BoardTest_sChannelInfo.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_BOARDTEST;
  BoardTest_sChannelInfo.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_BOARDTEST;

  BoardTest_sChannelInfo.AntDeviceIdHi       = ANT_DEVICEID_HI_BOARDTEST;
  BoardTest_sChannelInfo.AntDeviceIdLo       = ANT_DEVICEID_LO_BOARDTEST;
  BoardTest_sChannelInfo.AntDeviceType       = ANT_DEVICE_TYPE_BOARDTEST;
  BoardTest_sChannelInfo.AntTransmissionType = ANT_TRANSMISSION_TYPE_BOARDTEST;

  BoardTest_sChannelInfo.AntFrequency        = ANT_FREQUENCY_BOARDTEST;
  BoardTest_sChannelInfo.AntTxPower          = ANT_TX_POWER_BOARDTEST;

  BoardTest_sChannelInfo.AntNetwork = ANT_NETWORK_DEFAULT;
  for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
  {
    BoardTest_sChannelInfo.AntNetworkKey[i] = ANT_DEFAULT_NETWORK_KEY;
  }
  
  /* Queue the channel assignment and go to wait state */
  AntAssignChannel(&BoardTest_sChannelInfo);
  BoardTest_u32Timeout = G_u32SystemTime1ms;
  DebugPrintf("Board test task started\n\r");
  BoardTest_StateMachine = BoardTestSM_SetupAnt;

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
void BoardTestSM_SetupAnt(void)
{
  /* Check to see if the channel assignment is successful */
  if(AntRadioStatusChannel(ANT_CHANNEL_BOARDTEST) == ANT_CONFIGURED)
  {
    DebugPrintf("Board test ANT Master ready\n\r");
    DebugPrintf("Device ID: ");
    DebugPrintNumber(ANT_DEVICEID_DEC_BOARDTEST);
    DebugPrintf(", Device Type 96, Trans Type 1, Frequency 50\n\r");

    BoardTest_StateMachine = BoardTestSM_Idle;
    
                              /* "01234567890123456789" */
    LCDCommand(LCD_CLEAR_CMD);
    LCDMessage(LINE1_START_ADDR, "MONSTER SHIELD      ");
    LCDMessage(LINE2_START_ADDR, "0                   ");
    LedOff(WHITE);
    LedOff(PURPLE);
    LedOff(BLUE);
    LedOff(CYAN);
    LedOff(GREEN);
    LedOff(YELLOW);
    LedOff(ORANGE);
    LedOff(RED);

  }

  /* Watch for timeout */
  if(IsTimeUp(&BoardTest_u32Timeout, 3000))
  {
    /* Init failed */
    DebugPrintf("Board test cannot assign ANT channel\n\r");
    BoardTest_StateMachine = BoardTestSM_Idle;
  }

} /* end BoardTestSM_SetupAnt */

/*--------------------------------------------------------------------------------------------------------------------*/
void BoardTestSM_Idle(void)
{
  static u8 u8Button2Test = 0;
  static u8 u8LightNumber = 0;
 
  static u16 au16Frequencies[] = {NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4,
                                  NOTE_A4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4,
                                  NOTE_E4, NOTE_D4, NOTE_D4, NOTE_D4, NOTE_D4,
                                  NOTE_C4, NOTE_G4, NOTE_G4, NOTE_F4, NOTE_E4,
                                  NOTE_E4, NOTE_D4, NOTE_G4, NOTE_F4, NOTE_E4,
                                  NOTE_E4, NOTE_D4,
                                  NOTE_C4, NOTE_C4, NOTE_G4, NOTE_G4, NOTE_A4,
                                  NOTE_A4, NOTE_G4, NOTE_F4, NOTE_F4, NOTE_E4,
                                  NOTE_E4, NOTE_D4, NOTE_D4, NOTE_C4
                                 };
  static u8 u8CurrentFrequency = 0;
  static bool bBuzzerStart = FALSE;
#define U8_NUMBER_FREQUENCIES  (u8)(sizeof(au16Frequencies) / sizeof(u16))
  
  static u32 u32Counter = 1;
  static u16 au16CounterSpeed[] = {0, 1000, 100, 10};
  static u8 u8CurrentSpeed = 0;
  static u16 u16CurrentCounter = 0;
  u8 au8CounterAscii[11] = "0000000000";
  u8 u8Digits = 0;

  /* BUTTON0 toggles LEDs */
  if( WasButtonPressed(BUTTON0) )
  {
    ButtonAcknowledge(BUTTON0);
    u8LightNumber++;
    if(u8LightNumber > 8)
    {
      u8LightNumber = 0;
    }
    
    switch (u8LightNumber)
    {
    case 8:
      {
        LedOn(WHITE);
        /* Fall through */
      }
    case 7:
      {
        LedOn(PURPLE);
        /* Fall through */
      }
    case 6:
      {
        LedOn(BLUE);
        /* Fall through */
      }
    case 5:
      {
        LedOn(CYAN);
        /* Fall through */
      }
    case 4:
      {
        LedOn(GREEN);
        /* Fall through */
      }
    case 3:
      {
        LedOn(YELLOW);
        /* Fall through */
      }
    case 2:
      {
        LedOn(ORANGE);
        /* Fall through */
      }
    case 1:
      {
        LedOn(RED);
        break;
      }
    case 0:
      {
        LedOff(WHITE);
        LedOff(PURPLE);
        LedOff(BLUE);
        LedOff(CYAN);
        LedOff(GREEN);
        LedOff(YELLOW);
        LedOff(ORANGE);
        LedOff(RED);
        break;
      }
    default:
      {
        break;
      }
      
    } /* end switch */
            
  } /* End of BUTTON 0 test */

  /* BUTTON1 changes the counter speed */
  if( WasButtonPressed(BUTTON1) )
  {
    ButtonAcknowledge(BUTTON1);
    u8CurrentSpeed++;
    if(u8CurrentSpeed == sizeof(au16CounterSpeed) / sizeof(u16))
    {
      u8CurrentSpeed = 0;
    }

    u16CurrentCounter = 0;
  }
  
  if(u8CurrentSpeed != 0)
  {
    u16CurrentCounter++;
    if(u16CurrentCounter == au16CounterSpeed[u8CurrentSpeed])
    {
      u8Digits = NumberToAscii(u32Counter, au8CounterAscii);
      au8CounterAscii[u8Digits] = '\0';
      LCDMessage(LINE2_START_ADDR, au8CounterAscii);

      u16CurrentCounter = 0;
      u32Counter++;
    }
  }
  
  if( (u32Counter % 5000) == 0)
  {
    LCDClearChars(LINE2_START_ADDR, 20);
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

        LedOn(LCD_RED);
        LedPWM(LCD_GREEN, LED_PWM_20);
        LedOff(LCD_BLUE);
        break;

      case 3:
        u8Button2Test = 4;

        LedOn(LCD_RED);
        LedOn(LCD_GREEN);
        LedOff(LCD_BLUE);
        break;

      case 4:
        u8Button2Test = 5;

        LedOff(LCD_RED);
        LedOn(LCD_GREEN);
        LedOff(LCD_BLUE);
        break;
        
      case 5:
        u8Button2Test = 6;

        LedOff(LCD_RED);
        LedOn(LCD_GREEN);
        LedOn(LCD_BLUE);
        break;

      case 6:
        u8Button2Test = 7;

        LedOff(LCD_RED);
        LedOff(LCD_GREEN);
        LedOn(LCD_BLUE);
        break;

      case 7:
        u8Button2Test = 8;

        LedPWM(LCD_RED, LED_PWM_30);
        LedOff(LCD_GREEN);
        LedOn(LCD_BLUE);
        break;

      case 8:
        u8Button2Test = 9;

        LedOn(LCD_RED);
        LedOff(LCD_GREEN);
        LedOn(LCD_BLUE);
        break;
  
      case 9:
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
    bBuzzerStart = TRUE;
  }
    
  if(IsButtonPressed(BUTTON3))
  {
    if(bBuzzerStart)
    {
      PWMAudioSetFrequency(BUZZER1, au16Frequencies[u8CurrentFrequency]);
      PWMAudioSetFrequency(BUZZER2, au16Frequencies[u8CurrentFrequency]);
      PWMAudioOn(BUZZER1);
      PWMAudioOn(BUZZER2);

      bBuzzerStart = FALSE;
      u8CurrentFrequency++;
      if(u8CurrentFrequency == U8_NUMBER_FREQUENCIES)
      {
        u8CurrentFrequency = 0;
      }
    }
  }
  else
  {
    PWMAudioOff(BUZZER1);
    PWMAudioOff(BUZZER2);
  }
    
#if 0
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
#endif

#if 0  
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
#endif
  
} /* end BoardTestSM_Idle() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
void BoardTestSM_Error()
{
  static bool bErrorStateEntered = FALSE;
  
  /* Print error state entry message once; application hangs here */
  if(!bErrorStateEntered)
  {
   DebugPrintf("\n\r***BOARDTEST ERROR STATE***\n\n\r");
   bErrorStateEntered = TRUE;
  }
  
} /* end BoardTestSM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
