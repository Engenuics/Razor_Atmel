// Configuration struct
extern AntSetupDataType G_stAntSetupData;                         // From ant.c

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

