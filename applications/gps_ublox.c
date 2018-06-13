/*******************************************************************************
* File: gps_ublox.c                                                                
* Description:
* GPS functions and state machine for use with ublox AMY GPS module.
*******************************************************************************/

#include "configuration.h"
#include "gps_ublox.h"

/*******************************************************************************
* "Global Global" Definitions (public variables for data transfer between files)
* All Global Global variable names defined in this file shall start with GGxx
*******************************************************************************/
/* New variables */
fnCode_type GGGPSStateMachine;          /* The GPS state machine */
u32 GGu32GPSFlags;                      /* GPS state flags */

u8 *GGpu8GpsRxBufferNextChar;           /* Pointer to the chars in LGau8GPSRxBuffer */
GPSDataStructType GGstGPSCurrentData;   /* Current GPS data struct */

/*----------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword
and indicate which file they are defined in). */
extern u32 GGu32SystemFlags;            /* From main.c */

extern u32 GGu32SysTime1ms;             /* From development.c */
extern u32 GGu32SysTime1s;              /* From development.c */


/*******************************************************************************
* "Local Global" Definitions (private variables for data transfer between functions in this file)
* Variables names shall start with "LG"
*******************************************************************************/
u8 LGau8GPSRxBuffer[GPS_RX_BUFFER_SIZE];         /* Space for incoming characters of GPS messages */
u8 *LGpu8GPSRxBufferParser;                      /* Pointer to loop through the LGau8GPSRxBuffer */
u8 *LGpu8GPSRxBufferCurrentChar;                 /* Pointer to unread chars in LGau8GPSRxBuffer */
u8 LGu32GPSMessageCounter;                       /* Count of GPS messages received */

UbxMessageType LGsUbxMessage;                    /* Structure for data of an incoming ublox binary message */
u16 LGu16LengthCopy;                             /* Copy of received message length of an incoming UBX message */

UbxResponseListNodeType *LGsResponseList = NULL; /* Pointer to the response linked list */
UbxFailMsgListNodeType *LGsFailedMsgList = NULL; /* Pointer to the failed message linked list */

u8  LGu8ErrorCode;                               /* State machine error code */
u32 LGu32GPSTimeout;                             /* Timeout timer across states */
u32 LGu32GPSDebugDataRate;                       /* Counter for timing data dumps to debug port if active */

GPSSVDataType LGastGPSSVData1[GPS_MAX_SV_DATA];  /* Array 1 of satellite vehicle data */
GPSSVDataType LGastGPSSVData2[GPS_MAX_SV_DATA];  /* Array 2 of satellite vehicle data */
GPSSVDataType LGastGPSSVData3[GPS_MAX_SV_DATA];  /* Array 3 of satellite vehicle data */
GPSSVDataType* LGpastGPSCurrentSVData = NULL;    /* Pointer to current array of satellite vehicle data */
GPSSVDataType* LGpastGPSNewSVData = NULL;        /* Pointer to new (incoming) array of satellite vehicle data */

/* Debug variables */
u32 LGu32GPGGACount = 0;
u32 LGu32GPGSACount = 0;
u32 LGu32GPVTGCount = 0;
u32 LGu32GPGSVCount = 0;
u32 LGu32OtherCount = 0;
u32 LGu32MessageCount = 0;


/*******************************************************************************
* Function Definitions
*******************************************************************************/

/*-----------------------------------------------------------------------------/
void GpsPowerOn(void)

Description:
Turns on power to GPS hardware.

Requires:
  - GPS_PWR_EN is configured to output
 
Promises:
  - GPS power control line is activated (low) and UART lines are configured
    for communication (so GPS does not have parasitic power)
*/
void GpsPowerOn(void)
{
  /* Turn on MOSFET to enable GPS power */
  //GPIO1->FIOCLR = GPS_PWR_EN;
  GPIO0->FIOCLR = GPS_SAFEBOOT;
  
  /* Set IO pins to UART mode */
  PINCON->PINSEL9 |= 0x0F000000;
  
} /* end GpsPowerOn() */


/*-----------------------------------------------------------------------------/
void GpsPowerOff(void)

Description:
Turns off all power to GPS hardware.

Requires:
  - GPS_PWR_EN is configured to output
 
Promises:
  - GPS power control line is deactivated and UART lines are configured
    for output low so GPS does not have parasitic power.
*/
void GpsPowerOff(void)
{
  /* Set IO pins to GPIO mode */
  PINCON->PINSEL9 &= 0xF0FFFFFF;
  GPIO4->FIOCLR = (GPS_UART_PIMO + GPS_UART_POMI);

  /* Turn off MOSFET to disable GPS power */
  GPIO0->FIOSET = GPS_SAFEBOOT;
  //GPIO1->FIOSET = GPS_PWR_EN;
  
  /* Clear all GPS data since no fix is possible when GPS is off ! */
  GGu32GPSFlags &= ~(_GPS_FLAGS_2D_FIX | _GPS_FLAGS_3D_FIX);
  GpsClearAllData();
  
} /* end GpsPowerOff() */


/*-----------------------------------------------------------------------------/
void GpsIOInitialize(void)

Requires:
 
Promises:
  - GPS_UART hardware is configured
  - GPS_1PPS is input
  - GPS_BACKUP_RST is low so reset is not active
  - GPS_SAFEBOOT is high for normal operation
  - GPS_PWR_EN is high to keep power off for GPS

*/
void GpsIOInitialize(void)
{
  /* Configure IO functions for required GPS mode */
  PINCON->PINSEL0 &= GPS_GPIO0_PINSEL0_CLEAR;
  PINCON->PINSEL3 &= GPS_GPIO1_PINSEL3_CLEAR;
  PINCON->PINSEL9 &= 0xF0FFFFFF;
  GPIO0->FIODIR |= GPS_BACKUP_RST + GPS_SAFEBOOT;
  GPIO1->FIODIR |= GPS_PWR_EN;
  GPIO1->FIODIR &= ~GPS_1PPS;
  GPIO1->FIOSET = GPS_PWR_EN;
  //GPIO0->FIOSET = GPS_SAFEBOOT;
  GPIO0->FIOCLR = GPS_BACKUP_RST;
  GPIO4->FIOCLR = (GPS_UART_PIMO + GPS_UART_POMI);

} /* end GpsIOInitialize() */


/*-----------------------------------------------------------------------------/
void GpsInitialize(void)

Requires:
 
Promises:
  - GPS is powered on and firmware information is read
  - GPS_UART is configured and ready
  - GPS_PWR_EN is low to power on GPS
  - GPS_1PPS is input
  - GPS_BACKUP_RST is low so reset is not active
  - GPS_SAFEBOOT is high for normal operation

*/
void GpsInitialize(void)
{
  u8 au8GPSInitMsg[]   = {"Initializing GPS\n\r"};
  u8 au8GPSFailedMsg[] = {"No GPS response\r\n"};
  UbxMessageType sUbxMessage;
  u8 au8PowerSaveMode[]  = {RXM_RESERVED, RXM_LPMODE_POWERSAVE};
  u8 au8PowerMgtConfig[] = {PM_MSG_VERSION, 0,0,0,
                            PM_FLAGS0, PM_FLAGS1, PM_FLAGS2, PM_FLAGS3, 
                            PM_UPDATE_PERIOD0, PM_UPDATE_PERIOD1, PM_UPDATE_PERIOD2, PM_UPDATE_PERIOD3,
                            PM_SEARCH_PERIOD0, PM_SEARCH_PERIOD1, PM_SEARCH_PERIOD2, PM_SEARCH_PERIOD3,
                            PM_GRID_OFFSET0, PM_GRID_OFFSET1, PM_GRID_OFFSET2, PM_GRID_OFFSET3,
                            PM_ON_TIME0, PM_ON_TIME1, PM_MIN_SEARCH_TIME0, PM_MIN_SEARCH_TIME0};
  u8 au8FixNowConfig[]   = {FXN_FLAGS0, FXN_FLAGS1, FXN_FLAGS2, FXN_FLAGS3,
                            FXN_T_REACQ0, FXN_T_REACQ1, FXN_T_REACQ2, FXN_T_REACQ3,
                            FXN_T_ACQ0, FXN_T_ACQ1, FXN_T_ACQ2, FXN_T_ACQ3,
                            FXN_T_REACQ_OFF0, FXN_T_REACQ_OFF1, FXN_T_REACQ_OFF2, FXN_T_REACQ_OFF3,
                            FXN_T_ACQ_OFF0, FXN_T_ACQ_OFF1, FXN_T_ACQ_OFF2, FXN_T_ACQ_OFF3,
                            FXN_T_ON0, FXN_T_ON1, FXN_T_ON2, FXN_T_ON3,
                            FXN_T_OFF0, FXN_T_OFF1, FXN_T_OFF2, FXN_T_OFF3,
                            0, 0, 0, 0, 0, 0, 0, 0};

  u8 au8CFG_MSG_POLL_PAYLOAD[] = {UBXMSG_CLASS_NMEA, UBXMSG_NMEA_ID_GGA};
  u32 u32Timeout;
  
  /* Announce that GPS is starting up */
  QueueTxMessage(DEBUG_MSG_SRC, sizeof(au8GPSInitMsg), &au8GPSInitMsg[0]);
  MsgSenderForceSend();
  
  /* Initialize buffer pointers and receive buffer */
  LGpu8GPSRxBufferParser = &LGau8GPSRxBuffer[0];
  LGpu8GPSRxBufferCurrentChar = &LGau8GPSRxBuffer[0];
  GGpu8GpsRxBufferNextChar = &LGau8GPSRxBuffer[0];
  LGsUbxMessage.pu8Payload = NULL;
  
  for(u32 i = 0; i < GPS_RX_BUFFER_SIZE; i++)
  {
    GGpu8GpsRxBufferNextChar[i] = 0;
  }

  /* Initialize the GPS tracking data */
  GpsClearAllData();

  /* Initialize GPS system variables */  
  LGu32GPSMessageCounter = 0;
  GpsIOInitialize();

  /* Apply power to GPS */
  GpsPowerOn();
  
  /* Enable GPS UART interrupts */
  BLADE_UART->IER |= 0x00000007;

#if 0
  /* After 100ms start up time, take GPS module out of reset and set SM start state */
  u32Timeout = GGu32SysTime1ms;
  while( !IsTimeUp(&GGu32SysTime1ms, &u32Timeout, GPS_STARTUP_TIME, RESET_TARGET_TIMER) );
#endif
  LGu32GPSMessageCounter = 0;
  GGGPSStateMachine = GPSSMIdle;
  
  /* Configure GPS receiver using UBX protocol */
  /* Set specific messages */
  /* Set power mode */
  
  /* Properly decide if GPS is ready and set the flag */
  /* Get GPS version */
  
  /* Build the Version request message to send */
  sUbxMessage.u8Class = UBXMSG_CLASS_MON; 
  sUbxMessage.u8MsgID = UBXMSG_ID_VER; 
  sUbxMessage.u16Length = 0; 
  sUbxMessage.pu8Payload = NULL; 

  /* Send the message */
  QueueUbxTxMessage(&sUbxMessage, TRUE);
  MsgSenderForceSend();

  /* Build the power save mode message to send */
  sUbxMessage.u8Class = UBXMSG_CLASS_CFG; 
  sUbxMessage.u8MsgID = UBXMSG_ID_RXM; 
  sUbxMessage.u16Length = UBXMSG_LENGTH_RXM; 
  sUbxMessage.pu8Payload = &au8PowerSaveMode[0]; 

  /* Send the message */
  QueueUbxTxMessage(&sUbxMessage, TRUE);
  MsgSenderForceSend();
  
  /* Query the power save mode */
  sUbxMessage.u8Class = UBXMSG_CLASS_CFG; 
  sUbxMessage.u8MsgID = UBXMSG_ID_RXM; 
  sUbxMessage.u16Length = 0; 
  sUbxMessage.pu8Payload = NULL; 

  /* Send the message */
  QueueUbxTxMessage(&sUbxMessage, TRUE);
  MsgSenderForceSend();

  /* Build the FixNOW config command */
  sUbxMessage.u8Class = UBXMSG_CLASS_CFG; 
  sUbxMessage.u8MsgID = UBXMSG_ID_FXN; 
  sUbxMessage.u16Length = UBXMSG_LENGTH_FXN; 
  sUbxMessage.pu8Payload = &au8FixNowConfig[0]; 

  /* Send the message */
  QueueUbxTxMessage(&sUbxMessage, TRUE);
  MsgSenderForceSend();

  /* Build the Power Management configuration message to send */
  sUbxMessage.u8Class = UBXMSG_CLASS_CFG; 
  sUbxMessage.u8MsgID = UBXMSG_ID_PM; 
  sUbxMessage.u16Length = UBXMSG_LENGTH_PM; 
  sUbxMessage.pu8Payload = &au8PowerMgtConfig[0]; 

  /* Send the message */
  QueueUbxTxMessage(&sUbxMessage, TRUE);
  MsgSenderForceSend();

  /* Query the Power Management configuration */
  sUbxMessage.u8Class = UBXMSG_CLASS_CFG; 
  sUbxMessage.u8MsgID = UBXMSG_ID_PM; 
  sUbxMessage.u16Length = 0; 
  sUbxMessage.pu8Payload = NULL; 

  /* Send the message */
  QueueUbxTxMessage(&sUbxMessage, TRUE);
  MsgSenderForceSend();

  /* Query the FixNOW config */
  sUbxMessage.u8Class = UBXMSG_CLASS_CFG; 
  sUbxMessage.u8MsgID = UBXMSG_ID_FXN; 
  sUbxMessage.u16Length = 0; 
  sUbxMessage.pu8Payload = NULL; 

  /* Send the message */
  QueueUbxTxMessage(&sUbxMessage, TRUE);
  MsgSenderForceSend();
  
#if 0
  /* Build the message configuration message to send */
  sUbxMessage.u8Class = UBXMSG_CLASS_CFG; 
  sUbxMessage.u8MsgID = UBXMSG_ID_MSG; 
  sUbxMessage.u16Length = 2; 
  sUbxMessage.pu8Payload = &au8CFG_MSG_POLL_PAYLOAD[0]; 

  /* Send the message */
  QueueUbxTxMessage(&sUbxMessage, TRUE);
  MsgSenderForceSend();
#endif 

  /* Only our init messages are in the response list, so wait until the pointer returns
  to NULL which indicates either a response was received or a message timed out */
  u32Timeout = GGu32SysTime1ms;
  while(LGsResponseList != NULL)
  {
    GGGPSStateMachine();
    while( !IsTimeUp(&GGu32SysTime1ms, &u32Timeout, 1, RESET_TARGET_TIMER) );
  }
   
  /* If there are no error messages, then the messages were successful */
  if(LGsFailedMsgList == NULL)
  {
    /* Four messages will have been queued to report software and hardware version */
    MsgSenderForceSend();
    MsgSenderForceSend();
    MsgSenderForceSend();
    MsgSenderForceSend();
  }
  /* Otherwise report that the message failed */
  else
  {
    QueueTxMessage(DEBUG_MSG_SRC, sizeof(au8GPSFailedMsg), &au8GPSFailedMsg[0]);
    MsgSenderForceSend();
  }
           
  /* Configure messages */
  /* Set and confirm power mode */

  /* Init complete: print a blank line to separate GPS info and set Idle state */
  DebugLineFeed();
  MsgSenderForceSend();
  GGGPSStateMachine = GPSSMIdle;

} /* end GpsInitialize() */


/*-----------------------------------------------------------------------------/
Function: GpsClearAllData

Description:
Clears all data fields for GPS data.  This should occur at the start of GPS
and whenever a fix is not currently available.

Requires:
 
Promises:

*/
void GpsClearAllData(void)
{
  LGpastGPSCurrentSVData = &LGastGPSSVData1[0];
  LGpastGPSNewSVData = &LGastGPSSVData2[0];
  GPSClearSVData(&LGpastGPSCurrentSVData[0]);
  GPSClearSVData(&LGpastGPSNewSVData[0]);
  
  GGstGPSCurrentData.strLatitude[0]       = '\0'; 
  GGstGPSCurrentData.strLongitude[0]      = '\0'; 
  GGstGPSCurrentData.strMSLAltitude[0]    = '\0'; 
  GGstGPSCurrentData.strFixStatus[0]      = '\0'; 
  GGstGPSCurrentData.strNumSVsTracking[0] = '\0'; 
  GGstGPSCurrentData.strNumSVsUsed[0]     = '\0'; 
  GGstGPSCurrentData.strSpeed[0]          = '\0'; 
  GGstGPSCurrentData.strHeading[0]        = '\0'; 
  
} /* end GpsClearAllData() */


/*-----------------------------------------------------------------------------/
Function: GPSClearSVData

Description:
Parse

Requires:
  - pstGPSSVDataArray_ points to an array of GPSSVDataType with size GPS_MAX_SV_DATA
 
Promises:
  - All *pstGPSSVDataArray_.u8SVID = 0 and all *pstGPSSVDataArray_.u8CNo = 0
*/
void GPSClearSVData(GPSSVDataType* pastGPSSVDataArray_)
{
  for(u8 i = 0; i < GPS_MAX_SV_DATA; i++)
  {
    pastGPSSVDataArray_->u8SVID[0] = '\0';
    pastGPSSVDataArray_->u8CNo  = 0;
    pastGPSSVDataArray_++;
  }
  
} /* end GPSClearSVData */


/*-----------------------------------------------------------------------------/
Function: QueueUbxTxMessage

Description:
Creates a ublox GPS transmit message.  The data is passed to Message Sender
and the message is optionally queued for tracking. 

Requires:
  - Enough space is available on the heap for the complete message
  - psMessage_ points to a valid message structure with complete and correct data
  - bResponseExpected_ is TRUE if a response is expected

Promises:
 - A ublox formatted message is constructed and passed to Message Sender
 - If bResponseExpected_ is TRUE, the message ID and timestamp is queued to the response queue
*/
bool QueueUbxTxMessage(UbxMessageType* psMessage_, bool bResponseExpected_)
{
  u8 u8CK_A = 0, u8CK_B = 0;
  u8 *pu8Message;
  UbxResponseRecordType sResponseRecord;
  
  /* Allocate memory for new message and check for fail */
  pu8Message = malloc( (psMessage_->u16Length) + UBX_MSG_OVERHEAD_SIZE );
  if (pu8Message == NULL)
  {
    while(1); /* #PCUR# Update to do something useful */
  }
  
  /* Fill in the header information */
  *(pu8Message + UBX_SYNC_CHAR1_INDEX) = UBX_SYNC_CHAR1;
  *(pu8Message + UBX_SYNC_CHAR2_INDEX) = UBX_SYNC_CHAR2;
  
  *(pu8Message + UBX_CLASS_INDEX)      = psMessage_->u8Class;
  sResponseRecord.u8Class              = psMessage_->u8Class;
  
  *(pu8Message + UBX_ID_INDEX)         = psMessage_->u8MsgID;
  sResponseRecord.u8MsgID              = psMessage_->u8MsgID;
  
  *(pu8Message + UBX_LENGTH_LO_INDEX)  = (u8)(psMessage_->u16Length & 0x00FF);
  *(pu8Message + UBX_LENGTH_HI_INDEX)  = (u8)( (psMessage_->u16Length >> 8) & 0x00FF );
    
  /* Add the data payload */
  for(u16 i = 0; i < psMessage_->u16Length; i++)
  {
   *(pu8Message + UBX_PAYLOAD_INDEX + i) = *(psMessage_->pu8Payload + i);
  }
  
  /* Calculate and add the checksum */
  CalculateFletcherChecksum(pu8Message + UBX_SYNC_BYTES_SIZE, (psMessage_->u16Length + UBX_MSG_HEADER_SIZE - UBX_SYNC_BYTES_SIZE), &u8CK_A, &u8CK_B);
  *(pu8Message + UBX_MSG_HEADER_SIZE + psMessage_->u16Length) = u8CK_A;
  *(pu8Message + UBX_MSG_HEADER_SIZE + psMessage_->u16Length + 1) = u8CK_B;

  
  /* If PASSTHROUGH is enabled, queue the message to DEBUG Message Sender first */
  if(GGu32GPSFlags & _GPS_FLAGS_PASSTHROUGH)
  {
    QueueTxMessage(DEBUG_MSG_SRC, (psMessage_->u16Length + UBX_MSG_OVERHEAD_SIZE), pu8Message);
  }
  
  /* Queue the message to Message Sender then release temporary memory */
  QueueTxMessage(GPS_UART_MSG_SRC, (psMessage_->u16Length + UBX_MSG_OVERHEAD_SIZE), pu8Message);
  free(pu8Message);
  
  /* Queue the message ID to the response queue if required */
  if(bResponseExpected_)
  {
    sResponseRecord.u32TimeStamp = GGu32SysTime1ms;
    QueueMsgResponse(&sResponseRecord);
  }
  
  return(TRUE);
  
} /* end QueueUbxTxMessage() */


/*-----------------------------------------------------------------------------/
Function: QueueMsgResponse

Description:
When the system expects a response to a message it sends, the message details are
captured here.  This function adds a node to the Message Response queue.

Requires:
  - psResponseRecord_ points to a complete Response Record that should be inserted to the list.
  - Only one instance of any given Message ID is queued

Promises:
 -  A message response record is added to the linked list. 
*/
void QueueMsgResponse(UbxResponseRecordType* psResponseRecord_)
{
  UbxResponseListNodeType* psResponseNode; 
  
  /* Create the node */
  psResponseNode = malloc( sizeof(UbxResponseListNodeType) );
  if(psResponseNode == NULL)
  {
    while(1);  /* #PCUR# Handle this */
  }
  
  /* Fill in the data for the node */
  psResponseNode->sRecord.u8Class = psResponseRecord_->u8Class;
  psResponseNode->sRecord.u8MsgID = psResponseRecord_->u8MsgID;
  psResponseNode->sRecord.u32TimeStamp = psResponseRecord_->u32TimeStamp;
  psResponseNode->pNextRecord = NULL;
  
  /* Link the new node into LGsResponseList */
  if (LGsResponseList == NULL)
  {
    LGsResponseList = psResponseNode;
  }
  else
  {
    LGsResponseList->pNextRecord = psResponseNode;
  }
  
} /* end QueueMsgResponse() */


/*-----------------------------------------------------------------------------/
Function: DequeueMsgResponse

Description:
Removes a message from the Response queue.  Does nothing if the message number
is not present.  If more than one of the message type is in the list, all instances
are removed.

Requires:
  - u8Class_ is the message's class
  - u8MsgID_ is the message's ID

Promises:
 -  A message response record is added to the linked list. 
*/
void DequeueMsgResponse(u8 u8Class_, u8 u8MsgID_)
{
  UbxResponseListNodeType *psRecordParser, *psDoomedNode;
  
  psRecordParser = LGsResponseList;
  
  /* Parse through the whole list killing any records that match Class and ID */
  while(psRecordParser != NULL)
  {
    psDoomedNode = psRecordParser;
    psRecordParser = psRecordParser->pNextRecord;

    /* Look for matching data at potentially doomed node */
    if( (psDoomedNode->sRecord.u8Class == u8Class_) && (psDoomedNode->sRecord.u8MsgID == u8MsgID_) )
    {
      /* Special case: Node is at start of list */
      if(psDoomedNode == LGsResponseList)
      {
        LGsResponseList = psRecordParser;
      }

      /* Release the record's memory allocation */
      free(psDoomedNode);
    }
  }
  
} /* end DequeueMsgResponse() */


/*-----------------------------------------------------------------------------/
Function: CheckMessageTimeOut

Description:
Parses the current message queue looking for messages that have timed out.  If
found, the messages are removed and added to the failure queue.

If a message times out, the ID is added to the fail queue and tagged FAIL_TIMEOUT

Requires:
  - Enough space is available on the heap for the complete message

Promises:
  -
*/
void CheckMessageTimeOut(void)
{
  UbxFailRecordType sFailRecord;
  UbxResponseListNodeType* psRecordParser;
  
  /* Traverse the list looking for timeouts */
  psRecordParser = LGsResponseList;
  while(psRecordParser != NULL)
  {
    /* Check the entry for timeout */
    if( (GGu32SysTime1ms - psRecordParser->sRecord.u32TimeStamp) > GPS_RESPONSE_TIMEOUT )
    {
      /* Create a record of the timeout */
      sFailRecord.u8Class = psRecordParser->sRecord.u8Class;
      sFailRecord.u8MsgID = psRecordParser->sRecord.u8MsgID;
      sFailRecord.eFailReason = FAIL_TIMEOUT;
      QueueMsgFail(&sFailRecord);
      
      /* Delete the record from the Response list */
      DequeueMsgResponse(psRecordParser->sRecord.u8Class, psRecordParser->sRecord.u8MsgID);
      
      /* Reset the record parser pointer since the record it was pointing to has just been deleted */
      psRecordParser = LGsResponseList;
    }
    
    /* Otherwise just advance the parser */
    else
    {
      psRecordParser = psRecordParser->pNextRecord;
    }
  }
 
} /* end CheckMessageTimeOut() */
                        
                        
/*-----------------------------------------------------------------------------/
Function: QueueMsgFail

Description:
Adds message information for a message that has failed (either NAKed or timed out).

Requires:
  - psResponseRecord_ points to a complete Response Record that should be inserted to the list.
  - Only one instance of any given Message ID is queued

Promises:
 -  A message response record is added to the linked list. 
*/
void QueueMsgFail(UbxFailRecordType* psFailRecord_)
{
  UbxFailMsgListNodeType* psFailNode; 
  
  /* Create the node */
  psFailNode = malloc( sizeof(UbxFailMsgListNodeType) );
  if(psFailNode == NULL)
  {
    while(1);  /* #PCUR# Handle this */
  }
  
  /* Fill in the data for the node */
  psFailNode->sRecord.u8Class = psFailRecord_->u8Class;
  psFailNode->sRecord.u8MsgID = psFailRecord_->u8MsgID;
  psFailNode->sRecord.eFailReason = psFailRecord_->eFailReason;
  psFailNode->pNextRecord = NULL;
  
  /* Link the new node into LGsResponseList */
  if (LGsFailedMsgList == NULL)
  {
    LGsFailedMsgList = psFailNode;
  }
  else
  {
    LGsFailedMsgList->pNextRecord = psFailNode;
  }
  
} /* end QueueMsgFail() */


/*-----------------------------------------------------------------------------/
Function: GPSVerifyMessageNMEA

Description:
Checks a NMEA message for proper formatting and checksum.  
Based on NMEA 0183 Version 2.3, all NMEA messages have the form:

$<Address>{,<value>}*<checksum><CR><LF>

where checksum is an ASCII value of the hex number obtained by XORing all characters
between $ and * (not including the $ and *).

The function does not guarantee that the message address or any of the data is correct,
though checksum verification provides a reasonably strong indicator.

Requires:
  - pu8Message_ points to the start of the message to verify
  - the message at *pu8Message_ is terminated by <CR><LF>
  - the message checksum is in uppercase characters

Promises:
  - Returns TRUE if the message format and checksum are correct; otherwise returns FALSE
  - Queues the message for debug output if GPS pass-through is enabled.
*/
bool GPSVerifyMessageNMEA(u8* pu8Message_)
{
  bool bReturnValue = FALSE;  /* Assume the message is not good */
  u8 u8CalculatedChecksum = 0;
  u8 u8ExpectedChecksum;    
  u16 u16CharacterCount = 1;
  u8* pu8MessageStart = pu8Message_;
  
  /* Verify the correct starting character */
  if(*pu8Message_++ == NMEA_MSG_START_CHAR)
  {
    /* Compute checksum up to the checksum character or stop if <CR> is found */
    while( (*pu8Message_ != NMEA_MSG_CHECKSUM_CHAR) &&
           (*pu8Message_ != ASCII_CARRIAGE_RETURN) )
    {
      u8CalculatedChecksum ^= *pu8Message_++;
      u16CharacterCount++;
    }
    
    /* Verify the checksums match */
    if(*pu8Message_++ == NMEA_MSG_CHECKSUM_CHAR)
    {
      u8ExpectedChecksum = ASCIIHexCharToChar(*pu8Message_++) * 16;
      u8ExpectedChecksum += ASCIIHexCharToChar(*pu8Message_++);
      if(u8ExpectedChecksum == u8CalculatedChecksum)
      {
        bReturnValue = TRUE;
        if(GGu32GPSFlags & _GPS_FLAGS_PASSTHROUGH)
        {
          /* Adjust character count for checksum and termination string the queue the message */
          u16CharacterCount += 5;
          QueueTxMessage(DEBUG_MSG_SRC, u16CharacterCount, pu8MessageStart);
        }
      }
    }
  }
  
  return(bReturnValue);
  
} /* end GPSVerifyMessageNMEA() */


/*-----------------------------------------------------------------------------/
Function: GPSProcessMessageNMEA

Description:
Verifies and parses the NMEA message at LGpu8GPSRxBufferCurrentChar.
Queues to Debug port if GPS pass-through is enabled.

Requires:
  - LGpu8GPSRxBufferCurrentChar points to the start of a new character string that terminates
    in <cr><lf>
  - GPS buffer is large enough such that processing the message will
    always be finished long before any new data could come in and overwrite it.
 
Promises:
  - If the message is GPGGA, GGstGPSCurrentData is populated with Lat, Long, Sats used, Altitude strings
  - If the message is GPGSA, GGstGPSCurrentData is populated with Fix Status
  - If the message is GPVTG, GGstGPSCurrentData is populated with
  - If the message is GPGSV, LGastGPSSVDatax is updated
  - On any other message or unverified messages are ignored
*/

void GPSProcessMessageNMEA(void)
{
  u8 u8MessageNumber, u8Index;
  u8 strMessageAddress[NMEA_MSG_ADDRESS_SIZE + 1];
  u8 strFieldData[MAX_FIELD_LENGTH];
  u8 *pu8SavedRxBufferParser; 
  const u8 pcu8FixNA[] = "NA";
  const u8 pcu8Fix2D[] = "2D";
  const u8 pcu8Fix3D[] = "3D";
  const u8 pcu8SpeedUnits[] = "km/h";
  const u8 pcu8Degrees[] = " degrees";

  u8 u8SVMsgNumber, u8DataIndex;
  GPSSVDataType* pastTempSVData;
  
  u32 u32Whole, u32Fraction;
  u64 u64Fraction;
  
  u8 u8CharCount;
  s32 s32Multiplier, s32Altitude;
  u32 u32Multiplier, u32Speed, u32Heading;


  /* Array of message address strings.  The order of these definitions must match the
  order of the GPSMessageType enum type in gps_ublox.h. */
  static const char astrGPSMessage[GPS_MESSAGES][NMEA_MSG_ADDRESS_SIZE + 1] =
  { 
    {"GPGGA"}, {"GPGSA"}, {"GPVTG"}, {"GPGSV"}
  };
  
  /* Keep a copy of LGpu8GPSRxBufferCurrentChar so it can be used then restored during this function */
  pu8SavedRxBufferParser = LGpu8GPSRxBufferCurrentChar;
  
  /* Parse the message if it verifies */
  if( GPSVerifyMessageNMEA(LGpu8GPSRxBufferCurrentChar) )
  {
    LGu32GPSMessageCounter++;

    /* Copy the message address string to strMessageAddress */
    for(u8Index = 0; u8Index < NMEA_MSG_ADDRESS_SIZE; u8Index++)
    {
      GPSAdvanceRxCurrentCharPointer();

      /* Copy the character if it's not the end of the address field */
      if(*LGpu8GPSRxBufferCurrentChar != ',')
      {
        strMessageAddress[u8Index] = *LGpu8GPSRxBufferCurrentChar;
      }
    }
    /* Add the NULL terminator to make strMessageAddress a valid string */
    strMessageAddress[u8Index] = '\0';
 
    /* Run through the array of known messages and see if the current message matches */
    for(u8MessageNumber = 0; u8MessageNumber < GPS_MESSAGES; u8MessageNumber++)
    {
      if( !strcmp( (const char *)&strMessageAddress[0], &astrGPSMessage[u8MessageNumber][0]) )
        break;
    }
   
    /* u8MessageNumber now holds the correct index to the message to process */
    LGpu8GPSRxBufferCurrentChar = pu8SavedRxBufferParser;

    switch ( (GPSMessageType)u8MessageNumber )
    {
      case GPGGA:
        /* Extract Latitude including direction into a single string */
        GPSExtractFieldFromNMEA(GGA_LATITUDE, &strFieldData[0]);
        if(strFieldData[0] != '\0')
        {
          /* Flag that new direction data is being loaded */
          GGu32GPSFlags |= _GPS_NEW_DATA_LOCATION;

          /* Convert latitude whole. Multiply by 2^31 /180 */
          u32Whole = ( ((strFieldData[0] & 0x0f) * 10L) + (strFieldData[1] & 0x0f) ) * 11930465L;
  
          /* 2^31/(180*60*10) ie. mult by 1000 and then divide by 10, divide by 100 net result */
          u32Fraction = ( ((strFieldData[2] & 0x0f) * 10000L) + ((strFieldData[3] & 0x0f) * 1000L) +  
                          ((strFieldData[5] & 0x0f) * 100L) + ((strFieldData[6] & 0x0f) * 10L) + (strFieldData[7] & 0x0f) ) * 19884L;
  
          GGstGPSCurrentData.s32Latitude = (s32)(u32Whole + u32Fraction/100L);
          u64Fraction = (u64)u32Fraction;
          u64Fraction = (u64Fraction * 10000) / 11930465;
          u32Fraction = (u32)u64Fraction;
         
          /* Convert the completed latitude value into an ASCII string.  */
          strcpy( (char *)&GGstGPSCurrentData.strLatitude[0], (const char*)&strFieldData[0] );
          GGstGPSCurrentData.strLatitude[2] = '.';
          GGstGPSCurrentData.strLatitude[3] = (u32Fraction / 100000) + NUMBER_ASCII_TO_DEC;
          u32Fraction %= 100000;
          GGstGPSCurrentData.strLatitude[4] = (u32Fraction /  10000) + NUMBER_ASCII_TO_DEC;
          u32Fraction %= 10000;
          GGstGPSCurrentData.strLatitude[5] = (u32Fraction /   1000) + NUMBER_ASCII_TO_DEC;
          u32Fraction %= 1000;
          GGstGPSCurrentData.strLatitude[6] = (u32Fraction /    100) + NUMBER_ASCII_TO_DEC;
          u32Fraction %= 100;
          GGstGPSCurrentData.strLatitude[7] = (u32Fraction /     10) + NUMBER_ASCII_TO_DEC;
          u32Fraction %= 10;
          GGstGPSCurrentData.strLatitude[8] = u32Fraction + NUMBER_ASCII_TO_DEC;

          GGstGPSCurrentData.strLatitude[9] = 'N';
          GGstGPSCurrentData.strLatitude[10] = '\0';
          
          
          /* Adjust for hemisphere */
          GPSExtractFieldFromNMEA(GGA_LAT_DIRECTION, &strFieldData[0]);
          if(strFieldData[0] == 'S')
          {
            GGstGPSCurrentData.s32Latitude = -GGstGPSCurrentData.s32Latitude;
            GGstGPSCurrentData.strLatitude[9] = 'S';
          }
        }
        
        /* Extract Longitude including direction into a single string */
        GPSExtractFieldFromNMEA(GGA_LONGITUDE, &strFieldData[0]);
        if(strFieldData[0] != '\0')
        {
          /* Convert longitude whole. Multiply by 2^31 /180 */
          u32Whole = ( ((strFieldData[0] & 0x0f) * 100L) + ((strFieldData[1] & 0x0f) * 10L) + (strFieldData[2] & 0x0f) ) * 11930465L;
  
          /* 2^31/(180*60*10) */
          u32Fraction = ( ((strFieldData[3] & 0x0f) * 10000L) + ((strFieldData[4] & 0x0f) * 1000L) +  
                          ((strFieldData[6] & 0x0f) * 100L) + ((strFieldData[7] & 0x0f) * 10L) + (strFieldData[8] & 0x0f) ) * 19884L;
  
          GGstGPSCurrentData.s32Longitude = (s32)(u32Whole + u32Fraction/100L);
          u64Fraction = (u64)u32Fraction;
          u64Fraction = (u64Fraction * 10000) / 11930465;
          u32Fraction = (u32)u64Fraction;
  
          /* Convert the completed longitude value into an ASCII string.  */
          strcpy( (char *)&GGstGPSCurrentData.strLongitude[0], (const char*)&strFieldData[0] );
          GGstGPSCurrentData.strLongitude[3] = '.';
          GGstGPSCurrentData.strLongitude[4] = (u32Fraction / 100000) + NUMBER_ASCII_TO_DEC;
          u32Fraction %= 100000;
          GGstGPSCurrentData.strLongitude[5] = (u32Fraction /  10000) + NUMBER_ASCII_TO_DEC;
          u32Fraction %= 10000;
          GGstGPSCurrentData.strLongitude[6] = (u32Fraction /   1000) + NUMBER_ASCII_TO_DEC;
          u32Fraction %= 1000;
          GGstGPSCurrentData.strLongitude[7] = (u32Fraction /    100) + NUMBER_ASCII_TO_DEC;
          u32Fraction %= 100;
          GGstGPSCurrentData.strLongitude[8] = (u32Fraction /     10) + NUMBER_ASCII_TO_DEC;
          u32Fraction %= 10;
          GGstGPSCurrentData.strLongitude[9] = u32Fraction + NUMBER_ASCII_TO_DEC;

          GGstGPSCurrentData.strLongitude[10] = 'E';
          GGstGPSCurrentData.strLongitude[11] = '\0';

          /* Add correct polarity */
          GPSExtractFieldFromNMEA(GGA_LONG_DIRECTION, &strFieldData[0]);
          if (strFieldData[0] == 'W')
          {
            GGstGPSCurrentData.s32Longitude = -GGstGPSCurrentData.s32Longitude;
            GGstGPSCurrentData.strLongitude[10] = 'W';
          }
        }                   
        
        /* Extract Satellites used in fix */
        GPSExtractFieldFromNMEA(GGA_SATS_USED, &strFieldData[0]);
        if(strFieldData[0] != '\0')
        {
          strcpy( (char *)&GGstGPSCurrentData.strNumSVsUsed[0], (const char*)&strFieldData[0] );
        }

        /* Extract altitude (always MSL (mean sea level) in meters)*/
        GPSExtractFieldFromNMEA(GGA_ALTITUDE, &strFieldData[0]);
        if(strFieldData[0] != '\0')
        {
          /* Search through string counting characters until the decimal point is found */
          pu8SavedRxBufferParser = &strFieldData[0];
          u8CharCount = 0;
          while(*pu8SavedRxBufferParser != '.')
          {
            u8CharCount++;
            pu8SavedRxBufferParser++;
          }
          
          /* Calculate the numeric altitude */
          s32Multiplier = 1;
          s32Altitude = 0;
          for(u8 i = u8CharCount; i > 0; i--)
          {
            s32Altitude += (strFieldData[i - 1] & 0x0f) * s32Multiplier;
            s32Multiplier *= 10;
          }
          GGstGPSCurrentData.s32Altitude = s32Altitude;
          
          /* Move and format the string into GPSCurrentData */
          strcpy( (char *)&GGstGPSCurrentData.strMSLAltitude[0], (const char*)&strFieldData[0] );
          strFieldData[0] = 'm';
          strFieldData[1] = '\0';
          strcat( (char *)&GGstGPSCurrentData.strMSLAltitude[0], (const char*)&strFieldData[0] );
        }

        LGu32GPGGACount++;
        break;

      case GPGSA:
        /* Extract the fix indicator and write the appropriate string value */
        GGu32GPSFlags &= ~(_GPS_FLAGS_2D_FIX | _GPS_FLAGS_3D_FIX);
        GPSExtractFieldFromNMEA(GSA_FIX_INDICATOR, &strFieldData[0]);
        if(strFieldData[0] != '\0')
        {
          /* Flag that fix data is being loaded */
          GGu32GPSFlags |= _GPS_NEW_DATA_FIX;

          switch (strFieldData[0])
          {
            case '2':
              strcpy( (char *)&GGstGPSCurrentData.strFixStatus[0], (const char*)&pcu8Fix2D[0] );
              GGu32GPSFlags |= _GPS_FLAGS_2D_FIX;
              GGu32GPSFlags &= ~_GPS_FLAGS_3D_FIX;
              break;
              
            case '3':
              strcpy( (char *)&GGstGPSCurrentData.strFixStatus[0], (const char*)&pcu8Fix3D[0] );
              GGu32GPSFlags &= ~_GPS_FLAGS_2D_FIX;
              GGu32GPSFlags |= _GPS_FLAGS_3D_FIX;
              break;
              
            default:
              strcpy( (char *)&GGstGPSCurrentData.strFixStatus[0], (const char*)&pcu8FixNA[0] );
              GGu32GPSFlags &= ~(_GPS_FLAGS_2D_FIX | _GPS_FLAGS_3D_FIX);
              //GpsClearAllData();
              break;
          }
        }
        LGu32GPGSACount++;
        break;

      case GPVTG:
        /* Extract speed and add units */
        GPSExtractFieldFromNMEA(VTG_SPEED_KPH, &strFieldData[0]);
        if(strFieldData[0] != '\0')
        {
          /* Flag that heading data is being loaded */
          GGu32GPSFlags |= _GPS_NEW_DATA_SPEED;
  
          /* Search through string counting characters until the decimal point is found */
          pu8SavedRxBufferParser = &strFieldData[0];
          u8CharCount = 0;
          while(*pu8SavedRxBufferParser != '.')
          {
            u8CharCount++;
            pu8SavedRxBufferParser++;
          }
          
          /* Calculate the numeric speed * 1000 */
          u32Multiplier = 1000;

          /* First the decimal portion that is always 3 digits of precision */
          u32Speed = (strFieldData[u8CharCount + 1] & 0x0f) * 100 + 
                     (strFieldData[u8CharCount + 2] & 0x0f) * 10 +
                     (strFieldData[u8CharCount + 3] & 0x0f);

          for(u8 i = u8CharCount; i > 0; i--)
          {
            u32Speed += (strFieldData[i - 1] & 0x0f) * u32Multiplier;
            u32Multiplier *= 10;
          }
          
          /* Move and format the data into GPSCurrentData */
          GGstGPSCurrentData.u32Speed = u32Speed;
          strcpy( (char *)&GGstGPSCurrentData.strSpeed[0], (const char*)&strFieldData[0] );
          strcat( (char *)&GGstGPSCurrentData.strSpeed[0], (const char*)&pcu8SpeedUnits[0] );
        }

        /* Extract course and add units */
        GPSExtractFieldFromNMEA(VTG_COURSE, &strFieldData[0]);
        if(strFieldData[0] != '\0')
        {
          /* Search through string counting characters until the decimal point is found */
          pu8SavedRxBufferParser = &strFieldData[0];
          u8CharCount = 0;
          while(*pu8SavedRxBufferParser != '.')
          {
            u8CharCount++;
            pu8SavedRxBufferParser++;
          }
          
          /* Calculate the numeric heading * 100 */
          u32Multiplier = 100;

          /* First the decimal portion that is always 2 digits of precision */
          u32Heading = (strFieldData[u8CharCount + 1] & 0x0f) * 10 + 
                     (strFieldData[u8CharCount + 2] & 0x0f);

          for(u8 i = u8CharCount; i > 0; i--)
          {
            u32Heading += (strFieldData[i - 1] & 0x0f) * u32Multiplier;
            u32Multiplier *= 10;
          }
          
          /* Move and format the data into GPSCurrentData */
          GGstGPSCurrentData.u32Heading = u32Heading;
          strcpy( (char *)&GGstGPSCurrentData.strHeading[0], (const char*)&strFieldData[0] );
          strcat( (char *)&GGstGPSCurrentData.strHeading[0], (const char*)&pcu8Degrees[0] );
        }
                
        LGu32GPVTGCount++;
        break;

      case GPGSV:
        /* Extract number of satellites tracking */
        GPSExtractFieldFromNMEA(GSV_SATS_IN_VIEW, &strFieldData[0]);
        if(strFieldData[0] != '\0')
        {
          /* Flag that SV data is being loaded */
          GGu32GPSFlags |= _GPS_NEW_DATA_SV;
          
          /* Save the number of satellites being tracked */
          strcpy( (char *)&GGstGPSCurrentData.strNumSVsTracking[0], (const char*)&strFieldData[0] );
          
          /* Read all the SV data */
          GPSExtractFieldFromNMEA(GSV_MESSAGE_NUMBER, &strFieldData[0]);
          if(strFieldData[0] != '\0')
          {
            u8SVMsgNumber = atoi( (const char*)&strFieldData[0] );
            
            /* If this is message 1, swap the current data pointer and clear the SV data */
            if(u8SVMsgNumber == 1)
            {
              pastTempSVData = LGpastGPSCurrentSVData;
              LGpastGPSCurrentSVData = LGpastGPSNewSVData;
              LGpastGPSNewSVData = pastTempSVData;
              GPSClearSVData(&LGpastGPSNewSVData[0]);
            }
            
            /* Populate the current data */
            u8DataIndex = (u8SVMsgNumber - 1) * 4;
            
            /* Can always loop 4 times as GPSExtractFieldFromNMEA will return NULL if data not present */
            for(u8 i = 0; i < 4; i++)
            {
              GPSExtractFieldFromNMEA( (GSV_SAT_PRN + (4 * i) ), &strFieldData[0] );
              if(strFieldData[0] != '\0')
              {
                strcpy( (char *)LGpastGPSNewSVData[u8DataIndex].u8SVID, (const char*)&strFieldData[0] );
              
                GPSExtractFieldFromNMEA( ( GSV_SAT_CNO + (4 * i) ), &strFieldData[0]);
                if(strFieldData[0] != '\0')
                {
                  LGpastGPSNewSVData[u8DataIndex].u8CNo = atoi( (const char*)&strFieldData[0] );
                }
              }
              u8DataIndex++;
            }
          }
        }
        LGu32GPGSVCount++;
        break;

      default:
        LGu32OtherCount++;
        break;
    } /* end switch u8MessageNumber */
  } /* end if( GPSVerifyMessage(LGpu8GPSRxBufferCurrentChar) ) */
     
} /* end GPSProcessMessageNMEA() */


/*-----------------------------------------------------------------------------/
Function: GPSProcessMessageUBX

Description:
Parses a UBX message.

If a message is ACKed, it is removed from the watch queue
If a message is NAKed, the message ID is added to the fail queue and tagged FAIL_NAK

Requires:
  - UbxMessage_ points to a verified UbxMessageType to be processed
  - GPS buffer is large enough such that processing the message will
    always be finished long before any new data could come in and overwrite it.
 
Promises:
  - Message is parsed out according to its Class and ID.
*/
void GPSProcessMessageUBX(UbxMessageType* UbxMessage_)
{
  UbxFailRecordType sFailRecord;

  /* Parse Class */
  switch (UbxMessage_-> u8Class)
  {
    case UBXMSG_CLASS_NAV:
      /* Parse messages within class UBXMSG_CLASS_NAV*/
      switch(UbxMessage_->u8MsgID)
      {
        default:
          while(1); /* #PCUR# Trap for now */
          break;
      } /* end switch Message ID in UBXMSG_CLASS_NAV */
      
      break;

    case UBXMSG_CLASS_RXM:
      switch(UbxMessage_->u8MsgID)
      {
        default:
          while(1); /* #PCUR# Trap for now */
          break;
      } /* end switch Message ID in UBXMSG_CLASS_RXM */
      break;
      
    case UBXMSG_CLASS_INF:
      switch(UbxMessage_->u8MsgID)
      {
        default:
          while(1); /* #PCUR# Trap for now */
          break;
      } /* end switch Message ID in UBXMSG_CLASS_INF */
      break;
      
    case UBXMSG_CLASS_ACK:
      switch(UbxMessage_->u8MsgID)
      {
        case UBXMSG_ID_NAK:
          /* Create a record of the NAK */
          sFailRecord.u8Class = *(UbxMessage_->pu8Payload + UBXMSG_NAK_CLSID_INDEX);
          sFailRecord.u8MsgID = *(UbxMessage_->pu8Payload + UBXMSG_NAK_MSGID_INDEX);
          sFailRecord.eFailReason = FAIL_NAK;
          QueueMsgFail(&sFailRecord);
          
          /* Fall through to remove the message */
        
        case UBXMSG_ID_ACK:
          /* The most common case should be simplest: just remove the Acked message from the Response list */
          DequeueMsgResponse(*(UbxMessage_->pu8Payload + UBXMSG_ACK_CLSID_INDEX), 
                             *(UbxMessage_->pu8Payload + UBXMSG_ACK_MSGID_INDEX) );
          break;
        
        default:
          while(1); /* #PCUR# Trap for now */
          break;
      } /* end switch Message ID in UBXMSG_CLASS_ACK */
      break;
      
    case UBXMSG_CLASS_CFG:
      switch(UbxMessage_->u8MsgID)
      {
        default:
          //while(1); /* #PCUR# Trap for now */
          break;
      } /* end switch Message ID in UBXMSG_CLASS_CFG */
      break;
      
    case UBXMSG_CLASS_MON:
      switch(UbxMessage_->u8MsgID)
      {
        case UBXMSG_ID_VER:
          /* Display the SW, HW and ROM version information */
          QueueTxMessage(DEBUG_MSG_SRC, UBXMSG_VER_SW_LENGTH, UbxMessage_->pu8Payload + UBXMSG_VER_SW_INDEX);
          DebugLineFeed();
          QueueTxMessage(DEBUG_MSG_SRC, UBXMSG_VER_HW_LENGTH, UbxMessage_->pu8Payload + UBXMSG_VER_HW_INDEX);
          DebugLineFeed();
          //QueueTxMessage(DEBUG_MSG_SRC, UBXMSG_VER_ROM_LENGTH, UbxMessage_->pu8Payload + UBXMSG_VER_ROM_INDEX);

          /* This is a polled message so it may be in the response queue, so dequeue it */
          DequeueMsgResponse(UBXMSG_CLASS_MON, UBXMSG_ID_VER);
          break;

        default:
          while(1); /* #PCUR# Trap for now */
          break;
      } /* end switch Message ID in UBXMSG_CLASS_MON */
      break;
      
    case UBXMSG_CLASS_AID:
      switch(UbxMessage_->u8MsgID)
      {
        default:
          while(1); /* #PCUR# Trap for now */
          break;
      } /* end switch Message ID in UBXMSG_CLASS_AID */
      break;
      
    case UBXMSG_CLASS_TIM:
      switch(UbxMessage_->u8MsgID)
      {
        default:
          while(1); /* #PCUR# Trap for now */
          break;
      } /* end switch Message ID in UBXMSG_CLASS_TIM */
      break;
      
    case UBXMSG_CLASS_ESF:
      switch(UbxMessage_->u8MsgID)
      {
        default:
          while(1); /* #PCUR# Trap for now */
          break;
      } /* end switch Message ID in UBXMSG_CLASS_ESF */
      break;
      
    case UBXMSG_CLASS_NMEA:
      switch(UbxMessage_->u8MsgID)
      {
        default:
          while(1); /* #PCUR# Trap for now */
          break;
      } /* end switch Message ID in UBXMSG_CLASS_ESF */
      break;

    default:
      while(1); /* #PCUR# Trap for now */
      break;

  } /* end switch Class */
      
} /* end GPSProcessMessageUBX() */

                                            
/*-----------------------------------------------------------------------------/
Function: GPSExtractFieldFromNMEA

Description:
Parse through an NMEA message to the field of interest and extract the string data.

Requires:
  - LGpu8GPSRxBufferCurrentChar points to the start of a verified NMEA message (pointing to $)
  - u8FieldNumber_ is a valid field in the verified pu8Message_ (user of the
    function must pass appropriate field arguments for the message being processed)
  - pau8MessageData_ points to an array where the message data will be copied;
    space available at pau8MessageData_ is >= MAX_FIELD_LENGTH;
 
Promises:
  - pau8MessageData_ is populated with a null-terminated string of field data;
    otherwise, pau8MessageData_ is an empty string (just the '\0' character).
  - LGpu8GPSRxBufferCurrentChar is restored to its starting location
*/

void GPSExtractFieldFromNMEA(u8 u8FieldNumber_, u8* pau8MessageData_)
{
  u8 u8FieldLength = MAX_FIELD_LENGTH - 1;
  u8 *pu8SavedRxBufferParser = LGpu8GPSRxBufferCurrentChar;                    
  
	/* Parse through the buffer until the field of interest is found */
  while(u8FieldNumber_ != 0)
  {
    /* Check for '*' as well as ',' to handle NMEA messages with variable # of fields */
    while( (*LGpu8GPSRxBufferCurrentChar != ',') && (*LGpu8GPSRxBufferCurrentChar != '*') )
    {
      GPSAdvanceRxCurrentCharPointer();
    }
    
    if(*LGpu8GPSRxBufferCurrentChar == '*')
      break;

    GPSAdvanceRxCurrentCharPointer();
    u8FieldNumber_--;
  }

  /* If the field is empty, we will be pointing at either a comma or the '*' of a checksum
  in which case we should just return with pau8MessageData_ = NULL .*/
  if( (*LGpu8GPSRxBufferCurrentChar == ',') || (*LGpu8GPSRxBufferCurrentChar == '*') )
  {
     *pau8MessageData_ = '\0';
  }
  /* Otherwise LGpu8GPSRxBufferCurrentChar is pointing to the data in the field */
  else
  {
    /* Read and copy the field data; maximum (MAX_FIELD_LENGTH - 1) chars */
    while( (*LGpu8GPSRxBufferCurrentChar != ',') && 
           (*LGpu8GPSRxBufferCurrentChar != '*') && 
           (u8FieldLength-- != 0) )
    {
      *pau8MessageData_++ = *LGpu8GPSRxBufferCurrentChar;
      GPSAdvanceRxCurrentCharPointer();
    }
 
    /* Terminate buffer and flag if the value was truncated */
    *pau8MessageData_ = '\0';	
    if(u8FieldLength == 0)
    {
      GGu32GPSFlags |= _GPS_FLAGS_DATA_TRUNCATED;
    }
  }

  /* Always restore LGpu8GPSRxBufferCurrentChar */
  LGpu8GPSRxBufferCurrentChar = pu8SavedRxBufferParser;

} /* GPSExtractFieldFromNMEA */

        
/*-----------------------------------------------------------------------------/
Function: GPSShowData

Description:
Queues message showing current GPS data if a 3D fix is available with the form
GPS Data:
  Fix status: 3D
  SVs tracking, used: 14, 8
  Location: 47.15392359N, 158.1243255235W, 1012m
  Velocity: 50km/h, 77 degrees

If a fix is not available, the output is simply "No GPS Fix"
*/
void GPSShowData(void)
{
  u8 strGPSHeaderMsg[]    = "GPS Data:";
  u8 strGPSNoFixMsg[]     = "No GPS fix";
  u8 strGPSFixStatusMsg[] = "\n\r  Fix status: ";
  u8 strGPSSVsMsg[]       = "\n\r  SVs tracking, used: ";
  u8 strGPSLocationMsg[]  = "\n\r  Location: ";
  u8 strGPSVelocityMsg[]  = "\n\r  Velocity: ";
  u8 strMsgComma[]        = ", ";
  u8 strMsgBuffer[128];
  
  if( GGu32GPSFlags & (_GPS_FLAGS_2D_FIX | _GPS_FLAGS_3D_FIX) )
  {
    QueueTxMessage(DEBUG_MSG_SRC, strlen( (const char*)&strGPSHeaderMsg[0]), &strGPSHeaderMsg[0]);
    
    /* Prepare and queue fix status message */
    strMsgBuffer[0] = '\0';
    strcat( (char*)&strMsgBuffer[0], (const char*)&strGPSFixStatusMsg[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&GGstGPSCurrentData.strFixStatus[0]);
    QueueTxMessage(DEBUG_MSG_SRC, strlen( (const char*)&strMsgBuffer[0]), &strMsgBuffer[0]);

    /* Prepare and queue SVs tracking message */
    strMsgBuffer[0] = '\0';
    strcat( (char*)&strMsgBuffer[0], (const char*)&strGPSSVsMsg[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&GGstGPSCurrentData.strNumSVsTracking[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&strMsgComma[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&GGstGPSCurrentData.strNumSVsUsed[0]);
    QueueTxMessage(DEBUG_MSG_SRC, strlen( (const char*)&strMsgBuffer[0]), &strMsgBuffer[0]);

    /* Prepare and queue location message */
    strMsgBuffer[0] = '\0';
    strcat( (char*)&strMsgBuffer[0], (const char*)&strGPSLocationMsg[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&GGstGPSCurrentData.strLatitude[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&strMsgComma[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&GGstGPSCurrentData.strLongitude[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&strMsgComma[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&GGstGPSCurrentData.strMSLAltitude[0]);
    QueueTxMessage(DEBUG_MSG_SRC, strlen( (const char*)&strMsgBuffer[0]), &strMsgBuffer[0]);

    /* Prepare and queue velocity message */
    strMsgBuffer[0] = '\0';
    strcat( (char*)&strMsgBuffer[0], (const char*)&strGPSVelocityMsg[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&GGstGPSCurrentData.strSpeed[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&strMsgComma[0]);
    strcat( (char*)&strMsgBuffer[0], (const char*)&GGstGPSCurrentData.strHeading[0]);

    QueueTxMessage(DEBUG_MSG_SRC, strlen( (const char*)&strMsgBuffer[0]), &strMsgBuffer[0]);
  }
  else
  {
    QueueTxMessage(DEBUG_MSG_SRC, sizeof(strGPSNoFixMsg), &strGPSNoFixMsg[0]);
  }

  DebugLineFeed();
  DebugLineFeed();

} /* end GPSShowData() */


/*-----------------------------------------------------------------------------/
Function: GPSAdvanceRxBufferParserPointer
        
Description:
Increments the Rx buffer pointer and handles the wrap-around.

Requires:
  -

Promises:
  - LGpu8GPSRxBufferParser is pointing to the next char in the circular buffer
*/
void GPSAdvanceRxBufferParserPointer(void)
{
  LGpu8GPSRxBufferParser++;
  if(LGpu8GPSRxBufferParser >= &LGau8GPSRxBuffer[GPS_RX_BUFFER_SIZE])
  {
    LGpu8GPSRxBufferParser = &LGau8GPSRxBuffer[0];
  }

} /* end GPSAdvanceRxBufferParserPointer() */


/*-----------------------------------------------------------------------------/
Function: GPSAdvanceRxCurrentCharPointer(void)

Description:
Increments the Rx buffer pointer and handles the wrap-around.

Requires:
  -

Promises:
  - LGpu8GPSRxBufferCurrentChar is pointing to the next char in the circular buffer
*/
void GPSAdvanceRxCurrentCharPointer(void)
{
  LGpu8GPSRxBufferCurrentChar++;
  if(LGpu8GPSRxBufferCurrentChar >= &LGau8GPSRxBuffer[GPS_RX_BUFFER_SIZE])
  {
    LGpu8GPSRxBufferCurrentChar = &LGau8GPSRxBuffer[0];
  }

} /* end GPSAdvanceRxCurrentCharPointer() */


/*-----------------------------------------------------------------------------/
Function: GPSAdvanceRxNextCharPointer

Description:
Increments the GGpu8GpsRxBufferNextChar pointer and handles the wrap-around.

Requires:
  - Function should be called only in ISR or interrupts must be disabled.

Promises:
  - GGpu8GpsRxBufferNextChar is pointing to the next char in the circular buffer
  - *GGpu8GpsRxBufferNextChar is cleared since it is expired data
*/
void GPSAdvanceRxNextCharPointer(void)
{
  GGpu8GpsRxBufferNextChar++;
  if(GGpu8GpsRxBufferNextChar >= &LGau8GPSRxBuffer[GPS_RX_BUFFER_SIZE])
  {
    GGpu8GpsRxBufferNextChar = &LGau8GPSRxBuffer[0];
  }
  
  *GGpu8GpsRxBufferNextChar = '\0';
  
} /* end GPSAdvanceRxNextCharPointer() */


/*******************************************************************************
* State Machine Function Definitions
*******************************************************************************/
/* The GPS state machine monitors and parses incoming messages.  Both NMEA and
UBX binary messages are handled. The assumption is that the system will work
reliably enough if message decisions are made based on the initial character received
in a transmission ('$' [0x24] for NMEA and 0xb5 for binary)
*/

/*------------------------------------------------------------------------------
Waits for data to appear in the Rx buffer or handle a GPS power event.
*/
void GPSSMIdle(void)               
{
  /* Watch for characters arriving */
  if( LGpu8GPSRxBufferParser != GGpu8GpsRxBufferNextChar)
  {
    LGu32GPSTimeout = GGu32SysTime1ms;

    /* Check for NMEA start character */
    if(*LGpu8GPSRxBufferParser == NMEA_MSG_START_CHAR)
    {
      GGGPSStateMachine = GPSSMIncomingNMEA;
    }

    /* Check for UBX start character */
    else if(*LGpu8GPSRxBufferParser == UBX_SYNC_CHAR1)
    {
      /* Advance CurrentChar since the SYNC byte has served its purpose! */
      GPSAdvanceRxCurrentCharPointer();
      GGGPSStateMachine = GPSSMIncomingUBX;
    }
    /* Otherwise skip over the character */
    else
    {
      GPSAdvanceRxCurrentCharPointer();
    }
    
    /* In all cases, advance the buffer parser (non-valid characters will be ignored and abandoned) */
    GPSAdvanceRxBufferParserPointer();
  }

  /* Check for expired messages */
  CheckMessageTimeOut();
  
  /* Check for failed messages */
  if(LGsFailedMsgList != NULL)
  {
    //while(1); /* #PCUR# Trap */
  }
  
  /* Display the latest GPS data if the output is active */
  if( (GGu32GPSFlags & _GPS_FLAGS_DATA_OUTPUT) && 
       IsTimeUp(&GGu32SysTime1ms, &LGu32GPSDebugDataRate, GPS_DEBUG_DATA_RATE, RESET_TARGET_TIMER) )
  {
    GPSShowData();
  }

} /* end GPSSMIdle() */


/*------------------------------------------------------------------------------
Assume we have an NMEA message coming in.  Since no information about the message
is contained in the message, wait until the <CR><LF> sequence is found.
*/
void GPSSMIncomingNMEA(void)
{
  /* Do nothing unless there is new data and the current character is CR */
  while( (LGpu8GPSRxBufferParser != GGpu8GpsRxBufferNextChar) &&
         (*LGpu8GPSRxBufferParser != ASCII_CARRIAGE_RETURN) )
  {
    GPSAdvanceRxBufferParserPointer();
  }
  
  /* If the current character is a CR, then advance the state machine */
  if(*LGpu8GPSRxBufferParser == ASCII_CARRIAGE_RETURN)
  {
    GPSAdvanceRxBufferParserPointer();
    GGGPSStateMachine = GPSSMWaitForLF;
    LGu32GPSTimeout = GGu32SysTime1ms;
  }
  
  if(IsTimeUp(&GGu32SysTime1ms, &LGu32GPSTimeout, GPS_MESSAGE_TIMEOUT, RESET_TARGET_TIMER) )
  {
    LGu8ErrorCode = GPS_ERROR_INCOMING_TIMEOUT;
    GGGPSStateMachine = GPSSMError;
  }
  
} /* end GPSSMIncomingNMEA() */
     

/*------------------------------------------------------------------------------
At the start of this state, LGpu8GPSRxBufferParser is pointing to a CR so
any characters from LGpu8GPSRxBufferCurrentChar to LGpu8GPSRxBufferParser-1 
are part of the candidate command.  Allow a maximum of GPS_MESSAGE_TIMEOUT
cycles to wait for the next character which should be ASCII_LINE_FEED
*/
          
void GPSSMWaitForLF(void)        
{
  /* Look for one more character */
  if(LGpu8GPSRxBufferParser != GGpu8GpsRxBufferNextChar)
  {
    /* If the next char is ASCII_LINE_FEED, then process the message */
    if(*LGpu8GPSRxBufferParser == ASCII_LINEFEED)
    {
      GPSProcessMessageNMEA();
      
      /* Set the Current and Parser pointers to the character after the line feed */
      GPSAdvanceRxBufferParserPointer();
      LGpu8GPSRxBufferCurrentChar = LGpu8GPSRxBufferParser;

      /* Return to Idle state */
      GGGPSStateMachine = GPSSMIdle;
      GGu32GPSFlags &= ~_GPS_FLAGS_SM_MANUAL;
    }
    
    /* If the character is not ASCII_LINE_FEED, then consider all the preceding characters garbage */
    else
    {
      /* LGpu8GPSRxBufferParser is not advanced because the char that is not ASCII_LINE_FEED could be
      the start of the next non-garbage message. */
      
      /* Move the unread char pointer passed all the garbage */
      LGpu8GPSRxBufferCurrentChar = LGpu8GPSRxBufferParser;
      
      LGu8ErrorCode = GPS_ERROR_NMEA_FAIL_LF;
      GGGPSStateMachine = GPSSMError;
    }
  }

  /* Check for timeout */
  if(IsTimeUp(&GGu32SysTime1ms, &LGu32GPSTimeout, GPS_MESSAGE_TIMEOUT, RESET_TARGET_TIMER) )
  {
    /* Assume the LF will never come, so junk the message and start fresh */
    //GPSAdvanceRxBufferParserPointer();
    LGpu8GPSRxBufferCurrentChar = LGpu8GPSRxBufferParser;

    LGu8ErrorCode = GPS_ERROR_INCOMING_TIMEOUT;
    GGGPSStateMachine = GPSSMError;
  }

} /* end GPSSMWaitForLF() */

     
/*------------------------------------------------------------------------------
Assume we have a UBX binary message coming in.  Since the binary message contains
information about the message, use the state machine to verify it as it comes in.
On entry, BufferParser is pointing to SYNC1.
*/
void GPSSMIncomingUBX(void)
{
  /* Wait for the next char which should be UBX_SYNC_CHAR2 */
  if(LGpu8GPSRxBufferParser != GGpu8GpsRxBufferNextChar)
  {
    /* If it's UBX_SYNC_CHAR2, then proceed */
    if(*LGpu8GPSRxBufferParser == UBX_SYNC_CHAR2)
    {
      GGGPSStateMachine = GPSSMWaitUbxHeader;
      LGu32GPSTimeout = GGu32SysTime1ms;
    }
    /* Else flag the error */
    else
    {
      LGu8ErrorCode = GPS_ERROR_UBX_FAIL_SYNC2;
      GGGPSStateMachine = GPSSMError;
    }
    
    /* In either case, advance the pointers */
    GPSAdvanceRxCurrentCharPointer();
    GPSAdvanceRxBufferParserPointer();
  }  
  
  /* Watch for time out */
  if(IsTimeUp(&GGu32SysTime1ms, &LGu32GPSTimeout, GPS_MESSAGE_TIMEOUT, RESET_TARGET_TIMER) )
  {
    LGu8ErrorCode = GPS_ERROR_INCOMING_TIMEOUT;
    GGGPSStateMachine = GPSSMError;
  }
  
} /* end GPSSMIncomingUBX() */
     

/*------------------------------------------------------------------------------
Read the expected 4 bytes of the message header:
CLASS(1)  ID(1)  LENGTH(2)
*/
void GPSSMWaitUbxHeader(void)
{
  static u8 u8ByteCounter = 0;
    
  while( (LGpu8GPSRxBufferParser != GGpu8GpsRxBufferNextChar) && 
         (u8ByteCounter < (UBX_MSG_HEADER_SIZE - UBX_SYNC_BYTES_SIZE) ) )
  {
    u8ByteCounter++;
    
    switch(u8ByteCounter)
    {
      case 1:
        /* Save the byte as the message's class */
        LGsUbxMessage.u8Class = *LGpu8GPSRxBufferParser;
        break;
        
      case 2:
        /* Save the byte as the message's ID */
        LGsUbxMessage.u8MsgID = *LGpu8GPSRxBufferParser;
        break;
        
      case 3:
        /* Save the byte as the low byte of the message's length */
        LGsUbxMessage.u16Length = 0;
        LGsUbxMessage.u16Length |= (u16)(*LGpu8GPSRxBufferParser) & 0x00FF;
        break;
        
      case 4:
        /* Save the byte as the high byte of the message's length */
        LGsUbxMessage.u16Length |= ((u16)(*LGpu8GPSRxBufferParser) << 8) & 0xFF00;
        
        /* Make a copy of the length for later use */
        LGu16LengthCopy = LGsUbxMessage.u16Length;
        
        /* Create space to store a copy of the message as it comes in for processing purposes */
        LGsUbxMessage.pu8Payload = malloc(LGu16LengthCopy);
        if (LGsUbxMessage.pu8Payload == NULL)
        {
          while(1); /* #PCUR# Update to do something useful */
        }

        /* Advance states */
        GGGPSStateMachine = GPSSMUbxPayload;
        break;
        
      default:
        /* Flag an error - this can't, technically, occur, so don't worry about ByteCounter for now. */
        LGu8ErrorCode = GPS_ERROR_UBX_FAIL_HEADER;
        GGGPSStateMachine = GPSSMError;
        break;
    } /* end switch */
    
    GPSAdvanceRxBufferParserPointer();
    
  } /* end while */

  /* Reset the byte counter for next time if we're advancing states */
  if (u8ByteCounter == 4)
  { 
    u8ByteCounter = 0;
  }

  /* Watch for time out */
  if( IsTimeUp(&GGu32SysTime1ms, &LGu32GPSTimeout, GPS_MESSAGE_TIMEOUT, RESET_TARGET_TIMER) )
  {
    /* Reset the byte counter for next time */
    u8ByteCounter = 0;

    LGu8ErrorCode = GPS_ERROR_INCOMING_TIMEOUT;
    GGGPSStateMachine = GPSSMError;
  }
  
} /* end GPSSMWaitUbxHeader() */


/*------------------------------------------------------------------------------
Read in the full data payload based on the length information from the header.
*/
void GPSSMUbxPayload(void)        
{
  static u8 u8ByteCounter = 0;
  
  /* Capture any new characters that have arrived */
  while( (LGpu8GPSRxBufferParser != GGpu8GpsRxBufferNextChar) && 
         (LGu16LengthCopy > 0) )
  {
    LGu16LengthCopy--;
    *(LGsUbxMessage.pu8Payload + u8ByteCounter) = *LGpu8GPSRxBufferParser;
    GPSAdvanceRxBufferParserPointer();
    u8ByteCounter++;
  }
  
  /* If all the payload bytes have arrived, get the next two bytes (checksum) */
  if(LGu16LengthCopy == 0)
  {
    u8ByteCounter = 0;
    GGGPSStateMachine = GPSSMUbxWaitChecksum;
  }

  /* Abort if time out */
  if( IsTimeUp(&GGu32SysTime1ms, &LGu32GPSTimeout, GPS_MESSAGE_TIMEOUT, RESET_TARGET_TIMER) )
  {
    u8ByteCounter = 0;
    LGu8ErrorCode = GPS_ERROR_INCOMING_TIMEOUT;
    GGGPSStateMachine = GPSSMError;
  }
  
} /* end GPSSMUbxPayload() */


/*------------------------------------------------------------------------------
Wait for the two checksum bytes to arrive.  Once received, verify the checksum and
process the received message. GPSAdvanceRxCurrentCharPointer is still pointing at
the CLASS byte in the main message buffer.
*/
void GPSSMUbxWaitChecksum(void)        
{
  static u8 u8ByteCounter = 0;
  static u8 u8Received_CK_A, u8Received_CK_B;
  u8 u8Calculated_CK_A, u8Calculated_CK_B;
 
  /* Wait for an incoming characters */
  while( (LGpu8GPSRxBufferParser != GGpu8GpsRxBufferNextChar) && 
         (u8ByteCounter < 2) )
  {
    u8ByteCounter++;
    
    /* First checksum byte is "CK_A" */
    if(u8ByteCounter == 1)
    {
      /* Grab the char and advance the buffer parser to the next buffer character */
      u8Received_CK_A = *LGpu8GPSRxBufferParser;
      GPSAdvanceRxBufferParserPointer();
    }
 
    /* Second checksum byte is "CK_B" */
    if(u8ByteCounter == 2)
    {
      /* Capture the current byte as CK_B */
      u8Received_CK_B = *LGpu8GPSRxBufferParser;
      GPSAdvanceRxBufferParserPointer();

      /* The complete message is now received, so calculate and verify the checksum */
      CalculateFletcherChecksum(LGpu8GPSRxBufferCurrentChar, (UBX_MSG_HEADER_SIZE - UBX_SYNC_BYTES_SIZE + LGsUbxMessage.u16Length), 
                                &u8Calculated_CK_A, &u8Calculated_CK_B);

      /* Queue message to debug output if enabled (don't worry about SYNC bytes */
      if(GGu32GPSFlags & _GPS_FLAGS_PASSTHROUGH)
      {
        QueueTxMessage(DEBUG_MSG_SRC, LGsUbxMessage.u16Length + UBX_MSG_HEADER_SIZE, LGpu8GPSRxBufferCurrentChar);
      }

      /* Done with CurrentChar, so jump the pointer to the end of the message */
      LGpu8GPSRxBufferCurrentChar = LGpu8GPSRxBufferParser;
        
      /* If the checksums match, process the message */
      if((u8Received_CK_A == u8Calculated_CK_A) && (u8Received_CK_B == u8Calculated_CK_B))
      {
        LGu32GPSMessageCounter++;
        GPSProcessMessageUBX(&LGsUbxMessage);
        
        /* Free dynamic memory allocated for the UBX message */
        free(LGsUbxMessage.pu8Payload);
        LGsUbxMessage.pu8Payload = NULL;
        
        /* Message is complete, so return to Idle */
        GGu32GPSFlags &= ~_GPS_FLAGS_SM_MANUAL;
        GGGPSStateMachine = GPSSMIdle;
      }
      
      /* If the checksums don't match, then flag an error */
      else
      {
        /* Move the unread char pointer passed all the garbage */
        //LGpu8GPSRxBufferCurrentChar = LGpu8GPSRxBufferParser;

        LGu8ErrorCode = GPS_ERROR_UBX_FAIL_CHECKSUM;
        GGGPSStateMachine = GPSSMError;
      }

      /* Zero the byte counter for next run and bail out of the while loop */
      u8ByteCounter = 0;
      break;
    }
  } /* end while */

  /* Watch for time out */
  if( IsTimeUp(&GGu32SysTime1ms, &LGu32GPSTimeout, GPS_MESSAGE_TIMEOUT, RESET_TARGET_TIMER) )
  {
    /* Reset the byte counter for next time */
    u8ByteCounter = 0;

    LGu8ErrorCode = GPS_ERROR_INCOMING_TIMEOUT;
    GGGPSStateMachine = GPSSMError;
  }

} /* end GPSSMUbxPayload() */

     
/*------------------------------------------------------------------------------
An error of some sort has occured.  Queue a debug message and return to idle.
*/
void GPSSMError(void)
{
  static u8 au8ErrorMessage[] = "GPS Error ";
  static u32 u32ErrorCount = 0;
  
  /* Print a debug message about the error including the error code */
  QueueTxMessage(DEBUG_MSG_SRC, sizeof(au8ErrorMessage) - 1, &au8ErrorMessage[0]);
  PrintNumber(LGu8ErrorCode);
  DebugLineFeed();
  
  /* Free dynamic memory if it were allocated for the UBX message */
  if(LGsUbxMessage.pu8Payload != NULL)
  {
    free(LGsUbxMessage.pu8Payload);
    LGsUbxMessage.pu8Payload = NULL;
  }
  
  /* Reset SM and return to Idle */
  LGu8ErrorCode = 0;
  u32ErrorCount++;
  GGu32GPSFlags &= ~_GPS_FLAGS_SM_MANUAL;
  GGGPSStateMachine = GPSSMIdle;
  
} /* end GPSSMError() */
