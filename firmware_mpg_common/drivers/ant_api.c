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
Types

typedef enum {ANT_UNCONFIGURED, ANT_CONFIGURED, ANT_OPEN, ANT_CLOSED} AntChannelStatusType;
typedef enum {BROADCAST, ACKNOWLEDGED, BURST, CONTROL} AntDataMessageType;


***ANT CONFIGURATION / STATUS FUNCTIONS***
bool AntChannelConfig(bool)
All channel configuration is sent to the ANT device and TRUE is returned if successful.
This requires a global data structure to be set up in the task.  It is intended to run to completion inside
the application's initialization section.

bool AntOpenChannel(void)
Queues a request to open the configured channel.
Returns TRUE if message is successfully queued.  Application should monitor AntRadioStatus()
for channel status.
e.g. 
if(AntOpenChannel())
{
  ApplicationState = ApplicationWaitForOpen;
}

bool AntCloseChannel(void)
Queues a request to close the configured channel.
Returns TRUE if message is successfully queued.  Application should monitor AntRadioStatus()
for channel status.
if(AntCloseChannel())
{
  ApplicationState = ApplicationWaitForClose;
}

bool AntUnassignChannel(void)
Quees a request to unassign the ANT channel.
Returns TRUE if message is successfully queued.  Application should monitor AntRadioStatus()
for channel status.
if(AntUnassignChannel())
{
  ApplicationState = ApplicationWaitForChannelAssignment;
}

AntChannelStatus AntRadioStatus(void)
Query the ANT radio channel status.


***ANT DATA FUNCTIONS***
bool AntQueueBroadcastMessage(u8 *pu8Data_)
Queue a broadcast data message.

bool AntQueueAcknowledgedMessage(u8 *pu8Data_)
Queue an acknowledged data message.

bool AntReadData(void)
Check the incoming message buffer for any message from the ANT system.  If no messages are present,
returns FALSE.  If message is there, returns TRUE and application can read G_asAntApiCurrentData
to get message info.


***********************************************************************************************************************/

#include "configuration.h"


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>AntApi"
***********************************************************************************************************************/
u32 G_u32AntApiCurrentDataTimeStamp = 0;                                /* Current G_u32SystemTime1s */

AntApplicationMessageType G_eAntApiCurrentMessageClass = ANT_EMPTY;  /* Type of data */
u8 G_au8AntApiCurrentData[ANT_APPLICATION_MESSAGE_BYTES];               /* Array for message data */

/*----------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) 
and indicate what file the variable is defined in. */
extern u32 G_u32SystemFlags;                              /* From main.c */
extern u32 G_u32ApplicationFlags;                         /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                   /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                    /* From board-specific source file */

extern volatile u32 ANT_SSP_FLAGS;                        /* From configuration.h */

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


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

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
  u8 au8ANTAssignChannel0[]    = {MESG_ASSIGN_CHANNEL_SIZE, MESG_ASSIGN_CHANNEL_ID, G_stAntSetupData.AntChannel, G_stAntSetupData.AntChannelType, G_stAntSetupData.AntNetwork, CS};
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
Function: AntReadData

Description:
Checks for any new messages from ANT. The messages are of type AntLocalMessageType
so the application must decide what the data means.

Requires:
  - 

Promises:
  - Returns TRUE if there is new data; G_asAntApiCurrentData holds the message type and data message
  - Returns FALSE if no new data is present (G_asAntApiCurrentData unchanged)
*/
bool AntReadData(void)
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
  
} /* end AntReadData() */

