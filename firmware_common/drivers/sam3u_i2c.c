/*!**********************************************************************************************************************
@file sam3u_i2c.c                                                                
@brief MASTER ONLY.  Provides a driver to use TWI0 (IIC/I2C) peripheral to send and receive data using 
interrupts and PDC direct memory access.

Currently Set at 200kHz Master Mode.

Due to the nature of I2C use-cases, this driver does not require tasks to request and release it.
Read / write messages information is queued locally with all required details.  The driver will
continually cycle through the local message buffer and perform the reads or writes on a FIFO basis.
Read messages stand alone.  Write messages will have associated Message task messages.

Clock stretching is supported automatically by the peripheral in Master mode for both read and write.

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- G_u32Twi0ApplicationFlags

CONSTANTS
- NONE

TYPES
- TwiStopType
- TwiMessageType
- TwiPeripheralType
- TwiMessageQueueType

PUBLIC FUNCTIONS
- bool TwiReadByte(u8 u8SlaveAddress_, u8* pu8RxBuffer_)
- bool TwiReadData(u8 u8SlaveAddress_, u8* pu8RxBuffer_, u32 u32Size_)
- u32 TwiWriteByte(u8 u8SlaveAddress_, u8 u8Byte_, TwiStopType eSend_)
- u32 TwiWriteData(u8 u8SlaveAddress_, u32 u32Size_, u8* u8Data_, TwiStopType Send_)

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

static TwiMessageQueueType TWI_asMessageBuffer[U8_TWI_MSG_BUFFER_SIZE]; /*!< @brief Local circular buffer for TWI msgs */
static TwiMessageQueueType* TWI_psMsgBufferNext;                        /*!< @brief Next position to place a message */
static TwiMessageQueueType* TWI_psMsgBufferCurrent;                     /*!< @brief Current message that is being processed */
static u8 TWI_u8MsgQueueCount;                                          /*!< @brief Counter to track the number of messages in the queue */


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn bool TwiReadData(u8 u8SlaveAddress_, u8* pu8RxBuffer_, u32 u32Size_)

@brief Queues a TWI Read Message into TWI_asMessageBuffer

Read operations do not have an associated message in the Message task queue.

Requires:
- Master mode

@param u8SlaveAddress_ holds the target's I²C address
@param pu8RxBuffer_ has the space to save the data
@param u32Size_ is the number of bytes to receive

Promises:
- Queues a multi byte command into the command array
- Returns TRUE if the queue was successful

*/
bool TwiReadData(u8 u8SlaveAddress_, u8* pu8RxBuffer_, u32 u32Size_)
{
  if(TWI_u8MsgQueueCount == U8_TWI_MSG_BUFFER_SIZE)
  {
    /* TWI Message Task Queue Full or the Tx transmit isn't complete */
    return FALSE;
  }
  
  /* Critical section: TWI buffer management must be done with interrutps off since 
  an ISR can also manage the buffer values and pointers */
  __disable_irq();

  /* Queue Relevant data for TWI register setup */
  TWI_psMsgBufferNext->eDirection = TWI_READ;
  TWI_psMsgBufferNext->u32Size = u32Size_;
  TWI_psMsgBufferNext->u8Address = u8SlaveAddress_;
  TWI_psMsgBufferNext->pu8RxBuffer = pu8RxBuffer_;
  
  /* Stop condition type and message token do not apply for Rx */
  TWI_psMsgBufferNext->eStopType  = TWI_NA; 
  TWI_psMsgBufferNext->u32MessageTaskToken = 0;
      
  /* Update array indexers and size */
  TWI_u8MsgQueueCount++;
  TWI_psMsgBufferNext++;
  if( TWI_psMsgBufferNext == &TWI_asMessageBuffer[U8_TWI_MSG_BUFFER_SIZE] )
  {
    TWI_psMsgBufferNext = &TWI_asMessageBuffer[0];
  }
  
  /* Clear the new location to avoid confusion */
  TWI_psMsgBufferNext->eDirection = TWI_EMPTY;
  TWI_psMsgBufferNext->u32Size = 0;
  TWI_psMsgBufferNext->u8Address = 0;
  TWI_psMsgBufferNext->pu8RxBuffer = NULL;
  TWI_psMsgBufferNext->eStopType = TWI_NA; 
  TWI_psMsgBufferNext->u32MessageTaskToken = 0;

  /* End of critical section */
  __enable_irq();
    
  /* If the system is initializing, manually cycle the TWI task through one iteration to send the message */
  if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
  {
    TwiManualMode();
  }

  return TRUE;
  
} /* end TwiReadData() */


/*!--------------------------------------------------------------------------------------------------------------------
@fn u32 TwiWriteData(u8 u8SlaveAddress_, u32 u32Size_, u8* u8Data_, TwiStopType eSend_)

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
u32 TwiWriteData(u8 u8SlaveAddress_, u32 u32Size_, u8* u8Data_, TwiStopType eStop_)
{
  u32 u32Token;
    
  if(TWI_u8MsgQueueCount == U8_TWI_MSG_BUFFER_SIZE)
  {
    /* TWI Message Task Queue Full or the Tx transmit isn't complete */
    return 0;
  }

  /* Queue Message in message system */
  u32Token = QueueMessage(&TWI_Peripheral0.pTransmitBuffer, u32Size_, u8Data_);
  if(u32Token == 0)
  {
    /* TWI Message Task Queue Full or the Tx transmit isn't complete */
    return 0;
  }

  /* Critical section: TWI buffer management must be done with interrutps off since 
  an ISR can also manage the buffer values and pointers */
  __disable_irq();

  /* Queue Relevant data for TWI register setup */
  TWI_psMsgBufferNext->u32MessageTaskToken = u32Token;
  TWI_psMsgBufferNext->eDirection = TWI_WRITE;
  TWI_psMsgBufferNext->u32Size    = u32Size_;
  TWI_psMsgBufferNext->u8Address  = u8SlaveAddress_;
  TWI_psMsgBufferNext->eStopType  = eStop_; 
  
  /* Not used by Transmit */
  TWI_psMsgBufferNext->pu8RxBuffer = NULL;
  
  /* Update array pointers and size */
  TWI_u8MsgQueueCount++;
  TWI_psMsgBufferNext++;
  if( TWI_psMsgBufferNext == &TWI_asMessageBuffer[U8_TWI_MSG_BUFFER_SIZE] )
  {
    TWI_psMsgBufferNext = &TWI_asMessageBuffer[0];
  }

  /* Clear the new location to avoid confusion */
  TWI_psMsgBufferNext->eDirection  = TWI_EMPTY;
  TWI_psMsgBufferNext->u32Size     = 0;
  TWI_psMsgBufferNext->u8Address   = 0;
  TWI_psMsgBufferNext->pu8RxBuffer = NULL;
  TWI_psMsgBufferNext->eStopType   = TWI_NA; 
  TWI_psMsgBufferNext->u32MessageTaskToken = 0;

  /* End of critical section */
  __enable_irq();

  /* If the system is initializing, manually cycle the TWI task through one iteration to send the message */
  if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
  {
    TwiManualMode();
  }

  return(u32Token);
  
} /* end TwiWriteData() */


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void TwiInitialize(void)

@brief Initializes the TWI application and its variables. 

Requires:
- NONE 

Promises:
- TWI peripheral objects are ready 
- TWI application set to Idle

*/
void TwiInitialize(void)
{
  /* Enable the peripheral */
  AT91C_BASE_PMC->PMC_PCER |= (1 << AT91C_ID_TWI0);
  
  /* Init flags, pointers and globals */
  TWI_u32Flags = 0;
  TWI_psMsgBufferNext = TWI_asMessageBuffer;
  TWI_psMsgBufferCurrent = TWI_asMessageBuffer;
  TWI_u8MsgQueueCount = 0;
  
  /* Clear the local message buffer */
  for(u8 i = 0; i < U8_TWI_MSG_BUFFER_SIZE; i++)
  {
    TWI_asMessageBuffer[i].eDirection = TWI_EMPTY;
    TWI_asMessageBuffer[i].eStopType = TWI_NA;
    TWI_asMessageBuffer[i].pu8RxBuffer = NULL ;
    TWI_asMessageBuffer[i].u32MessageTaskToken = 0;
    TWI_asMessageBuffer[i].u32Size = 0;
    TWI_asMessageBuffer[i].u8Address = 0;
  }
   
  /* Initialize the TWI peripheral structures */
  TWI_Peripheral0.pBaseAddress    = AT91C_BASE_TWI0;
  TWI_Peripheral0.pTransmitBuffer = NULL;
  TWI_Peripheral0.u32PrivateFlags = 0;

  /* Software reset of peripheral */
  TWI_Peripheral0.pBaseAddress->TWI_CR = AT91C_TWI_SWRST;
  TWI_u32Timer = G_u32SystemTime1ms;
  while( !IsTimeUp(&TWI_u32Timer, 1) );
  
  /* Configure Peripheral for Master mode */
  TWI_Peripheral0.pBaseAddress->TWI_CWGR = TWI0_CWGR_INIT;
  TWI_Peripheral0.pBaseAddress->TWI_CR   = TWI0_CR_INIT;
  TWI_Peripheral0.pBaseAddress->TWI_MMR  = TWI0_MMR_INIT;
  TWI_Peripheral0.pBaseAddress->TWI_IER  = TWI0_IER_INIT;
  TWI_Peripheral0.pBaseAddress->TWI_IDR  = ~TWI0_IER_INIT;
  
  /* Enable TWI interrupts */
  NVIC_ClearPendingIRQ( (IRQn_Type)AT91C_ID_TWI0 );
  NVIC_EnableIRQ( (IRQn_Type)AT91C_ID_TWI0 );

  /* Set application pointer */
  TWI_pfnStateMachine = TwiSM_Idle;
  
} /* end TwiInitialize() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void TwiRunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void TwiRunActiveState(void)
{
  TWI_pfnStateMachine();

} /* end TwiRunActiveState */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void TwiManualMode(void)

@brief Runs a transmit cycle of the TWI application to clock a message.  
This function is used only during initialization.

Requires:
- G_u32SystemFlags _SYSTEM_INITIALIZING is set
- TWI application has been initialized.

Promises:
- All pending messages sent
- TWI_u8MsgQueueCount = 0
    
*/
void TwiManualMode(void)
{
  u32 u32Timer;
  
  TWI_u32Flags |=_TWI_INIT_MODE;
  
  while(TWI_u32Flags &_TWI_INIT_MODE)
  {
    TWI_pfnStateMachine();
    MessagingRunActiveState();
    DebugRunActiveState();
    
    u32Timer = G_u32SystemTime1ms;
    while( !IsTimeUp(&u32Timer, 1) );
  }
      
} /* end TwiManualMode() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void TWI0_IrqHandler(void)

@brief Handles the TWI0 Peripheral interrupts

Requires:
- NONE

Promises:
- NACK: flags error, disables ENDTX and sets Error state
- ENDTX: disables interrupt & PDC, writes STOP (if applicable), and clears _TWI_TRANSMITTING
- ENDRX: disables interrupt & PDC and writes STOP

*/
void TWI0_IrqHandler(void)
{
  u32 u32InterruptStatus;
  
  /* Grab active interrupts and compare with status */
  u32InterruptStatus = AT91C_BASE_TWI0->TWI_IMR;
  u32InterruptStatus &= AT91C_BASE_TWI0->TWI_SR;
  
  /*** NACK Received (Master only) ***/
  if(u32InterruptStatus & AT91C_TWI_NACK_MASTER )
  {
    /* Error has occurred, abort the message */
    TWI_u32Flags |= _TWI_ERROR_NACK;
    TWI_Peripheral0.pBaseAddress->TWI_IDR = AT91C_TWI_ENDTX;
    TWI_Peripheral0.pBaseAddress->TWI_PTCR = AT91C_PDC_TXTDIS;
    TWI_pfnStateMachine = TwiSM_Error;
  }

  /*** ENDTX (transmit has finished) ***/
  if(u32InterruptStatus & AT91C_TWI_ENDTX )
  {
    /* Disable interrupt and PDC transfer */
    TWI_Peripheral0.pBaseAddress->TWI_IDR = AT91C_TWI_ENDTX;
    TWI_Peripheral0.pBaseAddress->TWI_PTCR = AT91C_PDC_TXTDIS;

    /* Set stop condition if multi-byte transfer */
    if( (TWI_Peripheral0.pTransmitBuffer->u32Size != 1) &&
        (TWI_psMsgBufferCurrent->eStopType == TWI_STOP) )
    {
      TWI_Peripheral0.pBaseAddress->TWI_CR = AT91C_TWI_STOP;
    }
    
    TWI_Peripheral0.u32PrivateFlags &= ~_TWI_TRANSMITTING;

  } /* end ENDTX handler */
  
  
  /*** ENDRX (receive has finished ALL BUT ONE bytes) ***/
  if(u32InterruptStatus & AT91C_TWI_ENDRX )
  {
    /* Disable interrupt and PDC transfer */
    TWI_Peripheral0.pBaseAddress->TWI_IDR = AT91C_TWI_ENDRX;
    TWI_Peripheral0.pBaseAddress->TWI_PTCR = AT91C_PDC_RXTDIS;

    /* Set stop condition and change states */
    TWI_Peripheral0.pBaseAddress->TWI_CR = AT91C_TWI_STOP;
    TWI_pfnStateMachine = TwiSM_ReceiveLastByte;

  } /* end ENDRX handler */
  
} /* end TWI0_IrqHandler() */


/*----------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*----------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
State Machine Function Definitions
***********************************************************************************************************************/

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_Idle(void)
@brief Wait for a message to be queued then process that message.
*/
static void TwiSM_Idle(void)
{
  u32 u32Byte;

  /* Do nothing unless new Tx or Rx messages have been queued */
  if(TWI_u8MsgQueueCount != 0)
  {
    if(TWI_psMsgBufferCurrent->eDirection == TWI_WRITE)
    {
      /* Check that the local buffer Message token matches the message queued
      and the transmit buffer */
      if(TWI_psMsgBufferCurrent->u32MessageTaskToken != TWI_Peripheral0.pTransmitBuffer->u32Token)
      {
        DebugPrintf("TWI transmit message out of sync!\n\r");
        TWI_Peripheral0.u32PrivateFlags |= _TWI_ERROR_TX_MSG_SYNC;
      }
      else
      {
        /* Update the message's status */
        UpdateMessageStatus(TWI_Peripheral0.pTransmitBuffer->u32Token, SENDING);

        /* Set up to transmit the message */
        TWI_Peripheral0.u32PrivateFlags |= (_TWI_TRANSMITTING | _TWI_TRANS_NOT_COMP);
        u32Byte = (TWI_psMsgBufferCurrent->u8Address) << TWI_MMR_ADDRESS_SHIFT;
        TWI_Peripheral0.pBaseAddress->TWI_MMR |= u32Byte; 

        /* Setup PDC and interrupts */
        TWI_Peripheral0.pBaseAddress->TWI_TPR = (u32)TWI_Peripheral0.pTransmitBuffer->pu8Message; 
        TWI_Peripheral0.pBaseAddress->TWI_TCR = TWI_Peripheral0.pTransmitBuffer->u32Size;

        /* Enable Tx interrupt and the transmitter (triggers THR load) */
        TWI_Peripheral0.pBaseAddress->TWI_IER = AT91C_TWI_ENDTX;
        TWI_Peripheral0.pBaseAddress->TWI_PTCR = AT91C_PDC_TXTEN;
             
        /* Single byte transfers need STOP immediately (if applicable) */
        if(TWI_Peripheral0.pTransmitBuffer->u32Size == 1)
        {
          /* Set up the stop condition immediately if applicable */
          if(TWI_psMsgBufferCurrent->eStopType == TWI_STOP)
          {
            TWI_Peripheral0.pBaseAddress->TWI_CR = AT91C_TWI_STOP;
          }
        }

        TWI_pfnStateMachine = TwiSM_Transmit;

      } /* end WRITE setup */
    } /* end TWI_WRITE */
    
    else if(TWI_psMsgBufferCurrent->eDirection == TWI_READ)
    {
      /* Set up for READ transaction */
      u32Byte = AT91C_TWI_MREAD | (TWI_psMsgBufferCurrent->u8Address << TWI_MMR_ADDRESS_SHIFT);
      TWI_Peripheral0.pBaseAddress->TWI_MMR |= u32Byte; 
      TWI_Peripheral0.u32PrivateFlags |= _TWI_RECEIVING;

      /* Set up to receive the message based on number of bytes */
      if(TWI_psMsgBufferCurrent->u32Size == 1)
      {
        /* Single byte direct receive (no PDC required) */
        TWI_Peripheral0.pBaseAddress->TWI_CR = (AT91C_TWI_START | AT91C_TWI_STOP);
        TWI_pfnStateMachine = TwiSM_ReceiveLastByte;
      }
      else
      {
        /* Multi-byte PDC-based receive */
        TWI_Peripheral0.pBaseAddress->TWI_RPR = (u32)TWI_psMsgBufferCurrent->pu8RxBuffer;
        TWI_Peripheral0.pBaseAddress->TWI_RCR = TWI_psMsgBufferCurrent->u32Size - 1;
        TWI_Peripheral0.pBaseAddress->TWI_IER = AT91C_TWI_ENDRX;
        TWI_Peripheral0.pBaseAddress->TWI_PTCR = AT91C_PDC_RXTEN;

        /* Trigger the peripheral to start */
        TWI_Peripheral0.pBaseAddress->TWI_CR = AT91C_TWI_START;

        /* Proceed to receiving state*/
        TWI_u32Timer = G_u32SystemTime1ms;
        TWI_pfnStateMachine = TwiSM_PdcReceive;
      }
    } /* end TWI_READ */ 
  } /* if(TWI_u8MsgQueueCount != 0) */  
  
} /* end TwiSM_Idle() */
     

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_Transmit(void)
@brief Transmit in progress until until ISR clears _TWI_TRANSMITTING.
*/
static void TwiSM_Transmit(void)
{
  /* Watch _TWI_TRANSMITTING to indicate transmit is complete */
  if( !(TWI_Peripheral0.u32PrivateFlags & _TWI_TRANSMITTING) )
  {
    /*  Clean up the Message task message */
    UpdateMessageStatus(TWI_Peripheral0.pTransmitBuffer->u32Token, COMPLETE);
    DeQueueMessage(&TWI_Peripheral0.pTransmitBuffer);

    
    /* Advance states depending on whether TXCOMP is expected */
    if(TWI_psMsgBufferCurrent->eStopType == TWI_STOP)
    {
      /* If a STOP condition is requested, need to wait for TXCOMP */
      TWI_pfnStateMachine = TwiSM_TxWaitComplete;
    }
    else
    {
      /* Otherwise leave the bus active */
      TWI_u32Timer = U8_NEXT_TRANSFER_DELAY_MS;
      TWI_pfnStateMachine = TwiSM_NextTransferDelay;
    }
  }
    
} /* end TwiSM_Transmit() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_TxWaitComplete(void)
@brief Optional state to wait for the TXCOMP bit to be set (indicates STOP condition
has been placed on the bus). Some Master transmit states will bypass this. */
static void TwiSM_TxWaitComplete(void)
{
  /* Wait for TX to complete */
  if(TWI_Peripheral0.pBaseAddress->TWI_SR & AT91C_TWI_TXCOMP_MASTER)
  {
    /*  Clear flags and advance states */
    TWI_Peripheral0.u32PrivateFlags &= ~_TWI_TRANS_NOT_COMP;
    
    TWI_u32Timer = U8_NEXT_TRANSFER_DELAY_MS;
    TWI_pfnStateMachine = TwiSM_NextTransferDelay;
  }
    
} /* end TwiSM_TxWaitComplete() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_PdcReceive(void)
@brief Receive in progress through PDC until 1 byte remains.
The ENDRX ISR is responsible for changing states to exit here.
*/
static void TwiSM_PdcReceive(void)
{
  if( IsTimeUp(&TWI_u32Timer, U32_RX_TIMEOUT_MS) )
  {
    TWI_u32Flags |= _TWI_ERROR_RX_TIMEOUT;
    TWI_pfnStateMachine = TwiSM_Error;
  }
     
} /* end TwiSM_PdcReceive() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_ReceiveLastByte(void)
@brief Wait for RXRDY on the last byte transfer.
*/
static void TwiSM_ReceiveLastByte(void)
{
  if( TWI_Peripheral0.pBaseAddress->TWI_SR & AT91C_TWI_RXRDY )
  {
    /* Read the final byte */
    *(TWI_psMsgBufferCurrent->pu8RxBuffer + TWI_psMsgBufferCurrent->u32Size - 1) =  TWI_Peripheral0.pBaseAddress->TWI_RHR;
    
    TWI_pfnStateMachine = TwiSM_ReceiveComplete;
  }
  
} /* end TwiSM_ReceiveLastByte() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_ReceiveComplete(void)
@brief Wait for final TXCOMP flag
*/
static void TwiSM_ReceiveComplete(void)
{
  if(TWI_Peripheral0.pBaseAddress->TWI_SR & AT91C_TWI_TXCOMP_MASTER)
  {   
    /* Clear RX flag and advance states */
    TWI_Peripheral0.u32PrivateFlags &= ~_TWI_RECEIVING;
    
    TWI_u32Timer = U8_NEXT_TRANSFER_DELAY_MS;
    TWI_pfnStateMachine = TwiSM_NextTransferDelay;
  }
     
} /* end TwiSM_ReceiveComplete() */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_NextTransferDelay(void)
@brief Provide a delay before next transfer starts then do final clean-up before Idle. 
*/
static void TwiSM_NextTransferDelay(void)          
{
  TWI_u32Timer--;
  
  if(TWI_u32Timer == 0)
  {
    /* Clean up the local message queue (interrupts off, so not critical) */
    TWI_u8MsgQueueCount--;
    TWI_psMsgBufferCurrent++;
    if(TWI_psMsgBufferCurrent == &TWI_asMessageBuffer[U8_TWI_MSG_BUFFER_SIZE])
    {
      TWI_psMsgBufferCurrent = &TWI_asMessageBuffer[0];
    }

    /* Make sure _TWI_INIT_MODE flag is clear if no more messages in case this was a manual cycle */
    if(TWI_u8MsgQueueCount == 0)
    {
      TWI_u32Flags &= ~_TWI_INIT_MODE;
    }

    TWI_pfnStateMachine = TwiSM_Idle;
  }
  
} /* TwiSM_NextTransferDelay */


/*!-------------------------------------------------------------------------------------------------------------------
@fn static void TwiSM_Error(void)
@brief Handle an error 
*/
static void TwiSM_Error(void)          
{
  /* NACK recieved (write only) */
  if(TWI_u32Flags & _TWI_ERROR_NACK)
  {
    /* Announce the error and clear flag */
    TWI_u32Flags &= ~_TWI_ERROR_NACK;
    DebugPrintNumber(TWI_Peripheral0.pTransmitBuffer->u32Token);
    DebugPrintf(" TWI NACK. Message deleted.\n\r");
    
    /* Clear flags and clean up the Message task message */
    UpdateMessageStatus(TWI_Peripheral0.pTransmitBuffer->u32Token, FAILED);
    DeQueueMessage(&TWI_Peripheral0.pTransmitBuffer);
    TWI_Peripheral0.u32PrivateFlags &= ~(_TWI_TRANSMITTING | _TWI_TRANS_NOT_COMP);
  }
  
  /* RX TIMEOUT (receive only) */
  if(TWI_u32Flags & _TWI_ERROR_RX_TIMEOUT)
  {
    DebugPrintf("TWI Rx Timeout. Message deleted.\n\r");
  }  

  /* Advance states */
  TWI_u32Timer = U8_NEXT_TRANSFER_DELAY_MS;
  TWI_pfnStateMachine = TwiSM_NextTransferDelay;

} /* end TwiSM_Error() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/