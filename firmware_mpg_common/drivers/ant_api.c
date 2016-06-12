/***********************************************************************************************************************
File: ant_api.c                                                               

Description:
ANT user interface.  No initialization or state machine requried.  This file exposes the source code
for all public functions that work with ant.c.  Seperating it keep sit a little more manageable.

Once the ANT radio has been configured, all messaging from the ANT device is handled through 
the incoming queue G_sAntApplicationMsgList.  The application is responsible for checking this
queue for messages that belong to it and must manage timing and handle appropriate updates per 
the ANT messaging protocol.  This should be no problem on the regular 1ms loop timing of the main 
system (assuming ANT message rate is less than 1kHz).  Faster systems or burst messaging will need 
to be handled seperately as an add-on to this API.


------------------------------------------------------------------------------------------------------------------------
API:

Globals
// Configuration struct
extern AntSetupDataType G_stAntSetupData;                         // From ant.c

// Globals for passing data from the ANT application to the API (import these to application)
extern u32 G_u32AntApiCurrentDataTimeStamp;                       // From ant_api.c
extern AntApplicationMessageType G_eAntApiCurrentMessageClass;    // From ant_api.c
extern u8 G_au8AntApiCurrentData[ANT_APPLICATION_MESSAGE_BYTES];  // From ant_api.c

Types
typedef enum {ANT_UNCONFIGURED, ANT_CONFIGURED, ANT_OPEN, ANT_CLOSED} AntChannelStatusType;
typedef enum {ANT_EMPTY, ANT_DATA, ANT_TICK} AntApplicationMessageType;


***ANT CONFIGURATION / STATUS FUNCTIONS***
bool AntChannelConfig(bool)
All channel configuration is sent to the ANT device and TRUE is returned if successful.
This requires a global data structure to be set up in the task.  It is intended to run to completion inside
the application's initialization section.  

To do this, copy the following code block into the application's Initialize()
function.

  G_stAntSetupData.AntChannel          = ANT_CHANNEL_USERAPP;
  G_stAntSetupData.AntSerialLo         = ANT_SERIAL_LO_USERAPP;
  G_stAntSetupData.AntSerialHi         = ANT_SERIAL_HI_USERAPP;
  G_stAntSetupData.AntDeviceType       = ANT_DEVICE_TYPE_USERAPP;
  G_stAntSetupData.AntTransmissionType = ANT_TRANSMISSION_TYPE_USERAPP;
  G_stAntSetupData.AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_USERAPP;
  G_stAntSetupData.AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_USERAPP;
  G_stAntSetupData.AntFrequency        = ANT_FREQUENCY_USERAPP;
  G_stAntSetupData.AntTxPower          = ANT_TX_POWER_USERAPP;

Copy the following definitions into the application's header file:

#define ANT_CHANNEL_USERAPP             (u8)                  // Channel 0 - 7
#define ANT_SERIAL_LO_USERAPP           (u8)                  // Low byte of two-byte Device #
#define ANT_SERIAL_HI_USERAPP           (u8)                  // High byte of two-byte Device #
#define ANT_DEVICE_TYPE_USERAPP         (u8)                  // 1 - 255
#define ANT_TRANSMISSION_TYPE_USERAPP   (u8)                  // 1-127 (MSB is pairing bit)
#define ANT_CHANNEL_PERIOD_LO_USERAPP   (u8)0x00              // Low byte of two-byte channel period 0x0001 - 0x7fff
#define ANT_CHANNEL_PERIOD_HI_USERAPP   (u8)0x20              // High byte of two-byte channel period 
#define ANT_FREQUENCY_USERAPP           (u8)50                // 2400MHz + this number 0 - 99
#define ANT_TX_POWER_USERAPP            RADIO_TX_POWER_0DBM   // RADIO_TX_POWER_0DBM, RADIO_TX_POWER_MINUS5DBM, RADIO_TX_POWER_MINUS10DBM, RADIO_TX_POWER_MINUS20DBM


AntChannelStatus AntRadioStatus(void)
Query the ANT radio channel status.  Returns ANT_UNCONFIGURED, ANT_CLOSING, ANT_OPEN, or ANT_CLOSED

AntChannelStatus AntChannelStatus(u8 u8AntChannelToOpen)
Query the ANT status of the specified channel.  Returns ANT_UNCONFIGURED, ANT_CLOSING, ANT_OPEN, or ANT_CLOSED

bool AntOpenChannel(void)
Queues a request to open the configured channel.
Returns TRUE if message is successfully queued - this can be ignored or checked.  
Application should monitor AntRadioStatus() for actual channel status.
e.g.
AntChannelStatusType eAntCurrentState;

// Request to open channel only on an already closed channel.
eAntCurrentState = AntRadioStatus();

if(eAntCurrentState == ANT_CLOSED )
{
   AntOpenChannel();
}

bool AntOpenChannelNumber(u8 u8AntChannelToOpen)
Queues a request to open the specified channel.
Returns TRUE if the channel is configured and the message is successfully queued - this can be ignored or checked.  
Application should monitor AntRadioStatus() for actual channel status.
e.g.
AntChannelStatusType eAntCurrentState;

// Request to open channel only on an already closed channel.
eAntCurrentState = AntRadioStatus();

if(eAntCurrentState == ANT_CLOSED )
{
   AntOpenChannel();
}


bool AntOpenScanningChannel(void)
Queues a request to open a scanning channel. Channel 0 setup parameters are used,
but note that all channel resources are used by this a scanning channel.
Returns TRUE if message is successfully queued - this can be ignored or checked.  
Application should monitor AntRadioStatus() for actual channel status.
AntChannelStatusType eAntCurrentState;

// Request to open channel only on an already closed channel.
eAntCurrentState = AntRadioStatus();

if(eAntCurrentState == ANT_CLOSED )
{
   AntOpenScanningChannel();
}

bool AntCloseChannel(void)
Queues a request to close the configured channel.
Returns TRUE if message is successfully queued - this can be ignored or checked.  
Application should monitor AntRadioStatus() for channel status.
e.g.
AntChannelStatusType eAntCurrentState;

// Request to close channel only on an open channel.
eAntCurrentState = AntRadioStatus();

if(eAntCurrentState == ANT_OPEN )
{
   AntCloseChannel();
}


bool AntUnassignChannel(void)
Queues a request to unassign the ANT channel.  Returns TRUE if message is successfully queued.  
Application should monitor AntRadioStatus()for channel status.
e.g.
AntChannelStatusType eAntCurrentState;

eAntCurrentState = AntRadioStatus();

if(eAntCurrentState == ANT_CLOSED )
{
   // Request to unassign channel (allowed only on a closed channel).
   AntUnassignChannel();
}


bool AntSendGenericMessage()
Queues a generic message to be sent to ANT.  Any of the ANT message codes can be sent.
The entire message string except the checksum must be provided.
Returns TRUE if a message can be sent to ANT.
Returns FALSE if the system is not available to send to ANT (i.e. a message is currently being sent)
Application must then monitor AntCheckGenericMessageStatus() to determine when 

e.g.
AntChannelStatusType eAntCurrentState;


***ANT DATA FUNCTIONS***
bool AntQueueBroadcastMessage(u8 *pu8Data_)
Queue a broadcast data message.
e.g.
u8 u8DataToSend[ANT_DATA_BYTES] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
AntQueueBroadcastMessage(&u8DataToSend[0]);


bool AntQueueAcknowledgedMessage(u8 *pu8Data_)
Queue an acknowledged data message.
e.g.
u8 u8DataToSend[ANT_DATA_BYTES] = {0x07, 0x06, 0x05, 0x04, 0x03, 0xdd, 0xee, 0xff};
AntQueueAcknowledgedMessage(u8DataToSend);


bool AntReadMessageBuffer(void)
Check the incoming message buffer for any message from the ANT system (either ANT_TICK or ANT_DATA).  
If no messages are present, returns FALSE.  If a message is there, returns TRUE and application can read:
- G_u32AntApiCurrentDataTimeStamp to see the system time stamp when the message arrived
- G_eAntApiCurrentMessageClass to see what kind of message is present
- G_asAntApiCurrentData[ANT_APPLICATION_MESSAGE_BYTES] to see the message bytes.

e.g.
u32 u32CurrentMessageTimeStamp;
u8 u8CurrentTickEventCode;
u8 u8CurrentMessageContents[ANT_APPLICATION_MESSAGE_BYTES];

if(AntReadMessageBuffer())
{
  // Report the time a message was received
  DebugPrintNumber(u32CurrentMessageTimeStamp);
  DebugPrintf(": message received\n\t");

  // Check the message class to determine how to process the message
  if(G_eAntApiCurrentMessageClass == ANT_TICK)
  {
    // Get the EVENT code from the ANT_TICK message 
    u8CurrentTickEventCode = G_asAntApiCurrentData[ANT_TICK_MSG_EVENT_CODE_INDEX];
  }

  if(G_eAntApiCurrentMessageClass == ANT_DATA)
  {
    // Copy the message data locally
    for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
    {
      u8CurrentMessageContents[i] = G_asAntApiCurrentData[i];
    }
  }
}

u32 AntReadRSSI(void)
Get the current RSSI value (if available)


***********************************************************************************************************************/

#include "configuration.h"


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>AntApi"
***********************************************************************************************************************/
volatile u32 G_u32AntApiFlags;                                          /* Global state flags */

u32 G_u32AntApiCurrentDataTimeStamp = 0;                                /* Current read message's G_u32SystemTime1ms */
AntApplicationMessageType G_eAntApiCurrentMessageClass = ANT_EMPTY;     /* Type of data */
u8 G_au8AntApiCurrentData[ANT_APPLICATION_MESSAGE_BYTES];               /* Array for message data */
AntExtendedDataType G_stCurrentExtendedData;                            /* Extended data struct for the current message */


/*----------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) 
and indicate what file the variable is defined in. */
extern u32 G_u32SystemFlags;                                  /* From main.c */
extern u32 G_u32ApplicationFlags;                             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                       /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                        /* From board-specific source file */

//extern volatile u32 ANT_SSP_FLAGS;                            /* From configuration.h */

extern u32 G_u32AntFlags;                                     /* From ant.c */
extern AntApplicationMsgListType *G_sAntApplicationMsgList;   /* From ant.c */
extern AntSetupDataType G_stAntSetupData;                     /* From ant.c */

extern u8 G_au8AntMessageOk[];                                /* From ant.c */
extern u8 G_au8AntMessageFail[];                              /* From ant.c */
extern u8 G_au8AntMessageAssign[];                            /* From ant.c */
extern u8 G_au8AntMessageUnassign[];                          /* From ant.c */
extern u8 G_au8AntMessageUnhandled[];                         /* From ant.c */
extern u8 G_au8AntMessageSetup[];                             /* From ant.c */
extern u8 G_au8AntMessageClose[];                             /* From ant.c */
extern u8 G_au8AntMessageOpen[];                              /* From ant.c */
extern u8 G_au8AntMessageInit[];                              /* From ant.c */
extern u8 G_au8AntMessageInitFail[];                          /* From ant.c */
extern u8 G_au8AntMessageNoAnt[];                             /* From ant.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "AntApi_<type>Name" and be declared as static.
***********************************************************************************************************************/
static fnCode_type AntApi_StateMachine;             /* The state machine function pointer */
static u32 AntApi_u32Timeout;                       /* Timeout counter used across states */

static AntAssignChannelInfoType AntApi_asChannelConfiguration[ANT_CHANNELS]; /* Keeps track of all configured ANT channels */
//static u32 AntApi_au32AssignChannelTokens


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
Function: AntAssignChannel
Description:
Completely configures an ANT channel with an application's required parameters 
for communication.

Requires:
  - All Global ANT configuration variables have been assigned to the application's
    required values.
  - An ANT channel is not currently opened.

Promises:
  - Channel, Channel ID, message period, radio frequency and radio power are configured.
  - Returns TRUE if configuration is successful
*/
bool AntAssignChannel(AntAssignChannelInfoType* psAntSetupInfo_);
{
  u8 au8ANTSetNetworkKey[]    = {MESG_NETWORK_KEY_SIZE, MESG_NETWORK_KEY_ID, psAntSetupInfo_->AntNetwork, 
                                 psAntSetupInfo_->AntNetworkKey[0], psAntSetupInfo_->AntNetworkKey[1], 
                                 psAntSetupInfo_->AntNetworkKey[2], psAntSetupInfo_->AntNetworkKey[3], 
                                 psAntSetupInfo_->AntNetworkKey[4], psAntSetupInfo_->AntNetworkKey[5], 
                                 psAntSetupInfo_->AntNetworkKey[6], psAntSetupInfo_->AntNetworkKey[7], CS};
  u8 au8ANTAssignChannel[]    = {MESG_ASSIGN_CHANNEL_SIZE, MESG_ASSIGN_CHANNEL_ID, psAntSetupInfo_->AntChannel, 
                                 psAntSetupInfo_->AntChannelType, psAntSetupInfo_->AntNetwork, CS};
  u8 au8ANTSetChannelID[]     = {MESG_CHANNEL_ID_SIZE, MESG_CHANNEL_ID_ID, psAntSetupInfo_->AntChannel, 
                                 psAntSetupInfo_->AntDeviceIdLo, psAntSetupInfo_->AntDeviceIdHi, psAntSetupInfo_->AntDeviceType, 
                                 psAntSetupInfo_->AntTransmissionType, CS};
  u8 au8ANTSetChannelPeriod[] = {MESG_CHANNEL_MESG_PERIOD_SIZE, MESG_CHANNEL_MESG_PERIOD_ID, psAntSetupInfo_->AntChannel, 
                                 psAntSetupInfo_->AntChannelPeriodLo, psAntSetupInfo_->AntChannelPeriodHi, CS};
  u8 au8ANTSetChannelRFFreq[] = {MESG_CHANNEL_RADIO_FREQ_SIZE, MESG_CHANNEL_RADIO_FREQ_ID, psAntSetupInfo_->AntChannel, 
                                 psAntSetupInfo_->AntFrequency, CS};           
  u8 au8ANTSetChannelPower[]  = {MESG_RADIO_TX_POWER_SIZE, MESG_RADIO_TX_POWER_ID, psAntSetupInfo_->AntChannel, 
                                 psAntSetupInfo_->AntTxPower, CS};        
  u8 au8ANTLibConfig[]        = {MESG_LIB_CONFIG_SIZE, MESG_LIB_CONFIG_ID, 0, LIB_CONFIG_CHANNEL_ID_EN | LIB_CONFIG_RSSI_EN, CS};        
 
  u8 u8ErrorCount = 0;	

  G_u32AntFlags &= ~_ANT_FLAGS_CMD_ERROR;

  /* Adjust the channel type if configuration for a slave device */
  if(!bMaster_)
  {
    au8ANTAssignChannel0[3] = CHANNEL_TYPE_SLAVE;
    G_stAntSetupData.AntChannelType = CHANNEL_TYPE_SLAVE;
  }
  
  /* Assign the channel */
  au8ANTAssignChannel0[5] = AntCalculateTxChecksum(au8ANTAssignChannel0);
  AntTxMessage(au8ANTAssignChannel0);
  u8ErrorCount += AntExpectResponse(MESG_ASSIGN_CHANNEL_ID, ANT_MSG_TIMEOUT_MS);

  /* Assign the channel ID */
  au8ANTSetChannelID0[7] = AntCalculateTxChecksum(au8ANTSetChannelID0);
  AntTxMessage(au8ANTSetChannelID0);
  u8ErrorCount += AntExpectResponse(MESG_CHANNEL_ID_ID, ANT_MSG_TIMEOUT_MS);
    
  /* Assign the channel period */
  au8ANTSetChannelPeriod0[5] = AntCalculateTxChecksum(au8ANTSetChannelPeriod0);
  AntTxMessage(au8ANTSetChannelPeriod0);
  u8ErrorCount += AntExpectResponse(MESG_CHANNEL_MESG_PERIOD_ID, ANT_MSG_TIMEOUT_MS);
    
  /* Assign the channel frequency */
  au8ANTSetChannelRFFreq0[4] = AntCalculateTxChecksum(au8ANTSetChannelRFFreq0);
  AntTxMessage(au8ANTSetChannelRFFreq0);
  u8ErrorCount += AntExpectResponse(MESG_CHANNEL_RADIO_FREQ_ID, ANT_MSG_TIMEOUT_MS);

  /* Assign the channel power */
  au8ANTSetChannelPower0[4] = AntCalculateTxChecksum(au8ANTSetChannelPower0);
  AntTxMessage(au8ANTSetChannelPower0);
  u8ErrorCount += AntExpectResponse(MESG_RADIO_TX_POWER_ID, ANT_MSG_TIMEOUT_MS);
  
  /* If any errors were collected, clear the ANT_GOOD flag */ 
  /* Announce channel status on the debug port */
  DebugPrintf(G_au8AntMessageSetup);
  if(u8ErrorCount)
  {
    G_u32SystemFlags &= ~_APPLICATION_FLAGS_ANT;  
    DebugPrintf(G_au8AntMessageFail);
    return(FALSE);
  }
  else
  {
    DebugPrintf(G_au8AntMessageOk);
    G_u32AntFlags |= _ANT_FLAGS_CHANNEL_CONFIGURED;
    return(TRUE);
  }

} /* end AntAssignChannel() */


/*------------------------------------------------------------------------------
Function: AntOpenChannelNumber

Description:
Queues the Open Channel message on the configured ANT channel.  This does not actually indicate
that the channel has been opened successfully -- the calling task must monitor _ANT_FLAGS_CHANNEL_OPEN
to determine if channel opens successfully.
  
Requires:
  - ANT channel is correctly configured.

Promises:
  - If channel open message is queued, returns TRUE (Ant_u32CurrentTxMessageToken will be non-zero)
  - Otherwise returns FALSE
 
*/
bool AntOpenChannelNumber(u8 u8Channel_)
{
  u8 au8AntOpenChannel[] = {MESG_OPEN_CHANNEL_SIZE, MESG_OPEN_CHANNEL_ID, 0, CS};

  /* Update the channel number (scanning channel is always 0) */
  if(u8Channel_ != ANT_CHANNEL_SCANNING)
  {
    au8AntOpenChannel[2] = u8Channel_;
  }

  /* Update the checksum value and queue the open channel message */
  au8AntOpenChannel[3] = AntCalculateTxChecksum(au8AntOpenChannel);
  G_u32AntFlags |= _ANT_FLAGS_CHANNEL_OPEN_PENDING;
 
  return( AntQueueOutgoingMessage(au8AntOpenChannel) );
  
} /* end AntOpenChannelNumber() */


/*------------------------------------------------------------------------------
Function: AntCloseChannelNumber

Description:
Requests that an ANT channel is closed.  Issuing MESG_CLOSE_CHANNEL_ID does not
guarantee that the channel closes, and ANT response to this message does not
indicate that the channel is closed (a seperate message will be sent when the 
channel actually closes which usually happens on the next ANT message period).
  
Requires:
  - u8Channel_ holds the ANT channel to close
  - ANT channel is correctly configured and should be open.

Promises:
  - If channel close message is queued, returns TRUE (Ant_u32CurrentTxMessageToken will be non-zero)
  - Otherwise returns FALSE
*/
bool AntCloseChannelNumber(u8 u8Channel_)
{
  u8 au8AntCloseChannel[] = {MESG_CLOSE_CHANNEL_SIZE, MESG_CLOSE_CHANNEL_ID, 0, CS};
  
  /* Update the channel number (scanning channel is always 0) */
  if(u8Channel_ != ANT_CHANNEL_SCANNING)
  {
    au8AntCloseChannel[2] = u8Channel_;
  }

  /* Update the checksum value and queue the close channel message*/
  au8AntCloseChannel[3] = AntCalculateTxChecksum(au8AntCloseChannel);
  G_u32AntFlags |= _ANT_FLAGS_CHANNEL_CLOSE_PENDING;
  
  return( AntQueueOutgoingMessage(au8AntCloseChannel) );

} /* end AntCloseChannelNumber() */


/*------------------------------------------------------------------------------
Function: AntUnassignChannelNumber

Description:
Queues message to unassigns the specified ANT channel so it can be reconfigured.
  
Requires:
  - ANT channel is closed

Promises:
  - ANT channel unassign message is queued; application should monitor AntRadioStatus()
*/
bool AntUnassignChannelNumber(u8 u8Channel_)
{
  u8 au8AntUnassignChannel[] = {MESG_UNASSIGN_CHANNEL_SIZE, MESG_UNASSIGN_CHANNEL_ID, 0, CS};

  /* Update the channel number (scanning channel is always 0) */
  if(u8Channel_ != ANT_CHANNEL_SCANNING)
  {
    au8AntUnassignChannel[2] = u8Channel_;
  }

  /* Update checksum and queue the unassign channel message */
  au8AntUnassignChannel[3] = AntCalculateTxChecksum(au8AntUnassignChannel);
  G_u32AntFlags &= ~_ANT_FLAGS_CHANNEL_CONFIGURED;
  return( AntQueueOutgoingMessage(au8AntUnassignChannel) );

} /* end AntUnassignChannelNumber() */


#ifdef ANT_API_LEGACY
/*------------------------------------------------------------------------------
Function: AntChannelConfig

Description:
Completely configures the ANT channel with an application's required parameters 
for communication.

*** This function violates 1ms system timing and should only be used during initialization ***

Requires:
  - All Global ANT configuration variables have been assigned to the application's
    required values.
  - An ANT channel is not currently opened.

Promises:
  - Channel, Channel ID, message period, radio frequency and radio power are configured.
  - Returns TRUE if configuration is successful
*/
bool AntChannelConfig(bool bMaster_)
{
  u8 au8ANTAssignChannel0[]    = {MESG_ASSIGN_CHANNEL_SIZE, MESG_ASSIGN_CHANNEL_ID, G_stAntSetupData.AntChannel, CHANNEL_TYPE_MASTER, G_stAntSetupData.AntNetwork, CS};
  u8 au8ANTSetChannelID0[]     = {MESG_CHANNEL_ID_SIZE, MESG_CHANNEL_ID_ID, G_stAntSetupData.AntChannel, G_stAntSetupData.AntSerialLo, G_stAntSetupData.AntSerialHi, G_stAntSetupData.AntDeviceType, G_stAntSetupData.AntTransmissionType, CS};
  u8 au8ANTSetChannelPeriod0[] = {MESG_CHANNEL_MESG_PERIOD_SIZE, MESG_CHANNEL_MESG_PERIOD_ID, G_stAntSetupData.AntChannel, G_stAntSetupData.AntChannelPeriodLo, G_stAntSetupData.AntChannelPeriodHi, CS};
  u8 au8ANTSetChannelRFFreq0[] = {MESG_CHANNEL_RADIO_FREQ_SIZE, MESG_CHANNEL_RADIO_FREQ_ID, G_stAntSetupData.AntChannel, G_stAntSetupData.AntFrequency, CS};           
  u8 au8ANTSetChannelPower0[]  = {MESG_RADIO_TX_POWER_SIZE, MESG_RADIO_TX_POWER_ID, G_stAntSetupData.AntChannel, G_stAntSetupData.AntTxPower, CS};        
 
  u8 u8ErrorCount = 0;	

  G_u32AntFlags &= ~_ANT_FLAGS_CMD_ERROR;

  /* Adjust the channel type if configuration for a slave device */
  if(!bMaster_)
  {
    au8ANTAssignChannel0[3] = CHANNEL_TYPE_SLAVE;
    G_stAntSetupData.AntChannelType = CHANNEL_TYPE_SLAVE;
  }
  
  /* Assign the channel */
  au8ANTAssignChannel0[5] = AntCalculateTxChecksum(au8ANTAssignChannel0);
  AntTxMessage(au8ANTAssignChannel0);
  u8ErrorCount += AntExpectResponse(MESG_ASSIGN_CHANNEL_ID, ANT_MSG_TIMEOUT_MS);

  /* Assign the channel ID */
  au8ANTSetChannelID0[7] = AntCalculateTxChecksum(au8ANTSetChannelID0);
  AntTxMessage(au8ANTSetChannelID0);
  u8ErrorCount += AntExpectResponse(MESG_CHANNEL_ID_ID, ANT_MSG_TIMEOUT_MS);
    
  /* Assign the channel period */
  au8ANTSetChannelPeriod0[5] = AntCalculateTxChecksum(au8ANTSetChannelPeriod0);
  AntTxMessage(au8ANTSetChannelPeriod0);
  u8ErrorCount += AntExpectResponse(MESG_CHANNEL_MESG_PERIOD_ID, ANT_MSG_TIMEOUT_MS);
    
  /* Assign the channel frequency */
  au8ANTSetChannelRFFreq0[4] = AntCalculateTxChecksum(au8ANTSetChannelRFFreq0);
  AntTxMessage(au8ANTSetChannelRFFreq0);
  u8ErrorCount += AntExpectResponse(MESG_CHANNEL_RADIO_FREQ_ID, ANT_MSG_TIMEOUT_MS);

  /* Assign the channel power */
  au8ANTSetChannelPower0[4] = AntCalculateTxChecksum(au8ANTSetChannelPower0);
  AntTxMessage(au8ANTSetChannelPower0);
  u8ErrorCount += AntExpectResponse(MESG_RADIO_TX_POWER_ID, ANT_MSG_TIMEOUT_MS);
  
  /* If any errors were collected, clear the ANT_GOOD flag */ 
  /* Announce channel status on the debug port */
  DebugPrintf(G_au8AntMessageSetup);
  if(u8ErrorCount)
  {
    G_u32SystemFlags &= ~_APPLICATION_FLAGS_ANT;  
    DebugPrintf(G_au8AntMessageFail);
    return(FALSE);
  }
  else
  {
    DebugPrintf(G_au8AntMessageOk);
    G_u32AntFlags |= _ANT_FLAGS_CHANNEL_CONFIGURED;
    return(TRUE);
  }

} /* end AntChannelConfig() */


/*------------------------------------------------------------------------------
Function: AntOpenChannel

Description:
Queues the Open Channel message on the configured ANT channel.  This does not actually indicate
that the channel has been opened successfully -- the calling task must monitor _ANT_FLAGS_CHANNEL_OPEN
to determine if channel opens successfully.
  
Requires:
  - ANT channel is correctly configured.

Promises:
  - If channel open message is queued, returns TRUE (Ant_u32CurrentTxMessageToken will be non-zero)
  - Otherwise returns FALSE
 
*/
bool AntOpenChannel(void)
{
  u8 au8AntOpenChannel[] = {MESG_OPEN_CHANNEL_SIZE, MESG_OPEN_CHANNEL_ID, G_stAntSetupData.AntChannel, CS};

  /* Update the checksum value and queue the open channel message */
  au8AntOpenChannel[3] = AntCalculateTxChecksum(au8AntOpenChannel);
  G_u32AntFlags |= _ANT_FLAGS_CHANNEL_OPEN_PENDING;
 
  return( AntQueueOutgoingMessage(au8AntOpenChannel) );
  
} /* end AntOpenChannel() */


/*------------------------------------------------------------------------------
Function: AntCloseChannel

Description:
Requests that an ANT channel is closed.  Issuing MESG_CLOSE_CHANNEL_ID does not
guarantee that the channel closes, and ANT response to this message does not
indicate that the channel is closed (a seperate message will be sent when the 
channel actually closes which usually happens on the next ANT message period).
  
Requires:
  - ANT channel is correctly configured and should be open.

Promises:
  - If channel close message is queued, returns TRUE (Ant_u32CurrentTxMessageToken will be non-zero)
  - Otherwise returns FALSE
*/
bool AntCloseChannel(void)
{
  u8 au8AntCloseChannel[] = {MESG_CLOSE_CHANNEL_SIZE, MESG_CLOSE_CHANNEL_ID, G_stAntSetupData.AntChannel, CS};

  /* Update the checksum value and queue the close channel message*/
  au8AntCloseChannel[3] = AntCalculateTxChecksum(au8AntCloseChannel);
  G_u32AntFlags |= _ANT_FLAGS_CHANNEL_CLOSE_PENDING;
  
  return( AntQueueOutgoingMessage(au8AntCloseChannel) );

} /* end AntCloseChannel() */


/*------------------------------------------------------------------------------
Function: AntUnassignChannel

Description:
Queues message to unassigns the current ANT channel so it can be reconfigured.
  
Requires:
  - ANT channel is closed

Promises:
  - ANT channel unassign message is queued; application should monitor AntRadioStatus()
*/
bool AntUnassignChannel(void)
{
  u8 au8AntUnassignChannel[] = {MESG_UNASSIGN_CHANNEL_SIZE, MESG_UNASSIGN_CHANNEL_ID, G_stAntSetupData.AntChannel, CS};

  /* Update checksum and queue the unassign channel message */
  au8AntUnassignChannel[3] = AntCalculateTxChecksum(au8AntUnassignChannel);
  G_u32AntFlags &= ~_ANT_FLAGS_CHANNEL_CONFIGURED;
  return( AntQueueOutgoingMessage(au8AntUnassignChannel) );

} /* end AntUnassignChannel() */


/*------------------------------------------------------------------------------
Function: AntRadioStatus

Description:
Returns the current radio status to the application.
  
Requires:
  - G_u32AntFlags are up to date

Promises:
  - Returns one of {ANT_UNCONFIGURED, ANT_CLOSING, ANT_OPEN, ANT_CLOSED}

*/
AntChannelStatusType AntRadioStatus(void)
{
  if(G_u32AntFlags & _ANT_FLAGS_CHANNEL_CONFIGURED)
  {
    if(G_u32AntFlags & _ANT_FLAGS_CHANNEL_CLOSE_PENDING)
    {
      return ANT_CLOSING;
    }
    else if(G_u32AntFlags & _ANT_FLAGS_CHANNEL_OPEN)
    {
      return ANT_OPEN;
    }
    else
    {
      return ANT_CLOSED;
    }
  }
  else
  {
    return ANT_UNCONFIGURED;
  }
   
} /* end AntRadioStatus () */
#endif /* ANT_API_LEGACY */


/***ANT DATA FUNCTIONS***/

/*-----------------------------------------------------------------------------/
Function: AntQueueBroadcastMessage

Description:
Adds an ANT broadcast message to the outgoing messages list.  

Requires:
  - pu8Data_ is a pointer to the first element of an array of 8 data bytes

Promises:
  - Returns TRUE if the entry is added successfully.
*/
bool AntQueueBroadcastMessage(u8 *pu8Data_)
{
  static u8 au8AntBroadcastDataMessage[] = {MESG_DATA_SIZE, MESG_BROADCAST_DATA_ID, CH, D_0, D_1, D_2, D_3, D_4, D_5, D_6, D_7, CS};

  /* Update the dynamic message data */
  au8AntBroadcastDataMessage[2] = G_stAntSetupData.AntChannel;
  for(u8 i = 0; i < ANT_DATA_BYTES; i++)
  {
    au8AntBroadcastDataMessage[3 + i] = *(pu8Data_ + i);
  }
 
  au8AntBroadcastDataMessage[11] = AntCalculateTxChecksum(au8AntBroadcastDataMessage);
  return( AntQueueOutgoingMessage(au8AntBroadcastDataMessage) );

} /* end AntQueueBroadcastMessage */


/*-----------------------------------------------------------------------------/
Function: AntQueueAcknowledgedMessage

Description:
Adds an ANT Acknowledged message to the outgoing messages list.  

Requires:
  - pu8Data_ is a pointer to the first element of an array of 8 data bytes

Promises:
  - Returns TRUE if the entry is added successfully.
*/
bool AntQueueAcknowledgedMessage(u8 *pu8Data_)
{
  static u8 au8AntAckDataMessage[] = {MESG_DATA_SIZE, MESG_ACKNOWLEDGED_DATA_ID, CH, D_0, D_1, D_2, D_3, D_4, D_5, D_6, D_7, CS};

  /* Update the dynamic message data */
  au8AntAckDataMessage[2] = G_stAntSetupData.AntChannel;
  for(u8 i = 0; i < ANT_DATA_BYTES; i++)
  {
    au8AntAckDataMessage[3 + i] = *(pu8Data_ + i);
  }
 
  au8AntAckDataMessage[11] = AntCalculateTxChecksum(au8AntAckDataMessage);
  return( AntQueueOutgoingMessage(au8AntAckDataMessage) );
 
} /* end AntQueueAcknowledgedMessage */

/*-----------------------------------------------------------------------------/
Function: AntReadAppMessageBuffer

Description:
Checks for any new messages from ANT. The messages are of type AntLocalMessageType
so the application must decide what the data means.

Requires:
  - 

Promises:
  - Returns TRUE if there is new data; G_asAntApiCurrentData holds the message type and data message
  - Returns FALSE if no new data is present (G_asAntApiCurrentData unchanged)
*/
bool AntReadAppMessageBuffer(void)
{
  u8 *pu8Parser;
  
  if(G_sAntApplicationMsgList != NULL)
  {
    G_u32AntApiCurrentDataTimeStamp = G_sAntApplicationMsgList->u32TimeStamp;
    G_eAntApiCurrentMessageClass = G_sAntApplicationMsgList->eMessageType;
    pu8Parser = &(G_sAntApplicationMsgList->au8MessageData[0]);
    for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
    {
      G_au8AntApiCurrentData[i] = *(pu8Parser + i);
    }
    AntDeQueueApplicationMessage();
    
    return TRUE;
  }
  
  /* Otherwise return FALSE and do not touch the current data array */
  return FALSE;
  
} /* end AntReadAppMessageBuffer() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------/
Function: AntApiInitialize

Description:
Configures the Ant API task.

Requires:
  - 

Promises:
  - 
*/
void AntApiInitialize(void)
{
  /* Initialize the AntApi_asChannelConfiguration data struct */
  for(u8 i = 0; i < ANT_CHANNELS; i++)
  {
    AntApi_asChannelConfiguration[i].AntChannel          = i;
    AntApi_asChannelConfiguration[i].AntChannelType      = ANT_CHANNEL_TYPE_DEFAULT;
    AntApi_asChannelConfiguration[i].AntNetwork          = ANT_NETWORK_DEFAULT;
    AntApi_asChannelConfiguration[i].AntDeviceIdLo       = ANT_DEVICE_ID_LO_DEFAULT;
    AntApi_asChannelConfiguration[i].AntDeviceIdHi       = ANT_DEVICE_ID_HI_DEFAULT;
    AntApi_asChannelConfiguration[i].AntDeviceType       = ANT_DEVICE_TYPE_DEFAULT;
    AntApi_asChannelConfiguration[i].AntTransmissionType = ANT_TRANSMISSION_TYPE_DEFAULT;
    AntApi_asChannelConfiguration[i].AntChannelPeriodLo  = ANT_CHANNEL_PERIOD_LO_DEFAULT;
    AntApi_asChannelConfiguration[i].AntChannelPeriodHi  = ANT_CHANNEL_PERIOD_HI_DEFAULT;
    AntApi_asChannelConfiguration[i].AntFrequency        = ANT_FREQUENCY_DEFAULT;
    AntApi_asChannelConfiguration[i].AntTxPower          = ANT_TX_POWER_DEFAULT;
    
    for(u8 j = 0; j < ANT_NETWORK_NUMBER_BYTES; j++)
    {
      AntApi_asChannelConfiguration[i].AntNetworkKey[j] = ANT_DEFAULT_NETWORK_KEY;
    }
  }
  
  /* If good initialization, set state to Idle */
  if( 1 )
  {
    AntApi_StateMachine = AntApiSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    AntApi_StateMachine = AntApiSM_FailedInit;
  }

} /* end AntApiInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function UserAppRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void AntApiRunActiveState(void)
{
  AntApi_StateMachine();

} /* end AntApiRunActiveState */


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a message to be queued */
static void AntApiSM_Idle(void)
{
    
} /* end AntApiSM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void  AntApiSM_Error(void)          
{
  
} /* end AntApiSM_Error() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void  AntApiSM_FailedInit(void)          
{
    
} /* end AntApiSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

