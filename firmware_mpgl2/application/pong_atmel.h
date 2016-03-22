/*******************************************************************************
File: pong_atmel.h   
*******************************************************************************/

#ifndef __PONG_H
#define __PONG_H


/*******************************************************************************
* Constants / Definitions
*******************************************************************************/
/* Pong_u32Flags */
#define _PONG_FLAGS_MASTER                (u32)0x00000001  /* Set when the device is the Master (Player 0) */


#define _PONG_LCD_UPDATING                (u32)0x00000100  /* Set when paddle and ball LCD updates are occurring */
#define _PONG_LCD_UPDATE_P0               (u32)0x00000200  /* Set when paddle 0 is updating */
#define _PONG_LCD_UPDATE_P1               (u32)0x00000400  /* Set when paddle 1 is updating */
#define _PONG_LCD_UPDATE_BALL             (u32)0x00000800  /* Set when ball is updating */
#define _PONG_LCD_LAST_UPDATE             (u32)0x00001000  /* Set when PongSM_GamePlay should run one more update */

#define _PONG_DATA_MISMATCH               (u32)0x00002000  /* Set if game data from slave doesn't match master */
#define _PONG_BALL_BOUNCE                 (u32)0x00004000  /* Set when ball bounces off something */

#define _PONG_CRITICAL_MSG_TX             (u32)0x00008000  /* Set when the master sends a critical message */

#define _PONG_STATUS_SEARCHING            (u32)0x00010000  /* Set when devices trying to connect */
#define _PONG_STATUS_ADVANCE_IDLE         (u32)0x00020000  /* Set when local device can advance to data messages */
#define _PONG_STATUS_ADVANCE_DATA         (u32)0x00040000  /* Set when devices have syncronized data messages */
#define _PONG_STATUS_ADVANCE_READY        (u32)0x00080000  /* Set when devices are ready to start game play */
#define _PONG_STATUS_LOCAL_READY          (u32)0x00100000  /* Set when the local device is ready to start game */
#define _PONG_STATUS_REMOTE_GAMEON        (u32)0x00200000  /* Set when the remote device thinks game play is in progress */
#define _PONG_STATUS_REMOTE_SCORED        (u32)0x00400000  /* Set when remote device scores */
#define _PONG_STATUS_SCORE_P0             (u32)0x01000000  /* Set when player 0 as scored */
#define _PONG_STATUS_SCORE_P1             (u32)0x02000000  /* Set when player 1 as scored */
#define _PONG_RECEIVING_GAME_MESSAGES     (u32)0x04000000  /* Set when game messages being received */
/* end of Pong_u32Flags definintions */

#define PONG_GAME_FLAG_MASK               (u32)0x077F1000  /* Mask for all game flags */

#define _PONG_GAME_VIRGIN                 (u32)0x80000000  /* Set on power-up */

/* Pong Error Codes */
#define ERROR_ANT_RF_TIMEOUT              (u8)0            /* Expected ANT radio communication timed out */

/* Game definitions */
#define PONG_UPDATE_TIME                  (u32)40          /* Time in ms between game object updates */

#define COMMUNICATION_TIMEOUT             (u32)300000      /* Number of ms for communication timeouts */
#define ERROR_DISPLAY_TIME                (u32)8000        /* Number of seconds to display error messages */
#define GAME_START_DELAY_MASTER           (u32)1900        /* Number of ms to pause before the master starts game play */
#define GAME_START_DELAY_SLAVE            (u32)2000        /* Number of ms to pause before the slave starts game play */
#define SCORE_DISPLAY_TIME                (u32)4000        /* Number of ms to display score messages */

#define BALL_VECTOR_ROW_DEFAULT           (u8)1            /* Default row direction of ball */
#define BALL_VECTOR_COLUMN_DEFAULT        (u8)1            /* Default column direction of ball */
#define BALL_MOVEMENT_ROWS                (u8)1            /* Vertical movement of ball per update */
#define BALL_MOVEMENT_COLUMNS             (u8)1            /* Horizontal movement of ball per update */
#define PONG_P0_STARTING_SCORE            (u8)0            /* Player 0's starting score */
#define PONG_P1_STARTING_SCORE            (u8)0            /* Player 1's starting score */

#define PONG_POINTS_TO_WIN                (u8)5            /* Number of points required to win */
#define PONG_POINTS_TO_WIN_SINGLE         (u8)3            /* Number of points required to win in single player mode */

/* Pong LCD values */
#define LCD_GAMESCREEN_SCOREBOX_COL_START (u16)0
#define LCD_GAMESCREEN_SCOREBOX_COL_END   (u16)46
#define LCD_GAMESCREEN_LINE_SCOREBOX_ROW1 (u8)0
#define LCD_GAMESCREEN_LINE_SCOREBOX_ROW2 (u8)26
#define LCD_GAMESCREEN_LINE_SCOREBOX_ROW3 (u8)37
#define LCD_GAMESCREEN_LINE_SCOREBOX_ROW4 (u8)(LCD_ROWS - 1)

#define LCD_GAMESCREEN_EDGE_COL_RIGHT     (u16)(LCD_COLUMNS - 1)
#define LCD_GAMESCREEN_EDGE_COL_LEFT      (u16)47
#define LCD_GAMESCREEN_EDGE_ROW_TOP       (u16)0
#define LCD_GAMESCREEN_EDGE_ROW_BOTTOM    (u16)(LCD_ROWS-1)

#define LCD_GAME_SIZE_ROW                 (u32)LCD_ROWS    /* Vertical game space size */
#define LCD_GAME_SIZE_COLUMN              (u32)(LCD_COLUMNS - LCD_GAMESCREEN_EDGE_COL_LEFT - 1)   /* Horizontal game space size */
#define LCD_PADDLE0_ROW                   (u8)0            /* Row where top left of paddle 0 image always occupies */
#define LCD_PADDLE1_ROW                   (u8)62           /* Row where top left of paddle 1 image always occupies */
#define LCD_BALL_START_ROW                (u8)((LCD_GAME_SIZE_ROW - LCD_IMAGE_BALL_ROW_SIZE) / 2)
#define LCD_BALL_START_COLUMN             (u8)(LCD_GAMESCREEN_EDGE_COL_LEFT + (LCD_GAME_SIZE_COLUMN / 2))

#define LCD_PLAYER_TEXT_COLUMN            (u8)3
#define LCD_PLAYER0_TEXT_ROW              (u8)2
#define LCD_PLAYER1_TEXT_ROW              (u8)39

#define LCD_PLAYER_SCORE_COLUMN           (u8)18
#define LCD_PLAYER0_SCORE_ROW             (u8)11
#define LCD_PLAYER1_SCORE_ROW             (u8)48

#define LCD_PLAYER_ARROW_COLUMN           (u8)6
#define LCD_PLAYER0_ARROW_ROW             (u8)(LCD_PLAYER0_SCORE_ROW + 1)
#define LCD_PLAYER1_ARROW_ROW             (u8)(LCD_PLAYER1_SCORE_ROW + 1)

/* Pong initial position settings */
#define SLIDER_HORIZONTAL_START_PONG   (u8)( (LCD_COLUMNS - LCD_IMAGE_PADDLE_COL_SIZE) / 2)


/* Pong ANT radio settings */
#define	ANT_SERIAL_LO_PONG			          (u8)0x34
#define ANT_SERIAL_HI_PONG			          (u8)0x12
#define ANT_SERIAL_PONG                   (u16)0x1234
#define	ANT_DEVICE_TYPE_PONG					    DEVICE_TYPE_PONG
#define	ANT_TRANSMISSION_TYPE_PONG		    (u8)0x01

/* Default channel configuration parameters */
#define ANT_CHANNEL_PONG                  (u8)0
#define ANT_CHANNEL_TYPE_PONG             CHANNEL_TYPE_MASTER
#define ANT_NETWORK_PONG                  (u8)0
#define ANT_CHANNEL_PERIOD_LO_PONG		    (u8)0xcd
#define ANT_CHANNEL_PERIOD_HI_PONG		    (u8)0x0c
#define ANT_FREQUENCY_PONG						    (50)
#define ANT_TX_POWER_PONG					  	    RADIO_TX_POWER_0DBM

/* MPG Level 2 specific Device Types */
#define	DEVICE_TYPE_PONG		              (u8)0x60
 

/*******************************************************************************
* Pong ANT message protocol
********************************************************************************
To communicate properly, data transferred between two Pong devices must follow a
particular message format.  Since the ANT protocol takes care of the data
integrity and since all communication messages will be fire-and-forget with non-
critical data, the Pong message protocol can simply worry about the data that needs
to be transferred.  Therefore, messages will be defined for specific purposes,
where the first data byte will indicate the message purpose and the 7 remaining
data bytes will be the corresponding message data.

The following messages are defined for the protocol.  For messages with a message
counter, the sending device should 0 this counter at the start of a state when the
message is sent, and increment it with every message it sends.  The message 
counter can be observed to see how well the systems are synchronized.
Population of the other fields is described with each message.
*/

#define INDEX_PONG_MESSAGE_ID         (u8)0
#define GAME_MESSAGE_BIT_MASK         (u8)0x80

/*------------------------------------------------------------------------------
This message is defined in ant.h but repeated here as it is used as part of the
PONG protocol.

ANT_TICK message: communicates the message period to the application.  

If ANT is running as a master, ANT_TICK occurs every time a broadcast or acknowledged
data message is sent. 

If ANT is running as a slave, ANT_TICK occurs when ever a message is received from the
master or if ANT misses a message that it was expecting based on the established timing 
of a paired channel (EVENT_RX_FAIL event is generated).  This should be communicated 
in case a missed message is important to any application using ANT.  

MSG_NAME  MSG_ID     D0       D1       D2      D3      D4      D5      D6
ANT_TICK   0x7F     0xFF     0xFF     0xFF    0xFF   MISSED  MISSED  MISSED
                                                      MSG #   MSG #   MSG #
                                                      HIGH    MID     LOW
--------------------------------------------------------------------------------
#define   MESSAGE_ANT_TICK                  (u8)0x7F
#define   ANT_TICK_MSG_ID                   (u8)0
#define   ANT_TICK_MSG_SENTINEL0            (u8)1
#define   ANT_TICK_MSG_SENTINEL1            (u8)2
#define   ANT_TICK_MSG_SENTINEL2            (u8)3
#define   ANT_TICK_MSG_SENTINEL3            (u8)4
#define   ANT_TICK_MSG_MISSED_HIGH_BYTE     (u8)5
#define   ANT_TICK_MSG_MISSED_MID_BYTE      (u8)6
#define   ANT_TICK_MSG_MISSED_LOW_BYTE      (u8)7
*/


/*------------------------------------------------------------------------------
IDLE message: sent when the radio is first activated.  The Master will send idle
messages and the slave will respond with an idle message.  The first two bytes
are fixed, bytes D2-D4 are unused, D5 and D6 are a two-byte message number counter
that should increment every message.

MSG_NAME  MSG_ID     D0       D1       D2      D3      D4      D5      D6
IDLE      0x01      0xAA     0x55     0x00    0x00   STATUS   MSG #   MSG #
                                                              HIGH    LOW
------------------------------------------------------------------------------*/
#define   PONG_MESSAGE_IDLE             (u8)0x01
#define   IDLE_MSG_ID                   (u8)0
#define   IDLE_MSG_SENTINEL_HIGH_BYTE   (u8)1
#define   IDLE_MSG_SENTINEL_LOW_BYTE    (u8)2
#define   IDLE_MSG_UNUSED0              (u8)3
#define   IDLE_MSG_UNUSED1              (u8)4
#define   IDLE_MSG_STATUS               (u8)5
#define   IDLE_MSG_HIGH_BYTE            (u8)6
#define   IDLE_MSG_LOW_BYTE             (u8)7

/*------------------------------------------------------------------------------
DATA message: sent by the Master to set the game parameters.  The slave should
send the message back to confirm that it has received the data and will use the
same settings for game play.  The Master always sets the game parameters.

MSG_NAME  MSG_ID     D0       D1       D2      D3      D4      D5      D6
DATA      0x02      ROW     COLUMN    ROW    COLUMN  STATUS   MSG #   MSG #
                   VECTOR   VECTOR   MOTION  MOTION           HIGH    LOW
                  (signed) (signed)
------------------------------------------------------------------------------*/
#define   PONG_MESSAGE_DATA             (u8)0x02
#define   DATA_MSG_ID                   (u8)0
#define   DATA_MSG_VECTOR_ROW_BYTE      (u8)1
#define   DATA_MSG_VECTOR_COLUMN_BYTE   (u8)2
#define   DATA_MSG_MOTION_ROW_BYTE      (u8)3
#define   DATA_MSG_MOTION_COLUMN_BYTE   (u8)4
#define   DATA_MSG_STATUS               (u8)5
#define   DATA_MSG_HIGH_BYTE            (u8)6
#define   DATA_MSG_LOW_BYTE             (u8)7


/*------------------------------------------------------------------------------
READY message: sent when game play is about to begin.  Score information is exchanged
and status bytes for the users are traded.  The system should be waiting on the
users to confirm their readiness at this point, and game play should commence once
both systems have sent and received two ready messages where both players are ready.
The Master populates all data fields except D3 which should contain the echoed
value last received from the Slave.  The Slave should echo back all data except
D3 which it should populate with its user data.

MSG_NAME  MSG_ID     D0       D1      D2       D3       D4     D5      D6
READY     0x03    PLAYER0  PLAYER1  PLAYER0 PLAYER1   0x00    MSG #   MSG #
                   SCORE    SCORE   STATUS  STATUS            HIGH    LOW
------------------------------------------------------------------------------*/
#define   PONG_MESSAGE_READY            (u8)0x03
#define   READY_MSG_ID                  (u8)0
#define   READY_MSG_SCORE_P0_BYTE       (u8)1
#define   READY_MSG_SCORE_P1_BYTE       (u8)2
#define   READY_MSG_STATUS_P0_BYTE      (u8)3
#define   READY_MSG_STATUS_P1_BYTE      (u8)4
#define   READY_MSG_UNUSED0             (u8)5
#define   READY_MSG_HIGH_BYTE           (u8)6
#define   READY_MSG_LOW_BYTE            (u8)7

#define   STATUS_NOT_READY              (u8)0
#define   STATUS_READY                  (u8)1


/*------------------------------------------------------------------------------
GAME message: sent during game play to provide paddle & ball position and game
status.  Though each device is essentially playing its own game, data for both
sides is exchanged for synchronization purposes.  This is a mini state machine that
runs to verify the state of the game as it progresses. Note that messages from the slave are sent on 
the back channel during the same ANT message period as the Master's message, except the Slave's message
data corresponds to the previous Master message.  Both sides then have a complete message period to react 
and queue the correct data for the next message period.

Whenever a "Verify" state occurs, the local system updates the game objects, clears the yellow and red LEDs, 
and resets the concecutive error counter.
Whenever a message fails, the system must revert to the last known confirmed message. No object updates are done, 
the yellow LED is turned on, and the consecutive error counter is incremented.  If the error count reaches
the maximum, the red LED is turned on, the point is stopped, and message is shown to the user.  The point is then
restarted.

A sample communication scenario is shown below
(a,b,c,... = sequence, # is current msg number in Game ID, P = paddle information, B = ball information):

MASTER                                              DIRECTION     SLAVE
a: 1, P0, B                                             >         Note msg #, save candidate P0 location, confirm B location
Verify 1, save candidate P1, confirm B                  <         b: 1, P1, B

c: 2, P0, B                                             >         Msg # advanced, so 'b' was received = update objects from 1, clear LED
Verify 2 so 'c' was received = update objects from 1    <         d: 2, P1, B

Fail scenario: response from slave not received
e: 3, P0, B                                             >         Msg # advanced, so 'd' was received = update objects from 2, clear LED
Fail 2, resend 'e', yellow LED on                       <         f: 3, P1, B

e: 3, P0, B                                             >         Msg # not advanced, so 'f' was not received = resend f
Verify 3 so 'e' was received = update objects from 2    <         f: 3, P1, B

Fail scenario: slave does not receive master's message
g: 4, P0, B                                             >         Fail, resend 'f', yellow LED on
Verify 3 so 'e' was received = update objects from 2    <         f: 3, P1, B       

g: 4, P0, B                                             >         Msg # advanced, so 'f' was received = update objects from 3, clear LED
Verify 4 so 'g' was received = update objects from 3    <         h: 4, P1, B

Note that it initially takes 5 message transfers before both Master and Slave
have first confirmed object positions and update their gameplay areas, but by the 5th message
there already is information about the next two updates in the pipeline.  As long as the pipeline is 
full, object updates occur every two messages unless an error occurs in which case the system must
revert back to the previous message.

If too many consecutive messages are missed, then the game play should be
paused with notification to the players and the point will be re-started when the system has
resynchronized.

MSG_NAME  MSG_ID     D0        D1       D2      D3       D4       D5       D6
GAME      0x8_    CURRENT   CURRENT  PADDLE0  PADDLE1   BALL     BALL     GAME
                    ROW     COLUMN   CURRENT  CURRENT  CURRENT  CURRENT  STATUS
                   VECTOR   VECTOR    ROW      ROW      ROW     COLUMN    
                  (signed) (signed)

The game status bits are:
Bit 0: Set if game in progress
Bit 1: Set if player 0 has scored
Bit 2: Set if player 1 has scored
Bit 3: Set when Player 0 has won
Bit 4: Set when Player 1 has won
Bit 5: Game play is paused
Bit 6: Player 0 has quit
Bit 7: Player 1 has quit

------------------------------------------------------------------------------*/
#define   PONG_MESSAGE_GAME             (u8)0x80
#define   PONG_MSG_ID                   (u8)0
#define   PONG_MSG_VECTOR_ROW_BYTE      (u8)1
#define   PONG_MSG_VECTOR_COLUMN_BYTE   (u8)2
#define   PONG_MSG_PADDLE0_BYTE         (u8)3
#define   PONG_MSG_PADDLE1_BYTE         (u8)4
#define   PONG_MSG_BALL_ROW_BYTE        (u8)5
#define   PONG_MSG_BALL_COLUMN_BYTE     (u8)6
#define   PONG_MSG_STATUS_BYTE          (u8)7

/* LGu8PongGameStatus bits */
#define   _PONG_MSG_STATUS_IN_PROGRESS  (u8)0x01
#define   _PONG_MSG_STATUS_P0_SCORED    (u8)0x02
#define   _PONG_MSG_STATUS_P1_SCORED    (u8)0x04
#define   _PONG_MSG_STATUS_P0_WINS      (u8)0x08
#define   _PONG_MSG_STATUS_P1_WINS      (u8)0x10
#define   _PONG_MSG_STATUS_PAUSED       (u8)0x20
#define   _PONG_MSG_STATUS_P0_QUIT      (u8)0x40
#define   _PONG_MSG_STATUS_P1_QUIT      (u8)0x80

#define   CONSECUTIVE_MSGS_REQUIRED     (u8)3
#define   CONSECUTIVE_MISSED_MSGS_MAX   (u8)10

/*******************************************************************************
* Function Declarations
*******************************************************************************/
/* Public functions */

/* Protected functions */
void PongInitialize(void);         

/* Private functions */
void PongResetGameObjects(void);
void PongResetGameFlags(void);
void PongSetBallVector(void);
void PongDrawGameScreen(void);
void PongUpdateScoresLCD(void);
void PongProcessGameMessages(void);
void PongUpdateBallPosition(void);
void PongUpdateGameMessage(void);
void PongErrorHandler(u8 u8ErrorNumber_);

/*******************************************************************************
* State Machine Declarations
*******************************************************************************/

void PongSM_Idle(void);       
void PongSM_Searching(void);
void PongSM_DataExchange(void);
void PongSM_Ready(void);
void PongSM_WaitRemote(void);
void PongSM_StartSync(void);
void PongSM_MasterSync(void);
void PongSM_MasterVerifySync(void);
void PongSM_MasterSyncFail(void);
void PongSM_StartDelay(void);
void PongSM_GamePlay(void);
void PongSM_PointScored(void);
void PongSM_PointWait(void);
void PongSM_PointSync(void);
void PongSM_AntChannelClose(void);
void PongSM_Error(void);



#endif /* __PONG_H */
