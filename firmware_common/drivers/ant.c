/***********************************************************************************************************************
File: ant.c                                                               

Description:
ANT implementation for Atmel SAM3U2 Cortex-M3 / Ant slave SPI.  This driver requires synchronous
SPI slave connection to an AP2 or AP2-emulated ANT device (the ANT device is the Master).  
All interface layer code is maintained here.  
Transmitted messages use the Message task; received messages use an SSP peripheral
with SPI_SLAVE_FLOW_CONTROL.

------------------------------------------------------------------------------------------------------------------------


***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
* Bookmarks
************************************************************************************************************************
!!!!! ANT Serial-layer Functions
@@@@@ ANT Interface-layer Functions
##### ANT State Machine Definition                                             
***********************************************************************************************************************/

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>Ant"
***********************************************************************************************************************/

/* New variables */
u32 G_u32AntFlags;                                    /* Flag bits for ANT-related information */

AntAssignChannelInfoType G_asAntChannelConfiguration[ANT_NUM_CHANNELS]; /* Keeps track of all configured ANT channels */
AntMessageResponseType G_stMessageResponse;           /* Holds the latest message response info */

AntApplicationMsgListType *G_sAntApplicationMsgList;  /* Public linked list of messages from ANT to the application */

u8 G_au8AntMessageOk[]     = "OK\n\r";
u8 G_au8AntMessageFail[  ] = "FAIL\n\r";

/* Replace 'd' at [12] with channel number */
u8 G_au8AntMessageAssign[]    = "ANT channel d assign "; 
u8 G_au8AntMessageUnassign[]  = "ANT channel d unassign ";
u8 G_au8AntMessageUnhandled[] = "ANT channel d message 0xxx response dd ";
u8 G_au8AntMessageSetup[] = "ANT channel d setup ";
u8 G_au8AntMessageClose[] = "ANT channel d close ";
u8 G_au8AntMessageOpen[]  = "ANT channel d open ";

u8 G_au8AntMessageInit[]  = "Initializing ANT... ";
u8 G_au8AntMessageInitFail[] = "failed. Host IOs set to HiZ.\r\n";
u8 G_au8AntMessageNoAnt[] = "\n\r### nRF51422 Programming Mode: no ANT functionality ####\n\r";


/* ANT message structures */
u8 G_au8AntSetNetworkKey[]    = {MESG_NETWORK_KEY_SIZE, MESG_NETWORK_KEY_ID, 0 /* Network number */, 
                                 0 /* KEY0 */, 0 /* KEY1 */, 0 /* KEY2 */, 0 /* KEY3 */, 
                                 0 /* KEY4 */, 0 /* KEY5 */, 0 /* KEY6 */, 0 /* KEY7 */, CS};
u8 G_au8AntAssignChannel[]    = {MESG_ASSIGN_CHANNEL_SIZE, MESG_ASSIGN_CHANNEL_ID, 0 /* AntChannel */, 
                                 0 /* AntChannelType */, 0 /* AntNetwork */, CS};
u8 G_au8AntSetChannelID[]     = {MESG_CHANNEL_ID_SIZE, MESG_CHANNEL_ID_ID, 0 /* AntChannel */, 
                                 0 /* AntDeviceIdLo */, 0 /* AntDeviceIdHi */, 0 /* AntDeviceType */, 
                                 0 /* AntTransmissionType */, CS};
u8 G_au8AntSetChannelPeriod[] = {MESG_CHANNEL_MESG_PERIOD_SIZE, MESG_CHANNEL_MESG_PERIOD_ID, 0 /* AntChannel */, 
                                 0 /* AntChannelPeriodLo */, 0 /* AntChannelPeriodHi */, CS};
u8 G_au8AntSetChannelRFFreq[] = {MESG_CHANNEL_RADIO_FREQ_SIZE, MESG_CHANNEL_RADIO_FREQ_ID, 0 /* AntChannel */, 
                                 0 /* AntFrequency */, CS};           
u8 G_au8AntSetChannelPower[]  = {MESG_RADIO_TX_POWER_SIZE, MESG_RADIO_TX_POWER_ID, 0 /* AntChannel */, 
                                 0 /* AntTxPower */, CS};        
u8 G_au8AntLibConfig[]        = {MESG_LIB_CONFIG_SIZE, MESG_LIB_CONFIG_ID, 0, LIB_CONFIG_CHANNEL_ID_FLAG | LIB_CONFIG_RSSI_FLAG, CS};        

u8 G_au8AntBroadcastDataMessage[] = {MESG_DATA_SIZE, MESG_BROADCAST_DATA_ID, CH, D_0, D_1, D_2, D_3, D_4, D_5, D_6, D_7, CS};
u8 G_au8AntAckDataMessage[] = {MESG_DATA_SIZE, MESG_ACKNOWLEDGED_DATA_ID, CH, D_0, D_1, D_2, D_3, D_4, D_5, D_6, D_7, CS};
u8 G_au8ANTGetVersion[]     = {MESG_REQUEST_SIZE, MESG_REQUEST_ID, 0, MESG_VERSION_ID, 0};   


/*----------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) 
and indicate what file the variable is defined in. */
extern u32 G_u32SystemFlags;                            /* From main.c */
extern u32 G_u32ApplicationFlags;                       /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                 /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                  /* From board-specific source file */

extern volatile u32 ANT_SSP_FLAGS;                      /* From configuration.h */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Ant_<type>Name" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Ant_pfnStateMachine;                 /* The ANT state machine function pointer */
static u32 Ant_u32RxTimer;                              /* Dedicated timer for receiving bytes */
static u32 Ant_u32TxTimer;                              /* Dedicated timer for transmitting bytes */

static u32 Ant_u32TxByteCounter = 0;                    /* Counter counts callbacks on sent bytes */
static u32 Ant_u32RxByteCounter = 0;                    /* Counter counts callbacks on received bytes */
static u32 Ant_u32RxTimeoutCounter = 0;                 /* Increments any time an ANT reception times out */
static u32 Ant_u32UnexpectedByteCounter = 0;            /* Increments any time a byte is received that was not expected value */
static u32 Ant_u32CurrentTxMessageToken = 0;            /* Token for message currently being sent to ANT */

static SspConfigurationType Ant_sSspConfig;             /* Configuration information for SSP peripheral */
static SspPeripheralType* Ant_Ssp;                      /* Pointer to Ant's SSP peripheral object */

static u8 Ant_u8AntVersion[MESG_VERSION_SIZE];          /* ANT device version string */

static u8 Ant_au8AntRxBuffer[ANT_RX_BUFFER_SIZE];       /* Space for verified received ANT messages */
static u8 *Ant_pu8AntRxBufferNextChar;                  /* Pointer to next char to be written in the AntRxBuffer */
static u8 *Ant_pu8AntRxBufferCurrentChar;               /* Pointer to the current char in the AntRxBuffer */
static u8 *Ant_pu8AntRxBufferUnreadMsg;                 /* Pointer to unread chars in the AntRxBuffer */
static u8 Ant_u8AntNewRxMessages;                       /* Counter for number of new messages in AntRxBuffer */

static u32 Ant_u32ApplicationMessageCount = 0;          /* Counts messages queued on G_sAntApplicationMsgList */
static AntOutgoingMessageListType *Ant_psDataOutgoingMsgList; /* Linked list of outgoing ANT-formatted messages */
static u32 Ant_u32OutgoingMessageCount = 0;             /* Counts messages queued on Ant_psDataOutgoingMsgList */

static u8 Ant_u8SlaveMissedMessageHigh = 0;             /* Counter for missed messages if device is a slave */
static u8 Ant_u8SlaveMissedMessageMid = 0;              /* Counter for missed messages if device is a slave */
static u8 Ant_u8SlaveMissedMessageLow = 0;              /* Counter for missed messages if device is a slave */


/* Debug variables */
static u32 Ant_DebugRxMessageCounter = 0;
static u32 Ant_DebugTotalRxMessages = 0;
static u32 Ant_DebugProcessRxMessages = 0;
static u32 Ant_DebugQueuedDataMessages = 0;


/***********************************************************************************************************************
!!!!! ANT Serial-layer Functions
***********************************************************************************************************************/

/* ANT Private Serial-layer Functions */

/*------------------------------------------------------------------------------
Function: AntSyncSerialInitialize

Description:
Properly sets up the ANT SPI interface and tests Host <-> ANT communications.

Requires:
  - ANT_SPI is configured
  - !CS (SEN) interrupt should be enabled
// - GPIO interrupts are configured and enabled

Promises:
  - Ant_pu8AntRxBufferNextChar is initialized to start of AntRxBuffer
  - Ant_pu8AntRxBufferUnreadMsg is initialized to start of AntRxBuffer
  - Ant_u8AntNewRxMessages = 0;
  - ANT SSP CS and RX interrupts are enabled
  - If ANT starts up correctly and responds to version request, then 
    G_u32SystemFlags _APPLICATION_FLAGS_ANT is set and Ant_u8AntVersion is populated
    with the returned version information from the ANT IC.  
*/
static void AntSyncSerialInitialize(void)
{
  u32 u32EventTimer;
  bool bErrorStatus = FALSE;
  
  /* Initialize buffer pointers */  
  Ant_pu8AntRxBufferNextChar    = Ant_au8AntRxBuffer;
  Ant_pu8AntRxBufferCurrentChar = Ant_au8AntRxBuffer;
  Ant_pu8AntRxBufferUnreadMsg   = Ant_au8AntRxBuffer;
  Ant_u8AntNewRxMessages = 0;
 
  /* Reset the 51422 and initialize SRDY and MRDY */
  u32EventTimer = G_u32SystemTime1ms;
  ANT_RESET_ASSERT();
  SYNC_MRDY_DEASSERT();
  SYNC_SRDY_DEASSERT();
  while( !IsTimeUp(&u32EventTimer, ANT_RESET_WAIT_MS) );
  ANT_RESET_DEASSERT();
  u32EventTimer = G_u32SystemTime1ms;
  while( !IsTimeUp(&u32EventTimer, ANT_RESTART_DELAY_MS) );

  
  /* ANT should want to send message 0x6F now to indicate it has reset */
  u32EventTimer = G_u32SystemTime1ms;
  while( !IS_SEN_ASSERTED() && !bErrorStatus )
  {
    bErrorStatus = IsTimeUp(&u32EventTimer, ANT_MSG_TIMEOUT_MS);
  }

  if (!bErrorStatus)
  {
    /* Receive and process the restart message */
    AntRxMessage();
    AntProcessMessage();   

  /* Send out version request message and expect response */
    G_au8ANTGetVersion[4] = AntCalculateTxChecksum(&G_au8ANTGetVersion[0]);
    AntTxMessage(&G_au8ANTGetVersion[0]);    
    AntExpectResponse(MESG_VERSION_ID, ANT_MSG_TIMEOUT_MS);
  }
 
} /* end AntSyncSerialInitialize */


/*-----------------------------------------------------------------------------
Function: AntSrdyPulse

Description:
Pulses Srdy with added delays on the front and middle. 

Requires:
  - 

Promises:
  - A delay controlled by ANT_SRDY_DELAY is passed
  - SRDY is asserted
  - A delay of ANT_SRDY_PERIOD is passed
  - SRDY is deasserted
*/
static void AntSrdyPulse(void)
{
  for(u32 i = 0; i < ANT_SRDY_DELAY; i++);
  SYNC_SRDY_ASSERT();
  
  for(u32 i = 0; i < ANT_SRDY_PERIOD; i++);
  SYNC_SRDY_DEASSERT();

} /* end AntSrdyPulse() */


/*-----------------------------------------------------------------------------
Function: AntRxMessage

Description:
Completely receive a message from ANT to the Host.  Incoming bytes are deposited directly into the receive
buffer from the SSP ISR which should be extremely fast and complete in a maximum of 500us.  

Requires:
  - _SSP_CS_ASSERTED is set indicating a message is ready to come in 
  - G_u32AntFlags _ANT_FLAGS_TX_INTERRUPTED is set if the system wanted to transmit
    but ANT wanted to send a message at the same time (so MESG_TX_SYNC has already 
    been received); _SSP_RX_COMPLETE must still be set from this.
  - Ant_pu8AntRxBufferCurrentChar points to the first byte of the message

Promises:
  - If a good new message has been received, then Ant_u8AntNewMessages is incremented
    and the message is at Ant_pu8AntRxBufferUnreadMsg in Ant_au8AntRxBuffer
  - If a good message is not received, then Ant_u8AntNewMessages is unchanged.
  - In both cases, Ant_pu8AntRxBufferNextChar points at the next empty buffer location
*/
static void AntRxMessage(void)
{
  u8 u8Checksum;
  u8 u8Length;
  u32 u32CurrentRxByteCount;
  u8 au8RxTimeoutMsg[] = "AntRx: timeout\n\r";
  u8 au8RxFailMsg[] = "AntRx: message failed\n\r";
  bool bReceptionError = FALSE;
  
  /* Ensure we have CS flag */
  if( !( IS_SEN_ASSERTED() ) )
  {
    return;
  }
  
  /* Initialize the receive timer and get a snapshot of current byte count */
  u32CurrentRxByteCount = Ant_u32RxByteCounter;
  Ant_u32RxTimer = 0;
  
  /* If the Global _ANT_FLAGS_TX_INTERRUPTED flag has been set, then we have already read the TX_SYNC byte */
  if(G_u32AntFlags & _ANT_FLAGS_TX_INTERRUPTED)
  {
    /* Clear flag and load the value we know was already received to allow the function to proceed*/
    G_u32AntFlags &= ~_ANT_FLAGS_TX_INTERRUPTED;
    
    /* Adjust the starting byte counter since it came in during AntTxMessage */
    u32CurrentRxByteCount--;
    
  }
  /* Otherwise we need to first read the sync byte  */
  else
  {
    /* Do short delay then cycle SRDY to get the first byte */
    AntSrdyPulse();

    /* Begin the receive cycle that takes place using interrupts and callbacks and is monitored by a timeout of 
    about 500us - this should be plenty of time to receive even the longest ANT message but still only half the 
    allowed 1ms loop time for the system. */
    
    /* Read the first byte when it comes in */
    while( !(ANT_SSP_FLAGS & _SSP_RX_COMPLETE) &&
           (Ant_u32RxTimer < ANT_ACTIVITY_TIME_COUNT) )
    {
      Ant_u32RxTimer++;
    }
  }
  
  if( Ant_u32RxTimer > ANT_ACTIVITY_TIME_COUNT) 
  {
    AntAbortMessage();
    DebugPrintf(au8RxTimeoutMsg);
    return;
  }

  /* _SSP_RX_COMPLETE flag will be set and the Rx callback will have run.  
  The callback does NOT toggle SRDY yet.  _SSP_RX_COMPLETE should still
  be set from AntTxMessage if that's what got us here. */
  ANT_SSP_FLAGS &= ~_SSP_RX_COMPLETE;
   
  /* One way or the other, we now have a potential SYNC byte at Ant_pu8AntRxBufferCurrentChar.  
  Proceed to test it and receive the rest of the message */
  if (*Ant_pu8AntRxBufferCurrentChar == MESG_TX_SYNC)                     
  {
    /* Flag that a reception is in progress */
    G_u32AntFlags |= _ANT_FLAGS_RX_IN_PROGRESS;
    
    /* Delay and then cycle SRDY to get the next byte (length) */
    AntSrdyPulse();
    
    /* The SSP interrupts and Rx callback handle the rest of the reception until a full message is received. 
    We know it is received when SEN is deasserted. */
    while( IS_SEN_ASSERTED() && (Ant_u32RxTimer < ANT_ACTIVITY_TIME_COUNT) )
    {
      Ant_u32RxTimer++;
    }
  
    /* One way or another, this Rx is done! */
    G_u32AntFlags &= ~_ANT_FLAGS_RX_IN_PROGRESS;
    ANT_SSP_FLAGS &= ~_SSP_RX_COMPLETE;

    /* Check that the above loop ended as expected and didn't time out */
    if(Ant_u32RxTimer < ANT_ACTIVITY_TIME_COUNT)
    {  
      /* Update counter to see how many bytes we should have */
      u32CurrentRxByteCount = Ant_u32RxByteCounter - u32CurrentRxByteCount;
    
      /* RxBufferCurrentChar is still pointing to the SYNC byte. Validate what should be a complete message now. */
      u8Checksum = *Ant_pu8AntRxBufferCurrentChar;
      AdvanceAntRxBufferCurrentChar();
      
      /* Read the length byte and add two to count the length byte and message ID but not checksum as length will be our checksum counter */
      u8Length = *Ant_pu8AntRxBufferCurrentChar + 2;  
      
      /* Optional check (u8Length does not include the SYNC byte or Checksum byte so add 2) */
      if(u32CurrentRxByteCount != (u8Length + 2) )
      {
        /* Could throw out the message right away - this could save some potential weird memory accesses
        if there was any corruption or a wild u8Length value */
        G_u32AntFlags |= _ANT_FLAGS_LENGTH_MISMATCH;
      }
 
      /* Validate the remaining bytes based on u8Length*/
      do
      {
        u8Checksum ^= *Ant_pu8AntRxBufferCurrentChar;                     
        AdvanceAntRxBufferCurrentChar();
      } while (--u8Length);
      
      /* AntRxBufferCurrentChar is pointing to the last received byte that should be the checksum. */
      if (u8Checksum == *Ant_pu8AntRxBufferCurrentChar)      
      {
        Ant_u8AntNewRxMessages++;
        Ant_DebugTotalRxMessages++;
      }
      /* If the message was not good, then move Ant_pu8AntRxBufferUnreadMsg passed the garbage data */
      else
      {
        Ant_pu8AntRxBufferUnreadMsg = Ant_pu8AntRxBufferCurrentChar;
        AdvanceAntRxBufferUnreadMsgPointer();
      }
    } 
    else
    {
      Ant_u32RxTimeoutCounter++;
      bReceptionError = TRUE;
    }
  } /* end if(*Ant_pu8AntRxBufferCurrentChar == MESG_TX_SYNC) */
  else
  {
    /* Otherwise we have received an unexpected byte -- flag it, clear Ssp flags and abandon the byte */
    Ant_u32UnexpectedByteCounter++;
    bReceptionError = TRUE;
  }

  /* If a reception error has occured, */
  if(bReceptionError)
  {
    /* Toggle SRDY until CS deasserts and throw out the message */
    DebugPrintf(au8RxFailMsg);
    while( IS_SEN_ASSERTED()  && (Ant_u32RxTimer < ANT_ACTIVITY_TIME_COUNT) )
    {
      Ant_u32RxTimer++;
      AntSrdyPulse();
    }
   
    /* Since we have flow control, we can safely assume that no other messages
    have come in and Ant_pu8AntRxBufferNextChar is pointing to where the next 
    valid message WILL come in - so push all the pointers there. */
    Ant_pu8AntRxBufferCurrentChar = Ant_pu8AntRxBufferNextChar;
    Ant_pu8AntRxBufferUnreadMsg = Ant_pu8AntRxBufferNextChar;
    ANT_SSP_FLAGS &= ~(_SSP_TX_COMPLETE | _SSP_RX_COMPLETE);

  }
  else
  {
    /* In all other cases, finish by advancing the current byte pointer */
    AdvanceAntRxBufferCurrentChar();
  }
  
} /* end AntRxMessage() */


/*-----------------------------------------------------------------------------
Function: AntAbortMessage

Description:
Kills the current message in progress with ANT and resets all of the pointers.
Any existing received buffer data is lost.

*** Warning: all interrupts are disabled, so this could impact system tick time ***

Requires:
  - 

Promises:
  - Ant_pu8AntRxBufferNextChar, Ant_pu8AntRxBufferCurrentChar, Ant_pu8AntRxBufferUnreadMsg,
    and Ant_u8AntNewRxMessages reset.
*/
static void AntAbortMessage(void)
{
  __disable_interrupt();
  Ant_pu8AntRxBufferNextChar = &Ant_au8AntRxBuffer[0];
  Ant_pu8AntRxBufferCurrentChar = &Ant_au8AntRxBuffer[0];
  Ant_pu8AntRxBufferUnreadMsg = &Ant_au8AntRxBuffer[0];
  Ant_u8AntNewRxMessages = 0;
  __enable_interrupt();
  
} /* end AntAbortMessage() */


/*-----------------------------------------------------------------------------
Function: AdvanceAntRxBufferCurrentChar

Description:
Safely advances the Ant_pu8AntRxBufferCurrentChar within the circular buffer

Requires:
  - No interrupt functions using Ant_pu8AntRxBufferCurrentChar are allowed to run

Promises:
  - Ant_pu8AntRxBufferCurrentChar points to the next char to write in Ant_au8AntRxBuffer
*/
static void AdvanceAntRxBufferCurrentChar(void)
{
  Ant_pu8AntRxBufferCurrentChar++;
  if(Ant_pu8AntRxBufferCurrentChar == &Ant_au8AntRxBuffer[ANT_RX_BUFFER_SIZE])
  {
    Ant_pu8AntRxBufferCurrentChar = &Ant_au8AntRxBuffer[0];
  }
  
} /* end AdvanceAntRxBufferCurrentChar() */


/*-----------------------------------------------------------------------------/
Function: AdvanceAntRxBufferUnreadMsgPointer

Description:
Increments the Rx buffer pointer and handles the wrap-around.

Requires:
  - Ant_pu8AntRxBufferUnreadMsg cannot be moved in an ISR

Promises:
  - Ant_pu8AntRxBufferUnreadMsg is pointing to the next char in the circular buffer
*/
static void AdvanceAntRxBufferUnreadMsgPointer()
{
  Ant_pu8AntRxBufferUnreadMsg++;
  if (Ant_pu8AntRxBufferUnreadMsg == &Ant_au8AntRxBuffer[ANT_RX_BUFFER_SIZE])
  {
    Ant_pu8AntRxBufferUnreadMsg = &Ant_au8AntRxBuffer[0];
  }
  
} /* end AdvanceAntRxBufferUnreadMsgPointer() */

  
/*------------------------------------------------------------------------------
Function: AntParseExtendedData

Description:
Reads extended data based on the flags that are set.
Loads into AntExtendedDataType which currently has these fields:
{
  u8 u8Channel;                        
  u16 u16DeviceID;                         
  u8 u8DeviceType;                         
  u8 u8TransType;                          
  u8 u8Flags;                             
  s8 s8RSSI;                              
} AntExtendedDataType;

Requires:
  - pu8SourceMessage_ points to an ANT message buffer that holds a complete ANT data
    message structure except for SYNC byte.  antmessage.h Buffer Indices an then be used.
  - psExtDataTarget_ points to the target AntExtendedDataType structure

Promises:
  - If extended data is present, all values are read into local variables and then
    loaded into psExtDataTarget_ and the function returns TRUE.
  - If no extended data is present, psExtDataTarget_ is set to default values and
    the function returns FALSE.
*/
static bool AntParseExtendedData(u8* pu8SourceMessage, AntExtendedDataType* psExtDataTarget_)
{
  bool bReturnValue;
  u8 u8MessageSize;
  u8 u8Channel;                        
  u8 u8Flags = 0;       
  u8 u8BufferOffset = 0;
  
  /* Channel ID extended data */
  u16 u16DeviceID = 0xFFFF;;                         
  u8 u8DeviceType = 0xFF;                        
  u8 u8TransType = 0xFF;                          

  /* RSSI extended data */
  s8 s8RSSI = 0xFF;   
  //For now, we don't have a use for these two:
  //u8 u8MeasurementType = 0xFF;
  //u8 u8Threshold = 0xFF;
  
  /* RF Timestamp data */
  u16 u16RxTimestamp = 0xFFFF;
    
  /* Get generic data */
  u8MessageSize = *(pu8SourceMessage + BUFFER_INDEX_MESG_SIZE);
  u8Channel = *(pu8SourceMessage + BUFFER_INDEX_CHANNEL_NUM);

  /* Check to see if the message is the regular size (MESG_MAX_DATA_SIZE) */
  if(u8MessageSize == MESG_MAX_DATA_SIZE)
  {
    bReturnValue = FALSE;
  }
  
  /* Check for a message that is too big or too small */
  else if( (u8MessageSize > MESG_MAX_SIZE) ||
           (u8MessageSize < MESG_MAX_DATA_SIZE) )
  {
    DebugPrintf("\n\rUnexpected ANT message size\n\n\r");
    bReturnValue = FALSE;
  }
  
  /* Otherwise we have some extended message data */
  else 
  {
    /* Byte after data must be flag byte */
    u8Flags = *(pu8SourceMessage + BUFFER_INDEX_EXT_DATA_FLAGS);
    bReturnValue = TRUE;
    
    /* Channel ID information is always first if it's there */
    if(u8Flags & LIB_CONFIG_CHANNEL_ID_FLAG)
    {
      u16DeviceID = (u16)(*(pu8SourceMessage + BUFFER_INDEX_EXT_DATA + u8BufferOffset)) & 0x00FF;
      u8BufferOffset++;
      u16DeviceID |= ( (u16)(*(pu8SourceMessage + BUFFER_INDEX_EXT_DATA + u8BufferOffset)) << 8 ) & 0xFF00;
      u8BufferOffset++;
      u8DeviceType = *(pu8SourceMessage + BUFFER_INDEX_EXT_DATA + u8BufferOffset);
      u8BufferOffset++;
      u8TransType = *(pu8SourceMessage + BUFFER_INDEX_EXT_DATA + u8BufferOffset);
      u8BufferOffset++;
    }
    
    /* RSSI information is always next if it's there */
    if(u8Flags & LIB_CONFIG_RSSI_FLAG)
    {
      //u8MeasurementType = *(pu8SourceMessage + BUFFER_INDEX_EXT_DATA + u8BufferOffset);
      u8BufferOffset++;
      s8RSSI = *(pu8SourceMessage + BUFFER_INDEX_EXT_DATA + u8BufferOffset);
      u8BufferOffset++;
      //u8Threshold = *(pu8SourceMessage + BUFFER_INDEX_EXT_DATA + u8BufferOffset);
      u8BufferOffset++;
   }   

    /* Timestamp information is always last */
    if(u8Flags & LIB_CONFIG_RX_TIMESTAMP_FLAG)
    {
      u16RxTimestamp = (u16)(*(pu8SourceMessage + BUFFER_INDEX_EXT_DATA + u8BufferOffset)) & 0x00FF;
      u8BufferOffset++;
      u16RxTimestamp |= ( (u16)(*(pu8SourceMessage + BUFFER_INDEX_EXT_DATA + u8BufferOffset)) << 8 ) & 0xFF00;
    }
  }
  
  /* Load psExtDataTarget_ and return */
  psExtDataTarget_->u8Flags      = u8Flags;
  psExtDataTarget_->u8Channel    = u8Channel;
  psExtDataTarget_->u16DeviceID  = u16DeviceID;
  psExtDataTarget_->u8DeviceType = u8DeviceType;
  psExtDataTarget_->u8TransType  = u8TransType;
  psExtDataTarget_->s8RSSI       = s8RSSI;

  return bReturnValue;
  
} /* end AntParseExtendedData */


/***********************************************************************************************************************
@@@@@ ANT Interface-layer Functions
***********************************************************************************************************************/
  
/*------------------------------------------------------------------------------
Function: AntInitialize

Description:
Intialize the ANT system.  ANT device is reset and communication checked through
a version request.  The main channel parameters are then set up to default values.
  
Requires:
  - ANT_SPI peripheral is correctly configured
  - Debug system is initialized so debug messages can be sent to UART

Promises:
  - G_stAntSetupData set to default ANT values
  - If all successful, G_u32SystemFlags.AntGood bit is set and ANT system is ready
  - Ant_pfnStateMachine = AntSM_Idle
*/
void AntInitialize(void)
{
  u32 u32AntPortAPins, u32AntPortBPins;

  if(G_u32SystemFlags & _SYSTEM_STARTUP_NO_ANT)
  {
    DebugPrintf(G_au8AntMessageNoAnt);
    Ant_pfnStateMachine = AntSM_NoResponse;
  }
  else
  {
    /* Give PIO control of ANT_RESET line */
    AT91C_BASE_PIOB->PIO_OER = PB_21_ANT_RESET;
    
    /* Announce on the debug port that ANT setup is starting and intialize pointers */
    DebugPrintf(G_au8AntMessageInit);
    G_sAntApplicationMsgList = 0;
    Ant_psDataOutgoingMsgList = 0;
  
  /* Initialize the G_asAntChannelConfiguration data struct */
  for(u8 i = 0; i < ANT_NUM_CHANNELS; i++)
  {
    G_asAntChannelConfiguration[i].AntChannel          = (AntChannelNumberType)i;
    G_asAntChannelConfiguration[i].AntChannelType      = 0xFF;
    G_asAntChannelConfiguration[i].AntNetwork          = 0xFF;
    G_asAntChannelConfiguration[i].AntDeviceIdLo       = 0xFF;
    G_asAntChannelConfiguration[i].AntDeviceIdHi       = 0xFF;
    G_asAntChannelConfiguration[i].AntDeviceType       = 0xFF;
    G_asAntChannelConfiguration[i].AntTransmissionType = 0xFF;
    G_asAntChannelConfiguration[i].AntChannelPeriodLo  = 0xFF;
    G_asAntChannelConfiguration[i].AntChannelPeriodHi  = 0xFF;
    G_asAntChannelConfiguration[i].AntFrequency        = 0xFF;
    G_asAntChannelConfiguration[i].AntTxPower          = 0xFF;
    G_asAntChannelConfiguration[i].AntFlags            = 0;
    
    for(u8 j = 0; j < ANT_NETWORK_NUMBER_BYTES; j++)
    {
      G_asAntChannelConfiguration[i].AntNetworkKey[j] = 0;
    }
  }
    
    /* Configure the SSP resource to be used for the application */
    Ant_sSspConfig.SspPeripheral      = ANT_SPI;
    Ant_sSspConfig.pCsGpioAddress     = ANT_SPI_CS_GPIO;
    Ant_sSspConfig.u32CsPin           = ANT_SPI_CS_PIN;
    Ant_sSspConfig.eBitOrder          = LSB_FIRST;
    Ant_sSspConfig.eSspMode           = SPI_SLAVE_FLOW_CONTROL;
    Ant_sSspConfig.fnSlaveTxFlowCallback = AntTxFlowControlCallback;
    Ant_sSspConfig.fnSlaveRxFlowCallback = AntRxFlowControlCallback;
    Ant_sSspConfig.pu8RxBufferAddress = Ant_au8AntRxBuffer;
    Ant_sSspConfig.ppu8RxNextByte     = &Ant_pu8AntRxBufferNextChar;
    Ant_sSspConfig.u16RxBufferSize    = ANT_RX_BUFFER_SIZE;

    Ant_Ssp = SspRequest(&Ant_sSspConfig);
    ANT_SSP_FLAGS = 0;
    
    /* Reset ANT, activate SPI interface and get a test message */
    AntSyncSerialInitialize();
    
    /* Report status out the debug port */
    if(G_u32ApplicationFlags & _APPLICATION_FLAGS_ANT)  
    {
      DebugPrintf(G_au8AntMessageOk);
      DebugPrintf("ANT version: ");
      DebugPrintf(Ant_u8AntVersion);
      DebugLineFeed();
      
      G_u32AntFlags &= ~_ANT_FLAGS_RESTART;
      Ant_pfnStateMachine = AntSM_Idle;
    }
    else
    {
      /* The ANT device is not responding -- it may be dead, or it may not yet
      be loaded with any firmware.  Regardless, float all of the interface lines so 
      that any programmer or other firmware will not be impacted by the Host MCU */
      DebugPrintf(G_au8AntMessageInitFail);

      /* Make sure all ANT pins are on the PIO controller */
      u32AntPortAPins = ANT_PIOA_PINS;
      u32AntPortBPins = ANT_PIOB_PINS;
      
      AT91C_BASE_PIOA->PIO_PDR = u32AntPortAPins;
      AT91C_BASE_PIOA->PIO_PER = u32AntPortAPins;
      AT91C_BASE_PIOB->PIO_PDR = u32AntPortBPins;
      AT91C_BASE_PIOB->PIO_PER = u32AntPortBPins;

      /* Disable all outputs (set to HiZ input) */
      AT91C_BASE_PIOA->PIO_ODR = u32AntPortAPins;
      AT91C_BASE_PIOB->PIO_ODR = u32AntPortBPins;
      
      Ant_pfnStateMachine = AntSM_NoResponse;
    }
  }
} /* end AntInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function AntRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void AntRunActiveState(void)
{
  Ant_pfnStateMachine();

} /* end AntRunActiveState */


/*-----------------------------------------------------------------------------
Function: AntTxMessage

Description:
Send a message from the Host to the ANT device.  To do this, we must tell ANT that we have
a message to send by asserting MRDY, wait for ANT to acknowlege with SEN, then read a byte from
ANT to confirm the transmission can proceed.  If ANT happens to wants to send a message at the
same time, the byte it sends will be an Rx byte so the AntTxMessage must suspend and go read the 
incoming message first.  The process would restart after that.

Once ANT confirms that the Host may transmit, the message to transmit is queued and data is sent byte-by-byte with SRDY used for flow
control after each byte.  Due to the speed of the chip-to-chip communications, even the longest ANT message
should be able to send in less than 500us so it will likely be done on the main program cycle that
immediately follows this call.  

Requires:
  - pu8AntTxMessage_ points to an Ant formatted message where the first data byte
    is the length byte (since ANT sends the SYNC byte) and the last byte is
    the checksum.

Promises:
  - Returns TRUE if the transmit message is queued successfully; Ant_u32CurrentTxMessageToken holds the message token
  - Returns FALSE if the transfer couldn't start or if receive message interrupted
    (G_u32AntFlags _ANT_FLAGS_TX_INTERRUPTED is set;  AntRxBufferCurrentChar pointing to the received byte).
  - MRDY is deasserted
*/
bool AntTxMessage(u8 *pu8AntTxMessage_)
{
  u8 u8Byte;
  u32 u32Length;
  u32 u32TimeOut = G_u32SystemTime1s;
  u8 au8TxInProgressMsg[] = "AntTx: msg already in progress\n\r";
  u8 au8TxTimeoutMsg[]    = "AntTx: SEN timeout\n\r";
  u8 au8TxNoTokenMsg[]    = "AntTx: No token\n\r";
  u8 au8TxNoSyncMsg[]     = "AntTx: No SYNC\n\r";

  /* Check G_u32AntFlags first */
  if(G_u32AntFlags & (_ANT_FLAGS_TX_IN_PROGRESS | _ANT_FLAGS_RX_IN_PROGRESS) )
  {
    DebugPrintf(au8TxInProgressMsg);
    return FALSE;
  }
  
  /* Initialize the timeout timer and notify ANT that the Host wishes to send a message */
  Ant_u32RxTimer = 0;
  SYNC_MRDY_ASSERT();                          

  /* Wait for SEN to be asserted indicating ANT is ready for a message */
  while ( !IS_SEN_ASSERTED() && (Ant_u32RxTimer < ANT_ACTIVITY_TIME_COUNT) )
  {
    Ant_u32RxTimer++;
  }
  
  /* If we timed out, then clear MRDY and exit */
  if(Ant_u32RxTimer > ANT_ACTIVITY_TIME_COUNT)
  {
    SYNC_MRDY_DEASSERT();                          
    DebugPrintf(au8TxTimeoutMsg);
    return(FALSE);
  }
  
  /* Else we have SEN flag; queue to read 1 byte after a short delay before toggling SRDY */
  AntSrdyPulse();

  /* Wait for the first byte to come in via the ISR / Rx Callback*/
  while( !(ANT_SSP_FLAGS & _SSP_RX_COMPLETE) && (Ant_u32RxTimer < ANT_ACTIVITY_TIME_COUNT) )
  {
    Ant_u32RxTimer++;
  }

  /* Ok to deassert MRDY now */
  SYNC_MRDY_DEASSERT();                     

  /* If we timed out now, then clear MRDY and exit.  Because CS is still asserted, the task
  will attempt to read a message but fail and eventually abort. */
  if(Ant_u32RxTimer > ANT_ACTIVITY_TIME_COUNT)
  {
   DebugPrintf(au8TxTimeoutMsg);
   return(FALSE);
  }
          
  /* When the byte comes in, the SSP module will set the _SSP_RX_COMPLETE flag and also call the 
  Rx callback but does not toggle SRDY at this time.  We must look at this byte to determine if ANT 
  initiated this particular communication and is telling us that a message is coming in, or if we 
  initiated the communication and ANT is allowing us to transmit. */

  /* Read the byte - don't advance the pointer yet */
  u8Byte = *Ant_pu8AntRxBufferCurrentChar;                       

  /* If the byte is TX_SYNC, then ANT wants to send a message which must be done first */
  if (u8Byte == MESG_TX_SYNC)                     
  {
    G_u32AntFlags |= _ANT_FLAGS_TX_INTERRUPTED;
    return(FALSE);
  }

  /* Since the Rx byte is in our Rx buffer, advance both pointers since it's not an incoming message */
  AdvanceAntRxBufferCurrentChar();
  AdvanceAntRxBufferUnreadMsgPointer();

  /* Clear the status flag and process the byte */
  ANT_SSP_FLAGS &= ~_SSP_RX_COMPLETE; /* !!!!! Odd for this to be here, but maybe it needs to be */
  
  /* If the byte is RX_SYNC, then proceed to send the message */
  if (u8Byte == MESG_RX_SYNC)                     
  {
    /* Flag that a transmit is in progress */
    G_u32AntFlags |= _ANT_FLAGS_TX_IN_PROGRESS;
    
    /* Read the message length and add three for the length, message ID and checksum */
    u32Length = (u32)(pu8AntTxMessage_[0] + 3); 
    
    /* Queue the message to the peripheral and capture the token */ 
    Ant_u32CurrentTxMessageToken = SspWriteData(Ant_Ssp, u32Length, pu8AntTxMessage_);

    /* Return TRUE only if we received a message token indicating the message has been queued */
    if(Ant_u32CurrentTxMessageToken != 0)
    {
      return(TRUE);
    }
    else
    {
      DebugPrintf(au8TxNoTokenMsg);
      return(FALSE);
    }
  }

  /* If we get here, not a sync byte, so return */
  DebugPrintf(au8TxNoSyncMsg);
  return(FALSE);

} /* end AntTxMessage() */


/*------------------------------------------------------------------------------
Function: AntExpectResponse

Description:
Waits a specified amount of time for a particular message to arrive from ANT in 
response to a message sent to ANT.  

*** This function violates the 1ms system rule, so should only be used during initialization. ***
  
Requires:
  - u8ExpectedMessageID_ is the ID of a message to which a response is expected
  - u32TimeoutMS_ is the maximum value in ms to wait for the response 
  - A message had been sent to ANT to which a response should be coming in
  - Ant_u8AntNewRxMessages == 0 as this function is meant to run one-to-one with
    transmitted messages.
  - SSP task should be in manual mode so it is busy sending the Tx message to which
    this function will wait for the ANT response.

Promises:
  - Returns 0 if the message is received and was successful
  - Returns 1 if a response is never received or if the response indicates the
    message was not successful.
*/
u8 AntExpectResponse(u8 u8ExpectedMessageID_, u32 u32TimeoutMS_) 
{
  bool bTimeout = FALSE;
  u8 u8ReturnValue = 1;
  u32 u32StartTime = G_u32SystemTime1s;
  u8 au8AntExpectMsgFail[] = "\r\nANT expected msg fail\n\r";

  /* Wait for current message to send */
  u32StartTime = G_u32SystemTime1ms;
  while( IS_SEN_ASSERTED() && !bTimeout )
  {
    bTimeout = IsTimeUp(&u32StartTime, ANT_MSG_TIMEOUT_MS);
  }

  if( !bTimeout )
  {
    /* Done with this message token, so it can be cleared */
    G_u32AntFlags &= ~_ANT_FLAGS_TX_IN_PROGRESS;
    AntDeQueueOutgoingMessage();
    Ant_u32CurrentTxMessageToken = 0;
    
    /* Wait for SEN */
    u32StartTime = G_u32SystemTime1ms;
    while( !IS_SEN_ASSERTED() && !bTimeout )
    {
      bTimeout = IsTimeUp(&u32StartTime, ANT_MSG_TIMEOUT_MS);
    }
  }
  
  /* If no timeout then read the incoming message */
  if( !bTimeout )
  {
    AntRxMessage();

    /* If there is a new message in the receive buffer, then check that it is a response to the expected
    message and that the response is no error */
    if(Ant_u8AntNewRxMessages)
    {
      /* Check if the response is an Event, the event is a reply to the expected message, and the reply is good.
      Since Ant_pu8AntRxBufferUnreadMsg is pointing to the SYNC byte, add 1 when using BUFFER_INDEX values. */
      if( (*(Ant_pu8AntRxBufferUnreadMsg + MESG_ID_OFFSET) == MESG_RESPONSE_EVENT_ID) &&    
          (*(Ant_pu8AntRxBufferUnreadMsg + MESG_RESPONSE_MESG_ID_OFFSET) == u8ExpectedMessageID_) &&
          (*(Ant_pu8AntRxBufferUnreadMsg + MESG_RESPONSE_CODE_OFFSET) == RESPONSE_NO_ERROR) )
      {
        u8ReturnValue = 0;
      }
    }
  }
  
  /* Process any message in the RxBuffer and return the result value */
  AntProcessMessage();
  
  if( bTimeout )
  {
    DebugPrintf(au8AntExpectMsgFail);
    /* !!!! What clean-up should be done here?  Reset ANT and restart init? */
  }

  return(u8ReturnValue);

} /* end AntExpectResponse */


/*-----------------------------------------------------------------------------
Function: AntTxFlowControlCallback

Description:
Callback function to toggle flow control during transmission.  The peripheral task
sending the message must invoke this function after each byte.  

Note: Since this function is called from an ISR, it should execute as quickly as possible. 

Requires:
  - 

Promises:
  - SRDY is toggled
  - Ant_u32TxByteCounter incremented
*/

void AntTxFlowControlCallback(void)
{
  /* Count the byte and toggle flow control lines */
  Ant_u32TxByteCounter++; 
  AntSrdyPulse();

} /* end AntTxFlowControlCallback() */


/*-----------------------------------------------------------------------------
Function: AntRxFlowControlCallback

Description:
Callback function to toggle flow control during reception.  The peripheral task
receiving the message must invoke this function after each byte.  

Note: Since this function is called from an ISR, it should execute as quickly as possible. 
Unfortunately, AntSrdyPulse() takes some time but the duty cycle of this interrupt
is low enough that we can survive (this interrupt priority could be dropped below everything
else to mitigate any issues).

Requires:
  - ISRs are off already since this is totally not re-entrant
  - A received byte was just written to the Rx buffer
  - _SSP_CS_ASSERTED in correct state: should be set on first byte, but application should 
    clear it for all subsequent bytes so flow control is handled entirely by this function

Promises:
  - Ant_pu8AntRxBufferNextChar is advanced safely so it is ready to receive the next byte
  - Ant_u32RxByteCounter incremented
  - SRDY is toggled if _ANT_FLAGS_RX_IN_PROGRESS is set
*/
void AntRxFlowControlCallback(void)
{
  /* Count the byte and safely advance the receive buffer pointer; this is called from the
  RX ISR, so it won't be interrupted and break Ant_pu8AntRxBufferNextChar */
  Ant_u32RxByteCounter++;
  Ant_pu8AntRxBufferNextChar++;
  if(Ant_pu8AntRxBufferNextChar == &Ant_au8AntRxBuffer[ANT_RX_BUFFER_SIZE])
  {
    Ant_pu8AntRxBufferNextChar = &Ant_au8AntRxBuffer[0];
  }
  
  /* Only toggle SRDY if a reception is flagged in progress */
  if( G_u32AntFlags & _ANT_FLAGS_RX_IN_PROGRESS )
  {
    AntSrdyPulse();
  }
  
} /* end AntRxFlowControlCallback() */


/*------------------------------------------------------------------------------
Function: AntCalculateTxChecksum

Description:
Calculates and returns the checksum for a Host > ANT message.

Requires:
  - pu8Message_ points to the message to transmit
  - the message to transmit is a complete ANT message except the SYNC byte (starts with length byte)

Promises:
  - Finds ANT checksum for the message and returns it
*/
u8 AntCalculateTxChecksum(u8* pu8Message_)
{
  u8 u8Size = *pu8Message_ + 2;
  u8 u8Checksum = MESG_RX_SYNC;
  
  for(u8 i = 0; i < u8Size; i++)
  {
    u8Checksum ^= *pu8Message_;
    pu8Message_++;
  }
  
  return(u8Checksum);
  
} /* end AntCalculateTxChecksum() */


/*-----------------------------------------------------------------------------/
Function: AntQueueOutgoingMessage

Description:
Creates a new ANT message structure and adds it into Ant_psDataOutgoingMsgList.
If the list is full, the message is not added.
The Outgoing message list are the messages sent from the Host to the ANT chip.

Requires:
  - pu8Message_ is an ANT-formatted message starting with LENGTH and ending with CHECKSUM
  - Enough space is available on the heap

Promises:
  - A new list item in the outgoing message linked list is created and inserted at the end
    of the list as long as there is enough room.
  - Returns TRUE if the entry is added successfully.
  - Returns FALSE on error.
*/
bool AntQueueOutgoingMessage(u8 *pu8Message_)
{
  u8 u8Length;
  u8 u8MessageCount = 0;
  AntOutgoingMessageListType *psNewDataMessage;
  AntOutgoingMessageListType *psListParser;
  u8 au8AddMessageFailMsg[] = "\n\rNo space in AntQueueOutgoingMessage\n\r";
  
  /* Add to the number of queued message */
  Ant_DebugQueuedDataMessages++;

  /* Allocate space for the new message - always do maximum message size */
  psNewDataMessage = malloc( sizeof(AntOutgoingMessageListType) );
  if (psNewDataMessage == NULL)
  {
    DebugPrintf(au8AddMessageFailMsg);
    return(FALSE);
  }
  
  /* Fill in all the fields of the newly allocated message structure */
  u8Length = *pu8Message_ + 3;
  for(u8 i = 0; i < u8Length; i++)
  {
    psNewDataMessage->au8MessageData[i] = *(pu8Message_ + i);
  }
  
  psNewDataMessage->u32TimeStamp  = G_u32SystemTime1ms;
  psNewDataMessage->psNextMessage = NULL;

  /* Insert into an empty list */
  if(Ant_psDataOutgoingMsgList == NULL)
  {
    Ant_psDataOutgoingMsgList = psNewDataMessage;
    Ant_u32OutgoingMessageCount++;
  }

  /* Otherwise traverse the list to find the end where the new message will be inserted */
  else
  {
    psListParser = Ant_psDataOutgoingMsgList;
    while(psListParser->psNextMessage != NULL)  
    {
      psListParser = psListParser->psNextMessage;
      u8MessageCount++;
    }
    
    /* Check for a full list */
    if(u8MessageCount < ANT_OUTGOING_MESSAGE_BUFFER_SIZE)
    {
      /* Insert the new message at the end of the list */
      psListParser->psNextMessage = psNewDataMessage;
      Ant_u32OutgoingMessageCount++;
    }
    else
    {
      DebugPrintf(au8AddMessageFailMsg);
      return(FALSE);
    }
  }
    
  return(TRUE);
  
} /* end AntQueueOutgoingMessage() */


/*-----------------------------------------------------------------------------/
Function: AntDeQueueApplicationMessage

Description:
Releases the first message in G_sAntApplicationMsgList 

Requires:
  - G_sAntApplicationMsgList points to the start of the list which is the entry to remove

Promises:
  - G_sAntApplicationMsgList = G_sAntApplicationMsgList.
*/
void AntDeQueueApplicationMessage(void)
{
  AntApplicationMsgListType *psMessageToKill;
  
  if(G_sAntApplicationMsgList != NULL)
  {
    psMessageToKill = G_sAntApplicationMsgList;
    G_sAntApplicationMsgList = G_sAntApplicationMsgList->psNextMessage;

    /* The doomed message is properly disconnected, so kill it */
    free(psMessageToKill);
    Ant_u32ApplicationMessageCount--;
  }
  
} /* end AntDeQueueApplicationMessage() */


/* ANT private Interface-layer Functions */

                                    
/*------------------------------------------------------------------------------
Function: AntProcessMessage

Description:
Reads the lastest received Ant message and updates system information accordingly. 
  
Requires:
  - Ant_u8AntNewRxMessages holds the number of unprocessed messages in the message queue
  - GGpu8AntRxBufferUnreadMsg points to the first byte of an unread verified ANT message

Promises:
  - Returns 1 if Ant_u8AntNewRxMessages == 0 or the message exceeds the maximum allowed length
  - Otherwise, returns 0 and:
    - Ant_u8AntNewRxMessages--
    - GGpu8AntRxBufferUnreadMsg points to the first byte of the next unread verified ANT message
    - System flags are updated
*/
static u8 AntProcessMessage(void)
{
  u8 u8MessageLength;
  u8 u8Channel;
  u8 au8MessageCopy[MESG_MAX_SIZE];
  AntExtendedDataType sExtendedData;
  
   /* Exit immediately if there are no messages in the RxBuffer */
	if (!Ant_u8AntNewRxMessages)
    return(1);
  
  Ant_DebugProcessRxMessages++;
  
  /* Otherwise decrement counter, and get a copy of the message (necessary since the rx buffer is circular)
  and we want to index the various bytes using the ANT byte definitions. */  
  Ant_u8AntNewRxMessages--;
  AdvanceAntRxBufferUnreadMsgPointer();
  u8MessageLength = *Ant_pu8AntRxBufferUnreadMsg;
  
  /* Check to ensure the message size is legit.  !!!!! Clean up pointers if not */
  if(u8MessageLength > MESG_MAX_SIZE)
  {
    return(1);
  }
  
  /* Copy the message so it can be indexed easily */ 
  for(u8 i = 0; i < (u8MessageLength + MESG_FRAME_SIZE - MESG_SYNC_SIZE); i++)
  {
    au8MessageCopy[i] = *Ant_pu8AntRxBufferUnreadMsg;
    AdvanceAntRxBufferUnreadMsgPointer();
  }
  /* Note: Ant_pu8AntRxBufferUnreadMsg is now pointing at the next unread message */
  
  /* Get the channel number since it is needed for many things below (this value
  will NOT be the channel for messages that do not include the channel number,
  but that should be fine as long as the value is used in the correct context. */
  u8Channel = au8MessageCopy[BUFFER_INDEX_CHANNEL_NUM];
  
  /* Decide what to do based on the Message ID */
  switch( au8MessageCopy[BUFFER_INDEX_MESG_ID] )
  {
    case MESG_RESPONSE_EVENT_ID:
    { 
      /* Channel Message received: it is a Channel Response or Channel Event */
      if( au8MessageCopy[BUFFER_INDEX_RESPONSE_MESG_ID] != MESG_EVENT_ID )
      {
        /* We have a Channel Response: parse it out based on the message ID to which the 
        response applies and post the result */
        G_stMessageResponse.u8Channel = u8Channel;
        G_stMessageResponse.u8MessageNumber = au8MessageCopy[BUFFER_INDEX_RESPONSE_MESG_ID];
        G_stMessageResponse.u8ResponseCode  = au8MessageCopy[BUFFER_INDEX_RESPONSE_CODE];      
        
        switch(au8MessageCopy[BUFFER_INDEX_RESPONSE_MESG_ID])
        {
          case MESG_OPEN_SCAN_CHANNEL_ID:
            DebugPrintf("Scanning ");
            /* Fall through */
            
          case MESG_OPEN_CHANNEL_ID:
            G_au8AntMessageOpen[12] = u8Channel + 0x30;
            DebugPrintf(G_au8AntMessageOpen);
            
            /* Only change the flags if the command was successful */
            if( au8MessageCopy[BUFFER_INDEX_RESPONSE_CODE] == RESPONSE_NO_ERROR )
            {
              G_asAntChannelConfiguration[u8Channel].AntFlags |= _ANT_FLAGS_CHANNEL_OPEN;
              G_asAntChannelConfiguration[u8Channel].AntFlags &= ~_ANT_FLAGS_CHANNEL_OPEN_PENDING;
            }
            break;

          case MESG_CLOSE_CHANNEL_ID:
            G_au8AntMessageClose[12] = au8MessageCopy[BUFFER_INDEX_CHANNEL_NUM] + 0x30;
            DebugPrintf(G_au8AntMessageClose);

            /* Only change the flags if the command was successful */
            if( au8MessageCopy[BUFFER_INDEX_RESPONSE_CODE] == RESPONSE_NO_ERROR )
            {
              G_asAntChannelConfiguration[u8Channel].AntFlags &= ~(_ANT_FLAGS_CHANNEL_CLOSE_PENDING | _ANT_FLAGS_CHANNEL_OPEN);
            }
            break;

          case MESG_ASSIGN_CHANNEL_ID:
            G_au8AntMessageAssign[12] = au8MessageCopy[BUFFER_INDEX_CHANNEL_NUM] + 0x30;
            DebugPrintf(G_au8AntMessageAssign);
            break;

          case MESG_UNASSIGN_CHANNEL_ID:
            G_au8AntMessageUnassign[12] = au8MessageCopy[BUFFER_INDEX_CHANNEL_NUM] + 0x30;
            DebugPrintf(G_au8AntMessageUnassign);

            /* Only change the flags if the command was successful */
            if( au8MessageCopy[BUFFER_INDEX_RESPONSE_CODE] == RESPONSE_NO_ERROR )
            {
              G_asAntChannelConfiguration[u8Channel].AntFlags &= ~(_ANT_FLAGS_CHANNEL_OPEN_PENDING | _ANT_FLAGS_CHANNEL_CLOSE_PENDING | _ANT_FLAGS_CHANNEL_OPEN); /* !!!! 2016-06-14 */
            }
            break;
 
          default:
            G_au8AntMessageUnhandled[12] = au8MessageCopy[BUFFER_INDEX_CHANNEL_NUM] + 0x30;
            G_au8AntMessageUnhandled[24] = HexToASCIICharLower( (au8MessageCopy[BUFFER_INDEX_RESPONSE_MESG_ID] >> 4) & 0x0F );
            G_au8AntMessageUnhandled[25] = HexToASCIICharLower( (au8MessageCopy[BUFFER_INDEX_RESPONSE_MESG_ID] & 0x0F) );
            G_au8AntMessageUnhandled[36] = HexToASCIICharLower( (au8MessageCopy[BUFFER_INDEX_RESPONSE_CODE] >> 4) & 0x0F );
            G_au8AntMessageUnhandled[37] = HexToASCIICharLower( (au8MessageCopy[BUFFER_INDEX_RESPONSE_CODE] & 0x0F) );
            DebugPrintf(G_au8AntMessageUnhandled);
            break;
        } /* end switch */
        
        /* All messages print an "ok" or "fail" */
        if( au8MessageCopy[BUFFER_INDEX_RESPONSE_CODE] == RESPONSE_NO_ERROR ) 
        {
          DebugPrintf(G_au8AntMessageOk);
        }
        else
        {
          DebugPrintf(G_au8AntMessageFail);
          G_u32AntFlags |= _ANT_FLAGS_CMD_ERROR;
        }

      }
      else /* The message is a Channel Event, so the Event Code must be parsed out */
      { 
        switch ( au8MessageCopy[BUFFER_INDEX_RESPONSE_CODE] )
        {
          case RESPONSE_NO_ERROR: 
          {
            AntTickExtended(RESPONSE_NO_ERROR);
            break;
          }

          case EVENT_RX_FAIL: /* slave did not receive a message when expected */
          {
            /* The slave missed a message it was expecting: communicate this to the
            application in case it matters. Could also queue a debug message here. */
            if(++Ant_u8SlaveMissedMessageLow == 0)
            {
              if(++Ant_u8SlaveMissedMessageMid == 0)
              {
                ++Ant_u8SlaveMissedMessageHigh;
              }
            }
            
            /* Queue an ANT_TICK message to the application message list. */
            AntTickExtended(au8MessageCopy);
            break;
          }

          case EVENT_RX_FAIL_GO_TO_SEARCH: /* slave has lost sync with Master (channel still open) */
          {
            /* The slave missed enough consecutive messages so it goes back to search: communicate this to the
            application in case it matters. Could also queue a debug message here. */
            AntTickExtended(au8MessageCopy);
            break;
          }

          case EVENT_TX: /* ANT has sent a data message */
          {
            /* If this is a master device, then EVENT_TX means it's time to queue the 
            next message */
            if(G_asAntChannelConfiguration[u8Channel].AntChannelType == CHANNEL_TYPE_MASTER)
            {
              AntTickExtended(au8MessageCopy);
            }
            break;
          } 

          case EVENT_TRANSFER_TX_COMPLETED: /* ACK received from an acknowledged data message */
          { 
            G_asAntChannelConfiguration[u8Channel].AntFlags |= _ANT_FLAGS_GOT_ACK;

            AntTickExtended(au8MessageCopy);
            break;
          } 

          case EVENT_TRANSFER_TX_FAILED: /* ACK was not received from an acknowledged data message */
          { 
            /* Regardless of complete or fail, it is time to send the next message */
            AntTickExtended(au8MessageCopy);
            break;
          } 

          case EVENT_RX_SEARCH_TIMEOUT: /* The ANT channel is going to close due to search timeout */
          {
            /* Forward this to application */
            AntTickExtended(au8MessageCopy);
            break;
          }
 
          case EVENT_CHANNEL_CLOSED: /* The ANT channel is now closed */
          {
            DebugPrintf("Channel closed\n\r");
            G_asAntChannelConfiguration[u8Channel].AntFlags &= ~(_ANT_FLAGS_CHANNEL_CLOSE_PENDING | _ANT_FLAGS_CHANNEL_OPEN);
            break;
          }
          
          /* All other messages are unexpected for now */
          default:
            DebugPrintNumber(au8MessageCopy[BUFFER_INDEX_RESPONSE_CODE]);
            DebugPrintf(": unexpected channel event\n\r");

            G_u32AntFlags |= _ANT_FLAGS_UNEXPECTED_EVENT;
            break;
        } /* end Ant_pu8AntRxBufferUnreadMsg[EVENT_CODE_INDEX] */
      } /* end else RF event */
      
      break; 
    } /* end case MESG_RESPONSE_EVENT_ID */

    case MESG_ACKNOWLEDGED_DATA_ID: /* An acknowledged data message was received */
    /* Fall through */
      
    case MESG_BROADCAST_DATA_ID: /* A broadcast data message was received */
    { 
      /* Parse the extended data and put the message to the application buffer */
      AntParseExtendedData(au8MessageCopy, &sExtendedData);
      AntQueueExtendedApplicationMessage(ANT_DATA, &au8MessageCopy[BUFFER_INDEX_MESG_DATA], &sExtendedData);
      
      /* If this is a slave device, then a data message received means it's time to send */
      if(G_asAntChannelConfiguration[u8Channel].AntChannelType == CHANNEL_TYPE_SLAVE)
      {
        AntTickExtended(RESPONSE_NO_ERROR);
      }
      
      break;
    } /* end case MESG_BROADCAST_DATA_ID */
    
    case MESG_CHANNEL_STATUS_ID: /* Message sent in response to a channel status request */
    { 
      break;
    } /* end case ChannelStatus_CMD */
    
    case MESG_VERSION_ID:
    {
      for(u8 i = 0; i < MESG_VERSION_SIZE; i++)
      {
        Ant_u8AntVersion[i] = au8MessageCopy[BUFFER_INDEX_VERSION_BYTE0 + i];
      }
      
      /* If we get a version message, we know that ANT comms is good */
      G_u32ApplicationFlags |= _APPLICATION_FLAGS_ANT;
      
      break;
    } /* end case MESG_VERSION_ID */

    case MESG_RESTART_ID:
    {
      G_u32AntFlags |= _ANT_FLAGS_RESTART;  
      break;
    } /* end case MESG_RESTART_ID */
    
    default:
      G_u32AntFlags |= _ANT_FLAGS_UNEXPECTED_MSG;
      break;
  } /* end switch( Ant_pu8AntRxBufferUnreadMsg[MESG_ID_OFFSET] ) */
           
  return(0);
  
} /* end AntProcessMessage() */


/*-----------------------------------------------------------------------------/
Function: AntTickExtended

Description:
Queues an ANT_TICK message to the application message queue.

Requires:
  - u8Code_ is payload byte indicating system info that may be relavent to the application

Promises:
  - A MESSAGE_ANT_TICK is queued to G_sAntApplicationMsgList
*/
static void AntTickExtended(u8* pu8AntMessage_)
{
  u8 au8Message[ANT_APPLICATION_MESSAGE_BYTES];
  AntExtendedDataType sExtData;

  /* Update data to communicate the ANT_TICK to the application */
  au8Message[ANT_TICK_MSG_ID_INDEX]               = MESSAGE_ANT_TICK;
  au8Message[ANT_TICK_MSG_CHANNEL_INDEX]          = *(pu8AntMessage_ + BUFFER_INDEX_CHANNEL_NUM);
  au8Message[ANT_TICK_MSG_RESPONSE_TYPE_INDEX]    = *(pu8AntMessage_ + BUFFER_INDEX_RESPONSE_MESG_ID);
  au8Message[ANT_TICK_MSG_EVENT_CODE_INDEX]       = *(pu8AntMessage_ + BUFFER_INDEX_RESPONSE_CODE);
  au8Message[ANT_TICK_MSG_SENTINEL3_INDEX]        = MESSAGE_ANT_TICK;
  au8Message[ANT_TICK_MSG_MISSED_HIGH_BYTE_INDEX] = Ant_u8SlaveMissedMessageHigh;
  au8Message[ANT_TICK_MSG_MISSED_MID_BYTE_INDEX]  = Ant_u8SlaveMissedMessageMid;
  au8Message[ANT_TICK_MSG_MISSED_LOW_BYTE_INDEX]  = Ant_u8SlaveMissedMessageLow;

  /* Extended data is not valid for an ANT_TICK message */
  sExtData.u8Channel    = au8Message[ANT_TICK_MSG_CHANNEL_INDEX];
  sExtData.u16DeviceID  = 0xFF;
  sExtData.u8DeviceType = 0xFF;
  sExtData.u8TransType  = 0xFF;
  sExtData.s8RSSI       = 0xFF;
  sExtData.u8Flags      = 0xFF;
  
  /* Data is ready so queue it in to the application buffer */
  AntQueueExtendedApplicationMessage(ANT_TICK, 
                                     &au8Message[ANT_TICK_MSG_ID_INDEX],
                                     &sExtData);

} /* end AntTickExtended() */


/*-----------------------------------------------------------------------------/
Function: AntQueueExtendedApplicationMessage

Description:
Creates a new ANT data message structure and adds it to G_sAntApplicationMsgList.
The Application list used to communicate message information between the ANT driver and
the ANT_API simplified interface task.

Requires:
  - eMessageType_ specifies the type of message
  - pu8DataSource_ is a pointer to the first element of an array of 8 data bytes
  - psTargetList_ is a pointer to the list pointer that is being updated
  - Enough space is available on the heap

Promises:
  - A new list item in the target linked list is created and inserted at the end
    of the list.
  - Returns TRUE if the entry is added successfully.
  - Returns FALSE if the malloc fails or the list is full.
*/
static bool AntQueueExtendedApplicationMessage(AntApplicationMessageType eMessageType_, 
                                               u8* pu8DataSource_, 
                                               AntExtendedDataType* psExtData_)
{
  AntApplicationMsgListType *psNewMessage;
  AntApplicationMsgListType *psListParser;
  u8 u8MessageCount = 0;
  u8 au8AddMessageFailMsg[] = "\n\rNo space in AntQueueApplicationMessage\n\r";
  
  /* Allocate space for the new message - always do maximum message size */
  psNewMessage = malloc( sizeof(AntApplicationMsgListType) );
  if (psNewMessage == NULL)
  {
    DebugPrintf(au8AddMessageFailMsg);
    return(FALSE);
  }
  
  /* Fill in all the fields of the newly allocated message structure */
  for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
  {
    psNewMessage->au8MessageData[i] = *(pu8DataSource_ + i);
  }
  
  /* Copy basic items */
  psNewMessage->u32TimeStamp  = G_u32SystemTime1ms;
  psNewMessage->eMessageType  = eMessageType_;
  
  /* Copy all extended data fields */
  psNewMessage->sExtendedData.u8Channel    = psExtData_->u8Channel;
  psNewMessage->sExtendedData.u16DeviceID  = psExtData_->u16DeviceID;
  psNewMessage->sExtendedData.u8DeviceType = psExtData_->u8DeviceType;
  psNewMessage->sExtendedData.u8TransType  = psExtData_->u8TransType;
  psNewMessage->sExtendedData.u8Flags      = psExtData_->u8Flags;
  psNewMessage->sExtendedData.s8RSSI       = psExtData_->s8RSSI;
    
  psNewMessage->psNextMessage = NULL;

  /* Insert into an empty list */
  if(G_sAntApplicationMsgList == NULL)
  {
    G_sAntApplicationMsgList = psNewMessage;
    Ant_u32ApplicationMessageCount++;
  }

  /* Otherwise traverse the list to find the end where the new message will be inserted */
  else
  {
    psListParser = G_sAntApplicationMsgList;
    while(psListParser->psNextMessage != NULL) 
    {
      psListParser = psListParser->psNextMessage;
      u8MessageCount++;
    }
    
    /* Check for full list */
    if(u8MessageCount < ANT_APPLICATION_MESSAGE_BUFFER_SIZE)
    {
      /* Insert the new message at the end of the list */
      psListParser->psNextMessage = psNewMessage;
      Ant_u32ApplicationMessageCount++;
    }
    /* Handle a full list */
    else
    {
      DebugPrintf(au8AddMessageFailMsg);
      return(FALSE);
    }
  }
    
  return(TRUE);
    
} /* end AntQueueExtendedApplicationMessage() */


/*-----------------------------------------------------------------------------/
Function: AntDeQueueOutgoingMessage

Description:
Removes the first entry of Ant_psDataOutgoingMsgList.

Requires:
  - 

Promises:
  - Ant_psDataOutgoingMsgList = Ant_psDataOutgoingMsgList->psNextMessage 
  and the memory is freed
*/
static void AntDeQueueOutgoingMessage(void)
{
  AntOutgoingMessageListType *psMessageToKill;
  
  if(Ant_psDataOutgoingMsgList != NULL)
  {
    psMessageToKill = Ant_psDataOutgoingMsgList;
    Ant_psDataOutgoingMsgList = Ant_psDataOutgoingMsgList->psNextMessage;
  
    /* The doomed message is properly disconnected, so kill it */
    free(psMessageToKill);
  }
  
} /* end AntDeQueueOutgoingMessage() */


/***********************************************************************************************************************
##### ANT State Machine Definition                                             
***********************************************************************************************************************/

/*------------------------------------------------------------------------------
Idle state that will process new messages if any are present, monitors ANT
for incoming messages, and sends broadcast messages that are waiting to be sent.
Incoming messages from ANT always get priority.  
*/
void AntSM_Idle(void)
{
  u32 u32MsgBitMask = 0x01;
  u8 u8MsgIndex = 0;
  static u8 au8AntFlagAlert[] = "ANT flags:\n\r"; 
  
  /* Error messages: must match order of G_u32AntFlags Error / event flags */
  static u8 au8AntFlagMessages[][20] = 
  {/* "012345678901234567\n\r" */
      "Length mismatch\n\r",
      "Command error\n\r",
      "Unexpected event\n\r",
      "Unexpected message\n\r"
  };
  
  /* Check flags */
  if(G_u32AntFlags & ANT_ERROR_FLAGS_MASK)
  {
    /* At least one flag is set, so print header and parse out */
    DebugPrintf(au8AntFlagAlert);
    for(u8 i = 0; i < ANT_ERROR_FLAGS_COUNT; i++)
    {
      /* Check if current flag is set */
      if(G_u32AntFlags & u32MsgBitMask)
      {
        /* Print the error message */
        DebugPrintf(au8AntFlagMessages[u8MsgIndex]);
      }
      u32MsgBitMask <<= 1;
      u8MsgIndex++;
    }
    
    /* Clear all the error flags now that they have been reported */
    G_u32AntFlags &= ~ANT_ERROR_FLAGS_MASK;
  }
  
  /* Process messages received from ANT */
  AntProcessMessage();

  /* Handle messages coming in from ANT */
  if( IS_SEN_ASSERTED() )
  {
    Ant_pfnStateMachine = AntSM_ReceiveMessage;
  }
  
  /* Send a message if the system is ready and there is one to send */ 
  else if( (Ant_u32CurrentTxMessageToken == 0 ) && 
           (Ant_psDataOutgoingMsgList != NULL) )
  {
    /* Give the message to AntTx which will set Ant_u32CurrentTxMessageToken */
    if(AntTxMessage(Ant_psDataOutgoingMsgList->au8MessageData))
    {
      Ant_u32TxTimer = G_u32SystemTime1ms;
      Ant_pfnStateMachine = AntSM_TransmitMessage;
    }
    else
    {
      /* Transmit attempt failed.  !!!! Do something? */
    }
  }
  
} /* end AntSM_Idle() */


/*------------------------------------------------------------------------------
Completely receive an ANT message.  Reception is very fast and should complete
in less than 600us for a 15-byte message.  AntRxMessage could just be called from Idle,
but giving it its own state minimizes the total time and allows for easier future
updates should they be required.
*/
void AntSM_ReceiveMessage(void)
{
  Ant_DebugRxMessageCounter++;
  AntRxMessage();
  
  Ant_pfnStateMachine = AntSM_Idle;

} /* end AntSM_ReceiveMessage() */


/*------------------------------------------------------------------------------
Wait for an ANT message to be transmitted.  This state only occurs once the 
handshaking transaction has been completed and transmit to ANT is verified 
and underway.
*/
void AntSM_TransmitMessage(void)
{
  static u8 au8TxTimeoutMsg[] = "\n\rTransmit message timeout\n\r";
  MessageStateType eCurrentMsgStatus;
  
  eCurrentMsgStatus = QueryMessageStatus(Ant_u32CurrentTxMessageToken);
  switch(eCurrentMsgStatus)
  {
    case TIMEOUT:
      /* Fall through */
      
    case COMPLETE:
      /* Kill the message and update flags */
      AntDeQueueOutgoingMessage();
      Ant_u32CurrentTxMessageToken = 0;
      G_u32AntFlags &= ~_ANT_FLAGS_TX_IN_PROGRESS;

      /* Wait for SEN to deassert so we know ANT is totally ready for the next
      transaction.  This takes about 170us, so block in this state until that's over */
      while ( IS_SEN_ASSERTED() && (Ant_u32TxTimer < ANT_ACTIVITY_TIME_COUNT) )
      {
        Ant_u32TxTimer++;
      }

      /* If we timed out, then ANT is stuck so print error and unstick ANT */
      if(Ant_u32RxTimer > ANT_ACTIVITY_TIME_COUNT)
      {
        DebugPrintf(au8TxTimeoutMsg);
        while( IS_SEN_ASSERTED() )
        {
          AntSrdyPulse();
        }
      }
      
      Ant_pfnStateMachine = AntSM_Idle;
      break;
      
    default:
      /* Do nothing for now */
      break;
      
  } /* end switch */
  
} /* end AntSM_TransmitMessage() */


/*------------------------------------------------------------------------------
Do-nothing state if ANT is dead (requires restart to retry initialization)
*/
void AntSM_NoResponse(void)
{
  
} /* end AntSM_NoResponse() */
