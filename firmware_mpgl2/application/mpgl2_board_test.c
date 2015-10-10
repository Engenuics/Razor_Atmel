/**********************************************************************************************************************
File: mpgl2_board_test.c                                                                

Description:
Application to completely test the MPGL2 Atmel development board, mpgl2-ehdw-01.

On startup:
- Observe all 4 discrete PLCC LEDs white (D13, D14, D15, D16)
- Observe all 4 discrete 0603 LEDs on ANT radio lit (D8 blue, D7 green, D6 yellow, D5 red)
- Observe both power and J-Link status LEDs lit green (D3, D18)
- Observe LCD backlight on
- Observe buzzer sound
- Observe RS-232 output of board startup sequence with 0 task init failures

Device check:
  - Type R, G, B, to toggle red, green, blue elements in RGB LEDs
  - BUTTON0: Toggle LEDs off and LCD pixel test on (LCD backlight stays on, verify all pixels lit)
  - BUTTON1: Turn on ANT radio and buzzer using test receiver to verify broadcast messages at 4Hz, 
             Frequency 50 (2.45GHz), Transmission type 55, Device ID 0xa5a5.
  - Captouch vertical slider slides on-screen logo up and down (serial output reports full range 0 to 255)
  - Captouch horizontal slider slides on-screen logo left and right (serial output reports full range 0 to 255)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_BoardTest"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32BoardTestFlags;                      /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern PixelBlockType G_sLcdClearWholeScreen;          /* From lcd_NHD-C12864LZ.c */
extern const u8 aau8EngenuicsLogoBlack[LCD_IMAGE_ROW_SIZE_50PX][LCD_IMAGE_COL_BYTES_50PX];   /* From lcd_bitmaps.c */

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
static fnCode_type BoardTest_pfnStateMachine;          /* The state machine function pointer */

static PixelBlockType BoardTest_sTestLogoPixelBlock;   /* Bitmap parameters for logo */

static u32 BoardTest_u32Flags;                         /* Application status flags */


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
  u8 au8BoardTestStartupMsg[] = "BoardTest task initialization ";

  /* Clear flags */
  BoardTest_u32Flags = 0;

  /* Start with all LEDs on */
#ifdef MPGL2_R01
  LedOn(BLUE);
  LedOn(GREEN);
  LedOn(RED);
  LedOn(YELLOW);

#else
  LedOn(BLUE0);
  LedOn(BLUE1);
  LedOn(BLUE2);
  LedOn(BLUE3);
  LedOn(GREEN0);
  LedOn(GREEN1);
  LedOn(GREEN2);
  LedOn(GREEN3);
  LedOn(RED0);
  LedOn(RED1);
  LedOn(RED2);
  LedOn(RED3);
#endif /* MPGL2_R01 */
  
  /* Activate CapTouch sensor an initialize readings */
  CapTouchOn();
  
  /* Draw the logo on screen */
  LcdClearPixels(&G_sLcdClearWholeScreen);

  BoardTest_sTestLogoPixelBlock.u16RowSize = LCD_IMAGE_ROW_SIZE_50PX;
  BoardTest_sTestLogoPixelBlock.u16ColumnSize = LCD_IMAGE_COL_SIZE_50PX;
  BoardTest_sTestLogoPixelBlock.u16RowStart = 7;
  BoardTest_sTestLogoPixelBlock.u16ColumnStart = 40;
  LcdLoadBitmap(&aau8EngenuicsLogoBlack[0][0], &BoardTest_sTestLogoPixelBlock);

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
  
  /* Set the buzzer frequency so it is ready to be enabled but keep it off for now */
  PWMAudioSetFrequency(BUZZER1, 500);
  PWMAudioOff(BUZZER1);

  
  /* If good initialization, set state to Idle */
  DebugPrintf(au8BoardTestStartupMsg);
  if( 1 /* Add condition for good init */)
  {
    DebugPrintf(G_au8MessageOK);
    BoardTest_pfnStateMachine = BoardTestSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    DebugPrintf(G_au8MessageFAIL);
    BoardTest_pfnStateMachine = BoardTestSM_FailedInit;
  }

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
  BoardTest_pfnStateMachine();

} /* end BoardTestRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------------------------------------
Function: BoardTestUpdateLogoPosition

Description:
Reads the current captouch slider locations and translates that to the current location of the logo.

Requires:
  - Logo size is 50x50

Promises:
  - Horizontal reading 0 - 255 is translated to a range of 0 - 77 (valid for top left pixel location)
    and set to BoardTest_TestLogoPixelBlock.u16ColumnStart
  - Vertical reading 0 - 255 is translated to a range of 0 - 13 (valid for top left pixel location)
    and set to BoardTest_TestLogoPixelBlock.u16RowStart
*/
bool BoardTestUpdateLogoPosition(void)
{
  u8 u8Horizontal, u8Vertical;
  bool bNewPosition = FALSE;
  
  u8Horizontal = CaptouchCurrentHSlidePosition();
  u8Vertical   = CaptouchCurrentVSlidePosition();
  
  /* The horizontal calculation pads the top and bottom of the range with 0 and 78 */
  if(u8Horizontal < 14)
  {
    u8Horizontal = 0;
  }
  else if (u8Horizontal < 246)
  {
    u8Horizontal = (u8Horizontal - 4) / 3;
  }
  else
  {
    u8Horizontal = 78;
  }
  
  /* The vertical position works out such that only a single calculation is required */
  u8Vertical = (u8Vertical + 5) / 18;

  /* Update positions if they have changed */
  if(BoardTest_sTestLogoPixelBlock.u16ColumnStart != u8Horizontal)
  {
    BoardTest_sTestLogoPixelBlock.u16ColumnStart = u8Horizontal;
    bNewPosition = TRUE;
  }
  
  if(  BoardTest_sTestLogoPixelBlock.u16RowStart != u8Vertical)
  {
    BoardTest_sTestLogoPixelBlock.u16RowStart = u8Vertical;
    bNewPosition = TRUE;
  }
  
  return(bNewPosition);
  
} /* end BoardTestUpdateLogoPosition() */


/***********************************************************************************************************************
State Machine Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
static void BoardTestSM_Idle(void)
{
  static bool bButton0Test = FALSE;
//  static bool bForceButton1Test = TRUE;
  static u32 u32LogoUpdateTimer = 0;
  static u8 au8TestMessage[] = {0, 0, 0, 0, 0, 0, 0, 0};
  static u8 au8DataMessage[] = "ANT data: ";
  u8 au8DataContent[26];
  AntChannelStatusType eAntCurrentState;
  
  /* Update logo position if it's time */
  if( IsTimeUp(&u32LogoUpdateTimer, LOGO_UPDATE_PERIOD) )
  {
    u32LogoUpdateTimer = G_u32SystemTime1ms;
    
    /* Update new position -- returns TRUE if a new position is required */
    if( BoardTestUpdateLogoPosition() )
    {
      /* Clear screen and update with latest logo position */
      LcdClearScreen();
      LcdLoadBitmap(&aau8EngenuicsLogoBlack[0][0], &BoardTest_sTestLogoPixelBlock);
    }
  }

  /* BUTTON0 toggles LEDs off and LCD pixel test on */
  if( WasButtonPressed(BUTTON0) )
  {
    ButtonAcknowledge(BUTTON0);
    
    /* If test is active, deactivate it, put all LEDs back on and move states to get an LCD Command in  */
    if(bButton0Test)
    {
      bButton0Test = FALSE;
      LcdCommand(LCD_PIXEL_TEST_OFF);

#ifdef MPGL2_R01
      LedOn(BLUE);
      LedOn(GREEN);
      LedOn(RED);
      LedOn(YELLOW);
      
#else
      LedOn(BLUE0);
      LedOn(BLUE1);
      LedOn(BLUE2);
      LedOn(BLUE3);
      LedOn(GREEN0);
      LedOn(GREEN1);
      LedOn(GREEN2);
      LedOn(GREEN3);
      LedOn(RED0);
      LedOn(RED1);
      LedOn(RED2);
      LedOn(RED3);
#endif /* MPGL2_R01 */
      
      BoardTest_pfnStateMachine = BoardTestSM_WaitPixelTestOff;
    }
    /* Else activate it: turn all LEDs off and move states to get an LCD Command in */
    else
    {
      bButton0Test = TRUE;

#ifdef MPGL2_R01
      LedOff(BLUE);
      LedOff(GREEN);
      LedOff(RED);
      LedOff(YELLOW);
#else
      LedOff(BLUE0);
      LedOff(BLUE1);
      LedOff(BLUE2);
      LedOff(BLUE3);
      LedOff(GREEN0);
      LedOff(GREEN1);
      LedOff(GREEN2);
      LedOff(GREEN3);
      LedOff(RED0);
      LedOff(RED1);
      LedOff(RED2);
      LedOff(RED3);
#endif /* MPGL2_R01 */
      
      BoardTest_pfnStateMachine = BoardTestSM_WaitPixelTestOn;
    }
  } /* End of BUTTON 0 test */

/* BUTTON1 toggles the radio and buzzer test.  When the button is pressed,
  an open channel request is made.  The system monitors _ANT_FLAGS_CHANNEL_OPEN
  to control wether or not the buzzer is on. */
  
  /* Toggle the beeper and ANT radio on BUTTON1 */
  if( WasButtonPressed(BUTTON1) )
  {
//    bForceButton1Test = TRUE;
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
  
} /* end BoardTestSM_Idle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to wait for successful pixel test on response */
static void BoardTestSM_WaitPixelTestOn(void)          
{
  if(LcdCommand(LCD_PIXEL_TEST_ON))
  {
    BoardTest_pfnStateMachine = BoardTestSM_Idle;
  }
    
} /* end BoardTestSM_WaitPixelTestOn() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* State to wait for successful pixel test on response */
static void BoardTestSM_WaitPixelTestOff(void)          
{
  if(LcdCommand(LCD_PIXEL_TEST_OFF))
  {
    BoardTest_pfnStateMachine = BoardTestSM_Idle;
  }
    
} /* end BoardTestSM_WaitPixelTestOf() */


#if 0 /* Don't need this just yet... */
/*--------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void BoardTestSM_Error(void)          
{
  
} /* end BoardTestSM_Error() */
#endif

/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void BoardTestSM_FailedInit(void)          
{
    
} /* end BoardTestSM_FailedInit() */
     

/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
