/**********************************************************************************************************************
File: LPC17xx_ssp.c                                                                

Description:
Provides a driver to use an SSP peripheral to send and receive data using interrupts.

API:
SspPeripheralType* SspRequest(SspConfigurationType* psSspConfig_)
void SspRelease(SspPeripheralType* psSspPeripheral_)

void SspAssertCS(SspPeripheralType* psSspPeripheral_)
void SspDeAssertCS(SspPeripheralType* psSspPeripheral_)

u32 SspWriteByte(SspPeripheralType* psSspPeripheral_, u8 u8Byte_)
u32 SspWriteData(SspPeripheralType* psSspPeripheral_, u32 u32Size_, u8* u8Data_)

u32 SspReadByte(SspPeripheralType* psSspPeripheral_)
u32 SspReadData(SspPeripheralType* psSspPeripheral_, u32 u32Size_)

INITIALIZATION (should take place in application's initialization function):
1. Create a variable of SspConfigurationType in your application and initialize it to the desired SSP peripheral,
the address of the receive buffer for the application and the size in bytes of the receive buffer.

2. Call SspRequest() with pointer to the configuration variable created in step 1.  The returned pointer is the
SspPeripheralType object created that will be used by your application.

3. If the application no longer needs the SSP resource, call SspRelease().  
Note: if multiple slaves share an SSP resource, SspRequest() and SspRelease() must be used frequently to
allow traffic to both devices. Since multi-slave buses will be using GPIOs for manual SSEL,
it is up to the calling application to deassert the correct SSEL GPIO and assert the new SSEL GPIO.

DATA TRANSFER:
1. Received bytes on the allocated peripheral will be dropped into the application's designated received
buffer.  The buffer is written circularly, with no provision to monitor bytes that are overwritten.  The 
application is responsible for processing all received data.  The application must provide its own parsing
pointer to read the receive buffer and properly wrap around.  This pointer will not be impacted by the interrupt
service routine that may add additional characters at any time.

2. Transmitted data is queued using one of two functions, SspWriteByte() and SspWriteData().  Once the data
is queued, it is sent as soon as possible.  Each SSP resource has a transmit queue, but only one SSP resource
will send data at any given time.  However, both SSP resources may receive data simultaneously.  Per the SPI protocol,
a receive byte is always read with every transmit byte.  Your application must process the received bytes
and determine if they are dummy bytes or useful data.

3. SSP traffic is always full duplex, but protocols are typically half duplex.  To receive
data requested from an SSP slave, call SspReadByte() for a single byte or SspReadData() for multiple
bytes.  These functions will automatically queue SSP_DUMMY bytes to transmit and activate the clock
to receive data into your application's receive buffer.

**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile fnCode_type G_SspStateMachine;        /* The SSP application state machine */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;        /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;         /* From board-specific source file */

extern volatile u32 G_u32SystemFlags;          /* From main.c */
extern volatile u32 G_u32ApplicationFlags;     /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "SSP_" and be declared as static.
***********************************************************************************************************************/
static u32 SSP_u32Timer;                     /* Timeout counter used across states */
u32 SSP_u32Flags;                                /* Application flags for SSP */

static SspPeripheralType SSP_Peripheral0;        /* SSP0 peripheral object */
static SspPeripheralType SSP_Peripheral1;        /* SSP1 peripheral object */

static SspPeripheralType* SSP_psCurrentSsp;      /* Current SSP peripheral being processed */
static u32 SSP_u32CurrentTxBytesRemaining;       /* Down counter for number of bytes being clocked out */
static u8* SSP_pu8CurrentTxData;                 /* Pointer to current message being clocked out */

static u8 SSP_au8Dummies[MAX_TX_MESSAGE_LENGTH]; /* Array of dummy bytes sent to receive bytes from a slave */


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: SspRequest

Description:
Requests access to an SSP resource.  If the resource is available, the transmit and receive parameters are set up
and the peripheral is made ready to use in the application.  

Requires:
  - SSP peripheral register initialization values in configuration.h must be set correctly; currently this does not support
    different SSP configurations for multiple slaves on the same bus - all peripherals on the bus must work with
    the same setup.
  - psSspConfig_ has the SSP peripheral number, address of the RxBuffer, and the RxBuffer size and the calling
    application is ready to start using the peripheral.

Promises:
  - Returns a pointer to the requested SSP peripheral object if the resource is available; otherwise returns NULL
  - Peripheral is enabled
  - Peripheral interrupts are enabled.
*/
SspPeripheralType* SspRequest(SspConfigurationType* psSspConfig_)
{
  SspPeripheralType* psRequestedSsp = &SSP_Peripheral0; 

  /* Set the peripheral pointer to the correct SSP resource */
  if(psSspConfig_->SspPeripheral == SSP1)
  {
    psRequestedSsp = &SSP_Peripheral1;
  }
  
  /* If the requested peripheral is already assigned, return NULL now */
  if(psRequestedSsp->u32Flags & _SSP_PERIPHERAL_BUSY)
  {
    return(NULL);
  }
  
  /* Apply the parameters if the resource is free */
  psRequestedSsp->pGpioAddress    = psSspConfig_->pGpioAddress;
  psRequestedSsp->u32Pin          = psSspConfig_->u32Pin;
  psRequestedSsp->pu8RxBuffer     = psSspConfig_->pu8RxBufferAddress;
  psRequestedSsp->u32RxBufferSize = psSspConfig_->u32RxBufferSize;
  psRequestedSsp->pu8RxNextByte   = psSspConfig_->pu8RxNextByte;
  psRequestedSsp->u32Flags       |= _SSP_PERIPHERAL_BUSY;
  
  /* Activate and configure the peripheral, enable interrupts */
  if(psSspConfig_->SspPeripheral == SSP0)
  {
    /* Turn peripheral power on */
    LPC_SC->PCONP |= _SSP0_PCONP_BIT;
    
    /* Set configuration registers */
    LPC_SSP0->CR0  = SSP0_CR0_INIT | psSspConfig_->SpiMode;
    LPC_SSP0->CR1  = SSP0_CR1_INIT;
    LPC_SSP0->CPSR = SSP0_CPSR_INIT;
    LPC_SSP0->IMSC = SSP0_IMSC_INIT;

    /* Enable SSP0 interrupts */
    NVIC->ICPR[0] = _SSP0_NVIC_BIT;
    NVIC->ISER[0] = _SSP0_NVIC_BIT;
  }
  else
  {
    /* Turn peripheral power control on */
    LPC_SC->PCONP |= _SSP1_PCONP_BIT;
    
    /* Set configuration registers */
    LPC_SSP1->CR0  = SSP1_CR0_INIT | psSspConfig_->SpiMode;
    LPC_SSP1->CR1  = SSP1_CR1_INIT;
    LPC_SSP1->CPSR = SSP1_CPSR_INIT;
    LPC_SSP1->IMSC = SSP1_IMSC_INIT;

    /* Enable SSP1 interrupts */
    NVIC->ICPR[0] = _SSP1_NVIC_BIT;
    NVIC->ISER[0] = _SSP1_NVIC_BIT;
  }
  
  return(psRequestedSsp);
  
} /* end SspRequest() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspRelease

Description:
Releases an SSP resource.  

Requires:
  - psSspPeripheral_ has the SSP peripheral number, address of the RxBuffer, and the RxBuffer size and the calling
    application is ready to start using the peripheral.

Promises:
  - Resets peripheral object's pointers and data to safe values
  - Peripheral is disabled
  - Peripheral interrupts are disabled.
*/
void SspRelease(SspPeripheralType* psSspPeripheral_)
{
  /* Check to see if the peripheral is already released */
  if(psSspPeripheral_->pu8RxBuffer == NULL)
  {
    return;
  }

  /* Disable interrupts, deactivate the peripheral */
  if(psSspPeripheral_->pBaseAddress == LPC_SSP0)
  {
    NVIC->ICER[0]  = _SSP0_NVIC_BIT;
    LPC_SC->PCONP &= ~_SSP0_PCONP_BIT;
  }
  else
  {
    NVIC->ICER[0]  = _SSP1_NVIC_BIT;
    LPC_SC->PCONP &= ~_SSP1_PCONP_BIT;
  }

  /* Now it's safe to release all of the resources in the target peripheral */
  psSspPeripheral_->pu8RxBuffer     = NULL;
  psSspPeripheral_->u32RxBufferSize = 0;
  psSspPeripheral_->pu8RxNextByte   = NULL;
  psSspPeripheral_->u32Flags        = 0;

  /* Empty the transmit buffer if there were leftover messages */
  while(psSspPeripheral_->pTransmitBuffer != NULL)
  {
    UpdateMessageStatus(psSspPeripheral_->pTransmitBuffer->u32Token, ABANDONED);
    DeQueueMessage(&psSspPeripheral_->pTransmitBuffer);
  }
  
  /* Ensure the SM is in the Idle state */
  G_SspStateMachine = SspIdle;
  
} /* end SspRelease() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspAssertCS

Description:
Asserts the chip select line for the SSP perihperal  

Requires:
  - psSspPeripheral_ has been requested 

Promises:
  - the CS line for the requested SSP peripheral is asserted
*/
void SspAssertCS(SspPeripheralType* psSspPeripheral_)
{
  psSspPeripheral_->pGpioAddress->FIOCLR = psSspPeripheral_->u32Pin;
    
} /* end SspAssertCS() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspDeAssertCS

Description:
Deasserts the chip select line for the SSP perihperal  

Requires:
  - psSspPeripheral_ has been requested 

Promises:
  - the CS line for the requested SSP peripheral is asserted
*/
void SspDeAssertCS(SspPeripheralType* psSspPeripheral_)
{
  psSspPeripheral_->pGpioAddress->FIOSET = psSspPeripheral_->u32Pin;
    
} /* end SspDeAssertCS() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspWriteByte

Description:
Queues a single byte for transfer on the target SSP peripheral.  

Requires:
  - psSspPeripheral_ has been requested.
  - The chip select line of the SSP device should be asserted

Promises:
  - Creates a 1-byte message at psSspPeripheral_->pTransmitBuffer that will be sent by the SSP application
    when it is available.
  - Returns the message token assigned to the message
*/
u32 SspWriteByte(SspPeripheralType* psSspPeripheral_, u8 u8Byte_)
{
  u32 u32Token;
  u8 u8Data = u8Byte_;
  
  u32Token = QueueMessage(1, &u8Data, &psSspPeripheral_->pTransmitBuffer);
  if(u32Token)
  {
    /* If the system is initializing, we want to manually cycle the SSP task through one iteration
    to send the message */
    if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
    {
      SspManualMode();
    }
  }
  
  return(u32Token);
  
} /* end SspWriteByte() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspWriteData

Description:
Queues a data array for transfer on the target SSP peripheral.  

Requires:
  - psSspPeripheral_ has been requested and holds a valid pointer to a transmit buffer; even if a transmission is
    in progress, the node in the buffer that is currently being sent will not be destroyed during this function.
  - The chip select line of the SSP device should be asserted
  - u32Size_ is the number of bytes in the data array
  - u8Data_ points to the first byte of the data array

Promises:
  - adds the data message at psSspPeripheral_->pTransmitBuffer that will be sent by the SSP application
    when it is available.
  - Returns the message token assigned to the message; 0 is returned if the message cannot be queued in which case
    G_u32MessagingFlags can be checked for the reason
*/
u32 SspWriteData(SspPeripheralType* psSspPeripheral_, u32 u32Size_, u8* pu8Data_)
{
  u8 u8FullCycles;
  u8 u8Remainder;
  u32 u32Index = 0;
  u32 u32Token;

  u8FullCycles = u32Size_ / MAX_TX_MESSAGE_LENGTH;
  u8Remainder  = u32Size_ % MAX_TX_MESSAGE_LENGTH;
  
  /* Slice up the data to the allowable message size */
  for(u8 i = 0; i < u8FullCycles; i++)
  {
    u32Token = QueueMessage(MAX_TX_MESSAGE_LENGTH, pu8Data_ + u32Index, &psSspPeripheral_->pTransmitBuffer);
    if(!u32Token)
    {
      return(0);
    }
    
    /* Only update the index if pu8Data_ isn't pointed at the dummy array since that data should be repeated */
    if(pu8Data_ != &SSP_au8Dummies[0])
    {
      u32Index += MAX_TX_MESSAGE_LENGTH;
    }

    /* If the system is initializing, manually cycle the SSP task through one iteration to send the message */
    if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
    {
      SspManualMode();
    }
  }

  /* Complete the remaining bytes and assign the token to this message */  
  u32Token = QueueMessage(u8Remainder, pu8Data_ + u32Index, &psSspPeripheral_->pTransmitBuffer);
  if(u32Token)
  {
    if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
    {
      SspManualMode();
    }
  }
  
  return(u32Token);

} /* end SspWriteData() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspReadByte

Description:
Gets a single byte from the slave on the target SSP peripheral.  

Requires:
  - psSspPeripheral_ has been requested.
  - The chip select line of the SSP device should be asserted
  - 

Promises:
  - Creates a message with one SSP_DUMMY_BYTE at psSspPeripheral_->pTransmitBuffer that will be sent by the SSP application
    when it is available and thus clock in a received byte to the target receive buffer.
  - Returns the Token of the transmitted dummy message used to read data.

*/
u32 SspReadByte(SspPeripheralType* psSspPeripheral_)
{
  return( SspWriteByte(psSspPeripheral_, SSP_DUMMY_BYTE) );

} /* end SspReadByte() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspReadData

Description:
Gets multiple bytes from the slave on the target SSP peripheral.  

Requires:
  - psSspPeripheral_ has been requested and holds a valid pointer to a transmit buffer; even if a transmission is
    in progress, the node in the buffer that is currently being sent will not be destroyed during this function.
  - The chip select line of the SSP device should be asserted
  - u32Size_ is the number of bytes in the data array

Promises:
  - Adds a dummy byte message at psSspPeripheral_->pTransmitBuffer that will be sent by the SSP application
    when it is available and thus clock in the received bytes to the designated Rx buffer.
  - Returns the message token of the dummy message used to read data
*/
u32 SspReadData(SspPeripheralType* psSspPeripheral_, u32 u32Size_)
{
  return( SspWriteData(psSspPeripheral_, u32Size_, &SSP_au8Dummies[0]) );
    
} /* end SspReadData() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: SspInitialize

Description:
Initializes the SSP application and its variables.  The peripherals themselves are not configured until
requested by a calling application.

Requires:
  - 

Promises:
  - SSP peripheral objects are ready 
  - SSP application set to Idle
*/
void SspInitialize(void)
{
  /* Initialize the SSP peripheral structures */
  SSP_Peripheral0.pBaseAddress    = LPC_SSP0;
  SSP_Peripheral0.pTransmitBuffer = NULL;
  SSP_Peripheral0.pu8RxBuffer     = NULL;
  SSP_Peripheral0.u32RxBufferSize = 0;
  SSP_Peripheral0.pu8RxNextByte   = NULL;
  SSP_Peripheral0.u32Flags        = 0;
  
  SSP_Peripheral1.pBaseAddress    = LPC_SSP1;
  SSP_Peripheral1.pTransmitBuffer = NULL;
  SSP_Peripheral1.pu8RxBuffer     = NULL;
  SSP_Peripheral1.u32RxBufferSize = 0;
  SSP_Peripheral1.pu8RxNextByte  = NULL;
  SSP_Peripheral1.u32Flags        = 0;

  SSP_psCurrentSsp                = &SSP_Peripheral1;
  SSP_u32CurrentTxBytesRemaining  = 0;
  SSP_pu8CurrentTxData            = NULL;
  
  /* Fill the dummy array with SSP_DUMMY bytes */
  for (int i = 0; i < MAX_TX_MESSAGE_LENGTH; i++)
  {
    SSP_au8Dummies[i] = SSP_DUMMY_BYTE;
  }

  /* Set application pointer */
  G_SspStateMachine = SspIdle;

} /* end SspInitialize() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: SspFillTxBuffer

Description:
Fills the SSP peripheral buffer with bytes from the current messsage that is sending.  
This function can be called from the SSP ISR!

Requires:
  - psSspPeripheral_ indicates which SSP peripheral being used.  
  - SSP_pu8CurrentTxData points to the next byte in the message to be sent
  - SSP_u32CurrentTxBytesRemaining has an accurate count of the bytes remaining in the message data to be sent
  - Transmit interrupts are off

Promises:
  - Data from *SSP_pu8CurrentTxData is added to the SSP peripheral Tx FIFO until the FIFO is full or there
    is no more data to send.
*/
static void SspFillTxBuffer(SspPeripheralType* psSspPeripheral_) 
{
  /* Use the active global variables to fill up the transmit FIFO */
  while( (psSspPeripheral_->pBaseAddress->SR & _SSP_SR_TNF_BIT) && (SSP_u32CurrentTxBytesRemaining != 0) )
  {
    psSspPeripheral_->pBaseAddress->DR = *SSP_pu8CurrentTxData;  // Check for interrupt after this
    SSP_pu8CurrentTxData++;
    SSP_u32CurrentTxBytesRemaining--;
  }
    
  /* Disable the SSP transmit interrupt if there is no additional data to be queued */
  if(SSP_u32CurrentTxBytesRemaining == 0)
  {
    psSspPeripheral_->pBaseAddress->IMSC &= ~_SSP_TXMI_BIT;
  }
  /* Otherwise make sure transmit interrupts are enabled */
  else
  {
    psSspPeripheral_->pBaseAddress->IMSC |= _SSP_TXMI_BIT;
  }
  
} /* end SspFillTxBuffer() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspReadRxBuffer

Description:
Reads all bytes from the SSP peripheral Rx FIFO and places them in the application receive buffer.  
This function is only called from the SSP ISR so interrupts will be off.

Requires:
  - SSP_pu8CurrentTxData points to the next byte in the message to be sent
  - SSP_u32CurrentTxBytesRemaining has an accurate count of the bytes remaining in the message data to be sent
  - Transmit interrupts are off

Promises:
  - All bytes currently in the SSP Rx FIFO are read out to the application receive circular buffer.
*/
static void SspReadRxBuffer(SspPeripheralType* psTargetSsp_) 
{
  u8 u8Test;

  /* Read all the bytes in the Rx FIFO */
  while(psTargetSsp_->pBaseAddress->SR & _SSP_SR_RNE_BIT)
  {
    u8Test = psTargetSsp_->pBaseAddress->DR;
    **(psTargetSsp_->pu8RxNextByte) = u8Test; 

    /* Safely advance the pointer */
    (*psTargetSsp_->pu8RxNextByte)++;
    if( *psTargetSsp_->pu8RxNextByte >= ( psTargetSsp_->pu8RxBuffer + psTargetSsp_->u32RxBufferSize ) )
    {
      *psTargetSsp_->pu8RxNextByte = psTargetSsp_->pu8RxBuffer; 
    }
  }
      
} /* end SspReadRxBuffer() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspManualMode

Description:
Runs a transmit cycle of the SSP application to clock out a message.  This function is used only during
initialization.

Requires:
  - SSP application has been initialized.

Promises:
  - All bytes currently in the SSP Tx FIFO are sent out and thus all the expected received bytes
    are read into the application receive circular buffer.
*/
static void SspManualMode(void)
{
  SSP_u32Flags |=_SSP_INIT_MODE;
  SSP_u32Timer  = G_u32SystemTime1ms;
  
  while(SSP_u32Flags &_SSP_INIT_MODE)
  {
    G_SspStateMachine();
    IsTimeUp(&G_u32SystemTime1ms, &SSP_u32Timer, 1, RESET_TARGET_TIMER);
  }
      
} /* end SspManualMode() */


/*----------------------------------------------------------------------------------------------------------------------
Interrupt Service Routine: SSP0_IRQHandler

Description:
Handles the enabled SSP0 interrupts. 
Receive: The SSP peripheral is always enabled and ready to receive data.  Receive interrupts will occur when the SSP FIFO
is at least half full or is not empty but no further bytes have been received for a period of 32 bits.  All incoming data
is dumped into the circular receive data buffer configured in SSP_Peripheral0.
No processing is done on the data - it is up to the processing application to parse incoming data to find useful information
and to manage dummy bytes.

Transmit: All data bytes in the transmit buffer are 

Requires:
  - Transmit and receive buffers should be correctly configured 

Promises:
  - Any transmit data will be filled in the Tx FIFO
  - Any received data will be flushed to the receiver's circular buffer
*/

void SSP0_IRQHandler(void)
{
  static u32 u32IntCount = 0;
  
  /* TXIM Interrupt when Tx FIFO is at least half empty: write bytes into the buffer until the buffer is full
  or until there are no more bytes to write.  Adding to the Tx FIFO clears the interrupt flag */
  if(LPC_SSP0->MIS & _SSP_TXMIS_BIT)
  {
    SspFillTxBuffer(&SSP_Peripheral0);
  }

  /* RXIM Interrupt when Rx FIFO is at least half full: read all data bytes until buffer is empty */
  /* RTIM Interrupt when Rx FIFO is not empty and receive timeout occurs (32 bits): read data bytes until buffer is empty */
  if(LPC_SSP0->MIS & (_SSP_RXMIS_BIT | _SSP_RTMIS_BIT) )
  {
    LPC_SSP0->ICR |= _SSP_RTIC_BIT;
    SspReadRxBuffer(&SSP_Peripheral0);
  }
 
  /* RORIM Interrupt if Rx FIFO overruns: flag that a buffer overrun has occurred */
   if(LPC_SSP0->MIS & _SSP_RORMIS_BIT)
  {
    SSP_Peripheral0.u32Flags |= _SSP_RX_BUFFER_OVERRUN;
    LPC_SSP0->ICR |= _SSP_RORMIC_BIT;
    u32IntCount++;
  }

} /* end SSP0_IRQHandler() */


/*----------------------------------------------------------------------------------------------------------------------
Interrupt Service Routine: SSP1_IRQHandler

Description:
Handles the enabled SSP1 interrupts. 
Receive: The SSP peripheral is always enabled and ready to receive data.  Receive interrupts will occur when the SSP FIFO
is at least half full or is not empty but no further bytes have been received for a period of 32 bits.  All incoming data
is dumped into the circular receive data buffer configured in SSP_Peripheral1.
No processing is done on the data - it is up to the processing application to parse incoming data to find useful information
and to manage dummy bytes.

Transmit: All data bytes in the transmit buffer are 

Requires:
  - Transmit and receive buffers should be correctly configured 

Promises:
  - Any transmit data will be filled in the Tx FIFO
  - Any received data will be flushed to the receiver's circular buffer
*/

void SSP1_IRQHandler(void)
{
  
  /* TXIM Interrupt when Tx FIFO is at least half empty: write bytes into the buffer until the buffer is full
  or until there are no more bytes to write.  Adding to the Tx FIFO clears the interrupt flag */
  if(LPC_SSP1->MIS & _SSP_TXMIS_BIT)
  {
    SspFillTxBuffer(&SSP_Peripheral1);
  }

  /* RXIM Interrupt when Rx FIFO is at least half full: read all data bytes until buffer is empty */
  /* RTIM Interrupt when Rx FIFO is not empty and receive timeout occurs (32 bits): read data bytes until buffer is empty */
  if(LPC_SSP1->MIS & (_SSP_RXMIS_BIT | _SSP_RTMIS_BIT) )
  {
    SspReadRxBuffer(&SSP_Peripheral1);
  }
 
  /* RORIM Interrupt if Rx FIFO overruns: flag that a buffer overrun has occurred */
   if(LPC_SSP1->MIS & _SSP_RORMIS_BIT)
  {
    SSP_Peripheral1.u32Flags |= _SSP_RX_BUFFER_OVERRUN;
  }

} /* end SSP1_IRQHandler() */



/***********************************************************************************************************************
State Machine Function Definitions

The SSP state machine monitors messaging activity on the available SSP peripherals.  It manages outgoing messages and will
transmit any message that has been queued.  Only one message will be transferred at once (SSP0 and SSP1 will not both 
send at the same time, though both peripherals will continue to receive simultaneously).
Since all transmit and receive bytes are transferred using interrupts, the SM does not have to worry about prioritizing.

Transmit: 
With the SSEL bit under manual control, it does not matter if the transmit buffer starves for data because the manual
SSEL bit is not deasserted until the SM detects that the full message has been sent.

API (does message system support peripherals or vice-versa):
WriteByteSSP: Pass SspPeripheralType pointer and single byte
WriteDataSSP: Pass SspPeripheralType pointer, pointer to data and size.

***********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a transmit message to be queued.  Received data is handled in interrupts. */
void SspIdle(void)
{
  /* Update for next iteration */
  if(SSP_psCurrentSsp->pBaseAddress == LPC_SSP0)
  {
    SSP_psCurrentSsp = &SSP_Peripheral1;
  }
  else if(SSP_psCurrentSsp->pBaseAddress == LPC_SSP1)
  {
    SSP_psCurrentSsp = &SSP_Peripheral0;
  }
  else 
  {
    SSP_u32Flags |= _SSP_ERROR_INVALID_SSP;
  }

  /* Check if a message has been queued on the current SSP */
  if(SSP_psCurrentSsp->pTransmitBuffer != NULL)
  {
    /* Set up to transmit the message */
    SSP_u32CurrentTxBytesRemaining = SSP_psCurrentSsp->pTransmitBuffer->u32Size;
    SSP_pu8CurrentTxData = SSP_psCurrentSsp->pTransmitBuffer->pu8Message;
    SspFillTxBuffer(SSP_psCurrentSsp);

    /* Update the message's status */
    UpdateMessageStatus(SSP_psCurrentSsp->pTransmitBuffer->u32Token, SENDING);
    
   /* Proceed to next state to let the current message send */
    G_SspStateMachine = SspTransmitting;
  }
  
} /* end SspIdle() */

        
/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit in progress until current bytes have reached 0.  
Note that received data will be coming in the whole time (may be dummy bytes).
On exit, the transmit message must be dequeued.*/
void SspTransmitting(void)
{
  if( (SSP_u32CurrentTxBytesRemaining == 0) && 
      (SSP_psCurrentSsp->pBaseAddress->SR & _SSP_SR_TFE_BIT) &&
      (!(SSP_psCurrentSsp->pBaseAddress->SR & _SSP_SR_RNE_BIT)) )
  {
    /* Update the status queue and then dequeue the message */
    UpdateMessageStatus(SSP_psCurrentSsp->pTransmitBuffer->u32Token, COMPLETE);
    DeQueueMessage(&SSP_psCurrentSsp->pTransmitBuffer);

    /* Make sure _SSP_INIT_MODE flag is clear in case this was a manual cycle */
    SSP_u32Flags &= ~_SSP_INIT_MODE;
    G_SspStateMachine = SspIdle;
  }
    
} /* end SspIdle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
void SspError(void)          
{
  G_SspStateMachine = SspIdle;
  
} /* end SspError() */

        
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

