/**********************************************************************************************************************
* File: ant_api.h      
*
* Description:
* ANT implementation for Cortex-M3 / AP2 SPI
**********************************************************************************************************************/

#ifndef __ANT_API_H
#define __ANT_API_H


/**********************************************************************************************************************
Constants
**********************************************************************************************************************/
#define ANT_OUTGOING_MESSAGE_BUFFER_SIZE    (u32)32
#define ANT_APPLICATION_MESSAGE_BUFFER_SIZE (u32)32
#define ANT_APPLICATION_MESSAGE_BYTES       (u8)8
#define ANT_DATA_BYTES                      (u8)8


/**********************************************************************************************************************
Type definitions
**********************************************************************************************************************/
typedef enum {ANT_UNCONFIGURED, ANT_CONFIGURED, ANT_OPENING, ANT_OPEN, ANT_CLOSING, ANT_CLOSED} AntChannelStatusType;
typedef enum {ANT_EMPTY, ANT_DATA, ANT_TICK} AntApplicationMessageType;

typedef struct
{
  u32 u32TimeStamp;                                  /* Current G_u32SystemTime1s */
  AntApplicationMessageType eMessageType;            /* Type of data */
  u8 au8MessageData[ANT_APPLICATION_MESSAGE_BYTES];  /* Array for message data */
  void *psNextMessage;                               /* Pointer to AntDataMessageStructType */
} AntApplicationMsgListType;


/**********************************************************************************************************************
Application messages
**********************************************************************************************************************/

/*----------------------------------------------------------------------------------------------------------------------
ANT_TICK communicates the message period to the application.  

If ANT is running as a master, ANT_TICK occurs every time a broadcast or acknowledged
data message is sent. 

If ANT is running as a slave, ANT_TICK occurs when ever a message is received from the
master or if ANT misses a message that it was expecting based on the established timing 
of a paired channel (EVENT_RX_FAIL event is generated).  This should be communicated 
in case a missed message is important to any application using ANT.  

MSG_NAME  MSG_ID     D_0      D_1      D_2     D_3     D_4     D_5     D_6
ANT_TICK   0xFF     EVENT    0xFF     0xFF    0xFF   MISSED  MISSED  MISSED
                    CODE                              MSG #   MSG #   MSG #
                                                      HIGH    MID     LOW
---------------------------------------------------------------------------------------------------------------------*/


#define   MESSAGE_ANT_TICK                        (u8)0xFF
#define   ANT_TICK_MSG_ID_INDEX                   (u8)0
#define   ANT_TICK_MSG_EVENT_CODE_INDEX           (u8)1
#define   ANT_TICK_MSG_SENTINEL1_INDEX            (u8)2
#define   ANT_TICK_MSG_SENTINEL2_INDEX            (u8)3
#define   ANT_TICK_MSG_SENTINEL3_INDEX            (u8)4
#define   ANT_TICK_MSG_MISSED_HIGH_BYTE_INDEX     (u8)5
#define   ANT_TICK_MSG_MISSED_MID_BYTE_INDEX      (u8)6
#define   ANT_TICK_MSG_MISSED_LOW_BYTE_INDEX      (u8)7



/**********************************************************************************************************************
Function prototypes
**********************************************************************************************************************/

/* ANT public Interface-layer Functions */
bool AntChannelConfig(bool);
bool AntOpenChannel(void);
bool AntCloseChannel(void);
bool AntUnassignChannel(void);
AntChannelStatusType AntRadioStatus(void);

bool AntQueueBroadcastMessage(u8 *pu8Data_);
bool AntQueueAcknowledgedMessage(u8 *pu8Data_);
bool AntReadData(void);


#endif /* __ANT_API_H */