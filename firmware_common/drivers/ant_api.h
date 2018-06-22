/*!**********************************************************************************************************************
@file ant_api.h                                                                
@brief Header file for ant_api.c
**********************************************************************************************************************/

#ifndef __ANT_API_H
#define __ANT_API_H

/**********************************************************************************************************************
Type definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
AntChannelStatusType AntRadioStatusChannel(AntChannelNumberType eChannel_);

bool AntAssignChannel(AntAssignChannelInfoType* psAntSetupInfo_);
bool AntUnassignChannelNumber(AntChannelNumberType eChannel_);

bool AntOpenChannelNumber(AntChannelNumberType eChannel_);
bool AntOpenScanningChannel(void);
bool AntCloseChannelNumber(AntChannelNumberType eChannel_);

bool AntQueueBroadcastMessage(AntChannelNumberType eChannel_, u8 *pu8Data_);
bool AntQueueAcknowledgedMessage(AntChannelNumberType eChannel_, u8 *pu8Data_);

bool AntReadAppMessageBuffer(void);

void AntGetdBmAscii(s8 s8RssiValue_, u8* pu8Result_);


/*-------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
void AntApiInitialize(void);
void AntApiRunActiveState(void);


/*-------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/



/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void AntApiSM_Idle(void);    
static void AntApiSM_AssignChannel(void);       

static void AntApiSM_Error(void);         
static void AntApiSM_FailedInit(void);    


/**********************************************************************************************************************
Constants
**********************************************************************************************************************/
#define ANT_ASSIGN_MESSAGES                 (u8)8    /* Number of messages in AntAssignChannel */       

#define ANT_OUTGOING_MESSAGE_BUFFER_SIZE    (u32)32
#define ANT_APPLICATION_MESSAGE_BUFFER_SIZE (u32)32
#define ANT_DATA_BYTES                      (u8)8


/**********************************************************************************************************************
Application messages
**********************************************************************************************************************/

/*----------------------------------------------------------------------------------------------------------------------
ANT_TICK communicates the message period to the application.  

If ANT is running as a Master, ANT_TICK occurs every time a broadcast or acknowledged
data message is sent (EVENT_TX). 

If ANT is running as a Slave, ANT_TICK occurs whenever ANT misses a message that it was expecting 
based on the established timing of a paired channel (EVENT_RX_FAIL).  If too many missed messages
occur, then EVENT_RX_FAIL_GO_TO_SEARCH will be sent.  If a search timeout is active, 
EVENT_RX_SEARCH_TIMEOUT will be sent if the timeout period expires.

This structure is set up to also relay other channel response messages, but is currently not used.

MSG_NAME  MSG_ID     D_0      D_1      D_2     D_3     D_4     D_5     D_6
ANT_TICK   0xFF    CHANNEL  RESPONSE  EVENT   0xFF   MISSED  MISSED  MISSED
                             TYPE     CODE            MSG #   MSG #   MSG #
                                                      HIGH    MID     LOW
---------------------------------------------------------------------------------------------------------------------*/

#define   MESSAGE_ANT_TICK                        (u8)0xFF
#define   MESSAGE_ANT_TICK_SENTINEL               (u8)0xFF

#define   ANT_TICK_MSG_ID_INDEX                   (u8)0
#define   ANT_TICK_MSG_CHANNEL_INDEX              (u8)1
#define   ANT_TICK_MSG_RESPONSE_TYPE_INDEX        (u8)2
#define   ANT_TICK_MSG_EVENT_CODE_INDEX           (u8)3
#define   ANT_TICK_MSG_SENTINEL_INDEX             (u8)4
#define   ANT_TICK_MSG_MISSED_HIGH_BYTE_INDEX     (u8)5
#define   ANT_TICK_MSG_MISSED_MID_BYTE_INDEX      (u8)6
#define   ANT_TICK_MSG_MISSED_LOW_BYTE_INDEX      (u8)7




#endif /* __ANT_API_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
