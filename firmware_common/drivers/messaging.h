/*!**********************************************************************************************************************
@file messaging.h                                                                
@brief Header file for messaging.c
**********************************************************************************************************************/

#ifndef __MESSAGING_H
#define __MESSAGING_H


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* G_u32MessagingFlags */
#define _MESSAGING_TX_QUEUE_FULL        (u32)0x00000001
#define _MESSAGING_TX_QUEUE_ALMOST_FULL (u32)0x00000002
#define _DEQUEUE_GOT_NULL               (u32)0x00000004
#define _DEQUEUE_MSG_NOT_FOUND          (u32)0x00000008
/* end G_u32MessagingFlags */
  
/* Tx buffer allocation: be aware of RAM usage when selecting the parameters below.
Queue size in bytes is U8_TX_QUEUE_SIZE x U16_MAX_TX_MESSAGE_LENGTH */
#define U16_MAX_TX_MESSAGE_LENGTH       (u16)128       /*!< @brief Max bytes in message payload */
#define U8_TX_QUEUE_SIZE                (u8)32         /*!< @brief Number of messages allowed in the queue MAX 127 */
#define U8_TX_QUEUE_WATERMARK           (u8)(U8_TX_QUEUE_SIZE - 3) /*!< @brief Number of messages in the queue that will trigger a warning flag */
#define U8_STATUS_QUEUE_SIZE            (u8)(2 * U8_TX_QUEUE_SIZE) /*!< @brief Number of message statuses to maintain */

/*! @cond DOXYGEN_EXCLUDE */
/* Future: possible time-to-live constants for messages in the queue */
#define U32_MSG_STATUS_COMPLETE_TIME    (u32)1000      /* Max time in ms that a message status can sit in the status queue in a COMPLETE state */
#define U32_MSG_STATUS_WAITING_TIME     (u32)3000      /* Max time in ms that a message can sit in the queue in a WAITING state */
#define U32_MSG_STATUS_TIMEOUT_TIME     (u32)5000      /* Max time in ms that a message status can sit in the status queue in a TIMEOUT state */
#define U32_MSG_STATUS_CLEANING_TIME    (u32)10000     /* Time in ms between cleaning the message queue */
/*! @endcond */


/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
/*! 
@enum MessageStateType
@brief Possible statuses of a message in the queue. 
*/
typedef enum {EMPTY = 0, WAITING, SENDING, COMPLETE, TIMEOUT, ABANDONED, NOT_FOUND = 0xff} MessageStateType;

/*! 
@enum MessageType
@brief Message struct for data messages 
*/
typedef struct
{
  u32 u32Token;                             /*!< @brief Unique token for this message */
  u32 u32Size;                              /*!< @brief Size of the data payload in bytes */
  u8 pu8Message[U16_MAX_TX_MESSAGE_LENGTH]; /*!< @brief Data payload array */
  void* psNextMessage;                      /*!< @brief Pointer to next message */
} MessageType;

/*! 
@enum MessageSlotType
@brief Message node in the message list 
*/
  typedef struct
{
  bool bFree;                               /*!< @brief TRUE if message slot is available */
  MessageType Message;                      /*!< @brief The slot's message */
} MessageSlotType;

/*! 
@enum MessageStatusType
@brief Message tracking information 
*/
typedef struct
{
  u32 u32Token;                             /*!< @brief Unique token for this message; a token is never 0 */
  MessageStateType eState;                  /*!< @brief State of the message */
  u32 u32Timestamp;                         /*!< @brief Time the message status was posted */          
} MessageStatusType;


/**********************************************************************************************************************
* Function Declarations
**********************************************************************************************************************/

/*------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
MessageStateType QueryMessageStatus(u32 u32Token_);


/*------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
void MessagingInitialize(void);
void MessagingRunActiveState(void);

u32 QueueMessage(MessageType** eTargetTxBuffer_, u32 u32MessageSize_, u8* pu8MessageData_);
void DeQueueMessage(MessageType** pTargetQueue_);

void UpdateMessageStatus(u32 u32Token_, MessageStateType eNewState_);


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
static void AddNewMessageStatus(u32 u32Token_);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void MessagingSM_Idle(void);             
static void MessagingSM_Error(void);         




#endif /* __MESSAGING_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
