/**********************************************************************************************************************
File: messaging.c                                                                

Description:
This file contains utilities to create and monitor outgoing messages.  It essentially provides a 
smart queue system for outgoing data.
Any message being transferred by a peripheral is kept here.  As the peripheral sends the message,
it talks to messaging.c to get data and update the message status.  
All interaction between the peripheral and this task is through unique message tokens that are assigned
to every message queued to messaging.c
------------------------------------------------------------------------------------------------------------------------
API:

TYPES
MessageStateType: enum status of a message in the queue
EMPTY, WAITING, SENDING, RECEIVING, COMPLETE, TIMEOUT, ABANDONED, NOT_FOUND

MessageType: linked-list style entry with token, size, data pointer and next MessageType pointer

MessageSlot: member of the message queue holding the free/full status of a particular queue location and a pointer to its data

MessageStatus: token, state and timestamp of a message in the queue

FUNCTIONS
Public:
MessageStateType QueryMessageStatus(u32 u32Token_)
Queries the current status of the message with u32Token.  If the message has completed or timed out, the query will
cause the message status to be removed from the status queue.

Protected:
void MessagingInitialize(void)
One-time call to start the messaging application.

u32 QueueMessage(MessageType** ppeTargetTxBuffer_, u32 u32MessageSize_, u8* pu8MessageData_)
Adds a message to the correct data queue, assigns a token which is posted to the status queue and returned to the client.
This function is Protected because tasks that can queue messages should be managed carefully and not granted free reign
to queue messages.  The message queue is a finite resource with TX_QUEUE_SIZE slots available for messages.
We avoid dynamic allocation due to the inherent issues with fragmentation on resource-limited systems.

void DeQueueMessage(MessageType** pTargetQueue_)
Removes a message from the message queue (typically since all the bytes have been submitted to the communication peripheral
which is sending the message.  The message status is updated in the status queue.

void UpdateMessageStatus(u32 u32Token_, MessageStateType eNewState_)
Changes the status of a message in the statue queue.

**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxMessaging"
***********************************************************************************************************************/
/* New variables */
u32 G_u32MessagingFlags;                                 /* Global state flags */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                    /* From main.c */
extern volatile u32 G_u32ApplicationFlags;               /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                  /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                   /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Msg_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Messaging_pfnStateMachine;            /* The state machine function pointer */
static u32 Msg_u32Token;                                 /* Incrementing message token used for all external communications */

static MessageSlot Msg_Pool[TX_QUEUE_SIZE];              /* Array of MessageSlot used for the transmit queue */
static u8 Msg_u8QueuedMessageCount;                      /* Number of messages slots currently occupied */

/* A separate status queue needs to be maintained since the message information in Msg_Pool will be lost when the message
has been dequeued.  Applications must be able to query to determine the status of their message, particularly if
it has been sent. */
static MessageStatus Msg_StatusQueue[STATUS_QUEUE_SIZE]; /* Array of MessageStatus used to monitor message status */
static MessageStatus* Msg_pNextStatus;                   /* Pointer to next available message status */


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: QueryMessageStatus()

Description:
Checks the state of a message.  If the state is COMPLETE or TIMEOUT, the status is deleted from the message queue.
Since the queue is quite short, most of the time it will hold very little entires.  New entries are always filled at the front,
using a simple linear search starting at index 0.

Requires:
  - u32Token_ is the token of the message of interest

Promises:
  - Returns MessageStateType indicating the status of the message
  - if the message is found in COMPLETE or TIMEOUT state, the status is removed from the queue
*/
MessageStateType QueryMessageStatus(u32 u32Token_)
{
  MessageStateType eStatus   = NOT_FOUND;
  MessageStatus* pListParser = &Msg_StatusQueue[0];
  
  /* Brute force search for the token - the queue will never be large enough on this system to require a more
  intelligent search algorithm */
  while( (pListParser->u32Token != u32Token_) && (pListParser != &Msg_StatusQueue[STATUS_QUEUE_SIZE]) )
  {
    pListParser++;
  }

  /* If the token was found pListParser is pointing at it, take appropriate action */
  if(pListParser != &Msg_StatusQueue[STATUS_QUEUE_SIZE])
  {
    /* Save the status */
    eStatus = pListParser->eState;

    /* Release the slot if the message state is final (the client must deal with it now) */
    if( (eStatus == COMPLETE) || (eStatus == TIMEOUT) )
    {
      pListParser->u32Token = 0;
      pListParser->eState = EMPTY;
    }
  }

  return(eStatus);
  
} /* end QueryMessageStatus() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: QueueMessage

Description:
Allocates one of the positions in the message queue to the calling function's send queue.

Requires:
  - ppeTargetTxBuffer_ is the peripheral transmit buffer where the message will be queued
  - u32MessageSize_ is the size of the message data array in bytes
  - pu8MessageData_ points to the message data array
  - Msg_Pool should not be full 

Promises:
  - The message is inserted into the target list and assigned a token
  - If the message is created successfully, the message token is returned; otherwise, NULL is returned
*/
u32 QueueMessage(MessageType** ppeTargetTxBuffer_, u32 u32MessageSize_, u8* pu8MessageData_)
{
  MessageSlot *psSlotParser;
  MessageType *psNewMessage;
  MessageType *psListParser;
  u32 u32BytesRemaining = u32MessageSize_;
  u32 u32CurrentMessageSize = 0;
  
  /* Check for available space in the message pool */
  if(Msg_u8QueuedMessageCount == TX_QUEUE_SIZE)
  {
    G_u32MessagingFlags |= _MESSAGING_TX_QUEUE_FULL;
    return(0);
  }

  /* Space available, so proceed with allocation.  Though only one message is queued at a time, we
  use a while loop to handle messages that are too big and must be split into different slots.  The slots
  are always sequential and the message processor will send the bytes continuously across slots */
  while(u32BytesRemaining)
  {
    Msg_u8QueuedMessageCount++;
    
    /* Flag if we're above the high watermark */
    if(Msg_u8QueuedMessageCount >= TX_QUEUE_WATERMARK)
    {
      G_u32MessagingFlags |= _MESSAGING_TX_QUEUE_ALMOST_FULL;
    }
    else
    {
      G_u32MessagingFlags &= ~_MESSAGING_TX_QUEUE_ALMOST_FULL;
    }
    
    /* Find an empty slot: this is non-circular and there must be at least one free slot if we're here */
    psSlotParser = &Msg_Pool[0];
    while(!psSlotParser->bFree)
    {
      psSlotParser++;
    }
    
    /* Allocate the slot and set the message pointer */
    psSlotParser->bFree = FALSE;
    psNewMessage = &(psSlotParser->Message);
  
    /* Check the message size and split the message up if necessary */
    if(u32BytesRemaining > MAX_TX_MESSAGE_LENGTH)
    {
      u32CurrentMessageSize = MAX_TX_MESSAGE_LENGTH;
      u32BytesRemaining -= MAX_TX_MESSAGE_LENGTH;
    }
    else
    {
      u32CurrentMessageSize = u32BytesRemaining;
      u32BytesRemaining = 0;
    }
    
    /* Copy all the data to the allocated message structure */
    psNewMessage->u32Token      = Msg_u32Token;
    psNewMessage->u32Size       = u32CurrentMessageSize;
    psNewMessage->psNextMessage = NULL;
    
    /* Add the data into the payload */
    for(u32 i = 0; i < psNewMessage->u32Size; i++)
    {
      *(psNewMessage->pu8Message + i) = *pu8MessageData_++;
    }
  
    /* Link the new message into the client's transmit buffer */
    /* Handle an empty list */
    if(*ppeTargetTxBuffer_ == NULL)
    {
      *ppeTargetTxBuffer_ = psNewMessage;
    }

    /* Add the message to the end of the list */
    else
    {
      /* Find the last node */
      psListParser =  *ppeTargetTxBuffer_;
      while(psListParser->psNextMessage != NULL)
      {
        psListParser = psListParser->psNextMessage;
      }
     
      /* Found the end: add the new node */
      psListParser->psNextMessage = psNewMessage;
    }
  
    /* Update the Public status of the message in the status queue */
    AddNewMessageStatus(Msg_u32Token);
  
    /* Increment message token and catch the rollover every 4 billion messages... Token 0 is not allowed. */
    if(++Msg_u32Token == 0)
    {
      Msg_u32Token = 1;
    }
  
  } /* end while */

  /* Return only the current (and highest) message token, as it will be the last portion to be sent if the message was split up */
  return(psNewMessage->u32Token);
  
} /* end QueueMessage() */


/*----------------------------------------------------------------------------------------------------------------------
Function: DeQueueMessage

Description:
Removes a message from a message queue and adds it back to the pool.

Requires:
  - pTargetQueue_ points to the list queue where the message to be deleted is located
  - pTargetQueue_ is a FIFO linked-list where the message that needs to be killed is at the front of the list
  - The message to be removed has been completely sent and is no longer in use
  - New message cannot be added into the list during this function (via interrupts)

Promises:
  - The first message in the list is deleted; the list is hooked back up
  - The message space is added back to the available message queue
*/
void DeQueueMessage(MessageType** pTargetQueue_)
{
  MessageSlot *psSlotParser;
      
  /* Make sure there is a message to kill */
  if(*pTargetQueue_ == NULL)
  {
    G_u32MessagingFlags |= _DEQUEUE_GOT_NULL;
    return;
  }
  
  /* Find the message's slot: this message pool is non-circular and the message must be one of the slots */
  psSlotParser = &Msg_Pool[0];
  while( (&psSlotParser->Message != *pTargetQueue_) && (psSlotParser != &Msg_Pool[TX_QUEUE_SIZE]) )
  {
    psSlotParser++;
  }

  /* Make sure the message has been found */
  if(psSlotParser == &Msg_Pool[TX_QUEUE_SIZE])
  {
    G_u32MessagingFlags |= _DEQUEUE_MSG_NOT_FOUND;
    return;
  }

  /* Unhook the message from the current owner's queue and put it back in the pool */
  *pTargetQueue_ = (*pTargetQueue_)->psNextMessage;
  psSlotParser->bFree = TRUE;
  Msg_u8QueuedMessageCount--;
  
} /* end DeQueueMessage() */


/*--------------------------------------------------------------------------------------------------------------------
Function: MessagingInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  - No messaging in progress

Promises:
  - Message queues are zeroed
  - Flags and state machine are initialized
*/
void MessagingInitialize(void)
{
  /* Inititalize variables */
  Msg_u8QueuedMessageCount = 0;
  Msg_u32Token = 1;

  /* Ensure all message slots are deallocated and the message status queue is empty */
  for(u16 i = 0; i < TX_QUEUE_SIZE; i++)
  {
    Msg_Pool[i].bFree = TRUE;
  }

  for(u16 i = 0; i < STATUS_QUEUE_SIZE; i++)
  {
    Msg_StatusQueue[i].u32Token = 0;
    Msg_StatusQueue[i].eState = EMPTY;
    Msg_StatusQueue[i].u32Timestamp = 0;
  }

  Msg_pNextStatus = &Msg_StatusQueue[0];

  G_u32MessagingFlags = 0;
  Messaging_pfnStateMachine = MessagingIdle;

} /* end MessagingInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function MessagingRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void MessagingRunActiveState(void)
{
  Messaging_pfnStateMachine();

} /* end MessagingRunActiveState */


/*----------------------------------------------------------------------------------------------------------------------
Function: UpdateMessageStatus()

Description:
Changes the status of a message in the statue queue.

Requires:
  - u32Token_ is message that should be in the status queue
  - eNewState_ is the desired status setting for the message

Promises:
  - eState of the message is set to eNewState_
*/
void UpdateMessageStatus(u32 u32Token_, MessageStateType eNewState_)
{
  MessageStatus* pListParser = &Msg_StatusQueue[0];
  
  /* Search for the token */
  while( (pListParser->u32Token != u32Token_) && (pListParser != &Msg_StatusQueue[STATUS_QUEUE_SIZE]) )
  {
    pListParser++;
  }

  /* If the token was found, change the status */
  if(pListParser != &Msg_StatusQueue[STATUS_QUEUE_SIZE])
  {
    pListParser->eState = eNewState_;
  }
  
} /* end UpdateMessageStatus() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: AddNewMessageStatus()

Description:
Adds a new mesage into the status queue.  Due to the tendancy of applications to forget that they wrote
a message here, this buffer is circular and will overwite the oldest message if it needs space for a 
new message.

Requires:
  - u32Token_ is the message of interest

Promises:
  - A new status is created indexed by u32Token_
*/
static void AddNewMessageStatus(u32 u32Token_)
{
  /* Install the new message message */
  Msg_pNextStatus->u32Token = u32Token_;
  Msg_pNextStatus->eState = WAITING;
  Msg_pNextStatus->u32Timestamp = G_u32SystemTime1ms;
  
  /* Safely advance the pointer */
  Msg_pNextStatus++;
  if(Msg_pNextStatus == &Msg_StatusQueue[STATUS_QUEUE_SIZE])
  {
    Msg_pNextStatus = &Msg_StatusQueue[0];
  }
  
} /* end AddNewMessageStatus() */


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Do nothing for now */
void MessagingIdle(void)
{
  static u32 u32CleaningTime = MSG_STATUS_CLEANING_TIME;
  
  /* Periodically check for stale messages */
  if(--u32CleaningTime == 0)
  {
    u32CleaningTime = MSG_STATUS_CLEANING_TIME;
    
    /* ??? Probably should add clean of the main message queue to detect any messages that have become stuck */
  }
    
} /* end MessagingIdle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
void MessagingError(void)          
{
  Messaging_pfnStateMachine = MessagingIdle;
  
} /* end MessagingError() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
