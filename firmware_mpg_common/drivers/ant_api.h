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
//#define ANT_API_LEGACY   /* Define to support non-channel specific legacy commands */

#define ANT_ASSIGN_MESSAGES                 (u8)7    /* Number of messages in AntAssignChannel */       

#define ANT_OUTGOING_MESSAGE_BUFFER_SIZE    (u32)32
#define ANT_APPLICATION_MESSAGE_BUFFER_SIZE (u32)32
#define ANT_DATA_BYTES                      (u8)8

/* Device Types */
#define	DEVICE_TYPE_BOARDTEST		            (u8)0x60


/* ANT Channels */
#define ANT_CHANNELS                        (u8)8
#define ANT_CHANNEL_0                       (u8)0
#define ANT_CHANNEL_1                       (u8)1
#define ANT_CHANNEL_2                       (u8)2
#define ANT_CHANNEL_3                       (u8)3
#define ANT_CHANNEL_4                       (u8)4
#define ANT_CHANNEL_5                       (u8)5
#define ANT_CHANNEL_6                       (u8)6
#define ANT_CHANNEL_7                       (u8)7
#define ANT_CHANNEL_SCANNING                (u8)0xff


/**********************************************************************************************************************
Type definitions
**********************************************************************************************************************/


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
#define   ANT_TICK_MSG_CHANNEL_INDEX              (u8)1
#define   ANT_TICK_MSG_RESPONSE_TYPE_INDEX        (u8)2
#define   ANT_TICK_MSG_EVENT_CODE_INDEX           (u8)3
#define   ANT_TICK_MSG_RESPONSE_CODE_INDEX        (u8)3
#define   ANT_TICK_MSG_SENTINEL1_INDEX            (u8)2
#define   ANT_TICK_MSG_SENTINEL2_INDEX            (u8)3
#define   ANT_TICK_MSG_SENTINEL3_INDEX            (u8)4
#define   ANT_TICK_MSG_MISSED_HIGH_BYTE_INDEX     (u8)5
#define   ANT_TICK_MSG_MISSED_MID_BYTE_INDEX      (u8)6
#define   ANT_TICK_MSG_MISSED_LOW_BYTE_INDEX      (u8)7

#ifdef ANT_API_LEGACY

#endif /* ANT_API_LEGACY */


/**********************************************************************************************************************
Function prototypes
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/
#ifdef ANT_API_LEGACY
bool AntChannelConfig(bool);
bool AntOpenChannel(void);
bool AntCloseChannel(void);
bool AntUnassignChannel(void);

AntChannelStatusType AntRadioStatus(void);

bool AntQueueBroadcastMessage(u8 *pu8Data_);
bool AntQueueAcknowledgedMessage(u8 *pu8Data_);
#endif /* ANT_API_LEGACY */

bool AntAssignChannel(AntAssignChannelInfoType* psAntSetupInfo_);
bool AntUnassignChannelNumber(u8 u8Channel_);

bool AntOpenChannelNumber(u8 u8Channel_);
bool AntCloseChannelNumber(u8 u8Channel_);

AntChannelStatusType AntRadioStatusChannel(u8 u8Channel_);

bool AntQueueBroadcastMessage(u8 u8Channel_, u8 *pu8Data_);
bool AntQueueAcknowledgedMessage(u8 u8Channel_, u8 *pu8Data_);

bool AntReadAppMessageBuffer(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/
void AntApiInitialize(void);
void AntApiRunActiveState(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions                                                                                                  */
/*--------------------------------------------------------------------------------------------------------------------*/



/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void AntApiSM_Idle(void);    
static void  AntApiSM_AssignChannel(void);       

static void AntApiSM_Error(void);         
static void AntApiSM_FailedInit(void);        

#endif /* __ANT_API_H */