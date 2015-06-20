/***********************************************************************************************************************
File: pong_atmel.c     

Description:
Classic Pong played over the ANT radio.
***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
* Pong Options
***********************************************************************************************************************/
#define SINGLE_PLAYER_MODE  1
//#define NO_PLAYER_MODE      1
//#define BALL_STOPPED        1

/***********************************************************************************************************************
* "Global Global" Definitions (public variables for data transfer between files)
* Global Global variable names shall always start with "G_<type>Pong".
***********************************************************************************************************************/
/*----------------------------------------------------------------------------*/
/* New variables */
fnCode_type G_PongStateMachine;                        /* The Pong application */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                         /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                          /* From board-specific source file */

extern volatile u32 G_u32SystemFlags;                           /* From main.c */
extern volatile u32 G_u32ApplicationFlags;                      /* From main.c */

extern u8 G_aau8LcdRamImage[LCD_IMAGE_ROWS][LCD_IMAGE_COLUMNS]; /* From lcd_NHD-C12864LZ.c */
extern PixelBlockType G_sLcdClearWholeScreen;                   /* From lcd_NHD-C12864LZ.c */

extern u32 G_u32AntFlags;                                       /* From ant.c */
extern AntSetupDataType G_stAntSetupData;                       /* From ant.c */
extern AntDataMessageStructType *G_sAntDataIncomingMsgList;     /* From ant.c */
extern AntDataMessageStructType *G_sAntDataOutgoingMsgList;     /* From ant.c */

extern const u8 aau8PongPaddleTop[LCD_IMAGE_PADDLE_ROW_SIZE][LCD_IMAGE_PADDLE_COL_SIZE];    /* From lcd_bitmaps.h */
extern const u8 aau8PongPaddleBottom[LCD_IMAGE_PADDLE_ROW_SIZE][LCD_IMAGE_PADDLE_COL_SIZE]; /* From lcd_bitmaps.h */
extern const u8 aau8PongBall[LCD_IMAGE_BALL_ROW_SIZE][LCD_IMAGE_BALL_COL_SIZE];             /* From lcd_bitmaps.h */
extern const u8 aau8PlayerArrow[LCD_IMAGE_ARROW_ROW_SIZE][LCD_IMAGE_ARROW_COL_SIZE];        /* From lcd_bitmaps.h */
//extern const u8 aau8AntLogo[LCD_IMAGE_YSIZE][LCD_IMAGE_XSIZE];                              /* From lcd_bitmaps.h */


/***********************************************************************************************************************
Global variable definitions with scope limited to this task.
Variable names shall start with "Pong_<type>" and be declared as static.
***********************************************************************************************************************/
static u32 Pong_u32Flags = _PONG_GAME_VIRGIN;  
//static u32 Pong_u32GameStatus;
static u32 Pong_u32Timeout;                 /* Timer for event timeouts */
static u32 Pong_u32GameStartDelay;          /* Timer for synchronized game start */

/* The main game data for Pong */
static struct PongGameData
{
  u8 u8Paddle0LocationLast;       /* Previous row location of Paddle 0 to erase */
  u8 u8Paddle0LocationCurrent;    /* Current row location of Paddle 0 to write */
  u8 u8Paddle1LocationLast;       /* Previous row location of Paddle 1 to erase */
  u8 u8Paddle1LocationCurrent;    /* Curret row location of Paddle 1 to write */
  u8 u8BallLocationRowLast;       /* Previous row location of ball to erase */
  u8 u8BallLocationColumnLast;    /* Previous column location of ball to erase */
  u8 u8BallLocationRowCurrent;    /* Current row location of ball to write */
  u8 u8BallLocationColumnCurrent; /* Current column location of ball to write */
  s8 s8BallVectorRow;             /* -1 or +1 to indicate which direction ball is moving (row) */
  s8 s8BallVectorColumn;          /* -1 or +1 to indicate which direction ball is moving (column) */
  u8 u8BallMotionRow;             /* Number of vertical pixels to move with each update */
  u8 u8BallMotionColumn;          /* Number of horizontal pixels to move with each update */
  u8 u8Player0Score;              /* Current Player 0 score */
  u8 u8Player1Score;              /* Current Player 1 score */
} Pong_stGameData;

/* Message counters */
static u8 Pong_u8MessageNumberLow;
static u8 Pong_u8MessageNumberHigh;  
static u8 Pong_u8GameMessageNumber;

/* Text strings used throughout application */
static const u8 Pong_au8Player0Msg[]   = "PLAYER0";            
static const u8 Pong_au8Player1Msg[]   = "PLAYER1";            
static const u8 Pong_au8ScoredMsg[]    = "Scored!";            
static const u8 Pong_au8WinnerMsg[]    = "Winner!";            
static const u8 Pong_au8StartMsg[]     = "B1 to start";  
static const u8 Pong_au8AntPongMsg[]   = "ANT PONG!!";  
static const u8 Pong_au8DeviceMsg[]    = "Device ID: ";
static const u8 Pong_au8ReadyMsg[]     = "Devices ready";
static const u8 Pong_au8SearchingMsg[] = "Searching...";

#ifndef SINGLE_PLAYER_MODE
static const u8 Pong_au8Player0MsgSelect[]  = "Press B0 for Player 0";
static const u8 Pong_au8Player1MsgSelect[]  = "Press B1 for Player 1";
#else
static const u8 Pong_au8Player0MsgSelect[]  = "Single player mode";
static const u8 Pong_au8Player1MsgSelect[]  = "B0 or B1 to start";
#endif /* SINGLE_PLAYER_MODE */

/* Error strings.  Error type messages are indexed by Pong Error Codes */
static u8 Pong_au8ErrorMsg[] = "Error!";
static u8 Pong_aau8ErrorMessages[][LCD_MAX_SMALL_CHARS] = { {"Communication timeout"} };

/* Pong message protocol -- see MessageProtocol.xlsx */
static u8 Pong_au8dleMessage[]  = {PONG_MESSAGE_IDLE, 0xAA, 0x55, 0x00, 0x00, STATUS_NOT_READY, 0xFF, 0xFF};
static u8 Pong_au8DataMessage[] = {PONG_MESSAGE_DATA, BALL_VECTOR_ROW_DEFAULT, BALL_VECTOR_COLUMN_DEFAULT, 
                                   BALL_MOVEMENT_ROWS, BALL_MOVEMENT_COLUMNS, STATUS_NOT_READY, 0xFF, 0xFF};
static u8 Pong_au8ReadyMsgMessage[] = {PONG_MESSAGE_READY, PONG_P0_STARTING_SCORE, PONG_P1_STARTING_SCORE,
                                       STATUS_NOT_READY, STATUS_NOT_READY, 0x00, 0xFF, 0xFF};
static u8 Pong_au8GameMessage[] = {PONG_MESSAGE_GAME, BALL_VECTOR_ROW_DEFAULT, BALL_VECTOR_COLUMN_DEFAULT,
                                   SLIDER_HORIZONTAL_START_PONG, SLIDER_HORIZONTAL_START_PONG, 
                                   LCD_BALL_START_ROW, LCD_BALL_START_COLUMN, 0x00};

static u8 Pong_u8GameStatus;   /* Status byte in GAME message */


/***********************************************************************************************************************
* Function Definitions
***********************************************************************************************************************/

/*----------------------------------------------------------------------------
Function: PongInitialize

Description:
Sets the starting point for the Pong program including:
  - LEDs
  - Slider positions
  - Pong_stGameData
  - ANT radio channel configuration

The pong splash screen is loaded and the application state machine takes over.

Requires:
  - LCD is initialized and ready (no error flag)
  - ANT radio is initialized and ready (no error flag)
  - Debug output is initialized

Promises:
  - Outputs system status and hands-off to the Idle state
*/
void PongInitialize(void)
{
  u8 au8PongInit[] = "Initializing Pong application\r\n";
  u8 aau8String[]  = "00000";
  u16 u16DeviceID;
  PixelAddressType sPixelAddress;

  /* Announce on the debug port that the Pong app is starting */
  DebugPrintf(au8PongInit;

#if 0
  /* MPGL2 part 2: force message only if this is the very first time this runs */
  /* Only force the message if the Pong app is starting for the first time. */
  if(Pong_u32Flags & _PONG_GAME_VIRGIN)
  {
    MsgSenderForceSend();
  }
#endif
  
  /* Initialize Pong application flag bits */
  Pong_u32Flags = 0;

  /* Setup all the LEDs */
  LedOn(RED);
  LedOn(YELLOW);
  LedOn(GREEN);
  LedOn(BLUE);
   
  /* Clear the LCD RAM and load the starting Pong game screen */
  LcdClearPixels(&G_sLcdClearWholeScreen);
  
  /* !!!!! Load the Pong splash image or starting animation image */
  
  /* Grab the ASCII value of the device serial number with a mini version of itoa() specific
  to a 5-digit number (with leading zeros) */
  u16DeviceID = ANT_SERIAL_PONG;
  for(u8 i = 0; i < 5; i++)
  {       
    aau8String[4 - i] = u16DeviceID % 10 + '0';   
    u16DeviceID /= 10;
  }
  
  /* Load the text */
  sPixelAddress.u16PixelRowAddress = LCD_SMALL_FONT_LINE2;
  sPixelAddress.u16PixelColumnAddress = 0;
  LcdLoadString(&Pong_au8AntPongMsg[0], LCD_FONT_SMALL, &sPixelAddress);

  sPixelAddress.u16PixelRowAddress = LCD_SMALL_FONT_LINE5;
  LcdLoadString(&Pong_au8DeviceMsg[0], LCD_FONT_SMALL, &sPixelAddress);

  sPixelAddress.u16PixelColumnAddress = 60;
  LcdLoadString(&aau8String[0], LCD_FONT_SMALL, &sPixelAddress);

  sPixelAddress.u16PixelRowAddress = LCD_SMALL_FONT_LINE6;
  sPixelAddress.u16PixelColumnAddress = 10;
  LcdLoadString(&Pong_au8Player0MsgSelect[0], LCD_FONT_SMALL, &sPixelAddress);

  sPixelAddress.u16PixelRowAddress = LCD_SMALL_FONT_LINE7;
  LcdLoadString(&Pong_au8Player1MsgSelect[0], LCD_FONT_SMALL, &sPixelAddress);
  
  /* Initialze game data */
  PongResetGameObjects();
  Pong_stGameData.s8BallVectorRow    = BALL_VECTOR_ROW_DEFAULT;
  Pong_stGameData.s8BallVectorColumn = BALL_VECTOR_COLUMN_DEFAULT;
  Pong_stGameData.u8BallMotionRow    = BALL_MOVEMENT_ROWS;
  Pong_stGameData.u8BallMotionColumn = BALL_MOVEMENT_COLUMNS;
  Pong_stGameData.u8Player0Score     = PONG_P0_STARTING_SCORE;
  Pong_stGameData.u8Player1Score     = PONG_P1_STARTING_SCORE;

  Pong_u32Flags = _PONG_FLAGS_MASTER;

  /* Configure the ANT radio */
  G_stAntSetupData.AntChannel          = ANT_CHANNEL_PONG;
  G_stAntSetupData.AntChannelType      = ANT_CHANNEL_TYPE_PONG;
  G_stAntSetupData.AntNetwork          = ANT_NETWORK_PONG;
  G_stAntSetupData.AntSerialLo         = ANT_SERIAL_LO_PONG;
  G_stAntSetupData.AntSerialHi         = ANT_SERIAL_HI_PONG;
  G_stAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_PONG;
  G_stAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_PONG;
  G_stAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_PONG;
  G_stAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_PONG;
  G_stAntSetupData.AntFrequency        = ANT_FREQUENCY_PONG;
  G_stAntSetupData.AntTxPower          = ANT_TX_POWER_PONG;
 
  /* Set the Pong application starting state */
  Pong_u8GameMessageNumber = 0;
  PongResetGameFlags();
  G_PongStateMachine = PongSM_Idle;
  
} /* end PongInitialize */


/*----------------------------------------------------------------------------
Function: PongResetGameObjects

Description:
Sets the initial values for the moveable item locations.

Requires:
  -

Promises:
  - Paddle0, Paddle1 and Ball last and current positions in Pong_stGameData are set to starting states
*/
void PongResetGameObjects(void)
{
  Pong_stGameData.u8Paddle0LocationLast       = SLIDER_HORIZONTAL_START_PONG;
  Pong_stGameData.u8Paddle0LocationCurrent    = SLIDER_HORIZONTAL_START_PONG;
  Pong_stGameData.u8Paddle1LocationLast       = SLIDER_HORIZONTAL_START_PONG;
  Pong_stGameData.u8Paddle1LocationCurrent    = SLIDER_HORIZONTAL_START_PONG;
  
  Pong_stGameData.u8BallLocationRowLast       = LCD_BALL_START_ROW;
  Pong_stGameData.u8BallLocationColumnLast    = LCD_BALL_START_COLUMN;
  Pong_stGameData.u8BallLocationRowCurrent    = LCD_BALL_START_ROW;
  Pong_stGameData.u8BallLocationColumnCurrent = LCD_BALL_START_COLUMN;
  
  PongUpdateGameMessage();

} /* end PongResetGameObjects() */


/*----------------------------------------------------------------------------
Function: PongResetGameFlags

Description:
Resets all the game flags.

Requires:
  - Game play is stopped

Promises:
  - All game flags in Pong_u32Flags are cleared
*/
void PongResetGameFlags(void)
{
  Pong_u32Flags &= ~PONG_GAME_FLAG_MASK;

  Pong_au8dleMessage[IDLE_MSG_STATUS] = STATUS_NOT_READY;
  Pong_au8DataMessage[DATA_MSG_STATUS] = STATUS_NOT_READY;
  Pong_au8ReadyMsgMessage[READY_MSG_STATUS_P0_BYTE] = STATUS_NOT_READY;
  Pong_au8ReadyMsgMessage[READY_MSG_STATUS_P1_BYTE] = STATUS_NOT_READY;
  
  Pong_u8GameStatus = 0;
  PongUpdateGameMessage();

} /* end PongResetGameFlags() */


/*----------------------------------------------------------------------------
Function: PongSetBallVector

Description:
Randomly sets the ball vector used to determine which way the ball starts
going first.

Requires:
  - _PONG_FLAGS_MASTER bit in Pong_u32Flags correctly set

Promises:
  - If the device is the master, Pong_au8DataMessage[DATA_MSG_VECTOR_ROW_BYTE] and 
    Pong_au8DataMessage[DATA_MSG_VECTOR_COLUMN_BYTE] are initialized to new random values
    based on the current ms tick - the user interface provides plenty of entropy for this
    to be random.
  - DATA message is updated with the new ball vector
*/
void PongSetBallVector(void)
{
  /* Initialize to garbage values */
  Pong_stGameData.s8BallVectorRow = 99;
  Pong_stGameData.s8BallVectorColumn = 99;

  /* For Master, randomize the starting ball vector so the ball doesn't always start the same way */
  if(Pong_u32Flags & _PONG_FLAGS_MASTER)
  {
    Pong_stGameData.s8BallVectorRow = -1;
    if(G_u32SystemTime1ms & 0x00000001)
    {
      Pong_stGameData.s8BallVectorRow = 1;
    }
    Pong_stGameData.s8BallVectorColumn = 1;
    if(G_u32SystemTime1ms & 0x00000010)
    {
      Pong_stGameData.s8BallVectorColumn = -1;
    }
  }
  
  /* Load the vectors; for slave, the vectors remain garbage so they will never initially match the master */
  Pong_au8DataMessage[DATA_MSG_VECTOR_ROW_BYTE] = Pong_stGameData.s8BallVectorRow;
  Pong_au8DataMessage[DATA_MSG_VECTOR_COLUMN_BYTE] = Pong_stGameData.s8BallVectorColumn;

} /* end PongSetBallVector() */


/*----------------------------------------------------------------------------
Function: PongDrawGameScreen

Description:
Draws the outline and score boxes with player names for the Pong arena in LCD RAM.

Requires:
  - LCD is initialized 

Promises:
  - LCD RAM is loaded with the game screen data
*/
void PongDrawGameScreen(void)
{
  PixelAddressType sPixelAddress;
  
  /* Draw the vertical score box lines and the far board edge line */
  for(u16 i = 0; i < LCD_ROWS; i++)
  {
    sPixelAddress.u16PixelRowAddress = i;

    sPixelAddress.u16PixelColumnAddress = LCD_GAMESCREEN_SCOREBOX_COL_START;
    LcdSetPixel(&sPixelAddress); 

    sPixelAddress.u16PixelColumnAddress = LCD_GAMESCREEN_SCOREBOX_COL_END;
    LcdSetPixel(&sPixelAddress); 

    sPixelAddress.u16PixelColumnAddress = LCD_GAMESCREEN_EDGE_COL_RIGHT;
    LcdSetPixel(&sPixelAddress); 
  }
    
  /* Draw the four horizontal lines to complete the score boxes */
  for(u16 i = 0; i < LCD_GAMESCREEN_SCOREBOX_COL_END; i++)
  {
    sPixelAddress.u16PixelColumnAddress = i;

    sPixelAddress.u16PixelRowAddress = LCD_GAMESCREEN_LINE_SCOREBOX_ROW1;
    LcdSetPixel(&sPixelAddress); 

    sPixelAddress.u16PixelRowAddress = LCD_GAMESCREEN_LINE_SCOREBOX_ROW2;
    LcdSetPixel(&sPixelAddress); 

    sPixelAddress.u16PixelRowAddress = LCD_GAMESCREEN_LINE_SCOREBOX_ROW3;
    LcdSetPixel(&sPixelAddress); 

    sPixelAddress.u16PixelRowAddress = LCD_GAMESCREEN_LINE_SCOREBOX_ROW4;
    LcdSetPixel(&sPixelAddress); 
  }
  
  /* Write the two "PLAYER" words */
  sPixelAddress.u16PixelColumnAddress = LCD_PLAYER_TEXT_COLUMN;
 
  sPixelAddress.u16PixelRowAddress = LCD_PLAYER0_TEXT_ROW;
  LcdLoadString(&Pong_au8Player0Msg[0], LCD_FONT_SMALL, &sPixelAddress);
  
  sPixelAddress.u16PixelRowAddress = LCD_PLAYER1_TEXT_ROW;
  LcdLoadString(&Pong_au8Player1Msg[0], LCD_FONT_SMALL, &sPixelAddress);

#ifdef SINGLE_PLAYER_MODE
  /* Draw a horizontal line across the entire Player game space */
  sPixelAddress.u16PixelRowAddress = 60;
  for(u16 i = LCD_GAMESCREEN_EDGE_COL_LEFT; i < LCD_GAMESCREEN_EDGE_COL_RIGHT; i++)
  {
    sPixelAddress.u16PixelColumnAddress = i;
    LcdSetPixel(&sPixelAddress); 
  }
#endif /* SINGLE_PLAYER_MODE */

#ifdef NO_PLAYER_MODE
  /* Draw the line for Player 0 */
  sPixelAddress.u16PixelRowAddress = 3;
  for(u16 i = LCD_GAMESCREEN_EDGE_COL_LEFT; i < LCD_GAMESCREEN_EDGE_COL_RIGHT; i++)
  {
    sPixelAddress.u16PixelColumnAddress = i;
    LcdSetPixel(&sPixelAddress); 
  }

  /* Draw the line for Player 1 */
  sPixelAddress.u16PixelRowAddress = 60;
  for(u16 i = LCD_GAMESCREEN_EDGE_COL_LEFT; i < LCD_GAMESCREEN_EDGE_COL_RIGHT; i++)
  {
    sPixelAddress.u16PixelColumnAddress = i;
    LcdSetPixel(&sPixelAddress); 
  }
#endif

} /* end PongDrawGameScreen() */


/*----------------------------------------------------------------------------
Function: PongUpdateScoresLCD

Description:
Clears the existing scores and writes the current scores to the LCD RAM.

Requires:
  - LCD is initialized
  - Game scores are never greater than 9

Promises:
  - LCD RAM is updated to hold the current game score
*/
void PongUpdateScoresLCD(void)
{
  PixelBlockType sPixelsToUpdate;
  PixelAddressType sCharAddress;
  u8 au8Score0[] = "0";
  u8 au8Score1[] = "0";
  
  /* Set common parameters for both player score updates */
  sCharAddress.u16PixelColumnAddress = LCD_PLAYER_SCORE_COLUMN;
  sPixelsToUpdate.u16ColumnStart     = LCD_PLAYER_SCORE_COLUMN;
  sPixelsToUpdate.u16RowSize         = LCD_BIG_FONT_ROWS;
  sPixelsToUpdate.u16ColumnSize      = LCD_BIG_FONT_COLUMNS;

  /* Erase Player 0's score */
  sPixelsToUpdate.u16RowStart = LCD_PLAYER0_SCORE_ROW;
  LcdClearPixels(&sPixelsToUpdate);

  /* Draw Player 0's current score */
  au8Score0[0] = Pong_stGameData.u8Player0Score + NUMBER_ASCII_TO_DEC;
  sCharAddress.u16PixelRowAddress = LCD_PLAYER0_SCORE_ROW;
  LcdLoadString(au8Score0, LCD_FONT_BIG, &sCharAddress);

  /* Erase Player 1's score */
  sPixelsToUpdate.u16RowStart = LCD_PLAYER1_SCORE_ROW;
  LcdClearPixels(&sPixelsToUpdate);

  /* Draw Player 1's current score */
  au8Score1[0] = Pong_stGameData.u8Player1Score + NUMBER_ASCII_TO_DEC;
  sCharAddress.u16PixelRowAddress = LCD_PLAYER1_SCORE_ROW;
  LcdLoadString(au8Score1, LCD_FONT_BIG, &sCharAddress);
  
} /* end PongUpdateScoresLCD() */


/*----------------------------------------------------------------------------
Function: PongProcessGameMessages

Description:
Reads any messages in G_sAntDataIncomingMsgList and uses the data accordingly.

Requires:
  - G_sAntDataIncomingMsgList is a valid linked list of AntDataMessageStructType;
    the pointer is NULL if the list is empty.

Promises:
  - Handles IDLE, DATA, READY, GAME and ANT_TICK messages
  - The processed message is dequeued from the list
  - All messages in the queue are processed
  - Does not touch messages that are not part of the Pong message protocol
*/
void PongProcessGameMessages(void)
{
  static u8 u8ConsecutiveMessages = 0;
  static u16 u16LastMessageNumber;
  static u16 u16CurrentMessageNumber;
  static u8 u8CurrentGameMessage;
  static u8 u8LastGameMessage;
  static u8 u8MissedMessageCounter;
  
  AntDataMessageStructType *psMessageToProcess;
  AntDataMessageStructType *psNextMessageToProcess = G_sAntDataIncomingMsgList;
  
  /* Run through the Data list looking for Pong messages */
  while(psNextMessageToProcess != NULL)
  {
    /* Set pointer to current message and advance next pointer to next message */
    psMessageToProcess = psNextMessageToProcess;
    psNextMessageToProcess = psNextMessageToProcess->psNextAntDataMessage;

    /* Check first if we have a GAME message */
    if(psMessageToProcess->au8MessageData[INDEX_PONG_MESSAGE_ID] & GAME_MESSAGE_BIT_MASK)
    {
      /* Capture the message number out of the ID byte */
      u8LastGameMessage = u8CurrentGameMessage;
      u8CurrentGameMessage = (psMessageToProcess->au8MessageData[INDEX_PONG_MESSAGE_ID]) & ~GAME_MESSAGE_BIT_MASK;
      
      /* If game messages are being received, be sure the slave is in the GAME state. */
      Pong_u32Flags |= (_PONG_STATUS_ADVANCE_READY | _PONG_RECEIVING_GAME_MESSAGES);
      
      /* Clear the CRITICAL flag (that may or may not be set) */
      //Pong_u32Flags &= ~_PONG_CRITICAL_MSG_TX;
      
      /* Flag if the remote device indicates the game is in progress */ 
      if(psMessageToProcess->au8MessageData[PONG_MSG_STATUS_BYTE] & _PONG_MSG_STATUS_IN_PROGRESS )
      {
        Pong_u32Flags |= _PONG_STATUS_REMOTE_GAMEON;
      }
      else
      {
        Pong_u32Flags &= ~_PONG_STATUS_REMOTE_GAMEON;
      }
      
      /* Choose the next action based on the current state of the game messaging system */
      
      
      /* Update remote device paddle position */
      if(Pong_u32Flags & _PONG_FLAGS_MASTER)
      {
        Pong_stGameData.u8Paddle1LocationCurrent = psMessageToProcess->au8MessageData[PONG_MSG_PADDLE1_BYTE];
      }
      else
      {
        Pong_stGameData.u8Paddle0LocationCurrent = psMessageToProcess->au8MessageData[PONG_MSG_PADDLE0_BYTE];
      }
      
      /* Check for scoring */
      if(Pong_u32Flags & _PONG_FLAGS_MASTER)
      {
        if(psMessageToProcess->au8MessageData[PONG_MSG_STATUS_BYTE] & _PONG_MSG_STATUS_P0_SCORED)
        {
          /* If a point was scored, then flag the bit and update the ball position */
          Pong_u32Flags |= _PONG_STATUS_SCORE_P0;
          Pong_stGameData.u8BallLocationRowCurrent = psMessageToProcess->au8MessageData[PONG_MSG_BALL_ROW_BYTE];
          Pong_stGameData.u8BallLocationColumnCurrent = psMessageToProcess->au8MessageData[PONG_MSG_BALL_COLUMN_BYTE];
        }
      }
      else 
      {
        if(psMessageToProcess->au8MessageData[PONG_MSG_STATUS_BYTE] & _PONG_MSG_STATUS_P1_SCORED)
        {
          /* If a point was scored, then flag the bit and update the ball position */
          Pong_u32Flags |= _PONG_STATUS_SCORE_P1;
          Pong_stGameData.u8BallLocationRowCurrent = psMessageToProcess->au8MessageData[PONG_MSG_BALL_ROW_BYTE];
          Pong_stGameData.u8BallLocationColumnCurrent = psMessageToProcess->au8MessageData[PONG_MSG_BALL_COLUMN_BYTE];
        }
      }
      
      AntDeQueueDataMessage(psMessageToProcess, &G_sAntDataIncomingMsgList);
    }
    else
    {
      /* Otherwise, we have a regular message so parse it out and handle it */
      switch( psMessageToProcess->au8MessageData[INDEX_PONG_MESSAGE_ID] )
      {
        case PONG_MESSAGE_IDLE:
        { 
          /* Look for enough consecutive IDLE messages before setting a flag */
          u16CurrentMessageNumber = ( ( (u16)(psMessageToProcess->au8MessageData[IDLE_MSG_HIGH_BYTE]) << 8)
                                      | psMessageToProcess->au8MessageData[IDLE_MSG_LOW_BYTE] );
          if( (u16CurrentMessageNumber - u16LastMessageNumber) == 1)
          {
            u8ConsecutiveMessages++;
          }
          else
          {
            u8ConsecutiveMessages = 0;
          }
          
          u16LastMessageNumber = u16CurrentMessageNumber;
  
          if(u8ConsecutiveMessages >= CONSECUTIVE_MSGS_REQUIRED)
          {
            /* If the master has received enough messages from the slave, then it
            sets the status byte in the IDLE message. */
            if(Pong_u32Flags & _PONG_FLAGS_MASTER)
            {
              Pong_au8dleMessage[IDLE_MSG_STATUS] = STATUS_READY;
            }
            /* If the slave sees enough messages from the Master where the status
            is READY then it may proceed. */
            else
            {
              if(psMessageToProcess->au8MessageData[IDLE_MSG_STATUS] == STATUS_READY)
              {
                Pong_u32Flags |= _PONG_STATUS_ADVANCE_IDLE;
              }
            }
            u8ConsecutiveMessages = 0;
          }
          
          AntDeQueueDataMessage(psMessageToProcess, &G_sAntDataIncomingMsgList);
          break;
        } /* end case PONG_MESSAGE_IDLE */
    
        case PONG_MESSAGE_DATA:
        { 
          /* If data messages received, be sure to advance IDLE state and clear GAMEON */
          Pong_u32Flags |= _PONG_STATUS_ADVANCE_IDLE;
          Pong_u32Flags &= ~_PONG_STATUS_REMOTE_GAMEON;
          
          /* Look for enough consecutive DATA messages before setting a flag */
          u16CurrentMessageNumber = ( ( (u16)(psMessageToProcess->au8MessageData[IDLE_MSG_HIGH_BYTE]) << 8)
                                      | psMessageToProcess->au8MessageData[IDLE_MSG_LOW_BYTE] );
          if( (u16CurrentMessageNumber - u16LastMessageNumber) == 1)
          {
            u8ConsecutiveMessages++;
          }
          else
          {
            u8ConsecutiveMessages = 0;
          }
          
          u16LastMessageNumber = u16CurrentMessageNumber;
          
          if(u8ConsecutiveMessages >= CONSECUTIVE_MSGS_REQUIRED)
          {
            /* If the master has received enough messages from the slave, then it
            sets the status byte in the DATA message. */
            if(Pong_u32Flags & _PONG_FLAGS_MASTER)
            {
              /* Could check that game data matches but for now do nothing */
              Pong_au8DataMessage[DATA_MSG_STATUS] = STATUS_READY;
            }
            /* If the slave sees enough messages from the master where the status
            is READY then it may proceed. */
            else
            {
              if(psMessageToProcess->au8MessageData[DATA_MSG_STATUS] == STATUS_READY)
              {
                /* Update the local Data message */
                Pong_au8DataMessage[DATA_MSG_VECTOR_ROW_BYTE]    = psMessageToProcess->au8MessageData[DATA_MSG_VECTOR_ROW_BYTE];
                Pong_au8DataMessage[DATA_MSG_VECTOR_COLUMN_BYTE] = psMessageToProcess->au8MessageData[DATA_MSG_VECTOR_COLUMN_BYTE];
                Pong_au8DataMessage[DATA_MSG_MOTION_ROW_BYTE]    = psMessageToProcess->au8MessageData[DATA_MSG_MOTION_ROW_BYTE];
                Pong_au8DataMessage[DATA_MSG_MOTION_COLUMN_BYTE] = psMessageToProcess->au8MessageData[DATA_MSG_MOTION_COLUMN_BYTE];
                Pong_u32Flags |= _PONG_STATUS_ADVANCE_DATA;
              }
            }
            u8ConsecutiveMessages = 0;
          }
         
          AntDeQueueDataMessage(psMessageToProcess, &G_sAntDataIncomingMsgList);
          break;
        } /* end case PONG_MESSAGE_DATA */
    
        case PONG_MESSAGE_READY:
        { 
          /* If ready messages being received, be sure to advance master to DATA state */
          Pong_u32Flags |= _PONG_STATUS_ADVANCE_DATA;
  
          /* By now, communication can be assumed reliable such that consecutive messages are not required */        
  
          /* When the master sees that the slave is ready, it can progress states. 
          This should not occur until the slave receives STATUS_READY from the master. */
          if( (Pong_u32Flags & _PONG_FLAGS_MASTER) &&
              (psMessageToProcess->au8MessageData[READY_MSG_STATUS_P1_BYTE] == STATUS_READY) )
          {
            Pong_u32Flags |= _PONG_STATUS_ADVANCE_READY;
          }
          
          /* The slave can say that it is ready if it actually is ready and the master has said it is also ready */
          if( !(Pong_u32Flags & _PONG_FLAGS_MASTER) &&
              (Pong_u32Flags & _PONG_STATUS_LOCAL_READY) &&
              (psMessageToProcess->au8MessageData[READY_MSG_STATUS_P0_BYTE] == STATUS_READY) )
          {
            Pong_au8ReadyMsgMessage[READY_MSG_STATUS_P1_BYTE] = STATUS_READY;
          }
  
          AntDeQueueDataMessage(psMessageToProcess, &G_sAntDataIncomingMsgList);
          break;
        } /* end case PONG_MESSAGE_READY */
        
  
        case MESSAGE_ANT_TICK:
        { 
          //u32PongFlags |= _PONG_UPDATE_TIME; 
          
          /* Clear the CRITICAL flag (that may or may not be set) */
          Pong_u32Flags &= ~_PONG_CRITICAL_MSG_TX;
  
          AntDeQueueDataMessage(psMessageToProcess, &G_sAntDataIncomingMsgList);
          break;
        } /* end case PONG_MESSAGE_GAME */
  
      default:
          /* The message was not for the Pong application */
          break;
         
      } /* end switch ( psMessageToProcess->au8MessageData[INDEX_PONG_MESSAGE_ID] ) */
    }
  } /* end while */

} /* end PongProcessGameMessages() */


/*----------------------------------------------------------------------------
Function: PongUpdateBallPosition

Description:
Looks at the next position of the ball based on the current ball vector.  If no
objects are in the way for the next movement, then the ball position data in 
Pong_stGameData is updated by adding the vector to the current position.
If the vector calculation concludes that an object will be in the way (either
a wall or a paddle), then both the ball position and ball vector are updated.
Lastly, if the ball will travel out of bounds on the next movement, then a flag
is set to indicate a point has been scored and that gameplay should be stopped at
that point.

Requires:
  - LCDRAM data is up to date with both player's current paddle and object data

Promises:
  - Ball is moved within LCD RAM
  - _PONG_STATUS_SCORE_P0 or _PONG_STATUS_SCORE_P1 may be set if a point was scored
*/
void PongUpdateBallPosition(void)
{
  u8 u8LocalRamBitGroup;
  u8 u8CurrentPixelBitInLocalRamMask;
  u8 u8RowsToMove, u8ColumnsToMove;
  u8 u8BallLocationRowNew, u8BallLocationColumnNew;
  u8 u8RowToCheck, u8ColumnToCheck;
  bool bCollision;
  
  /* Get the current movement amount */
  u8RowsToMove = Pong_stGameData.u8BallMotionRow;
  u8ColumnsToMove = Pong_stGameData.u8BallMotionColumn;

  /* Check for collisions 1 pixel of movement at a time to ensure no contacts are missed */
  while( (u8RowsToMove + u8ColumnsToMove) != 0 )
  {
    /* Check for column collisions first if there are any columns left to move in the current iteration */
    if(u8ColumnsToMove != 0)
    {
      /* The proposed new column position is the current column plus the column vector */
      u8BallLocationColumnNew = Pong_stGameData.u8BallLocationColumnCurrent + (u8)Pong_stGameData.s8BallVectorColumn;
      
      /* The column to check depends on which way the ball is currently moving */
      u8ColumnToCheck = u8BallLocationColumnNew;
      if(Pong_stGameData.s8BallVectorColumn > 0)
      {
        /* Ball is moving to the right, so need to check the other edge of the ball */
        u8ColumnToCheck = u8BallLocationColumnNew + LCD_IMAGE_BALL_COL_SIZE - 1;
      }
      
      /* Set the masks to isolate the bit in RAM that needs to be checked */
      u8LocalRamBitGroup = (u8ColumnToCheck / 8); 
      u8CurrentPixelBitInLocalRamMask = 0x01 << (u8ColumnToCheck % 8);
        
      /* Start by assuming no collision for this check */
      bCollision = FALSE;
  
      /* Check for any lit pixels in the LCD RAM in any position where the ball will occupy */
      for(u8 i = 0; i < LCD_IMAGE_BALL_ROW_SIZE; i++)
      {
        if(G_aau8LcdRamImage[Pong_stGameData.u8BallLocationRowCurrent + i][u8LocalRamBitGroup] & u8CurrentPixelBitInLocalRamMask)
        {
          bCollision = TRUE;
        }
      }

      /* If there is a collision flag, then bounce the ball: assign new position and reverse the vector */
      if(bCollision)
      {
        Pong_stGameData.s8BallVectorColumn = -Pong_stGameData.s8BallVectorColumn;
        u8BallLocationColumnNew = Pong_stGameData.u8BallLocationColumnCurrent + (u8)Pong_stGameData.s8BallVectorColumn;
        Pong_u32Flags |= _PONG_BALL_BOUNCE;
      }
      
      Pong_stGameData.u8BallLocationColumnCurrent = u8BallLocationColumnNew;
      u8ColumnsToMove--;
      
    } /* end  if(u8ColumnsToMove != 0) */
    
    /* Check for row collisions if there are still rows to move remaining in this update */
    if(u8RowsToMove != 0)
    {
      /* The new ball row is the current location plus the vector */
      u8BallLocationRowNew = Pong_stGameData.u8BallLocationRowCurrent + (u8)Pong_stGameData.s8BallVectorRow;
      
      /* The row to check depends on which way the ball is currently moving. If the ball is moving up on the screen,
      then check collisions with the top row of the ball graphic.  If the ball is moving down on the screen, then 
      check collisions with the bottom row of the ball graphic. */
      u8RowToCheck = u8BallLocationRowNew;
      if(Pong_stGameData.s8BallVectorRow > 0)
      {
        u8RowToCheck = u8BallLocationRowNew + LCD_IMAGE_BALL_ROW_SIZE - 1;
      }
      
      /* Check each of the columns that the current row is occupying */
      u8ColumnToCheck = Pong_stGameData.u8BallLocationColumnCurrent;
      u8LocalRamBitGroup = (u8ColumnToCheck / 8); 
      u8CurrentPixelBitInLocalRamMask = 0x01 << (u8ColumnToCheck % 8);

      /* Start by assuming no collision for this check */
      bCollision = FALSE;

      /* Check for any lit pixels in the LCD RAM in any position where the ball will occupy */
      for(u8 i = 0; i < LCD_IMAGE_BALL_COL_SIZE; i++)
      {
        if(G_aau8LcdRamImage[u8RowToCheck][u8LocalRamBitGroup] & u8CurrentPixelBitInLocalRamMask)
        {
          bCollision = TRUE;
        }

        /* Shift the LCD RAM mask */
        u8CurrentPixelBitInLocalRamMask <<= 1;
        if(u8CurrentPixelBitInLocalRamMask == 0x00)
        {
          u8CurrentPixelBitInLocalRamMask = 0x01;
          u8LocalRamBitGroup++;
        }
      }
      
      /* If there is a collision, then bounce the ball: assign new position and reverse the vector */
      if(bCollision)
      {
        Pong_stGameData.s8BallVectorRow = -Pong_stGameData.s8BallVectorRow;
        u8BallLocationRowNew = Pong_stGameData.u8BallLocationRowCurrent + (u8)Pong_stGameData.s8BallVectorRow;
        Pong_u32Flags |= _PONG_BALL_BOUNCE;
      }
      else
      {
        /* Now check to see if the ball has breached the boundary of either player */
        if( u8RowToCheck < (LCD_PADDLE0_ROW + LCD_IMAGE_PADDLE_ROW_SIZE) )
        {
          Pong_u32Flags |= (_PONG_STATUS_SCORE_P1 | _PONG_STATUS_REMOTE_SCORED);
          Pong_u8GameStatus |= _PONG_MSG_STATUS_P1_SCORED;
        }
        
        /* Check for the slave */
        if(u8RowToCheck > (LCD_PADDLE1_ROW + LCD_IMAGE_BALL_ROW_SIZE - 1) )
        {
          Pong_u32Flags |= (_PONG_STATUS_SCORE_P0 | _PONG_STATUS_REMOTE_SCORED);
          Pong_u8GameStatus |= _PONG_MSG_STATUS_P0_SCORED;
        }
      }

      Pong_stGameData.u8BallLocationRowCurrent = u8BallLocationRowNew;
      u8RowsToMove--;

      /* Update the message data */
      PongUpdateGameMessage();
      
    } /* end if(u8RowsToMove != 0) */
    

    /* If either player has scored, then break the loop and exit */
    if(Pong_u32Flags & (_PONG_STATUS_SCORE_P0 | _PONG_STATUS_SCORE_P1) )
    {
      break;
    }
  } /* end while (u8RowsToMove + u8ColumnsToMove) != 0)*/
  
} /* end PongUpdateBallPosition() */


/*----------------------------------------------------------------------------
Function: PongUpdateGameMessage

Description:
Loads the GAME message parameters with the current game data.

Requires:
  - All game data in Pong_stGameData is up to date

Promises:
  - Pong_au8GameMessage fields have the latest information for the current device to
    send over the radio.     
*/
void PongUpdateGameMessage(void)
{
  Pong_au8GameMessage[PONG_MSG_ID] = Pong_u8GameMessageNumber;

  Pong_au8GameMessage[PONG_MSG_VECTOR_ROW_BYTE]    = Pong_stGameData.s8BallVectorRow;
  Pong_au8GameMessage[PONG_MSG_VECTOR_COLUMN_BYTE] = Pong_stGameData.s8BallVectorColumn;
  Pong_au8GameMessage[PONG_MSG_PADDLE0_BYTE]       = Pong_stGameData.u8Paddle0LocationCurrent;
  Pong_au8GameMessage[PONG_MSG_PADDLE1_BYTE]       = Pong_stGameData.u8Paddle1LocationCurrent;
  Pong_au8GameMessage[PONG_MSG_BALL_ROW_BYTE]      = Pong_stGameData.u8BallLocationRowCurrent;
  Pong_au8GameMessage[PONG_MSG_BALL_COLUMN_BYTE]   = Pong_stGameData.u8BallLocationColumnCurrent;
  Pong_au8GameMessage[PONG_MSG_STATUS_BYTE]        = Pong_u8GameStatus;

} /* end PongUpdateGameMessage() */                          



/*----------------------------------------------------------------------------
Function: PongSM_ErrorHandler

Description:
Captures an error, displays information, then restores game to a known state.

Requires:
  - u8ErrorNumber_ holds a valid error number for which there is a corresponding error 
    message in Pong_aau8ErrorMessages;

Promises:
  - ANT channel is closed
  - All LEDs off except red on solid
  - LCD RAM cleared and updated with error message
*/
void PongSM_ErrorHandler(u8 u8ErrorNumber_)
{
  PixelAddressType sPixelAddress;

#ifndef SINGLE_PLAYER_MODE
  AntCloseChannel();
#endif
  
  /* Clear the entire LCD and write the error message */
  LcdClearPixels(&G_sLcdClearWholeScreen);
  sPixelAddress.u16PixelRowAddress = LCD_SMALL_FONT_LINE2;
  sPixelAddress.u16PixelColumnAddress = 0;
  LcdLoadString(&Pong_au8ErrorMsg[0], LCD_FONT_SMALL, &sPixelAddress);

  sPixelAddress.u16PixelRowAddress = LCD_SMALL_FONT_LINE3;
  LcdLoadString(&Pong_aau8ErrorMessages[u8ErrorNumber_][0], LCD_FONT_SMALL, &sPixelAddress);
 
  /* Reset all LEDs */
  LedOn(RED);
  LedOff(YELLOW);
  LedOff(GREEN);
  LedOff(BLUE);

  G_PongStateMachine = PongSM_Error;
      
} /* end PongSM_ErrorHandler() */          
                                 
                                 
/*******************************************************************************
* Pong State Machine Function Definitions                                  @@@@@
*******************************************************************************/
/* The Pong state machine is responsible for managing all aspects of game play
which include reading received ANT messages, updating ANT message to send, determining
the game play / managing the ball movement and bounce detection, and updating graphics
on the LCD.
*/


/*----------------------------------------------------------------------------
Function: PongSM_Idle

Description:
Waits for user input to open a channel in either master or slave mode.
If both buttons are pressed, the system defaults to master.

LCD: ANT PONG!!
LED: Blue
Radio: OFF

*/
void PongSM_Idle(void)
{
  PixelAddressType sPixelAddress;

  /* Exit only on button press */
  if( WasButtonPressed(BUTTON0) || WasButtonPressed(BUTTON1) )
  {

#ifndef SINGLE_PLAYER_MODE 
    /* Configure as Master on BUTTON0 */
    if( WasButtonPressed(BUTTON0) )
    {
      Pong_u32Flags |= _PONG_FLAGS_MASTER;
      AntChannelConfig(ANT_MASTER);
      ButtonAcknowledge(BUTTON0);
    }
      
    /* Configure as Slave on BUTTON1 */
    else
    {
      Pong_u32Flags &= ~_PONG_FLAGS_MASTER;
      AntChannelConfig(ANT_SLAVE);
      ButtonAcknowledge(BUTTON1);
    }

    /* In either case, update the UI, open the ANT channel and start looking for a match */
    LcdClearPixels(&G_sLcdClearWholeScreen);
    sPixelAddress.u16PixelRowAddress = LCD_SMALL_FONT_LINE2;
    sPixelAddress.u16PixelColumnAddress = 0;
    LcdLoadString(&Pong_au8SearchingMsg[0], LCD_FONT_SMALL, &sPixelAddress);
 
    LedBlink(YELLOW, LED_4HZ);
    LedOff(BLUE);

    /* Open the ANT channel and initialize state variables */
    AntOpenChannel();
    
    Pong_u32Timeout = G_u32SystemTime1ms;
    Pong_u32Flags |= _PONG_STATUS_SEARCHING;
    Pong_u32Flags &= ~(_PONG_STATUS_ADVANCE_IDLE | _PONG_STATUS_ADVANCE_DATA);
    Pong_u8MessageNumberLow = 0;
    Pong_u8MessageNumberHigh = 0;

    G_PongStateMachine = PongSearching;
    
#else
    /* For single player mode sent ANT as the MASTER */
    Pong_u32Flags |= _PONG_FLAGS_MASTER;
    ButtonAcknowledge(BUTTON0);
    ButtonAcknowledge(BUTTON1);

    //AntChannelConfig(ANT_MASTER);
    //AntOpenChannel();
    
    LedOn(GREEN);
    LedOff(BLUE);
    LedOff(RED);
    LedOff(YELLOW);

    Pong_u32Flags |= _PONG_STATUS_ADVANCE_DATA;
    G_PongStateMachine = PongSM_DataExchange;
#endif

  }  
  
} /* end PongSM_Idle() */


/*----------------------------------------------------------------------------
Function: PongSearching

Description:
A device waits to connect. The _PONG_STATUS_ADVANCE_IDLE flag to leave
this state is controlled in PongProcessGameMessages.

A master knows when it's connected when it has received enough IDLE messages
from the slave.  The master tells this to the slave by setting the status
byte to STATUS_READY and continues to send IDLE messages until the slave
has received enough messages and changes to DATA messages.

A slave knows when it's connected when it has received enough IDLE messages
from the master.  The slave tells this to the master by changing states and
sending DATA messages.

LCD: Searching...
LED: Yellow blinky
Radio: Sending IDLE messages and waiting to hear broadcast data IDLE messages
*/
void PongSearching(void)
{
  /* Process any messages that have been received */
  PongProcessGameMessages();
  
  /* Stay in this state until it's time to advance.  The flag is set differently 
  for Master and Slave inside ProcessPongGameMessage(); */
  if(Pong_u32Flags & _PONG_STATUS_ADVANCE_IDLE)
  {    
    /* Set the starting ball vector */
    PongSetBallVector();

    /* Update state variables and go to DataExchange */
    Pong_u32Flags &= ~(_PONG_STATUS_SEARCHING | _PONG_STATUS_ADVANCE_IDLE);

    LedOn(YELLOW);
    
    Pong_u8MessageNumberLow = 0;
    Pong_u8MessageNumberHigh = 0;
    G_PongStateMachine = PongSM_DataExchange;
  }
  
  /* If there is no message in the queue, increment IDLE message number and queue the message */
  else 
  {
    if(G_sAntDataIncomingMsgList == NULL)
    {
      /* Update the message number then queue it (don't care about high byte roll) */
      if(++Pong_u8MessageNumberLow == 0)
      {
        Pong_u8MessageNumberHigh++;
      }
      
      Pong_au8dleMessage[IDLE_MSG_HIGH_BYTE] = Pong_u8MessageNumberHigh;
      Pong_au8dleMessage[IDLE_MSG_LOW_BYTE]  = Pong_u8MessageNumberLow;
      
      AntQueueDataMessage(Pong_au8dleMessage, &G_sAntDataIncomingMsgList);
    }
  
    /* Check that a timeout has not occurred */
    if( IsTimeUp(&Pong_u32Timeout, COMMUNICATION_TIMEOUT) )
    {
      PongSM_ErrorHandler(ERROR_ANT_RF_TIMEOUT);
    }
  } /* end else */

} /* end PongSearching() */


/*----------------------------------------------------------------------------------------------------------------------
Function: PongSM_DataExchange

In single player mode, this state is used just to update the LCD. 

LCD: Searching...
LED: Yellow solid
Radio: Sending DATA messages and waiting to hear broadcast data DATA messages

*/
void PongSM_DataExchange(void)
{
  PixelAddressType sPixelAddress;
  PixelBlockType sPixelBlock;

  /* Process any messages that have been received */
  PongProcessGameMessages();

  /* If the devices have synchronized data move to Ready state */
  if(Pong_u32Flags & _PONG_STATUS_ADVANCE_DATA)
  {
    /* Make sure all the game flags are clear */
    PongResetGameFlags();
    
    /* Update the game information */
    Pong_stGameData.s8BallVectorRow    = Pong_au8DataMessage[DATA_MSG_VECTOR_ROW_BYTE];
    Pong_stGameData.s8BallVectorColumn = Pong_au8DataMessage[DATA_MSG_VECTOR_COLUMN_BYTE];
    Pong_stGameData.u8BallMotionRow    = Pong_au8DataMessage[DATA_MSG_MOTION_ROW_BYTE];
    Pong_stGameData.u8BallMotionColumn = Pong_au8DataMessage[DATA_MSG_MOTION_COLUMN_BYTE];

    /* Show ready to start message with game screen */
    LcdClearPixels(&G_sLcdClearWholeScreen);
    sPixelAddress.u16PixelColumnAddress = LCD_GAMESCREEN_EDGE_COL_LEFT + 1;

    sPixelAddress.u16PixelRowAddress = LCD_SMALL_FONT_LINE3;
    LcdLoadString(&Pong_au8ReadyMsg[0], LCD_FONT_SMALL, &sPixelAddress);
    sPixelAddress.u16PixelRowAddress = LCD_SMALL_FONT_LINE4;
    LcdLoadString(&Pong_au8StartMsg[0], LCD_FONT_SMALL, &sPixelAddress);

    PongUpdateScoresLCD();
    PongDrawGameScreen();

    /* Set all the location parameters to draw an arrow in the game message area to indicate which side of the screen the player is on */
    sPixelBlock.u16RowStart    = LCD_PLAYER1_ARROW_ROW;
    sPixelBlock.u16ColumnStart = LCD_PLAYER_ARROW_COLUMN;
    sPixelBlock.u16RowSize     = LCD_IMAGE_ARROW_ROW_SIZE;
    sPixelBlock.u16ColumnSize  = LCD_IMAGE_ARROW_COL_SIZE;
    
    /* Also prepare the READY status */
    Pong_au8ReadyMsgMessage[READY_MSG_SCORE_P1_BYTE] = STATUS_NOT_READY;
    
    if(Pong_u32Flags & _PONG_FLAGS_MASTER)
    {
      sPixelAddress.u16PixelRowAddress = LCD_PLAYER0_TEXT_ROW;
      sPixelBlock.u16RowStart = LCD_PLAYER0_ARROW_ROW;
      Pong_au8ReadyMsgMessage[READY_MSG_SCORE_P0_BYTE] = STATUS_NOT_READY;
    }

    /* Draw the arrow */
    LcdLoadBitmap(&aau8PlayerArrow[0][0], &sPixelBlock);
    
    /* Set the Ready message data */
    Pong_u8MessageNumberLow = 0;
    Pong_u8MessageNumberHigh = 0;
    Pong_au8ReadyMsgMessage[READY_MSG_SCORE_P0_BYTE] = Pong_stGameData.u8Player0Score;
    Pong_au8ReadyMsgMessage[READY_MSG_SCORE_P1_BYTE] = Pong_stGameData.u8Player1Score;
    Pong_au8ReadyMsgMessage[READY_MSG_HIGH_BYTE] = Pong_u8MessageNumberHigh;
    Pong_au8ReadyMsgMessage[READY_MSG_LOW_BYTE]  = Pong_u8MessageNumberLow;
     
    /* Update state variables and go to Ready state */
    LedOff(YELLOW);
    LedBlink(GREEN, LED_2HZ);
    
    G_PongStateMachine = PongSM_Ready;
  }
  
  /* If there is no message in the queue, increment DATA message number and queue the message */
  else 
  {
    if(G_sAntDataIncomingMsgList == NULL)
    {
      /* Update the message number then queue it */
      if(++Pong_u8MessageNumberLow == 0)
      {
        Pong_u8MessageNumberHigh++;
      }
      
      Pong_au8DataMessage[DATA_MSG_HIGH_BYTE] = Pong_u8MessageNumberHigh;
      Pong_au8DataMessage[DATA_MSG_LOW_BYTE]  = Pong_u8MessageNumberLow;
      
      AntQueueDataMessage(Pong_au8DataMessage, &G_sAntDataIncomingMsgList);
    }
  
    /* Check for timeout */
    if( IsTimeUp(&Pong_u32Timeout, COMMUNICATION_TIMEOUT) )
    {
      PongSM_ErrorHandler(ERROR_ANT_RF_TIMEOUT);
    }
  } /* end else */
  
 
} /* end PongSM_DataExchange() */


/*----------------------------------------------------------------------------
Function: PongSM_Ready

Send Ready messages along with player status byte NOT READY.  The receiving device will
be doing the same.  

LCD: "Devices ready"
LED: Green flashing
Radio: Sending READY messages
             
*/
void PongSM_Ready(void)
{
  PixelBlockType sPixelBlock;

  /* Process any messages that have been received */
  PongProcessGameMessages();

  /* Do nothing until button is pressed */
  if( WasButtonPressed(BUTTON1) )
  {
    /* Ack the button press and flag that the local device is ready for game play */
    ButtonAcknowledge(BUTTON1);
    Pong_u32Flags |= _PONG_STATUS_LOCAL_READY;
    sPixelBlock.u16RowStart = LCD_PLAYER1_ARROW_ROW;
    
    /* Master needs to set STATUS_READY */
    if( (Pong_u32Flags & _PONG_FLAGS_MASTER) )
    {
      Pong_au8ReadyMsgMessage[READY_MSG_STATUS_P0_BYTE] = STATUS_READY;
      sPixelBlock.u16RowStart = LCD_PLAYER0_ARROW_ROW;
    }
    
    /* Redraw the screen */
    LcdClearPixels(&G_sLcdClearWholeScreen);
    PongUpdateScoresLCD();
    PongDrawGameScreen();
    PongResetGameObjects();

    /* Draw the player indicator arrow (row position set already ) */
    sPixelBlock.u16ColumnStart = LCD_PLAYER_ARROW_COLUMN;
    sPixelBlock.u16RowSize     = LCD_IMAGE_ARROW_ROW_SIZE;
    sPixelBlock.u16ColumnSize  = LCD_IMAGE_ARROW_COL_SIZE;
    LcdLoadBitmap(&aau8PlayerArrow[0][0], &sPixelBlock);

    /* Draw Player 0 paddle */
    sPixelBlock.u16RowStart    = LCD_PADDLE0_ROW;
    sPixelBlock.u16ColumnStart = Pong_stGameData.u8Paddle0LocationCurrent;
    sPixelBlock.u16RowSize     = LCD_IMAGE_PADDLE_ROW_SIZE;
    sPixelBlock.u16ColumnSize  = LCD_IMAGE_PADDLE_COL_SIZE;
    LcdLoadBitmap(&aau8PongPaddleTop[0][0], &sPixelBlock);
    
    /* Draw  Player 1 paddle */
    sPixelBlock.u16RowStart    = LCD_PADDLE1_ROW;
    sPixelBlock.u16ColumnStart = Pong_stGameData.u8Paddle1LocationCurrent;
    LcdLoadBitmap(&aau8PongPaddleBottom[0][0], &sPixelBlock);
    
    /* Draw the ball */
    sPixelBlock.u16RowStart    = LCD_BALL_START_ROW;
    sPixelBlock.u16ColumnStart = LCD_BALL_START_COLUMN;
    sPixelBlock.u16RowSize     = LCD_IMAGE_BALL_ROW_SIZE;
    sPixelBlock.u16ColumnSize  = LCD_IMAGE_BALL_COL_SIZE;
    LcdLoadBitmap(&aau8PongBall[0][0], &sPixelBlock);
    Pong_u32Timeout = G_u32SystemTime1ms;

#ifndef SINGLE_PLAYER_MODE
    G_PongStateMachine = PongWaitRemote;
#else
    Pong_u32Flags |= _PONG_STATUS_REMOTE_GAMEON;
    Pong_u8GameStatus |= _PONG_MSG_STATUS_IN_PROGRESS;
    
    LedOn(GREEN);
    
    Pong_u32Timeout = G_u32SystemTime1ms;
    Pong_u32GameStartDelay = GAME_START_DELAY_MASTER;
    
    G_PongStateMachine = PongSM_StartDelay;
#endif /* SINGLE_PLAYER_MODE */

  } /* end of BUTTON action */
  
  /* If there is no message in the queue, increment READY message number and queue the message */
  else 
  {
#ifndef SINGLE_PLAYER_MODE
    if(G_sAntDataIncomingMsgList == NULL)
    {
      /* Increment message number, care only about low byte rollover */
      if(++Pong_u8MessageNumberLow == 0)
      {
        Pong_u8MessageNumberHigh++;
      }
      
      /* Update the message number then queue it */
      Pong_au8ReadyMsgMessage[IDLE_MSG_HIGH_BYTE] = Pong_u8MessageNumberHigh;
      Pong_au8ReadyMsgMessage[IDLE_MSG_LOW_BYTE]  = Pong_u8MessageNumberLow;
      
      AntQueueDataMessage(Pong_au8ReadyMsgMessage, &G_sAntDataIncomingMsgList);
    }
    
    /* Check that a timeout has not occurred */
    if( IsTimeUp(&Pong_u32Timeout, COMMUNICATION_TIMEOUT) )
    {
      PongSM_ErrorHandler(ERROR_ANT_RF_TIMEOUT);
    }
#endif /* SINGLE_PLAYER_MODE */
  } /* end else */

} /* end PongSM_Ready() */


/*----------------------------------------------------------------------------
Function: PongSM_StartDelay
             
The game is almost ready.  Both master and slave wait for the start delay time to 
go by and continue to send GAME messages with IN_PROGRESS set.  The green LED
is solid so the players know that the game will start in about two seconds.

Paddle motion still not allowed here.  A nice update would be
to add a count-down or something on the LCD, though the pseudo-randomness of the start
isn't a bad thing.  Maybe in Super ANT Pong.    

LCD: Game start position
LED: Green solid
Radio: On, sending GAME messages
*/
void PongSM_StartDelay(void)
{
  /* Process any messages that have been received */
  PongProcessGameMessages();

  /* Wait the startup delay time */
  if( IsTimeUp(&Pong_u32Timeout, Pong_u32GameStartDelay) )
  {
   //Pong_u32Flags |= _PONG_LCD_UPDATE_P0;
   G_PongStateMachine = PongSM_GamePlay;
  }

#ifndef SINGLE_PLAYER_MODE

  /* If either device receives a message that indicates the other is no longer ready,
  abort and return to Sync */
  if( !(Pong_u32Flags & _PONG_STATUS_REMOTE_GAMEON) )
  {
    /* Go back to blinking green LED */
    LedBlink(GREEN, LED_2HZ);

    /* Clear the IN_PROGRESS_BIT and return to StartSync */
    Pong_u8GameStatus &= ~_PONG_MSG_STATUS_IN_PROGRESS;
    PongUpdateGameMessage();

    G_PongStateMachine = PongSM_StartSync;
  }
  
  /* Otherwise just keep sending GAME messages */
  else if(G_sAntDataIncomingMsgList == NULL)
  {
    AntQueueDataMessage(Pong_au8GameMessage, &G_sAntDataIncomingMsgList);
  }
#endif
  
} /* end PongSM_StartDelay() */

                                 
/*----------------------------------------------------------------------------------------------------------------------
Function: PongSM_GamePlay

This is the main game state where the following actions take place:
1. Incoming Pong messages are processed
2. The LCD RAM is updated with any new Paddle0, Paddle1, and Ball locations.
3. Points scored are checked that will interrupt game play and advance states.
4. Future: add capability to pause or quit game and handle radios that drop out
       
In the background, the ANT radio continuously sends and receives GAME messages.
GAME messages are used to provide the remote device's paddle information and also
communication game status. 
       
LCD: Rivetting ANT Pong game action
LED: Green solid
Radio: On, sending GAME messages
*/
void PongSM_GamePlay(void)
{
  static u32 u32PongUpdateTimer = 0;
  static PixelBlockType sLcdGameScreen = 
  {
    .u16RowStart    = LCD_GAMESCREEN_EDGE_ROW_TOP,
    .u16ColumnStart = LCD_GAMESCREEN_EDGE_COL_LEFT,
    .u16RowSize     = LCD_GAMESCREEN_EDGE_ROW_BOTTOM + 1,
    .u16ColumnSize  = LCD_GAMESCREEN_EDGE_COL_RIGHT - LCD_GAMESCREEN_EDGE_COL_LEFT
  };
  PixelBlockType sPixelBlock;
  PixelAddressType sPixelAddress;
  u16 u16ProcessedCaptouchValue;
  
  /* Process any gameplay messages that have been queued */
  PongProcessGameMessages();

  /* Process the game loop only if the game is on or a final update is required */
  if( ( (Pong_u32Flags & _PONG_STATUS_REMOTE_GAMEON) && (Pong_u8GameStatus & _PONG_MSG_STATUS_IN_PROGRESS) ) ||
      (Pong_u32Flags & _PONG_LCD_LAST_UPDATE) )
  {
    if( IsTimeUp(&u32PongUpdateTimer, PONG_UPDATE_TIME) )
    {
      /* Clear the update flag */
      u32PongUpdateTimer = G_u32SystemTime1ms;
      
      /* Grab the current position and update the paddle position data for this player. Scale the captouch
      slider value into the size of the game screen accounting for the size of the paddle. */
      u16ProcessedCaptouchValue = ((u16)CaptouchCurrentHSlidePosition() * (LCD_GAME_SIZE_COLUMN - LCD_IMAGE_PADDLE_COL_SIZE + 1)) / 256;
      u16ProcessedCaptouchValue += LCD_GAMESCREEN_EDGE_COL_LEFT;
      
      if(Pong_u32Flags & _PONG_FLAGS_MASTER)
      {
        Pong_stGameData.u8Paddle0LocationCurrent = (u8)u16ProcessedCaptouchValue;
      }
      else
      {
        Pong_stGameData.u8Paddle1LocationCurrent = (u8)u16ProcessedCaptouchValue;
      }
  
      /* Clear the game screen area since everything will be updated */
      LcdClearPixels(&sLcdGameScreen);
      
#ifdef SINGLE_PLAYER_MODE
    /* Draw a horizontal line across the entire Player game space */
    sPixelAddress.u16PixelRowAddress = 60;
    for(u16 i = LCD_GAMESCREEN_EDGE_COL_LEFT; i < LCD_GAMESCREEN_EDGE_COL_RIGHT; i++)
    {
      sPixelAddress.u16PixelColumnAddress = i;
      LcdSetPixel(&sPixelAddress); 
    }
#endif /* SINGLE_PLAYER_MODE */
  
#ifdef NO_PLAYER_MODE
    /* Draw the line for Player 0 */
    sPixelAddress.u16PixelRowAddress = 3;
    for(u16 i = LCD_GAMESCREEN_EDGE_COL_LEFT; i < LCD_GAMESCREEN_EDGE_COL_RIGHT; i++)
    {
      sPixelAddress.u16PixelColumnAddress = i;
      LcdSetPixel(&sPixelAddress); 
    }
  
    /* Draw the line for Player 1 */
    sPixelAddress.u16PixelRowAddress = 60;
    for(u16 i = LCD_GAMESCREEN_EDGE_COL_LEFT; i < LCD_GAMESCREEN_EDGE_COL_RIGHT; i++)
    {
      sPixelAddress.u16PixelColumnAddress = i;
      LcdSetPixel(&sPixelAddress); 
    }
#endif      
  
      /* Draw Paddle 0 in its new spot */
      sPixelBlock.u16RowStart    = LCD_PADDLE0_ROW;
      sPixelBlock.u16ColumnStart = Pong_stGameData.u8Paddle0LocationCurrent;
      sPixelBlock.u16RowSize     = LCD_IMAGE_PADDLE_ROW_SIZE;
      sPixelBlock.u16ColumnSize  = LCD_IMAGE_PADDLE_COL_SIZE;
      LcdLoadBitmap(&aau8PongPaddleTop[0][0], &sPixelBlock);

      /* Draw Paddle 1 in its new spot */
      sPixelBlock.u16RowStart    = LCD_PADDLE1_ROW;
      sPixelBlock.u16ColumnStart = Pong_stGameData.u8Paddle1LocationCurrent;
      LcdLoadBitmap(&aau8PongPaddleBottom[0][0], &sPixelBlock);
  
#ifndef  BALL_STOPPED
      /* Update the ball position if this isn't a last update */
      if( !(Pong_u32Flags & _PONG_LCD_LAST_UPDATE) )
      {
        PongUpdateBallPosition();      
      }
      
      /* Draw the ball in the updated spot */
      sPixelBlock.u16RowStart    = Pong_stGameData.u8BallLocationRowCurrent;
      sPixelBlock.u16ColumnStart = Pong_stGameData.u8BallLocationColumnCurrent;
      sPixelBlock.u16RowSize     = LCD_IMAGE_BALL_ROW_SIZE;
      sPixelBlock.u16ColumnSize  = LCD_IMAGE_BALL_COL_SIZE;
      LcdLoadBitmap(&aau8PongBall[0][0], &sPixelBlock);
#endif
      
      /* Rotate the location data */
      Pong_stGameData.u8Paddle0LocationLast    = Pong_stGameData.u8Paddle0LocationCurrent;
      Pong_stGameData.u8Paddle1LocationLast    = Pong_stGameData.u8Paddle1LocationCurrent;
      Pong_stGameData.u8BallLocationRowLast    = Pong_stGameData.u8BallLocationRowCurrent;
      Pong_stGameData.u8BallLocationColumnLast = Pong_stGameData.u8BallLocationColumnCurrent;

      /* If this run was the last run, advance states */
      if(Pong_u32Flags & _PONG_LCD_LAST_UPDATE)
      {
        G_PongStateMachine = PongSM_PointScored;
      }
    } /* end of all updates */
    
  } /* end if _PONG_STATUS_GAMEON */
  
  /* One of the SCORE flags will be set, so a flag to force one one more LCD update.  
  Eventually this could be updated to change states for a paused game, quit game, loss of device sync, etc. */
  if(Pong_u32Flags & (_PONG_STATUS_SCORE_P0 | _PONG_STATUS_SCORE_P1) )
  {
    PongUpdateGameMessage();    
    Pong_u32Flags |= _PONG_LCD_LAST_UPDATE;
  }

  /* Otherwise update the game play message if it's time to do so */
  else if(G_sAntDataIncomingMsgList == NULL)
  {
    PongUpdateGameMessage();    
    AntQueueDataMessage(Pong_au8GameMessage, &G_sAntDataIncomingMsgList);
  }
  
} /* end PongSM_GamePlay() */


/*----------------------------------------------------------------------------
Function: PongSM_PointScored

This state runs through automatically regardless of who scored.

Wait for the current LCD refresh to finish, then update the LCD with the score
information. 
*/
void PongSM_PointScored(void)
{
  const u8* aau8String;
  PixelAddressType sStringLocation;

  /* Process any gameplay messages that have been queued */
  PongProcessGameMessages();

  /* Update based on who scored */
  if(Pong_u32Flags & _PONG_STATUS_SCORE_P0)
  {
    Pong_u8GameStatus |= _PONG_MSG_STATUS_P0_SCORED;
    
    /* Point at the Player0 message and increment the P0 score */
    aau8String = &Pong_au8Player0Msg[0];
    Pong_stGameData.u8Player0Score++;
  }
  else
  {
    Pong_u8GameStatus |= _PONG_MSG_STATUS_P1_SCORED;

    /* Point at the Player1 message and increment the P1 score */
    aau8String = &Pong_au8Player1Msg[0];
    Pong_stGameData.u8Player1Score++;
  }

  /* Load the "Point Scored" message to the LCD RAM */
  sStringLocation.u16PixelColumnAddress = LCD_GAMESCREEN_EDGE_COL_LEFT + 16;
  sStringLocation.u16PixelRowAddress = LCD_SMALL_FONT_LINE2;
  LcdLoadString(aau8String, LCD_FONT_SMALL, &sStringLocation);
    
  sStringLocation.u16PixelRowAddress = LCD_SMALL_FONT_LINE3;
  LcdLoadString(Pong_au8ScoredMsg, LCD_FONT_SMALL, &sStringLocation);

  /* Change LED status */
  LedOn(YELLOW);
  LedOff(GREEN);
  
  /* Check if the game is over */
#ifndef SINGLE_PLAYER_MODE
  if( (Pong_stGameData.u8Player0Score == PONG_POINTS_TO_WIN) ||
      (Pong_stGameData.u8Player1Score == PONG_POINTS_TO_WIN) )
#else
  if( (Pong_stGameData.u8Player0Score == PONG_POINTS_TO_WIN_SINGLE) ||
      (Pong_stGameData.u8Player1Score == PONG_POINTS_TO_WIN_SINGLE) )
#endif /* SINGLE_PLAYER_MODE */
  {
    /* End the game, blink the lights */
    Pong_u8GameStatus &= ~_PONG_MSG_STATUS_IN_PROGRESS;
    LedBlink(BLUE, LED_8HZ);
    LedBlink(GREEN,LED_4HZ);
    LedBlink(YELLOW,LED_8HZ);
    LedBlink(RED,LED_4HZ);

    /* Load the "Winner" message to the LCD RAM and update the GameStatus */
    sStringLocation.u16PixelColumnAddress = LCD_GAMESCREEN_EDGE_COL_LEFT + 16;
    sStringLocation.u16PixelRowAddress = LCD_SMALL_FONT_LINE5;
    LcdLoadString(Pong_au8WinnerMsg, LCD_FONT_SMALL, &sStringLocation);

    if(Pong_stGameData.u8Player0Score == PONG_POINTS_TO_WIN)
    {
      Pong_u8GameStatus |= _PONG_MSG_STATUS_P0_WINS;
    }
    else
    {
      Pong_u8GameStatus |= _PONG_MSG_STATUS_P1_WINS;
    }
  } 

  PongUpdateGameMessage();    
  PongUpdateScoresLCD();  
  
  /* Reset timer and advance states */
  Pong_u32Timeout = G_u32SystemTime1ms;
  G_PongStateMachine = PongSM_PointWait;

  /* Update the game play message if it's time to do so */
  if(G_sAntDataIncomingMsgList == NULL)
  {
    AntQueueDataMessage(Pong_au8GameMessage, &G_sAntDataIncomingMsgList);
  }
  
} /* end PongSM_PointScored() */


/*----------------------------------------------------------------------------
Function: PongSM_PointWait

Display the message for a minimum amount of time.
*/
void PongSM_PointWait()
{
  /* Process any gameplay messages that have been queued */
  PongProcessGameMessages();

  /* Wait the minimum delay time */
  if( IsTimeUp(&Pong_u32Timeout, SCORE_DISPLAY_TIME) )
  {
    /* Choose next state based on WINS flags */
    if(Pong_u8GameStatus & (_PONG_MSG_STATUS_P0_WINS | _PONG_MSG_STATUS_P1_WINS) )
    {
      /* Close the ANT channel and advance states to wait for channel to close */
#ifndef SINGLE_PLAYER_MODE
      AntCloseChannel();
#endif
      G_PongStateMachine = PongSM_AntChannelClose;
    }
    else
    {
      /* Make sure all game data is reset */
      PongResetGameObjects();
      PongResetGameFlags();
      PongSetBallVector();
    
#ifndef SINGLE_PLAYER_MODE
      /* If the local device detected the score, wait for the remote device to send DATA messages */
      if(Pong_u32Flags & _PONG_STATUS_REMOTE_SCORED)
      {
        G_PongStateMachine = PongPointSync;
      }
      else
      {
        G_PongStateMachine = PongSM_DataExchange;
      }
#else
      Pong_u32Flags |= _PONG_STATUS_ADVANCE_DATA;
      G_PongStateMachine = PongSM_DataExchange;
#endif /* SINGLE_PLAYER_MODE */
    }
  } /* end IsTimeUp() */
  
  /* Otherwise update the game play message if it's time to do so */
  else if(G_sAntDataIncomingMsgList == NULL)
  {
    AntQueueDataMessage(Pong_au8GameMessage, &G_sAntDataIncomingMsgList);
  }
  
} /* end PongSM_PointWait() */


/*----------------------------------------------------------------------------
Function: PongSM_AntChannelClose

Wait in this state until the ANT channel has closed, then proceed to start state.
*/
void PongSM_AntChannelClose(void)
{
  /* Process any gameplay messages that have been queued and advance states */
  PongProcessGameMessages();

  /* Wait until the channel has been closed */
  if( !(G_u32AntFlags & _ANT_FLAGS_CHANNEL_OPEN) )  /* #PCUR# */
  {
#ifndef SINGLE_PLAYER_MODE
    AntUnassignChannel();
#endif
    
    /* Return to the opening screen (sets SM to Idle) */
    PongInitialize();
  }

  
} /* end PongSM_AntChannelClose() */


/*----------------------------------------------------------------------------
Function: PongSM_Error
*/
void PongSM_Error(void)
{
  /* Process any gameplay messages that have been queued and advance states */
  PongProcessGameMessages();

  if( IsTimeUp(&Pong_u32Timeout, ERROR_DISPLAY_TIME) )
  {
    /* Wait until the channel has been closed */
    while(G_u32AntFlags & _ANT_FLAGS_CHANNEL_OPEN);
    {
#ifndef SINGLE_PLAYER_MODE
      AntUnassignChannel();
#endif
    }
    
    /* Reset all data and return to Idle state */
    PongInitialize();
  }

} /* end PongSM_Error() */



  