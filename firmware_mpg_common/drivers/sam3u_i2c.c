/**********************************************************************************************************************
File: sam3u_TWI.c                                                                

Description: 
Provides a driver to use TWI0 peripheral to send and receive data using interrupts.
Currently Set at - 200kHz Master Mode.
This is a simpler version of a serial system driver that does not use resource control
through Request() and Release() calls


------------------------------------------------------------------------------------------------------------------------
API:

Public use Functions:

bool TWI0ReadByte(u8 u8SlaveAddress_, u8* pu8RxBuffer_);
bool TWI0ReadData(u8 u8SlaveAddress_, u8* pu8RxBuffer_, u32 u32Size_);
u32 TWIWriteByte(TWIPeripheralType* psTWIPeripheral_, u8 u8Byte_, TWIStopType Send_);
u32 TWIWriteData(TWIPeripheralType* psTWIPeripheral_, u32 u32Size_, u8* u8Data_, TWIStopType Send_);

All of these functions return a value that should be checked to ensure the operation will be completed

Both TWI0ReadByte and TWI0ReadData require that pu8RxBuffer is large enough to hold the data
As well it is assumed, that since you know the amount of data to be sent, a stop can be sent
when all bytes have benn received (and not tie the data and clock line low).

WriteByte and WriteData have the option to hold the lines low as it waits for more data 
to be queue. If a stop condition is not sent only Writes can follow until a stop condition is
requested (as the current transmission isn't complete).

!!!!! ISSUES: 
    - No Debugging of Read functionality

**********************************************************************************************************************/

#include "configuration.h"


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;        /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;         /* From board-specific source file */

extern volatile u32 G_u32SystemFlags;          /* From main.c */
extern volatile u32 G_u32ApplicationFlags;     /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "TWI_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type TWI_StateMachine;              /* The TWI application state machine function pointer */

static u32 TWI_u32Timer;                          /* Counter used across states */
static u32 TWI_u32Flags;                          /* Application flags for TWI */

static TWIPeripheralType TWI_Peripheral0;         /* TWI0 peripheral object */
static TWIPeripheralType* TWI0;

static u32 TWI_u32CurrentBytesRemaining;                        /* Down counter for number of bytes being clocked out */
static u8* TWI_pu8CurrentTxData;                                /* Pointer to current message being clocked out */
static TWIMessageQueueType TWI_MessageBuffer[TX_QUEUE_SIZE];    /* A circular buffer that stores queued msgs stop condition */
static u8 TWI_MessageBufferNextIndex;                           /* A pointer to the next position to place a message */
static u8 TWI_MessageBufferCurIndex;                            /* A pointer to the current message that is being processed */
static u8 TWI_MessageQueueLength;                               /* Counter to track the number of messages stored in the queue */


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*----------------------------------------------------------------------------------------------------------------------
Function: TWI0ReadByte

Description:
Queues a TWI Read Message into TWI_MessageBuffer, will be processed after all msgs queued before it
  - Single byte

Requires:
  - Initialization of the task
  - Requires that pu8RxBuffer has the space to save the data

Promises:
  - Queues msg if there is space available
  - Returns TRUE if successful queue
*/
bool TWI0ReadByte(u8 u8SlaveAddress_, u8* pu8RxBuffer_)
{
  if(TWI_MessageQueueLength == TX_QUEUE_SIZE || (TWI0->u32Flags & _TWI_TRANS_NOT_COMP))
  {
    /* TWI Message Task Queue Full or the Tx transmit isn't complete */
    return FALSE;
  }
  else
  {
    /* Queue Relevant data for TWI register setup */
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].Direction     = READ;
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].u32Size       = 1;
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].u8Address     = u8SlaveAddress_;
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].pu8RxBuffer   = pu8RxBuffer_;
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].u8Attempts    = 0;
    
    /* Not used by Receive */
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].Stop = NA; 
     
    /* Update array pointers and size */
    TWI_MessageBufferNextIndex++;
    TWI_MessageQueueLength++;
    if(TWI_MessageBufferNextIndex == TX_QUEUE_SIZE)
    {
      TWI_MessageBufferNextIndex = 0;
    }
    
    /* If the system is initializing, we want to manually cycle the TWI task through one iteration
      to send the message */
    if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
    {
      TWIManualMode();
    }
    
    return TRUE;
  }

} /* end TWI0ReadByte() */


/*----------------------------------------------------------------------------------------------------------------------
Function: TWI0ReadData

Description:
Queues a TWI Read Message into TWI_MessageBuffer, will be processed after all msgs queued before it

Requires:
  - Initialization of the task
  - Requires pu8RxBuffer has the space to save the data

Promises:
  - Queues a multi byte command into the command array
  - Returns TRUE if the queue was successful
*/
bool TWI0ReadData(u8 u8SlaveAddress_, u8* pu8RxBuffer_, u32 u32Size_)
{
  if(TWI_MessageQueueLength == TX_QUEUE_SIZE || (TWI0->u32Flags & _TWI_TRANS_NOT_COMP))
  {
    /* TWI Message Task Queue Full or the Tx transmit isn't complete */
    return FALSE;
  }
  else
  {
    /* Queue Relevant data for TWI register setup */
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].Direction     = READ;
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].u32Size       = u32Size_;
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].u8Address     = u8SlaveAddress_;
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].pu8RxBuffer   = pu8RxBuffer_;
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].u8Attempts    = 0;
    
    /* Not used by Receive */
    TWI_MessageBuffer[TWI_MessageBufferNextIndex].Stop = NA; 
    
    /* Update array pointers and size */
    TWI_MessageBufferNextIndex++;
    TWI_MessageQueueLength++;
    if(TWI_MessageBufferNextIndex == TX_QUEUE_SIZE)
    {
      TWI_MessageBufferNextIndex = 0;
    }
    
    /* If the system is initializing, we want to manually cycle the TWI task through one iteration
      to send the message */
    if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
    {
      TWIManualMode();
    }
    
    return TRUE;
  }
  
} /* end TWI0ReadData() */


/*----------------------------------------------------------------------------------------------------------------------
Function: TWI0WriteByte

Description:
Queues a single byte for transfer on TWI0 peripheral.  

Requires:
  - Initialization of the task

Promises:
  - Creates a 1-byte message at TWI0->pTransmitBuffer that will be sent by the TWI application
    when it is available.
  - Returns the message token assigned to the message
*/
u32 TWI0WriteByte(u8 u8SlaveAddress_, u8 u8Byte_, TWIStopType Send_)
{
  u32 u32Token;
  u8 u8Data = u8Byte_;
  
  if(TWI_MessageQueueLength == TX_QUEUE_SIZE)
  {
    /* TWI Message Task Queue Full */
    return 0;
  }
  else
  {
    /* Queue Message in message system */
    u32Token = QueueMessage(&TWI0->pTransmitBuffer, 1, &u8Data);
    if(u32Token)
    {
      /* Queue Relevant data for TWI register setup */
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].Direction     = WRITE;
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].u32Size       = 1;
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].u8Address     = u8SlaveAddress_;
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].Stop          = Send_;
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].u8Attempts    = 0;
      
      /* Not used by Transmit */
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].pu8RxBuffer = NULL;
      
      /* Update array pointers and size */
      TWI_MessageBufferNextIndex++;
      TWI_MessageQueueLength++;
      if(TWI_MessageBufferNextIndex == TX_QUEUE_SIZE)
      {
        TWI_MessageBufferNextIndex = 0;
      }

      /* If the system is initializing, we want to manually cycle the TWI task through one iteration
      to send the message */
      if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
      {
        TWIManualMode();
      }
    }
    
    return(u32Token);
  }
  
} /* end TWIWriteByte() */


/*----------------------------------------------------------------------------------------------------------------------
Function: TWI0WriteData

Description:
Queues a data array for transfer on the  TWI0 peripheral.  

Requires:
  - if a transmission is in progress, the node in the buffer that is currently being sent will not be destroyed during this function.
  - u32Size_ is the number of bytes in the data array
  - u8Data_ points to the first byte of the data array

Promises:
  - adds the data message at TWI_Peripheral0->pTransmitBuffer buffer that will be sent by the TWI application
    when it is available.
  - Returns the message token assigned to the message; 0 is returned if the message cannot be queued in which case
    G_u32MessagingFlags can be checked for the reason
*/
u32 TWI0WriteData(u8 u8SlaveAddress_, u32 u32Size_, u8* u8Data_, TWIStopType Send_)
{
  u32 u32Token;
    
  if(TWI_MessageQueueLength == TX_QUEUE_SIZE)
  {
    /* Queue Message in message system */
    return 0;
  }
  else
  {
    /* Queue Message in message system */
    u32Token = QueueMessage(&TWI0->pTransmitBuffer, u32Size_, u8Data_);
    if(u32Token)
    {
      /* Queue Relevant data for TWI register setup */
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].Direction     = WRITE;
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].u32Size       = 1;
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].u8Address     = u8SlaveAddress_;
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].Stop          = Send_;
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].u8Attempts    = 0;
      
      /* Not used by Transmit */
      TWI_MessageBuffer[TWI_MessageBufferNextIndex].pu8RxBuffer = NULL;
      
      /* Update array pointers and size */
      TWI_MessageBufferNextIndex++;
      TWI_MessageQueueLength++;
      if(TWI_MessageBufferNextIndex == TX_QUEUE_SIZE)
      {
        TWI_MessageBufferNextIndex = 0;
      }

      /* If the system is initializing, manually cycle the TWI task through one iteration to send the message */
      if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
      {
        TWIManualMode();
      }
    }
  
    return(u32Token);
  }
  
} /* end TWIWriteData() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: TWIInitialize

Description:
Initializes the TWI application and its variables. 

Requires:
  - 

Promises:
  - TWI peripheral objects are ready 
  - TWI application set to Idle
*/
void TWIInitialize(void)
{
  u32 u32TargetPerpipheralNumber = AT91C_ID_TWI0;
  
  AT91C_BASE_PMC->PMC_PCER |= (1<<u32TargetPerpipheralNumber);
  
  TWI_u32Flags = 0;
  TWI0 = &TWI_Peripheral0;
  TWI_MessageBufferNextIndex = 0;
  TWI_MessageBufferCurIndex = 0;
  TWI_MessageQueueLength = 0;
  
  /* Initialize the TWI peripheral structures */
  TWI_Peripheral0.pBaseAddress    = AT91C_BASE_TWI0;
  TWI_Peripheral0.pTransmitBuffer = NULL;
  TWI_Peripheral0.pu8RxBuffer     = NULL;
  TWI_Peripheral0.u32Flags        = 0;

  /* Software reset of peripheral */
  TWI0->pBaseAddress->TWI_CR   |= _TWI_CR_SWRST_BIT;
  TWI_u32Timer = G_u32SystemTime1ms;
  while( !IsTimeUp(&TWI_u32Timer, 5) );
  
  /* Configure Peripheral */
  TWI0->pBaseAddress->TWI_CWGR = TWI0_CWGR_INIT;
  TWI0->pBaseAddress->TWI_CR   = TWI0_CR_INIT;
  TWI0->pBaseAddress->TWI_MMR  = TWI0_MMR_INIT;
  TWI0->pBaseAddress->TWI_IER  = TWI0_IER_INIT;
  TWI0->pBaseAddress->TWI_IDR  = TWI0_IDR_INIT;
  
  /* Enable TWI interrupts */
  NVIC_ClearPendingIRQ( (IRQn_Type)u32TargetPerpipheralNumber );
  NVIC_EnableIRQ( (IRQn_Type)u32TargetPerpipheralNumber );

  TWI_u32CurrentBytesRemaining   = 0;
  TWI_pu8CurrentTxData           = NULL;

  /* Set application pointer */
  TWI_StateMachine = TWISM_Idle;
  
} /* end TWIInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function TWIRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void TWIRunActiveState(void)
{
  TWI_StateMachine();

} /* end TWIRunActiveState */



/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: TWIFillTxBuffer

Description:
Fills the TWI peripheral buffer with bytes from the current messsage that is sending.  
This function can be called from the TWI ISR!
Note: if the implemented processor does not have a FIFO, this function can still be used but will only ever
add one byte to the transmitter.

Requires:
  - The TxBuffer is empty
  - TWI_pu8CurrentTxData points to the next byte in the message to be sent
  - TWI_u32CurrentBytesRemaining has an accurate count of the bytes remaining in the message data to be sent
  - Transmit interrupts are off

Promises:
  - Data from *TWI_pu8CurrentTxData is added to the TWI peripheral Tx FIFO until the FIFO is full or there
    is no more data to send.
*/
static void TWI0FillTxBuffer(void)
{
  u8 u8ByteCount = TWI_TX_FIFO_SIZE;
  
  /* Use the active global variables to fill up the transmit FIFO */
  while( (u8ByteCount != 0) && (TWI_u32CurrentBytesRemaining != 0) )
  {
    TWI0->pBaseAddress->TWI_THR = *TWI_pu8CurrentTxData;
    TWI_pu8CurrentTxData++;
    TWI_u32CurrentBytesRemaining--;
    u8ByteCount--;
  }
  
  /* If there are no remaining bytes to load to the TX FIFO, disable the TWI transmit 
  FIFO empty interrupt */
  if(TWI_u32CurrentBytesRemaining == 0)
  {
    TWI0->pBaseAddress->TWI_IDR = AT91C_TWI_TXRDY_MASTER;
    if(TWI_MessageBuffer[TWI_MessageBufferCurIndex].Stop == STOP)
    {
      TWI0->pBaseAddress->TWI_CR |= _TWI_CR_STOP_BIT;
    }
  }
  /* Otherwise make sure transmit interrupts are enabled */
  else
  {
    TWI0->pBaseAddress->TWI_IER = AT91C_TWI_TXRDY_MASTER;
  }
  
} /* end TWIFillTxBuffer() */


/*----------------------------------------------------------------------------------------------------------------------
Function: TWIManualMode

Description:
Runs a transmit cycle of the TWI application to clock out a message.  This function is used only during
initialization.

Requires:
  - TWI application has been initialized.

Promises:
  - All bytes currently in the TWI Rx FIFO are read out to the application receive circular buffer.
*/
static void TWIManualMode(void)
{
  TWI_u32Flags |=_TWI_INIT_MODE;
  TWI_u32Timer  = G_u32SystemTime1ms;
  
  while(TWI_u32Flags &_TWI_INIT_MODE)
  {
    TWI_StateMachine();
    IsTimeUp(&TWI_u32Timer, 1);
  }
      
} /* end TWIManualMode() */


/*----------------------------------------------------------------------------------------------------------------------
Function: TWI0_IrqHandler

Description:
Handles the TWI0 Peripheral interrupts

Requires:
  - TWI application has been initialized.

Promises:
  - More data has been queued or error flag raised
  - Or recieved byte is placed in desied buffer
*/
void TWI0_IrqHandler(void)
{
  u32 u32InterruptStatus;
  
  /* Grab active interrupts and compare with status */
  u32InterruptStatus = AT91C_BASE_TWI0->TWI_IMR;
  u32InterruptStatus &= AT91C_BASE_TWI0->TWI_SR;
  
  /* NACK Received */
  if(u32InterruptStatus & _TWI_SR_NACK )
  {
    /* Error has occurred, reset the msg */
    TWI_u32Flags |= _TWI_ERROR_NACK;
    
  }
  /* Receiving Bytes */
  else if(u32InterruptStatus & _TWI_SR_RXRDY && ( TWI0->u32Flags & _TWI_RECEIVING ) )
  {
    *TWI0->pu8RxBuffer = TWI0->pBaseAddress->TWI_RHR;
    TWI0->pu8RxBuffer++;
    TWI_u32CurrentBytesRemaining--;
    
    if(TWI_u32CurrentBytesRemaining == 1)
    {
      TWI0->pBaseAddress->TWI_CR |= _TWI_CR_STOP_BIT;
    }
  }
  /* Transmitting Bytes */
  else if(u32InterruptStatus & _TWI_SR_TXRDY && ( TWI0->u32Flags & _TWI_TRANSMITTING ) )
  {
    /* There is more data queued and peripheral ready */
    TWI0FillTxBuffer();
  }
  else
  {
    TWI_u32Flags |= _TWI_ERROR_INTERRUPT;
  }
}

/***********************************************************************************************************************
State Machine Function Definitions
***********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a transmit message to be queued.  Received data is handled in interrupts. */
void TWISM_Idle(void)
{
  if(TWI_MessageBufferNextIndex != TWI_MessageBufferCurIndex )
  {
    TWI0->pBaseAddress->TWI_MMR = TWI0_MMR_INIT;
    TWI0->pBaseAddress->TWI_CR = TWI0_CR_INIT;
    
    if(TWI_MessageBuffer[TWI_MessageBufferCurIndex].Direction == WRITE)
    {
      /* insert new address */
      TWI0->pBaseAddress->TWI_MMR |= ((TWI_MessageBuffer[TWI_MessageBufferCurIndex].u8Address << _TWI_MMR_ADDRESS_SHIFT));
      
      /* Set up to transmit the message */
      TWI_u32CurrentBytesRemaining = TWI0->pTransmitBuffer->u32Size;
      TWI_pu8CurrentTxData = TWI0->pTransmitBuffer->pu8Message;
      TWI0FillTxBuffer();    
      
      /* Update the message's status */
      UpdateMessageStatus(TWI0->pTransmitBuffer->u32Token, SENDING);
  
      /* Proceed to next state to let the current message send */
      TWI0->u32Flags |= (_TWI_TRANSMITTING | _TWI_TRANS_NOT_COMP);
      TWI_StateMachine = TWISM_Transmitting;
    }
    else if(TWI_MessageBuffer[TWI_MessageBufferCurIndex].Direction == READ)
    {
      /* insert new address and set Read bit */
      TWI0->pBaseAddress->TWI_MMR |= (((TWI_MessageBuffer[TWI_MessageBufferCurIndex].u8Address) << _TWI_MMR_ADDRESS_SHIFT) & _TWI_MMR_MREAD_BIT);
      
      /* Grab number of desired bytes and the pointer to store the buffer */
      TWI_u32CurrentBytesRemaining = TWI_MessageBuffer[TWI_MessageBufferCurIndex].u32Size;
      TWI0->pu8RxBuffer = TWI_MessageBuffer[TWI_MessageBufferCurIndex].pu8RxBuffer;
      
      if(TWI_u32CurrentBytesRemaining == 1)
      {
        /* Start and Stop need to be set at same time */
        TWI0->pBaseAddress->TWI_CR |= (_TWI_CR_START_BIT | _TWI_CR_STOP_BIT);
      }
      else
      {
        /* Just start bit, stop will be handled by interrupt */
        TWI0->pBaseAddress->TWI_CR |= _TWI_CR_START_BIT;
      }
      
      /* Proceed to receiving state and set flag */
      TWI0->u32Flags |= _TWI_RECEIVING;
      TWI_StateMachine = TWISM_Receiving;
      
    }  
    
    /* Check for errors */
    if(TWI_u32Flags & TWI_ERROR_FLAG_MASK)
    {
      /* Reset peripheral parser and go to error state */
      TWI_StateMachine = TWISM_Error;
    }
  }
} /* end TWISM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit in progress until current bytes have reached 0.  On exit, the transmit message must be dequeued.
*/
void TWISM_Transmitting(void)
{
  /* Check if a stop condition has been requested */
  if(TWI_MessageBuffer[TWI_MessageBufferCurIndex].Stop == STOP)
  {
    /* Check if all of the message bytes have completely finished sending and transmission complete */
    if( (TWI_u32CurrentBytesRemaining == 0) && 
        (TWI0->pBaseAddress->TWI_SR & _TWI_SR_TXRDY) &&
        (TWI0->pBaseAddress->TWI_SR & _TWI_SR_TXCOMP) )
    {
      /*  Clear flags */
      TWI0->u32Flags &= ~(_TWI_TRANSMITTING | _TWI_TRANS_NOT_COMP);
    }
  }
  else
  {
    /* Check if all of the message bytes have completely finished sending */
    if( (TWI_u32CurrentBytesRemaining == 0) && 
        (TWI0->pBaseAddress->TWI_SR & _TWI_SR_TXRDY) )
    { 
      /* Clear flag */
      TWI0->u32Flags &= ~_TWI_TRANSMITTING;
    }
  }
  
  if( !(TWI0->u32Flags & _TWI_TRANSMITTING) )
  {
    /* Update the status queue and then dequeue the message */
    UpdateMessageStatus(TWI0->pTransmitBuffer->u32Token, COMPLETE);
    DeQueueMessage(&TWI0->pTransmitBuffer);
    
    /* Make sure _TWI_INIT_MODE flag is clear in case this was a manual cycle */
    TWI_u32Flags &= ~_TWI_INIT_MODE;
    TWI_StateMachine = TWISM_Idle;
    
    /* Update queue pointers */
    TWI_MessageBufferCurIndex++;
    TWI_MessageQueueLength--;
    if(TWI_MessageBufferCurIndex == TX_QUEUE_SIZE)
    {
      TWI_MessageBufferCurIndex = 0;
    }
  }
  
  /* Check for errors */
  if(TWI_u32Flags & TWI_ERROR_FLAG_MASK)
  {
    /* Reset peripheral parser and go to error state */
    TWI_StateMachine = TWISM_Error;
  }
  
} /* end TWISM_Transmitting() */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Receive in progress until current bytes have reached 0.
*/
void TWISM_Receiving(void)
{
  if( (TWI_u32CurrentBytesRemaining == 0) &&
      (TWI0->pBaseAddress->TWI_SR & _TWI_SR_RXRDY) &&
      (TWI0->pBaseAddress->TWI_SR & _TWI_SR_TXCOMP) )
  {
    /* Clear flag */
    TWI0->u32Flags &= ~_TWI_RECEIVING;
    
    /* Make sure _TWI_INIT_MODE flag is clear in case this was a manual cycle */
    TWI_u32Flags &= ~_TWI_INIT_MODE;
    TWI_StateMachine = TWISM_Idle;
    
    /* Update queue pointers */
    TWI_MessageBufferCurIndex++;
    TWI_MessageQueueLength--;
    if(TWI_MessageBufferCurIndex == TX_QUEUE_SIZE)
    {
      TWI_MessageBufferCurIndex = 0;
    }
  }
  
  /* Check for errors */
  if(TWI_u32Flags & TWI_ERROR_FLAG_MASK)
  {
    /* Reset peripheral parser and go to error state */
    TWI_StateMachine = TWISM_Error;
  }  
  
}

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
void TWISM_Error(void)          
{
  /* NACK recieved */
  if( TWI_u32Flags & _TWI_ERROR_NACK )
  {
    /* Msg attempted too many times */
    if( ++TWI_MessageBuffer[TWI_MessageBufferNextIndex].u8Attempts == MAX_ATTEMPTS )
    {
      /* Remove the message from buffer queue */
      TWI_MessageBufferCurIndex++;
      TWI_MessageQueueLength--;
      if(TWI_MessageBufferCurIndex == TX_QUEUE_SIZE)
      {
        TWI_MessageBufferCurIndex = 0;
      }
      
      if( TWI0->u32Flags & _TWI_TRANSMITTING )
      {
        /* Dequeue Msg and Update Status */ 
        UpdateMessageStatus(TWI0->pTransmitBuffer->u32Token, ABANDONED);
        DeQueueMessage(&TWI0->pTransmitBuffer);
      }
    }

    /* Reset the msg flags */
    TWI0->u32Flags = 0;
  }
  
  /* Return to Idle */
  TWI_u32Flags &= ~TWI_ERROR_FLAG_MASK;
  TWI_StateMachine = TWISM_Idle;
  
} /* end TWISM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/