/*!**********************************************************************************************************************
@file sam3u_i2c.c                                                                
@brief Provides a driver to use TWI0 (I²C) peripheral to send and receive data using interrupts.

Currently Set at - 200kHz Master Mode.
This is a simpler version of a serial system driver that does not use resource control
through Request() and Release() calls

All of these functions return a value that should be checked to ensure the operation will be completed

Both TWI0ReadByte and TWI0ReadData require that pu8RxBuffer is large enough to hold the data
As well it is assumed, that since you know the amount of data to be sent, a stop can be sent
when all bytes have been received (and not tie the data and clock line low).

WriteByte and WriteData have the option to hold the lines low as it waits for more data 
to be queued. If a stop condition is not sent only Writes can follow until a stop condition is
requested (as the current transmission isn't complete).

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- G_u32Twi0ApplicationFlags

CONSTANTS
- NONE

TYPES
- SspBitOrderType
- SspModeType
- SspRxStatusType
- SspConfigurationType
- SspPeripheralType

PUBLIC FUNCTIONS
bool TWI0ReadByte(u8 u8SlaveAddress_, u8* pu8RxBuffer_);
bool TWI0ReadData(u8 u8SlaveAddress_, u8* pu8RxBuffer_, u32 u32Size_);
u32 TWIWriteByte(TwiPeripheralType* psTWIPeripheral_, u8 u8Byte_, TwiStopType eSend_);
u32 TWIWriteData(TwiPeripheralType* psTWIPeripheral_, u32 u32Size_, u8* u8Data_, TwiStopType eSend_);

PROTECTED FUNCTIONS
- void SspInitialize(void)
- void SspRunActiveState(void)
- void SspManualMode(void)
- void SSP0_IRQHandler(void)


**********************************************************************************************************************/

#include "configuration.h"


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>Twi"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32Twi0ApplicationFlags;          /*!< @brief Status flags meant for application using this peripheral */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;          /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;           /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;            /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;       /*!< @brief From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "TWI_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type TWI_pfnStateMachine;           /*!< @brief The application state machine */

static u32 TWI_u32Timer;                          /*!< @brief Timeout counter used across states */
static u32 TWI_u32Flags;                          /*!< @brief Application flags */

static TwiPeripheralType TWI_Peripheral0;         /*!< @brief TWI0 peripheral object */

static u32 TWI_u32CurrentBytesRemaining;                        /*!< @brief Counter for number of bytes being clocked out */
static u8* TWI_pu8CurrentTxData;                                /*!< @brief Pointer to current message being clocked out */
static TwiMessageQueueType TWI_MessageBuffer[U8_TX_QUEUE_SIZE]; /*!< @brief Ccircular buffer that stores queued msgs stop condition */
static u8 TWI_u8MsgBufferNextIndex;                             /*!< @brief Next position to place a message */
static u8 TWI_u8MsgBufferCurrentIndex;                          /*!< @brief Current message that is being processed */
static u8 TWI_u8MsgQueueLength;                                 /*!< @brief Counter to track the number of messages stored in the queue */


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!---------------------------------------------------------------------------------------------------------------------
@fn bool TWI0ReadByte(u8 u8SlaveAddress_, u8* pu8RxBuffer_)

@brief Queues a TWI Read Message into TWI_MessageBuffer, will be processed after all msgs queued before it
  - Single byte

Requires:
- Master mode

@param u8SlaveAddress_ holds the target's I²C address
@param pu8RxBuffer_ has the space to save the data

Promises:
- Queues msg if there is space available
- Returns TRUE if successful queue

*/
bool TWI0ReadByte(u8 u8SlaveAddress_, u8* pu8RxBuffer_)
{
  if(TWI_u8MsgQueueLength == U8_TX_QUEUE_SIZE || (TWI_Peripheral0.u32PrivateFlags & _TWI_TRANS_NOT_COMP))
  {
    /* TWI Message Task Queue Full or the Tx transmit isn't complete */
    return FALSE;
  }
  else
  {
    /* Queue Relevant data for TWI register setup */
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].Direction   = TWI_READ;
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u32Size     = 1;
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u8Address   = u8SlaveAddress_;
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].pu8RxBuffer = pu8RxBuffer_;
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u8Attempts  = 0;
    
    /* Not used by Receive */
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].Stop = TWI_NA; 
     
    /* Update array indexers and size */
    TWI_u8MsgBufferNextIndex++;
    TWI_u8MsgQueueLength++;
    if(TWI_u8MsgBufferNextIndex == U8_TX_QUEUE_SIZE)
    {
      TWI_u8MsgBufferNextIndex = 0;
    }
    
    /* If the system is initializing, manually cycle the TWI task through one iteration
      to send the message */
    if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
    {
      TWIManualMode();
    }
    
    return TRUE;
  }

} /* end TWI0ReadByte() */


/*!--------------------------------------------------------------------------------------------------------------------
@fn bool TWI0ReadData(u8 u8SlaveAddress_, u8* pu8RxBuffer_, u32 u32Size_)

@brief Queues a TWI Read Message into TWI_MessageBuffer, will be processed after all msgs queued before it

Requires:
- Master mode

@param u8SlaveAddress_ holds the target's I²C address
@param pu8RxBuffer_ has the space to save the data
@param u32Size_ is the number of bytes to receive

Promises:
- Queues a multi byte command into the command array
- Returns TRUE if the queue was successful

*/
bool TWI0ReadData(u8 u8SlaveAddress_, u8* pu8RxBuffer_, u32 u32Size_)
{
  if(TWI_u8MsgQueueLength == U8_TX_QUEUE_SIZE || (TWI_Peripheral0.u32PrivateFlags & _TWI_TRANS_NOT_COMP))
  {
    /* TWI Message Task Queue Full or the Tx transmit isn't complete */
    return FALSE;
  }
  else
  {
    /* Queue Relevant data for TWI register setup */
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].Direction   = TWI_READ;
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u32Size     = u32Size_;
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u8Address   = u8SlaveAddress_;
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].pu8RxBuffer = pu8RxBuffer_;
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u8Attempts  = 0;
    
    /* Not used by Receive */
    TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].Stop = TWI_NA; 
    
    /* Update array indexers and size */
    TWI_u8MsgBufferNextIndex++;
    TWI_u8MsgQueueLength++;
    if(TWI_u8MsgBufferNextIndex == U8_TX_QUEUE_SIZE)
    {
      TWI_u8MsgBufferNextIndex = 0;
    }
    
    /* If the system is initializing, manually cycle the TWI task through one iteration
    to send the message */
    if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
    {
      TWIManualMode();
    }
    
    return TRUE;
  }
  
} /* end TWI0ReadData() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn u32 TWI0WriteByte(u8 u8SlaveAddress_, u8 u8Byte_, TwiStopType eSend_)

@brief Queues a single byte for transmit on TWI0 peripheral.  

Requires:
@param u8SlaveAddress_ holds the target's I²C address
@param u8Byte_ is the byte to send
@param eSend_ is the type of operation

Promises:
- Creates a 1-byte message at TWI_Peripheral0.pTransmitBuffer that will be sent by the TWI application
  when it is available.
- Returns the message token assigned to the message

*/
u32 TWI0WriteByte(u8 u8SlaveAddress_, u8 u8Byte_, TwiStopType eSend_)
{
  u32 u32Token;
  u8 u8Data = u8Byte_;
  
  if(TWI_u8MsgQueueLength == U8_TX_QUEUE_SIZE)
  {
    /* TWI Message Task Queue Full */
    return 0;
  }
  else
  {
    /* Queue Message in message system */
    u32Token = QueueMessage(&TWI_Peripheral0.pTransmitBuffer, 1, &u8Data);
    if(u32Token)
    {
      /* Queue Relevant data for TWI register setup */
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].Direction     = TWI_WRITE;
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u32Size       = 1;
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u8Address     = u8SlaveAddress_;
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].Stop          = eSend_;
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u8Attempts    = 0;
      
      /* Not used by Transmit */
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].pu8RxBuffer = NULL;
      
      /* Update array pointers and size */
      TWI_u8MsgBufferNextIndex++;
      TWI_u8MsgQueueLength++;
      if(TWI_u8MsgBufferNextIndex == U8_TX_QUEUE_SIZE)
      {
        TWI_u8MsgBufferNextIndex = 0;
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


/*!--------------------------------------------------------------------------------------------------------------------
@fn u32 TWI0WriteData(u8 u8SlaveAddress_, u32 u32Size_, u8* u8Data_, TwiStopType eSend_)

@brief Queues a data array for transfer on the TWI0 peripheral.  

Requires:
- if a transmission is in progress, the node in the buffer that is currently being sent will not 
  be destroyed during this function.

@param u8SlaveAddress_ holds the target's I²C address
@param u8Byte_ is the byte to send
@param eSend_ is the type of operation

Promises:
- adds the data message at TWI_Peripheral0.pTransmitBuffer buffer that will be sent by the TWI application
  when it is available.
- Returns the message token assigned to the message; 0 is returned if the message cannot be queued in which case
  G_u32MessagingFlags can be checked for the reason

*/
u32 TWI0WriteData(u8 u8SlaveAddress_, u32 u32Size_, u8* u8Data_, TwiStopType eSend_)
{
  u32 u32Token;
    
  if(TWI_u8MsgQueueLength >= U8_TX_QUEUE_SIZE)
  {
    DebugPrintf("TWI message too long/n/r");
    return 0;
  }
  else
  {
    /* Queue Message in message system */
    u32Token = QueueMessage(&TWI_Peripheral0.pTransmitBuffer, u32Size_, u8Data_);
    if(u32Token)
    {
      /* Queue Relevant data for TWI register setup */
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].Direction  = TWI_WRITE;
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u32Size    = 1;
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u8Address  = u8SlaveAddress_;
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].Stop       = eSend_;
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u8Attempts = 0;
      
      /* Not used by Transmit */
      TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].pu8RxBuffer = NULL;
      
      /* Update array pointers and size */
      TWI_u8MsgBufferNextIndex++;
      TWI_u8MsgQueueLength++;
      if(TWI_u8MsgBufferNextIndex == U8_TX_QUEUE_SIZE)
      {
        TWI_u8MsgBufferNextIndex = 0;
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
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void TWIInitialize(void)

@brief Initializes the TWI application and its variables. 

Requires:
- NONE 

Promises:
- TWI peripheral objects are ready 
- TWI application set to Idle

*/
void TWIInitialize(void)
{
  u32 u32TargetPerpipheralNumber = AT91C_ID_TWI0;
  
  AT91C_BASE_PMC->PMC_PCER |= (1<<u32TargetPerpipheralNumber);
  
  /* Init flags, indexes and globals */
  TWI_u32Flags = 0;
  
  TWI_u8MsgBufferNextIndex = 0;
  TWI_u8MsgBufferCurrentIndex = 0;
  TWI_u8MsgQueueLength = 0;
  
  TWI_u32CurrentBytesRemaining = 0;
  TWI_pu8CurrentTxData = NULL;
  
  /* Initialize the TWI peripheral structures */
  TWI_Peripheral0.pBaseAddress    = AT91C_BASE_TWI0;
  TWI_Peripheral0.pTransmitBuffer = NULL;
  TWI_Peripheral0.pu8RxBuffer     = NULL;
  TWI_Peripheral0.u32PrivateFlags        = 0;

  /* Software reset of peripheral */
  TWI_Peripheral0.pBaseAddress->TWI_CR |= AT91C_TWI_SWRST;
  TWI_u32Timer = G_u32SystemTime1ms;
  while( !IsTimeUp(&TWI_u32Timer, 5) );
  
  /* Configure Peripheral */
  TWI_Peripheral0.pBaseAddress->TWI_CWGR = TWI0_CWGR_INIT;
  TWI_Peripheral0.pBaseAddress->TWI_CR   = TWI0_CR_INIT;
  TWI_Peripheral0.pBaseAddress->TWI_MMR  = TWI0_MMR_INIT;
  TWI_Peripheral0.pBaseAddress->TWI_IER  = TWI0_IER_INIT;
  TWI_Peripheral0.pBaseAddress->TWI_IDR  = TWI0_IDR_INIT;
  
  /* Enable TWI interrupts */
  NVIC_ClearPendingIRQ( (IRQn_Type)u32TargetPerpipheralNumber );
  NVIC_EnableIRQ( (IRQn_Type)u32TargetPerpipheralNumber );

  /* Set application pointer */
  TWI_pfnStateMachine = TwiSM_Idle;
  
} /* end TWIInitialize() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void TWIRunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void TWIRunActiveState(void)
{
  TWI_pfnStateMachine();

} /* end TWIRunActiveState */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void TWIManualMode(void)

Description:
Runs a transmit cycle of the TWI application to clock out a message.  This function is used only during
initialization.

Requires:
  - TWI application has been initialized.

Promises:
  - All bytes currently in the TWI Rx FIFO are read out to the application receive circular buffer.
*/
void TWIManualMode(void)
{
  TWI_u32Flags |=_TWI_INIT_MODE;
  TWI_u32Timer  = G_u32SystemTime1ms;
  
  while(TWI_u32Flags &_TWI_INIT_MODE)
  {
    TWI_pfnStateMachine();
    while( !IsTimeUp(&TWI_u32Timer, 1) );
  }
      
} /* end TWIManualMode() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void TWI0_IrqHandler(void)

@brief Handles the TWI0 Peripheral interrupts

Requires:
- NONE

Promises:
- More data has been queued or error flag raised
- Or recieved byte is placed in desired buffer

*/
void TWI0_IrqHandler(void)
{
  u32 u32InterruptStatus;
  
  /* Grab active interrupts and compare with status */
  u32InterruptStatus = AT91C_BASE_TWI0->TWI_IMR;
  u32InterruptStatus &= AT91C_BASE_TWI0->TWI_SR;
  
  /* NACK Received */
  if(u32InterruptStatus & AT91C_TWI_NACK_MASTER )
  {
    /* Error has occurred, reset the msg */
    TWI_u32Flags |= _TWI_ERROR_NACK;
  }


  /* Receiving Bytes */
  if(u32InterruptStatus & AT91C_TWI_RXRDY && ( TWI_Peripheral0.u32PrivateFlags & _TWI_RECEIVING ) )
  {
    *TWI_Peripheral0.pu8RxBuffer = TWI_Peripheral0.pBaseAddress->TWI_RHR;
    TWI_Peripheral0.pu8RxBuffer++;
    TWI_u32CurrentBytesRemaining--;
    
    if(TWI_u32CurrentBytesRemaining == 1)
    {
      TWI_Peripheral0.pBaseAddress->TWI_CR |= AT91C_TWI_STOP;
    }
  }
  
  
  /* Transmitting Bytes */
  if(u32InterruptStatus & AT91C_TWI_TXRDY_MASTER && ( TWI_Peripheral0.u32PrivateFlags & _TWI_TRANSMITTING ) )
  {
    /* There is more data queued and peripheral ready */
    TWI0FillTxBuffer();
  }
  
} /* end TWI0_IrqHandler() */


/*----------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*----------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn static void TWI0FillTxBuffer(void)

@brief Fills the TWI peripheral buffer with bytes from the current messsage that is sending.  
This function can be called from the TWI ISR!

If the implemented processor does not have a FIFO, this function can still be used but will only ever
add one byte to the transmitter.

Requires:
- The TxBuffer is empty
- TWI_pu8CurrentTxData is the index of the next byte in the message to be sent
- TWI_u32CurrentBytesRemaining has an accurate count of the bytes remaining in the message data to be sent
- Transmit interrupts are off

Promises:
- Data from *TWI_pu8CurrentTxData is added to the TWI peripheral Tx FIFO until the FIFO is full or there
  is no more data to send.

*/
static void TWI0FillTxBuffer(void)
{
  u8 u8ByteCount = U8_TWI_TX_FIFO_SIZE;
  
  /* Use the active global variables to fill up the transmit FIFO */
  while( (u8ByteCount != 0) && (TWI_u32CurrentBytesRemaining != 0) )
  {
    TWI_Peripheral0.pBaseAddress->TWI_THR = *TWI_pu8CurrentTxData;
    TWI_pu8CurrentTxData++;
    TWI_u32CurrentBytesRemaining--;
    u8ByteCount--;
  }
  
  /* If there are no remaining bytes to load to the TX FIFO, disable the TWI transmit 
  FIFO empty interrupt */
  if(TWI_u32CurrentBytesRemaining == 0)
  {
    TWI_Peripheral0.pBaseAddress->TWI_IDR = AT91C_TWI_TXRDY_MASTER;
    if(TWI_MessageBuffer[TWI_u8MsgBufferCurrentIndex].Stop == TWI_STOP)
    {
      TWI_Peripheral0.pBaseAddress->TWI_CR |= AT91C_TWI_STOP;
    }
  }
  /* Otherwise make sure transmit interrupts are enabled */
  else
  {
    TWI_Peripheral0.pBaseAddress->TWI_IER = AT91C_TWI_TXRDY_MASTER;
  }
  
} /* end TWI0FillTxBuffer() */


/***********************************************************************************************************************
State Machine Function Definitions
***********************************************************************************************************************/

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_Idle(void)
@brief Wait for a message to be queued 
*/
static void TwiSM_Idle(void)
{
  if(TWI_u8MsgBufferNextIndex != TWI_u8MsgBufferCurrentIndex )
  {
    TWI_Peripheral0.pBaseAddress->TWI_MMR = TWI0_MMR_INIT;
    TWI_Peripheral0.pBaseAddress->TWI_CR = TWI0_CR_INIT;
    
    if(TWI_MessageBuffer[TWI_u8MsgBufferCurrentIndex].Direction == TWI_WRITE)
    {
      /* insert new address */
      TWI_Peripheral0.pBaseAddress->TWI_MMR |= 
          ((TWI_MessageBuffer[TWI_u8MsgBufferCurrentIndex].u8Address << TWI_MMR_ADDRESS_SHIFT));
      
      /* Set up to transmit the message */
      TWI_u32CurrentBytesRemaining = TWI_Peripheral0.pTransmitBuffer->u32Size;
      TWI_pu8CurrentTxData = TWI_Peripheral0.pTransmitBuffer->pu8Message;
      TWI_Peripheral0.u32PrivateFlags |= (_TWI_TRANSMITTING | _TWI_TRANS_NOT_COMP);
      TWI0FillTxBuffer();    
      
      /* Update the message's status */
      UpdateMessageStatus(TWI_Peripheral0.pTransmitBuffer->u32Token, SENDING);
  
      /* Proceed to next state to let the current message send */
      TWI_pfnStateMachine = TwiSM_Transmitting;
    }
    else if(TWI_MessageBuffer[TWI_u8MsgBufferCurrentIndex].Direction == TWI_READ)
    {
      /* insert new address and set Read bit */
      TWI_Peripheral0.pBaseAddress->TWI_MMR |= 
        ( ((TWI_MessageBuffer[TWI_u8MsgBufferCurrentIndex].u8Address) << TWI_MMR_ADDRESS_SHIFT) & AT91C_TWI_MREAD);
      
      /* Grab number of desired bytes and the pointer to store the buffer */
      TWI_u32CurrentBytesRemaining = TWI_MessageBuffer[TWI_u8MsgBufferCurrentIndex].u32Size;
      TWI_Peripheral0.pu8RxBuffer = TWI_MessageBuffer[TWI_u8MsgBufferCurrentIndex].pu8RxBuffer;
      
      if(TWI_u32CurrentBytesRemaining == 1)
      {
        /* Start and Stop need to be set at same time */
        TWI_Peripheral0.pBaseAddress->TWI_CR |= (AT91C_TWI_START | AT91C_TWI_STOP);
      }
      else
      {
        /* Just start bit, stop will be handled by interrupt */
        TWI_Peripheral0.pBaseAddress->TWI_CR |= AT91C_TWI_START;
      }
      
      /* Proceed to receiving state and set flag */
      TWI_Peripheral0.u32PrivateFlags |= _TWI_RECEIVING;
      TWI_pfnStateMachine = TwiSM_Receiving;
      
    }  
    
    /* Check for errors */
    if(TWI_u32Flags & TWI_ERROR_FLAG_MASK)
    {
      /* Reset peripheral parser and go to error state */
      TWI_pfnStateMachine = TwiSM_Error;
    }
  }
} /* end TwiSM_Idle() */
     

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_Transmitting(void)
@brief Transmit in progress until current bytes have reached 0.  On exit, the transmit message must be dequeued.
*/
static void TwiSM_Transmitting(void)
{
  /* Check if a stop condition has been requested */
  if(TWI_MessageBuffer[TWI_u8MsgBufferCurrentIndex].Stop == TWI_STOP)
  {
    /* Check if all of the message bytes have completely finished sending and transmission complete */
    if( (TWI_u32CurrentBytesRemaining == 0) && 
        (TWI_Peripheral0.pBaseAddress->TWI_SR & AT91C_TWI_TXRDY_MASTER) &&
        (TWI_Peripheral0.pBaseAddress->TWI_SR & AT91C_TWI_TXCOMP_MASTER) )
    {
      /*  Clear flags */
      TWI_Peripheral0.u32PrivateFlags &= ~(_TWI_TRANSMITTING | _TWI_TRANS_NOT_COMP);
    }
  }
  else
  {
    /* Check if all of the message bytes have completely finished sending */
    if( (TWI_u32CurrentBytesRemaining == 0) && 
        (TWI_Peripheral0.pBaseAddress->TWI_SR & AT91C_TWI_TXRDY_MASTER) )
    { 
      /* Clear flag */
      TWI_Peripheral0.u32PrivateFlags &= ~_TWI_TRANSMITTING;
    }
  }
  
  if( !(TWI_Peripheral0.u32PrivateFlags & _TWI_TRANSMITTING) )
  {
    /* Update the status queue and then dequeue the message */
    UpdateMessageStatus(TWI_Peripheral0.pTransmitBuffer->u32Token, COMPLETE);
    DeQueueMessage(&TWI_Peripheral0.pTransmitBuffer);
    
    /* Make sure _TWI_INIT_MODE flag is clear in case this was a manual cycle */
    TWI_u32Flags &= ~_TWI_INIT_MODE;
    TWI_pfnStateMachine = TwiSM_Idle;
    
    /* Update queue pointers */
    TWI_u8MsgBufferCurrentIndex++;
    TWI_u8MsgQueueLength--;
    if(TWI_u8MsgBufferCurrentIndex == U8_TX_QUEUE_SIZE)
    {
      TWI_u8MsgBufferCurrentIndex = 0;
    }
  }
  
  /* Check for errors */
  if(TWI_u32Flags & TWI_ERROR_FLAG_MASK)
  {
    /* Reset peripheral parser and go to error state */
    TWI_pfnStateMachine = TwiSM_Error;
  }
  
} /* end TwiSM_Transmitting() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_Receiving(void)
@brief Receive in progress until current bytes have reached 0.
*/
static void TwiSM_Receiving(void)
{
  if( (TWI_u32CurrentBytesRemaining == 0) &&
      (TWI_Peripheral0.pBaseAddress->TWI_SR & AT91C_TWI_RXRDY) &&
      (TWI_Peripheral0.pBaseAddress->TWI_SR & AT91C_TWI_TXCOMP_MASTER) )
  {
    /* Clear flag */
    TWI_Peripheral0.u32PrivateFlags &= ~_TWI_RECEIVING;
    
    /* Make sure _TWI_INIT_MODE flag is clear in case this was a manual cycle */
    TWI_u32Flags &= ~_TWI_INIT_MODE;
    TWI_pfnStateMachine = TwiSM_Idle;
    
    /* Update queue pointers */
    TWI_u8MsgQueueLength--;
    TWI_u8MsgBufferCurrentIndex++;
    if(TWI_u8MsgBufferCurrentIndex == U8_TX_QUEUE_SIZE)
    {
      TWI_u8MsgBufferCurrentIndex = 0;
    }
  }
  
  /* Check for errors */
  if(TWI_u32Flags & TWI_ERROR_FLAG_MASK)
  {
    /* Reset peripheral parser and go to error state */
    TWI_pfnStateMachine = TwiSM_Error;
  }  
  
} /* end TwiSM_Receiving() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_Error(void)
@brief Handle an error 
*/
static void TwiSM_Error(void)          
{
  /* NACK recieved */
  if( TWI_u32Flags & _TWI_ERROR_NACK )
  {
    /* Msg attempted too many times */
    if( ++TWI_MessageBuffer[TWI_u8MsgBufferNextIndex].u8Attempts == U8_MAX_TWI_MSG_ATTEMPTS )
    {
      /* Remove the message from buffer queue */
      TWI_u8MsgBufferCurrentIndex++;
      TWI_u8MsgQueueLength--;
      if(TWI_u8MsgBufferCurrentIndex == U8_TX_QUEUE_SIZE)
      {
        TWI_u8MsgBufferCurrentIndex = 0;
      }
      
      if( TWI_Peripheral0.u32PrivateFlags & _TWI_TRANSMITTING )
      {
        /* Dequeue Msg and Update Status */ 
        UpdateMessageStatus(TWI_Peripheral0.pTransmitBuffer->u32Token, ABANDONED);
        DeQueueMessage(&TWI_Peripheral0.pTransmitBuffer);
      }
    }

    /* Reset the msg flags */
    TWI_Peripheral0.u32PrivateFlags = 0;
  }
  
  /* Return to Idle */
  TWI_u32Flags &= ~TWI_ERROR_FLAG_MASK;
  TWI_pfnStateMachine = TwiSM_Idle;
  
} /* end TwiSM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/