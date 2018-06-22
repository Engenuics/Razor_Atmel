/*!**********************************************************************************************************************
@file ant_api.c   
@brief ANT user interface.  No initialization or state machine required.  

This file holds the source code for all public functions that work with ant.c.  

Once the ANT radio has been configured, all messaging from the ANT device is handled through 
the incoming queue G_psAntApplicationMsgList.  The application is responsible for checking this
queue for messages that belong to it and must manage timing and handle appropriate updates per 
the ANT messaging protocol.  This should be no problem on the regular 1ms loop timing of the main 
system (assuming ANT message rate is less than 1kHz).  Faster systems or burst messaging will need 
to be handled seperately as an add-on to this API.

Copy the following definitions to your client task:
  
// Globals for passing data from the ANT application to the API

extern u32 G_u32AntApiCurrentMessageTimeStamp;                            // From ant_api.c

extern AntApplicationMessageType G_eAntApiCurrentMessageClass;            // From ant_api.c

extern u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];  // From ant_api.c

extern AntExtendedDataType G_sAntApiCurrentMessageExtData;                // From ant_api.c


------------------------------------------------------------------------------------------------------------------------
GLOBALS
- u32 G_u32AntApiCurrentMessageTimeStamp
- AntApplicationMessageType G_eAntApiCurrentMessageClass
- u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES]
- AntExtendedDataType G_sAntApiCurrentMessageExtData

CONSTANTS
- NONE

TYPES
- NONE (see ant.h for all types used)


PUBLIC ANT CONFIGURATION / STATUS FUNCTIONS
- AntChannelStatusType AntRadioStatusChannel(AntChannelNumberType eChannel_)
- bool AntAssignChannel(AntAssignChannelInfoType* psAntSetupInfo_)
- bool AntUnassignChannelNumber(AntChannelNumberType eChannel_)
- bool AntOpenChannelNumber(AntChannelNumberType eAntChannelToOpen)
- bool AntCloseChannelNumber(AntChannelNumberType eAntChannelToClose)
- bool AntOpenScanningChannel(void)


PUBLIC ANT DATA FUNCTIONS
- bool AntQueueBroadcastMessage(AntChannelNumberType eChannel_, u8 *pu8Data_)
- bool AntQueueAcknowledgedMessage(AntChannelNumberType eChannel_, u8 *pu8Data_)
- bool AntReadAppMessageBuffer(void)

PROTECTED FUNCTIONS
- void AntApiInitialize(void)
- void AntApiRunActiveState(void)


***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>AntApi"
***********************************************************************************************************************/
volatile u32 G_u32AntApiFlags;                                      /*!< @brief Global state flags */

u32 G_u32AntApiCurrentMessageTimeStamp = 0;                         /*!< @brief Current read message's G_u32SystemTime1ms */
AntApplicationMessageType G_eAntApiCurrentMessageClass = ANT_EMPTY; /*!< @brief Type of data */
u8 G_au8AntApiCurrentMessageBytes[ANT_APPLICATION_MESSAGE_BYTES];   /*!< @brief Array for message payload data */
AntExtendedDataType G_sAntApiCurrentMessageExtData;                 /*!< @brief Extended data struct for the current message */


/*----------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) 
and indicate what file the variable is defined in. */
extern volatile u32 G_u32SystemTime1ms;                /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                 /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                  /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;             /*!< @brief From main.c */

extern u32 G_u32AntFlags;                                     /* From ant.c */
extern AntApplicationMsgListType *G_psAntApplicationMsgList;  /* From ant.c */
extern AntAssignChannelInfoType G_asAntChannelConfiguration[ANT_NUM_CHANNELS]; /* From ant.c */
extern AntMessageResponseType G_stAntMessageResponse;         /* From ant.c */

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

extern u8 G_au8AntSetNetworkKey[];                            /* From ant.c */
extern u8 G_au8AntAssignChannel[];                            /* From ant.c */
extern u8 G_au8AntSetChannelID[];                             /* From ant.c */
extern u8 G_au8AntSetChannelPeriod[];                         /* From ant.c */
extern u8 G_au8AntSetChannelRFFreq[];                         /* From ant.c */
extern u8 G_au8AntSetChannelPower[];                          /* From ant.c */
extern u8 G_au8AntSetSearchTimeout[];                         /* From ant.c */
extern u8 G_au8AntLibConfig[];                                /* From ant.c */

extern u8 G_au8AntBroadcastDataMessage[];                     /* From ant.c */
extern u8 G_au8AntAckDataMessage[];                           /* From ant.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "AntApi_<type>Name" and be declared as static.
***********************************************************************************************************************/
static fnCode_type AntApi_StateMachine;             /*!< @brief The state machine function pointer */
static u32 AntApi_u32Timeout;                       /*!< @brief Timeout counter used across states */

static u8* AntApi_apu8AntAssignChannel[] = {G_au8AntSetNetworkKey, G_au8AntLibConfig, G_au8AntAssignChannel, G_au8AntSetChannelID, 
                                            G_au8AntSetChannelPeriod, G_au8AntSetChannelRFFreq, G_au8AntSetChannelPower,
                                            G_au8AntSetSearchTimeout
                                           };       /*!< @brief Message for channel assignment.  Set ANT_ASSIGN_MESSAGES for number of messages. */
    

/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!---------------------------------------------------------------------------------------------------------------------
@fn bool AntAssignChannel(AntAssignChannelInfoType* psAntSetupInfo_)

@brief Updates all configuration messages to completely configure an ANT channel with an application's 
required parameters for communication.  

The application should monitor AntRadioStatusChannel()
to see if all of the configuration messages are sent and the channel is configured properly.

e.g.
  AntAssignChannelInfoType sChannelInfo;

  if(AntRadioStatusChannel(ANT_CHANNEL_0) == ANT_UNCONFIGURED)
  {
    sChannelInfo.AntChannel = ANT_CHANNEL_0;
    sChannelInfo.AntChannelType = CHANNEL_TYPE_MASTER;
    sChannelInfo.AntChannelPeriodHi = ANT_CHANNEL_PERIOD_HI_DEFAULT;
    sChannelInfo.AntChannelPeriodLo = ANT_CHANNEL_PERIOD_LO_DEFAULT;
    
    sChannelInfo.AntDeviceIdHi = 0x00;
    sChannelInfo.AntDeviceIdLo = 0x01;
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

  // Go to a wait state that exits when AntRadioStatusChannel(ANT_CHANNEL_0) no longer returns ANT_UNCONFIGURED)

Requires:
- The ANT channel should not be currently assigned.

@param psAntSetupInfo_ points to a complete AntAssignChannelInfoType with all the required channel information.

Promises:
- Channel, Channel ID, message period, radio frequency and radio power are configured.
- Returns TRUE if the channel is ready to be set up; all global setup messages are updated with the values
  from psAntSetupInfo; 
- Returns FALSE if the channel is already configured

*/
bool AntAssignChannel(AntAssignChannelInfoType* psAntSetupInfo_)
{
  /* Check to ensure the selected channel is available */
  if(AntRadioStatusChannel(psAntSetupInfo_->AntChannel) != ANT_UNCONFIGURED)
  {
    DebugPrintf("AntAssignChannel error: channel is not unconfigured\n\r");
    return FALSE;
  }
  
  /* Setup the library config message (for extended data) - use defaults for now */
  G_au8AntLibConfig[4] = AntCalculateTxChecksum(G_au8AntLibConfig);

  /* Set Network key message */
  G_au8AntSetNetworkKey[2] = psAntSetupInfo_->AntNetwork;
  for(u8 i = 0; i < ANT_NETWORK_NUMBER_BYTES; i++)
  {
    G_au8AntSetNetworkKey[i + 3] = psAntSetupInfo_->AntNetworkKey[i];
    G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntNetworkKey[i] = psAntSetupInfo_->AntNetworkKey[i];
  }
  G_au8AntSetNetworkKey[11] = AntCalculateTxChecksum(G_au8AntSetNetworkKey);
    
  /* Setup the channel message */
  G_au8AntAssignChannel[2] = psAntSetupInfo_->AntChannel;
  G_au8AntAssignChannel[3] = psAntSetupInfo_->AntChannelType;
  G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntChannelType = psAntSetupInfo_->AntChannelType;

  G_au8AntAssignChannel[4] = psAntSetupInfo_->AntNetwork;
  G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntNetwork = psAntSetupInfo_->AntNetwork;

  G_au8AntAssignChannel[5] = AntCalculateTxChecksum(G_au8AntAssignChannel);

  /* Setup the channel ID message */
  G_au8AntSetChannelID[2] = psAntSetupInfo_->AntChannel;
  G_au8AntSetChannelID[3] = psAntSetupInfo_->AntDeviceIdLo;
  G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntDeviceIdLo = psAntSetupInfo_->AntDeviceIdLo;

  G_au8AntSetChannelID[4] = psAntSetupInfo_->AntDeviceIdHi;
  G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntDeviceIdHi = psAntSetupInfo_->AntDeviceIdHi;

  G_au8AntSetChannelID[5] = psAntSetupInfo_->AntDeviceType;
  G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntDeviceType = psAntSetupInfo_->AntDeviceType;

  G_au8AntSetChannelID[6] = psAntSetupInfo_->AntTransmissionType;
  G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntTransmissionType = psAntSetupInfo_->AntTransmissionType;

  G_au8AntSetChannelID[7] = AntCalculateTxChecksum(G_au8AntSetChannelID);
    
  /* Setup the channel period message */
  G_au8AntSetChannelPeriod[2] = psAntSetupInfo_->AntChannel;
  G_au8AntSetChannelPeriod[3] = psAntSetupInfo_->AntChannelPeriodLo;
  G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntChannelPeriodLo = psAntSetupInfo_->AntChannelPeriodLo;

  G_au8AntSetChannelPeriod[4] = psAntSetupInfo_->AntChannelPeriodHi;
  G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntChannelPeriodHi = psAntSetupInfo_->AntChannelPeriodHi;

  G_au8AntSetChannelPeriod[5] = AntCalculateTxChecksum(G_au8AntSetChannelPeriod);
    
  /* Setup the channel frequency message */
  G_au8AntSetChannelRFFreq[2] = psAntSetupInfo_->AntChannel;
  G_au8AntSetChannelRFFreq[3] = psAntSetupInfo_->AntFrequency;
  G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntFrequency = psAntSetupInfo_->AntFrequency;

  G_au8AntSetChannelRFFreq[4] = AntCalculateTxChecksum(G_au8AntSetChannelRFFreq);

  /* Setup the channel power message */
  G_au8AntSetChannelPower[3] = psAntSetupInfo_->AntTxPower;
  G_asAntChannelConfiguration[psAntSetupInfo_->AntChannel].AntTxPower = psAntSetupInfo_->AntTxPower;

  G_au8AntSetChannelPower[4] = AntCalculateTxChecksum(G_au8AntSetChannelPower);
     
  /* Setup the the channel search timeout (currently set at default for infinite search) */
  G_au8AntSetSearchTimeout[2] = psAntSetupInfo_->AntChannel;
  G_au8AntSetSearchTimeout[3] = ANT_INFINITE_SEARCH_TIMEOUT;

  G_au8AntSetSearchTimeout[4] = AntCalculateTxChecksum(G_au8AntSetSearchTimeout);
  
  /* Set the next state to begin transferring */
  AntApi_u32Timeout = G_u32SystemTime1ms;
  AntApi_StateMachine = AntApiSM_AssignChannel;
  return TRUE;

} /* end AntAssignChannel() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn bool AntUnassignChannelNumber(AntChannelNumberType eChannel_)

@brief Queues message to unassign the specified ANT channel so it can be reconfigured.

 
Requires:
- ANT channel is closed

@param eChannel_ is the channel to unassign

Promises:
- ANT channel unassign message is queued; application should monitor AntRadioStatus()

*/
bool AntUnassignChannelNumber(AntChannelNumberType eChannel_)
{
  u8 au8AntUnassignChannel[] = {MESG_UNASSIGN_CHANNEL_SIZE, MESG_UNASSIGN_CHANNEL_ID, 0, CS};

  /* Check if the channel is closed */
  if(AntRadioStatusChannel(eChannel_) != ANT_CLOSED)
  {
    DebugPrintf("AntUnssignChannel error: channel not closed\n\r");
    return FALSE;
  }
  
  /* Update the channel number */
   au8AntUnassignChannel[2] = eChannel_;

  /* Update checksum and queue the unassign channel message */
  au8AntUnassignChannel[3] = AntCalculateTxChecksum(au8AntUnassignChannel);
  return( AntQueueOutgoingMessage(au8AntUnassignChannel) );

} /* end AntUnassignChannelNumber() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn bool AntOpenChannelNumber(AntChannelNumberType eChannel_)

@brief Queues a request to open the specified channel.

Queues the Open Channel message on the configured ANT channel.  This does not actually indicate
that the channel has been opened successfully -- the calling task must monitor _ANT_FLAGS_CHANNEL_OPEN
to determine if channel opens successfully.

Returns TRUE if the channel is configured and the message is successfully queued - this can be ignored or checked.  
Application should monitor AntRadioStatusChannel() for actual channel status.

e.g.
AntChannelStatusType eAntCurrentState;

// Request to open channel only on an already closed channel.
eAntCurrentState = AntChannelStatus(ANT_CHANNEL_1);

if(eAntCurrentState == ANT_CLOSED )
{
   AntOpenChannelNumber(ANT_CHANNEL_1);
}

Requires:
- ANT channel to open should already be correctly configured.

@param eChannel_ is ANT_CHANNEL_0, ..., ANT_CHANNEL_7, ANT_CHANNEL_SCANNING

Promises:
- If channel open message is queued, returns TRUE (Ant_u32CurrentTxMessageToken will be non-zero)
- Otherwise returns FALSE
 
*/
bool AntOpenChannelNumber(AntChannelNumberType eChannel_)
{
  u8 au8AntOpenChannel[] = {MESG_OPEN_CHANNEL_SIZE, MESG_OPEN_CHANNEL_ID, 0, CS};
  
  /* Check if the channel is ready */
  if(AntRadioStatusChannel(eChannel_) != ANT_CONFIGURED)
  {
    DebugPrintf("AntOpenChannel error: channel not ready\n\r");
    return FALSE;
  }

  /* Update the channel number in the message for a regular channel */
  if(eChannel_ != ANT_CHANNEL_SCANNING)
  {
    au8AntOpenChannel[2] = eChannel_;
  }

  /* Update the checksum value and queue the open channel message */
  au8AntOpenChannel[3] = AntCalculateTxChecksum(au8AntOpenChannel);
  G_asAntChannelConfiguration[eChannel_].AntFlags |= _ANT_FLAGS_CHANNEL_OPEN_PENDING;
 
  return( AntQueueOutgoingMessage(au8AntOpenChannel) );
  
} /* end AntOpenChannelNumber() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn bool AntOpenScanningChannel(void)


@brief Queues a request to open a scanning channel. 

Queues the Open Scan Channel message.  Scanning channels must use Channel 0
and takes all channel resources.  It is up to the user to ensure
that the device is configured properly.  Attempting to open a scanning channel
when other channels are already open will result in a failure.

Returns TRUE if message is successfully queued - this can be ignored or checked.  
Application should monitor AntRadioStatusChannel() for actual channel status.

e.g.
AntChannelStatusType eAntCurrentState;

// Request to open channel only on an already closed channel.
if(AntRadioStatusChannel(ANT_CHANNEL_SCANNING) == ANT_CLOSED )
{
   AntOpenScanningChannel();
}

Requires:
- ANT channel 0 should be correctly configured as a SLAVE.

Promises:
- If channel open message is queued, returns TRUE (Ant_u32CurrentTxMessageToken will be non-zero)
- Otherwise returns FALSE
 
*/
bool AntOpenScanningChannel(void)
{
  u8 au8AntOpenScanChannel[] = {MESG_OPEN_CHANNEL_SIZE, MESG_OPEN_SCAN_CHANNEL_ID, 0, CS};
  
  /* Check if the channel is ready */
  if(AntRadioStatusChannel(ANT_CHANNEL_0) != ANT_CONFIGURED)
  {
    DebugPrintf("AntOpenScanningChannel error: channel not ready\n\r");
    return FALSE;
  }

  /* Update the checksum value and queue the open channel message */
  au8AntOpenScanChannel[3] = AntCalculateTxChecksum(au8AntOpenScanChannel);
  G_asAntChannelConfiguration[0].AntFlags |= _ANT_FLAGS_CHANNEL_OPEN_PENDING;
 
  return( AntQueueOutgoingMessage(au8AntOpenScanChannel) );
  
} /* end AntOpenScanningChannelNumber() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn bool AntCloseChannelNumber(AntChannelNumberType eChannel_)

@brief Queues a request to close the specified channel.

Requests that an ANT channel is closed.  Issuing MESG_CLOSE_CHANNEL_ID does not
guarantee that the channel closes, and ANT response to this message does not
indicate that the channel is closed (a seperate message will be sent when the 
channel actually closes which usually happens on the next ANT message period).

Returns TRUE if the message is successfully queued - this can be ignored or checked.  
Application should monitor AntRadioStatusChannel() for actual channel status.
e.g.
AntChannelStatusType eAntCurrentState;

// Request to close channel only on an open channel.
if(AntRadioStatusChannel(ANT_CHANNEL_1) == ANT_OPEN )
{
   AntCloseChannelNumber(ANT_CHANNEL_1);
}

Requires:
- ANT channel is correctly configured and should be open.

@param eChannel_ is ANT_CHANNEL_0, ..., ANT_CHANNEL_7, ANT_CHANNEL_SCANNING

Promises:
- If channel close message is queued, returns TRUE (Ant_u32CurrentTxMessageToken will be non-zero)
- Otherwise returns FALSE

*/
bool AntCloseChannelNumber(AntChannelNumberType eChannel_)
{
  u8 au8AntCloseChannel[] = {MESG_CLOSE_CHANNEL_SIZE, MESG_CLOSE_CHANNEL_ID, 0, CS};
  
  /* Check if the channel is ready */
  if(AntRadioStatusChannel(eChannel_) != ANT_OPEN)
  {
    DebugPrintf("AntCloseChannel error: channel not open\n\r");
    return FALSE;
  }

  /* Update the channel number */
  au8AntCloseChannel[2] = eChannel_;

  /* Update the checksum value and queue the close channel message*/
  au8AntCloseChannel[3] = AntCalculateTxChecksum(au8AntCloseChannel);
  G_asAntChannelConfiguration[eChannel_].AntFlags |= _ANT_FLAGS_CHANNEL_CLOSE_PENDING;
  
  return( AntQueueOutgoingMessage(au8AntCloseChannel) );

} /* end AntCloseChannelNumber() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn AntChannelStatusType AntRadioStatusChannel(AntChannelNumberType eChannel_)

@brief Query the status of the specified channel.  

Returns ANT_UNCONFIGURED, ANT_CONFIGURED, ANT_OPENING, ANT_OPEN, ANT_CLOSING, ANT_CLOSED
ANT_CONFIGURED and ANT_CLOSED are actually the same state.
e.g.
AntChannelStatus eAntCurrentStatus;

// Get the status of Channel 1
eAntCurrentStatus = AntRadioStatusChannel(ANT_CHANNEL_1);
  
Requires:
- AntFlags for the specified channel are up to date 

@param eChannel_ is the ANT channel whose status is desired

Promises:
  - Returns one of {ANT_UNCONFIGURED, ANT_CONFIGURED (ANT_CLOSED), ANT_CLOSING, ANT_OPEN, ANT_CLOSED}

*/
AntChannelStatusType AntRadioStatusChannel(AntChannelNumberType eChannel_)
{
  if(G_asAntChannelConfiguration[eChannel_].AntFlags & _ANT_FLAGS_CHANNEL_CONFIGURED)
  {
    if(G_asAntChannelConfiguration[eChannel_].AntFlags & _ANT_FLAGS_CHANNEL_CLOSE_PENDING)
    {
      return ANT_CLOSING;
    }
    else if(G_asAntChannelConfiguration[eChannel_].AntFlags & _ANT_FLAGS_CHANNEL_OPEN)
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
   
} /* end AntRadioStatusChannel () */


/***ANT DATA MESSAGE FUNCTIONS***/

/*!---------------------------------------------------------------------------------------------------------------------
@fn bool AntQueueBroadcastMessage(AntChannelNumberType eChannel_, u8 *pu8Data_)

@brief Adds an ANT broadcast message to the outgoing messages list.  

Requires:
@param eChannel_ is the channel number on which to broadcast
@param pu8Data_ is a pointer to the first element of an array of 8 data bytes

Promises:
- Returns TRUE if the entry is added successfully.

*/
bool AntQueueBroadcastMessage(AntChannelNumberType eChannel_, u8 *pu8Data_)
{
  /* Update the dynamic message data */
  G_au8AntBroadcastDataMessage[2] = eChannel_;
  for(u8 i = 0; i < ANT_DATA_BYTES; i++)
  {
    G_au8AntBroadcastDataMessage[3 + i] = *(pu8Data_ + i);
  }
 
  G_au8AntBroadcastDataMessage[11] = AntCalculateTxChecksum(G_au8AntBroadcastDataMessage);
  
  return( AntQueueOutgoingMessage(G_au8AntBroadcastDataMessage) );

} /* end AntQueueBroadcastMessage */


/*!---------------------------------------------------------------------------------------------------------------------
@fn bool AntQueueAcknowledgedMessage(AntChannelNumberType eChannel_, u8 *pu8Data_)

@brief Adds an ANT Acknowledged message to the outgoing messages list.  

Requires:
@param eChannel_ is the channel number on which to broadcast
@param pu8Data_ is a pointer to the first element of an array of 8 data bytes

Promises:
- Returns TRUE if the entry is added successfully.

*/
bool AntQueueAcknowledgedMessage(AntChannelNumberType eChannel_, u8 *pu8Data_)
{
  /* Update the dynamic message data */
  G_au8AntAckDataMessage[2] = eChannel_;
  for(u8 i = 0; i < ANT_DATA_BYTES; i++)
  {
    G_au8AntAckDataMessage[3 + i] = *(pu8Data_ + i);
  }
 
  G_au8AntAckDataMessage[11] = AntCalculateTxChecksum(G_au8AntAckDataMessage);
  return( AntQueueOutgoingMessage(G_au8AntAckDataMessage) );
 
} /* end AntQueueAcknowledgedMessage */


/*!---------------------------------------------------------------------------------------------------------------------
@fn bool AntReadAppMessageBuffer(void)

@brief Checks for any new messages from ANT.  

New messages are buffered by ant.c and made available to the application 
on a FIFO basis.  Whenever this function is called, the global 
parameters are updated:
- G_u32AntApiCurrentMessageTimeStamp
- G_eAntApiCurrentMessageClass
- G_au8AntApiCurrentMessageBytes
- G_sAntApiCurrentMessageExtData

The application should check G_eAntApiCurrentMessageClass to determine if the message
is ANT_DATA or ANT_TICK and then use G_au8AntApiCurrentMessageBytes and
G_sAntApiCurrentMessageExtData accordingly.

Requires:
- NONE

Promises:
- Returns TRUE if there is new data; 
  G_u32AntApiCurrentMessageTimeStamp
  G_eAntApiCurrentMessageClass
  G_au8AntApiCurrentMessageBytes
  G_sAntApiCurrentMessageExtData
  are all updated with the oldest data from G_psAntApplicationMsgList and the message
  is removed from the buffer.
- Returns FALSE if no new data is present (all variables unchanged)

*/
bool AntReadAppMessageBuffer(void)
{
  u8 *pu8Parser;
  
  if(G_psAntApplicationMsgList != NULL)
  {
    /* Grab the single bytes */
    G_u32AntApiCurrentMessageTimeStamp = G_psAntApplicationMsgList->u32TimeStamp;
    G_eAntApiCurrentMessageClass = G_psAntApplicationMsgList->eMessageType;
    
    /* Copy over all the payload data */
    pu8Parser = &(G_psAntApplicationMsgList->au8MessageData[0]);
    for(u8 i = 0; i < ANT_APPLICATION_MESSAGE_BYTES; i++)
    {
      G_au8AntApiCurrentMessageBytes[i] = *(pu8Parser + i);
    }
    
    /* Copy over the extended data */
    G_sAntApiCurrentMessageExtData.u8Channel    = G_psAntApplicationMsgList->sExtendedData.u8Channel;
    G_sAntApiCurrentMessageExtData.u8Flags      = G_psAntApplicationMsgList->sExtendedData.u8Flags;
    G_sAntApiCurrentMessageExtData.u16DeviceID  = G_psAntApplicationMsgList->sExtendedData.u16DeviceID;
    G_sAntApiCurrentMessageExtData.u8DeviceType = G_psAntApplicationMsgList->sExtendedData.u8DeviceType;
    G_sAntApiCurrentMessageExtData.u8TransType  = G_psAntApplicationMsgList->sExtendedData.u8TransType;
    G_sAntApiCurrentMessageExtData.s8RSSI       = G_psAntApplicationMsgList->sExtendedData.s8RSSI;
    
    /* Done, so message can be removed from the buffer */
    AntDeQueueApplicationMessage();    
    return TRUE;
  }
  
  /* Otherwise return FALSE and do not touch the current data array */
  return FALSE;
  
} /* end AntReadAppMessageBuffer() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void AntGetdBmAscii(s8 s8RssiValue_, u8* pu8Result_)

@brief Takes the binary RSSI value and writes back a string with the value in dBm.

The string INCLUDES the sign (- or +, even though it will ALWAYS be - for ANT)
but does NOT include the unit text dBm.  Leading 0s are always returned,
thus the returned string is always three chars (no terminating NULL).

Requires:
@param s8RssiValue_ is the signed 8-bit RSSI value to convert.  The number is two's complement.
@param pu8Result_ points to a string with at least 3 bytes of space to write

Promises:
- *pu8Result_ receive the '-xx' where xx is the value in dBm (does not send
  a terminating NULL)

*/
void AntGetdBmAscii(s8 s8RssiValue_, u8* pu8Result_)
{
  u8 u8AbsoluteValue;
  
  /* Handle the positive number */
  if(s8RssiValue_ >= 0)
  {
    /* Print '+' but only for numbers larger than 0 */
    *pu8Result_ = '+';
    if(s8RssiValue_ == 0)
    {
      *pu8Result_ = ' ';
    }
    
    u8AbsoluteValue = (u8)s8RssiValue_;
  }
  /* Handle the negative number */
  else
  {
    *pu8Result_ = '-';
    u8AbsoluteValue = (u8)(~s8RssiValue_ + 1);
  }
  
  /* Limit any display to two digit */
  if(u8AbsoluteValue > 99)
  {
    u8AbsoluteValue = 99;
  }
  

  /* Write the numeric value */
  pu8Result_++;
  *pu8Result_ = (u8AbsoluteValue / 10) + NUMBER_ASCII_TO_DEC;
  pu8Result_++;
  *pu8Result_ = (u8AbsoluteValue % 10) + NUMBER_ASCII_TO_DEC;
  
} /* end AntGetdBmAscii() */


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!---------------------------------------------------------------------------------------------------------------------
@fn void AntApiInitialize(void)

@brief Runs required initialization for the task.  

Should only be called once in main init section.

Requires:
- NONE 

Promises:
- Ant API set to Idle

*/
void AntApiInitialize(void)
{
  /* If good initialization, set state to Idle */
  if(G_u32ApplicationFlags & _APPLICATION_FLAGS_ANT)
  {
    DebugPrintf("ANT API ready\n\r");
    AntApi_StateMachine = AntApiSM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    AntApi_StateMachine = AntApiSM_FailedInit;
  }

} /* end AntApiInitialize() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void AntApiRunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

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


/*----------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*----------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void AntApiSM_Idle(void)

@brief Wait for a message to be queued.
*/
static void AntApiSM_Idle(void)
{
  /* Monitor requests to send generic ANT messages */
  
} /* end AntApiSM_Idle() */
     

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void AntApiSM_AssignChannel(void)          

@brief Send all configuration messages to ANT to fully assign the channel.

The selected channel has already been verified unconfigured, and all required setup
messages are ready to go.  
*/
static void AntApiSM_AssignChannel(void)          
{
  static u8 u8CurrentMessageToSend = 0;
  static u8 u8CurrentMesssageId = 0;
  static bool bMessageInProgress = FALSE;
  
  /* Queue the next message if it's time */
  if(bMessageInProgress == FALSE)
  {
    u8CurrentMesssageId = *(AntApi_apu8AntAssignChannel[u8CurrentMessageToSend] + BUFFER_INDEX_MESG_ID); 
    AntQueueOutgoingMessage(AntApi_apu8AntAssignChannel[u8CurrentMessageToSend]);
    bMessageInProgress = TRUE;
  }
  
  /* Check message status */
  if( u8CurrentMesssageId == G_stAntMessageResponse.u8MessageNumber )
  { 
    if(G_stAntMessageResponse.u8ResponseCode == RESPONSE_NO_ERROR)
    {
      /* Increment message pointer and check if complete */
      u8CurrentMessageToSend++;
      if(u8CurrentMessageToSend == ANT_ASSIGN_MESSAGES)
      {
        /* Print OK message and update the channel flags */
        G_au8AntMessageAssign[12] = G_stAntMessageResponse.u8Channel + NUMBER_ASCII_TO_DEC;
        DebugPrintf(G_au8AntMessageAssign);
        DebugPrintf(G_au8AntMessageOk);
        G_asAntChannelConfiguration[G_stAntMessageResponse.u8Channel].AntFlags |= _ANT_FLAGS_CHANNEL_CONFIGURED;

        /* Clean up and exit this state */
        u8CurrentMessageToSend = 0;
        AntApi_StateMachine = AntApiSM_Idle;
      }
    } 
    else
    {
      /* Report the error and return.  Channel flags will remain clear for application to check. */
      DebugPrintf(G_au8AntMessageAssign);
      DebugPrintf(G_au8AntMessageFail);
      AntApi_StateMachine = AntApiSM_Idle;
    }

    /* In either case, clean up the following: */
    bMessageInProgress = FALSE;

  } /* end if( u8CurrentMesssageId == G_stAntMessageResponse.u8MessageNumber ) */
  
  /* Check for timeout */
  if(IsTimeUp(&AntApi_u32Timeout, ANT_ACTIVITY_TIME_COUNT) )
  {
    /* Report the error and return.  Channel flags will remain clear for application to check. */
    DebugPrintf(G_au8AntMessageAssign);
    DebugPrintf(G_au8AntMessageFail);
    AntApi_StateMachine = AntApiSM_Idle;
  }
  
} /* end AntApiSM_AssignChannel() */


#if 0
/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void  AntApiSM_Error(void)          
{
  
} /* end AntApiSM_Error() */
#endif


/*-------------------------------------------------------------------------------------------------------------------*/
/* State to sit in if init failed */
static void  AntApiSM_FailedInit(void)          
{
    
} /* end AntApiSM_FailedInit() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

