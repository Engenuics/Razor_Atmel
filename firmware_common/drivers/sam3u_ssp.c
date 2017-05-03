/**********************************************************************************************************************
File: sam3u_ssp.c                                                                

Description:
Provides a driver to use a USART peripheral in SPI/SSP mode to send and receive data using interrupts and peripheral DMA.
Note that Master SPI devices keep !CS as a GPIO so it can be managed manually to more easily work with 
the multitude of variants in slave device !CS requirements.

This driver should work for SPI slaves with or without flow control, though you may need to make adjustments
to how data is timed.  A slave with flow control requires callback functions to manage flow control lines.

If LSB first transmission is required, we can't use the DMA if we let the SSP task manage the bit flipping.
For high-traffic or low-power devices, you might consider flipping at the task level so that DMA
can be used (the bytes would have to be pre-flipped for transmit and post-flipped on receive).

API:
SspPeripheralType* SspRequest(SspConfigurationType* psSspConfig_)  BLADE_SSP
Request a SSP peripheral for your task.  No other tasks in the system will be able to access 
the specific peripheral you request.
e.g. MyTaskSsp = SspRequest(&MyTaskSspConfig);

void SspRelease(SspPeripheralType* psSspPeripheral_)
If your task is done using the SSP it requested, call this function to "give it back" to the system.
e.g. SspRelease(&MyTaskSsp);

u32 SspWriteByte(SspPeripheralType* psSspPeripheral_, u8 u8Byte_)
Write a single byte to the SSP.  A token corresponding to the message is returned if you want to monitor
if the byte sends correctly.
e.g u32CurrentMessageToken = SspWriteByte(&MyTaskSsp, u8ByteToWRite);

u32 SspWriteData(SspPeripheralType* psSspPeripheral_, u32 u32Size_, u8* u8Data_)
Write an array of bytes to the SSP.  Returns the message token for monitoring the status of the message.
e.g. 
u8 au8SData[] = {1, 2, 3, 4, 5, 6};
u32CurrentMessageToken = SspWriteData(&MyTaskSsp, sizeof(au8SData), au8Sting);

Master mode only:
u32 SspReadByte(SspPeripheralType* psSspPeripheral_)
Creates a dummy byte message of 1 byte to transmit and subsequently receive a byte. Returns the message token that can be monitored
to see when the message has been sent, and thus when the received byte should be in the pre-configured receive buffer.
e.g. u32CurrentMessageToken = SspReadByte(&MyTaskSsp);

u32 SspReadData(SspPeripheralType* psSspPeripheral_, u32 u32Size_)
Creates a dummy byte array to transmit and subsequently receive u32Size_ bytes. Returns the message token that can be monitored
to see when the message has been sent, and thus when the received data should be in the pre-configured receive buffer.
e.g. u32CurrentMessageToken = SspReadData(&MyTaskSsp, 10);


INITIALIZATION (should take place in application's initialization function):
1. Create a variable of SspConfigurationType in your application and initialize it to the desired SSP peripheral,
and the size & address of the receive buffer in the application.

2. Call SspRequest() with pointer to the configuration variable created in step 1.  The returned pointer is the
SspPeripheralType object created that will be used by your application.

3. If the application no longer needs the SSP resource, call SspRelease().  
Note: if multiple slaves share an SSP resource, SspRequest() and SspRelease() must be used frequently to
allow traffic to both devices. 

MASTER MODE DATA TRANSFER:
Transmitted data is queued using one of two functions, SspWriteByte() and SspWriteData() which both return a message
token unique to the data.  Once the data is queued, it is sent by the SSP as soon as possible.  Different SSP resources may 
transmit and receive data simultaneously.  Regardless, the SPI protocol always receives a byte with every transmitted
byte.  This may be a defined dummy byte, or it may be 0xFF or 0x00 depending on the idle state of the MISO line.
Your application must process the received bytes and determine if they are dummy bytes or useful data.

Received bytes on the allocated peripheral will be dropped into the application's designated receive
buffer.  The buffer is written circularly, with no provision to monitor bytes that are overwritten.  The 
application is responsible for processing all received data.  The application must provide its own parsing
pointer to read the receive buffer and properly wrap around.  This pointer will not be impacted by the interrupt
service routine that may add additional characters at any time.

SSP traffic is always full duplex, but protocols are typically half duplex.  To receive
data requested from an SSP slave, call SspReadByte() for a single byte or SspReadData() for multiple
bytes.  These functions will automatically queue SSP_DUMMY bytes to transmit and activate the clock
to receive data into your application's receive buffer.


SLAVE MODE DATA TRANSFER:
In Slave mode, the peripheral is always ready to receive bytes from the Master.  
Received bytes on the allocated peripheral will be dropped into the receive
buffer that the application specifies upon requesting the SPI peripheral.  The buffer 
is written circularly, with no provision to monitor bytes that are overwritten.  The 
application is responsible for processing all received data.  The application must 
provide its own parsing pointer to read the receive buffer and properly wrap around.  
This pointer will not be impacted by the interrupt service routine that may add additional 
characters at any time.

Transmitted data is queued using one of two functions, SspWriteByte() and SspWriteData().  Once the data
is queued, it is sent as soon as possible.  Different SSP resources may transmit and receive data simultaneously.  
Per the SPI protocol, a receive byte is always read with every transmit byte.  Your application must process the received bytes
and determine if they are dummy bytes or useful data.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>Ssp"
***********************************************************************************************************************/
/* New variables */
u32 G_u32Ssp0ApplicationFlags;                   /* Status flags meant for application using this SSP peripheral */
u32 G_u32Ssp1ApplicationFlags;                   /* Status flags meant for application using this SSP peripheral */
u32 G_u32Ssp2ApplicationFlags;                   /* Status flags meant for application using this SSP peripheral */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;          /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;           /* From board-specific source file */

extern volatile u32 G_u32SystemFlags;            /* From main.c */
extern volatile u32 G_u32ApplicationFlags;       /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "SSP_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Ssp_pfnStateMachine;          /* The SSP application state machine */

static u32 SSP_u32Timer;                         /* Timeout counter used across states */
static u32 SSP_u32Flags;                         /* Application flags for SSP */

static SspPeripheralType SSP_Peripheral0;        /* SSP0 peripheral object */
static SspPeripheralType SSP_Peripheral1;        /* SSP1 peripheral object */
static SspPeripheralType SSP_Peripheral2;        /* SSP2 peripheral object */

static SspPeripheralType* SSP_psCurrentSsp;      /* Current SSP peripheral being processed task */
static SspPeripheralType* SSP_psCurrentISR;      /* Current SSP peripheral being processed in ISR */
static u32* SSP_pu32SspApplicationFlagsISR;      /* Current SSP application status flags in ISR */

//static u8 SSP_au8Dummies[MAX_TX_MESSAGE_LENGTH]; /* Array of dummy bytes sent to receive bytes from a slave */

static u32 SSP_u32Int0Count = 0;                 /* Debug counter for SSP0 interrupts */
static u32 SSP_u32Int1Count = 0;                 /* Debug counter for SSP1 interrupts */
static u32 SSP_u32Int2Count = 0;                 /* Debug counter for SSP2 interrupts */
static u32 SSP_u32AntCounter = 0;                /* Debug counter */
static u32 SSP_u32RxCounter = 0;                 /* Debug counter */

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
and the peripheral is made ready to use in the application. The peripheral will be configured in different ways
for different SSP modes.  The following modes are supported:
SPI_MASTER: transmit and receive using peripheral DMA controller; transmit occurs through the Message API
SPI_SLAVE: transmit through Message Task; receive set up per-byte using current and next DMA pointers and managed into circular buffer.
SPI_SLAVE_FLOW_CONTROL:

Requires:
  - SSP peripheral register initialization values in configuration.h must be set correctly; currently this does not support
    different SSP configurations for multiple slaves on the same bus - all peripherals on the bus must work with
    the same setup.
  - psSspConfig_ has the SSP peripheral number, address of the RxBuffer and the RxBuffer size
  - the calling application is ready to start using the peripheral

Promises:
  - Returns a pointer to the requested SSP peripheral object if the resource is available; otherwise returns NULL
  - Peripheral is enabled
  - Peripheral interrupts are enabled.
*/
SspPeripheralType* SspRequest(SspConfigurationType* psSspConfig_)
{
  SspPeripheralType* psRequestedSsp; 
  u32 u32TargetCR, u32TargetMR, u32TargetIER, u32TargetIDR, u32TargetBRGR;

  /* Set the peripheral pointer to the correct resource */
  switch(psSspConfig_->SspPeripheral)
  {
    case USART0:
    {
      psRequestedSsp = &SSP_Peripheral0;
      
      u32TargetCR   = USART0_US_CR_INIT;
      u32TargetMR   = USART0_US_MR_INIT; 
      u32TargetIER  = USART0_US_IER_INIT; 
      u32TargetIDR  = USART0_US_IDR_INIT;
      u32TargetBRGR = USART0_US_BRGR_INIT;
      break;
    }
    case USART1:
    {
      psRequestedSsp = &SSP_Peripheral1;
      
      u32TargetCR   = USART1_US_CR_INIT;
      u32TargetMR   = USART1_US_MR_INIT; 
      u32TargetIER  = USART1_US_IER_INIT; 
      u32TargetIDR  = USART1_US_IDR_INIT;
      u32TargetBRGR = USART1_US_BRGR_INIT;
      break;
    }
    case USART2:
    {
      psRequestedSsp = &SSP_Peripheral2;
      
      u32TargetCR   = USART2_US_CR_INIT;
      u32TargetMR   = USART2_US_MR_INIT; 
      u32TargetIER  = USART2_US_IER_INIT; 
      u32TargetIDR  = USART2_US_IDR_INIT;
      u32TargetBRGR = USART2_US_BRGR_INIT;
      break;
    }
    
    default:
    {
      return(NULL);
    }
  } /* end switch */
  
  /* If the requested peripheral is already assigned, return NULL now */
  if(psRequestedSsp->u32PrivateFlags & _SSP_PERIPHERAL_ASSIGNED)
  {
    return(NULL);
  }

  /* Activate and configure the peripheral */
  AT91C_BASE_PMC->PMC_PCER |= (1 << psRequestedSsp->u8PeripheralId);
  
  psRequestedSsp->pCsGpioAddress = psSspConfig_->pCsGpioAddress;
  psRequestedSsp->u32CsPin       = psSspConfig_->u32CsPin;
  psRequestedSsp->eBitOrder      = psSspConfig_->eBitOrder;
  psRequestedSsp->eSspMode       = psSspConfig_->eSspMode;
  psRequestedSsp->fnSlaveTxFlowCallback = psSspConfig_->fnSlaveTxFlowCallback;
  psRequestedSsp->fnSlaveRxFlowCallback = psSspConfig_->fnSlaveRxFlowCallback;
  psRequestedSsp->pu8RxBuffer     = psSspConfig_->pu8RxBufferAddress;
  psRequestedSsp->ppu8RxNextByte  = psSspConfig_->ppu8RxNextByte;
  psRequestedSsp->u16RxBufferSize = psSspConfig_->u16RxBufferSize;
  psRequestedSsp->u32PrivateFlags |= _SSP_PERIPHERAL_ASSIGNED;
   
  psRequestedSsp->pBaseAddress->US_CR   = u32TargetCR;
  psRequestedSsp->pBaseAddress->US_MR   = u32TargetMR;
  psRequestedSsp->pBaseAddress->US_IER  = u32TargetIER;
  psRequestedSsp->pBaseAddress->US_IDR  = u32TargetIDR;
  psRequestedSsp->pBaseAddress->US_BRGR = u32TargetBRGR;
  
  if(psRequestedSsp->eSspMode == SPI_SLAVE)
  {
    /* Preset the PDC pointers and counters; the receive buffer must be starting from [0] and be at least 2 bytes long)*/
    psRequestedSsp->pBaseAddress->US_RPR  = (u32)psSspConfig_->pu8RxBufferAddress;
    psRequestedSsp->pBaseAddress->US_RNPR = (u32)(psSspConfig_->pu8RxBufferAddress + 1);
    psRequestedSsp->pBaseAddress->US_RCR  = 1;
    psRequestedSsp->pBaseAddress->US_RNCR = 1;
    psRequestedSsp->ppu8RxNextByte = NULL; /* not used for SPI_SLAVE */

    /* Enable the receiver and transmitter requests so they are ready to go if the Master starts clocking */
    psRequestedSsp->pBaseAddress->US_PTCR = AT91C_PDC_RXTEN | AT91C_PDC_TXTEN;
    psRequestedSsp->pBaseAddress->US_IER = AT91C_US_CTSIC;
  }

  if(psRequestedSsp->eSspMode == SPI_SLAVE_FLOW_CONTROL)
  {
    /* Enable the CS and receiver requests so they are ready to go if the Master starts clocking */
    psRequestedSsp->pBaseAddress->US_IER = (AT91C_US_CTSIC | AT91C_US_RXRDY);
  }
  
  /* Enable SSP interrupts */
  NVIC_ClearPendingIRQ( (IRQn_Type)psRequestedSsp->u8PeripheralId );
  NVIC_EnableIRQ( (IRQn_Type)psRequestedSsp->u8PeripheralId );
  
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
  
  /* First disable the interrupts */
  NVIC_DisableIRQ( (IRQn_Type)(psSspPeripheral_->u8PeripheralId) );
  NVIC_ClearPendingIRQ( (IRQn_Type)(psSspPeripheral_->u8PeripheralId) );
 
  /* Now it's safe to release all of the resources in the target peripheral */
  psSspPeripheral_->pCsGpioAddress = NULL;
  psSspPeripheral_->pu8RxBuffer    = NULL;
  psSspPeripheral_->ppu8RxNextByte  = NULL;
  psSspPeripheral_->u32PrivateFlags = 0;
  psSspPeripheral_->fnSlaveTxFlowCallback = NULL;
  psSspPeripheral_->fnSlaveRxFlowCallback = NULL;

  /* Empty the transmit buffer if there were leftover messages */
  while(psSspPeripheral_->psTransmitBuffer != NULL)
  {
    UpdateMessageStatus(psSspPeripheral_->psTransmitBuffer->u32Token, ABANDONED);
    DeQueueMessage(&psSspPeripheral_->psTransmitBuffer);
  }
  
  /* Ensure the SM is in the Idle state */
  Ssp_pfnStateMachine = SspSM_Idle;
  
} /* end SspRelease() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspAssertCS

Description:
Asserts (CLEARS) the CS line on the target SSP peripheral.  

Requires:
  - psSspPeripheral_ has been requested.

Promises:
  - Target's CS line is LOW
*/
void SspAssertCS(SspPeripheralType* psSspPeripheral_)
{
  //u32 u32Test = psSspPeripheral_->u32CsPin;
  //u32 u32Test2 = psSspPeripheral_->pCsGpioAddress->PIO_CODR;
  psSspPeripheral_->pCsGpioAddress->PIO_CODR = psSspPeripheral_->u32CsPin;
  
} /* end SspAssertCS() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspDeAssertCS

Description:
Deasserts (SETS) the CS line on the target SSP peripheral.  

Requires:
  - psSspPeripheral_ has been requested.

Promises:
  - Target's CS line is HIGH
*/
void SspDeAssertCS(SspPeripheralType* psSspPeripheral_)
{
  psSspPeripheral_->pCsGpioAddress->PIO_SODR = psSspPeripheral_->u32CsPin;
  
} /* end SspDessertCS() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspWriteByte

Description:
Queues a single byte for transfer on the target SSP peripheral.  

Requires:
  - psSspPeripheral_ has been requested.
  - The chip select line of the SSP device should be asserted

Promises:
  - Creates a 1-byte message at psSspPeripheral_->psTransmitBuffer that will be sent by the SSP application
    when it is available.
  - Returns the message token assigned to the message
*/
u32 SspWriteByte(SspPeripheralType* psSspPeripheral_, u8 u8Byte_)
{
  u32 u32Token;
  u8 u8Data = u8Byte_;
  
  u32Token = QueueMessage(&psSspPeripheral_->psTransmitBuffer, 1, &u8Data);
  if( u32Token != 0 )
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
  - adds the data message at psSspPeripheral_->psTransmitBuffer that will be sent by the SSP application
    when it is available.
  - Returns the message token assigned to the message; 0 is returned if the message cannot be queued in which case
    G_u32MessagingFlags can be checked for the reason
*/
u32 SspWriteData(SspPeripheralType* psSspPeripheral_, u32 u32Size_, u8* pu8Data_)
{
  u32 u32Token;

  u32Token = QueueMessage(&psSspPeripheral_->psTransmitBuffer, u32Size_, pu8Data_);
  if( u32Token == 0 )
  {
    return(0);
  }
  
  /* If the system is initializing, manually cycle the SSP task through one iteration to send the message */
  if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
  {
    SspManualMode();
  }

  return(u32Token);

} /* end SspWriteData() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspReadByte

Description:
Master mode only.  Queues a request for a single byte from the slave on the target SSP peripheral.  

Requires:
  - psSspPeripheral_ has been requested.
  - 

Promises:
  - Creates a message with one SSP_DUMMY_BYTE at psSspPeripheral_->psTransmitBuffer that will be sent by the SSP application
    when it is available and thus clock in a received byte to the target receive buffer.
  - Returns the Token of the transmitted dummy message used to read data.

*/
bool SspReadByte(SspPeripheralType* psSspPeripheral_)
{
  /* Make sure no receive function is already in progress based on the bytes in the buffer */
  if( psSspPeripheral_->u16RxBytes != 0)
  {
    return FALSE;
  }
  
  /* Load the counter and return success */
  psSspPeripheral_->u16RxBytes = 1;
  return TRUE;
  
} /* end SspReadByte() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspReadData

Description:
Gets multiple bytes from the slave on the target SSP peripheral.  

Requires:
  - If CS is under manual control for the target SSP peripheral, it should already be asserted
  - u32Size_ is the number of bytes to receive

Promises:
  - Returns the message token of the dummy message used to read data
  - Returns FALSE if the message is too big, or the peripheral already has a read request
*/
bool SspReadData(SspPeripheralType* psSspPeripheral_, u16 u16Size_)
{
  u8 au8MsgTooBig[] = "\r\nSSP message to large\n\r";
  
  /* Do not allow if requested size is too large */
  if(u16Size_ > MAX_TX_MESSAGE_LENGTH)
  {
    DebugPrintf(au8MsgTooBig);
    return FALSE;
  }
  
  /* Make sure no receive function is already in progress based on the bytes in the buffer */
  if( psSspPeripheral_->u16RxBytes != 0)
  {
    return FALSE;
  }
  
  /* Load the counter and return success */
  psSspPeripheral_->u16RxBytes = u16Size_;
  return TRUE;
    
} /* end SspReadData() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SspQueryReceiveStatus

Description:
Returns status of currently requested receive data.

Requires:
  - SSP_psCurrentSsp->u16RxBytes

Promises:
  - Returns the message token of the dummy message used to read data
  - Returns FALSE if the message is too big, or the peripheral already has a read request
*/
SspRxStatusType SspQueryReceiveStatus(SspPeripheralType* psSspPeripheral_)
{
  /* Check for no current bytes queued */
  if(psSspPeripheral_->u16RxBytes == 0)
  {
    /* If a transfer just finished and has not be queried... */
    if(psSspPeripheral_->u32PrivateFlags & _SSP_PERIPHERAL_RX_COMPLETE)
    {
      psSspPeripheral_->u32PrivateFlags &= ~_SSP_PERIPHERAL_RX_COMPLETE;
      return SSP_RX_COMPLETE;
    }
    /* Otherwise it's just empty */
    else
    {
      return SSP_RX_EMPTY;
    }
  }
  /* If there are bytes waiting, check if waiting or in progress */
  else
  {
    if(psSspPeripheral_->u32PrivateFlags & _SSP_PERIPHERAL_RX)
    {
      return SSP_RX_RECEIVING;
    }
    else
    {
      return SSP_RX_WAITING;
    }
  }
  
} /* end SspQueryReceiveStatus() */


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
  static u8 au8SspStartupMessage[] = "SSP Peripherals Ready\n\r";
  
  /* Initialize the SSP peripheral structures */
  SSP_Peripheral0.pBaseAddress     = AT91C_BASE_US0;
  SSP_Peripheral0.pCsGpioAddress   = NULL;
  SSP_Peripheral0.psTransmitBuffer = NULL;
  SSP_Peripheral0.pu8RxBuffer      = NULL;
  SSP_Peripheral0.u16RxBufferSize  = 0;
  SSP_Peripheral0.ppu8RxNextByte    = NULL;
  SSP_Peripheral0.u32PrivateFlags  = 0;
  SSP_Peripheral0.u8PeripheralId   = AT91C_ID_US0;
  
  SSP_Peripheral1.pBaseAddress     = AT91C_BASE_US1;
  SSP_Peripheral1.pCsGpioAddress   = NULL;
  SSP_Peripheral1.psTransmitBuffer = NULL;
  SSP_Peripheral1.pu8RxBuffer      = NULL;
  SSP_Peripheral1.u16RxBufferSize  = 0;
  SSP_Peripheral1.ppu8RxNextByte    = NULL;
  SSP_Peripheral1.u32PrivateFlags  = 0;
  SSP_Peripheral1.u8PeripheralId   = AT91C_ID_US1;

  SSP_Peripheral2.pBaseAddress     = AT91C_BASE_US2;
  SSP_Peripheral2.pCsGpioAddress   = NULL;
  SSP_Peripheral2.psTransmitBuffer = NULL;
  SSP_Peripheral2.pu8RxBuffer      = NULL;
  SSP_Peripheral2.u16RxBufferSize  = 0;
  SSP_Peripheral2.ppu8RxNextByte    = NULL;
  SSP_Peripheral2.u32PrivateFlags  = 0;
  SSP_Peripheral2.u8PeripheralId   = AT91C_ID_US2;

  SSP_psCurrentSsp                = &SSP_Peripheral0;
  
  /* Fill the dummy array with SSP_DUMMY bytes */
  //memset(SSP_au8Dummies, SSP_DUMMY_BYTE, MAX_TX_MESSAGE_LENGTH);

  /* Set application pointer */
  Ssp_pfnStateMachine = SspSM_Idle;
  DebugPrintf(au8SspStartupMessage);

} /* end SspInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function SspRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void SspRunActiveState(void)
{
  Ssp_pfnStateMachine();

} /* end SspRunActiveState */


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
void SspManualMode(void)
{
  /* Set up for manual mode */
  SSP_u32Flags |= _SSP_MANUAL_MODE;
  SSP_psCurrentSsp = &SSP_Peripheral0;

  /* Run a full cycle of the SSP state machine so all SSP peripherals send their current message */  
  while(SSP_u32Flags & _SSP_MANUAL_MODE)
  {
    Ssp_pfnStateMachine();
    
    SSP_u32Timer = G_u32SystemTime1ms;
    IsTimeUp(&SSP_u32Timer, 1);
  }
      
} /* end SspManualMode() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------------------------------------
Interrupt Service Routine: SSP0_IRQHandler

Description:
Handles the enabled SSP0 interrupts. 

Transmit: An End Transmit interrupt will occur when the PDC has finished sending all of the bytes.
Receive: An End Receive interrupt will occur when the PDC has finished receiving all of the expected bytes.

Requires:
  - ISRs cannot require anything

Promises:
  - Status of message that has completed transferring will be set to COMPLETE.
  - Peripheral CS line is cleared and the PDC is disabled
  - _SSP_PERIPHERAL_RX/TX is cleared
*/
void SSP0_IRQHandler(void)
{
  /* Set the current ISR pointers to SSP0 targets */
  SSP_psCurrentISR = &SSP_Peripheral0;                         /* Current SSP ISR */
  SSP_pu32SspApplicationFlagsISR = &G_u32Ssp0ApplicationFlags; /* Current SSP application status flags */
  SSP_u32Int0Count++;

  /* Go to common SSP interrupt using psCurrentSspISR since the SSP cannot interrupt itself */
  SspGenericHandler();
  
} /* end SSP0_IRQHandler() */


/*----------------------------------------------------------------------------------------------------------------------
Interrupt Service Routine: SSP1_IRQHandler

Description:
Sets up to handles the enabled SSP1 interrupts. 

Requires:
  - ISRs cannot require anything

Promises:
  - See GenericSspHandler
*/
void SSP1_IRQHandler(void)
{
  /* Set the current ISR pointers to SSP1 targets */
  SSP_psCurrentISR = &SSP_Peripheral1;                      /* Current SSP ISR */
  SSP_pu32SspApplicationFlagsISR = &G_u32Ssp1ApplicationFlags; /* Current SSP application status flags */
  SSP_u32Int1Count++;

  /* Go to common SSP interrupt using psCurrentSspISR since the SSP cannot interrupt itself */
  SspGenericHandler();
  
} /* end SSP1_IRQHandler() */


/*----------------------------------------------------------------------------------------------------------------------
Interrupt Service Routine: SSP2_IRQHandler

Description:
Handles the enabled SSP2 interrupts. 

*/
void SSP2_IRQHandler(void)
{
  /* Set the current ISR pointers to SSP2 targets */
  SSP_psCurrentISR = &SSP_Peripheral2;                      /* Current SSP ISR */
  SSP_pu32SspApplicationFlagsISR = &G_u32Ssp2ApplicationFlags; /* Current SSP application status flags */
  SSP_u32Int2Count++;

  /* Go to common SSP interrupt using psCurrentSspISR since the SSP cannot interrupt itself */
  SspGenericHandler();
 
} /* end SSP2_IRQHandler() */


/*----------------------------------------------------------------------------------------------------------------------
Generic Interrupt Service Routine

Description:
Handles the enabled interrupts for the current SSP. 

Chip select: only enabled for SLAVE peripherals.  A Slave peripheral needs this signal to know it is communicating.  
If it is supposed to be transmitting and does not have any flow control, the data should already be ready.
Transmit: An End Transmit interrupt will occur when the PDC has finished sending all of the bytes for Master or Slave.
Receive: An End Receive interrupt will occur when the PDC has finished receiving all of the expected bytes for Master or a single byte for Slave.
Receive RXBUFF: An Rx Buffer empty interrupt occurs on a Slave when both the current and next send counters are 0.

Requires:
  - SSP_psCurrentISR points to the SSP peripheral who has triggered the interrupt
  - SSP_pu32SspApplicationFlagsISR points to the application flags from the SSP peripheral that triggered the interrupt
  - A different SSP peripheral cannot interrupt this ISR

Promises:
  - Status of message that has completed transferring will be set to COMPLETE.
  - For Master peripherals, the CS line is cleared and the PDC is disabled
  - _SSP_PERIPHERAL_RX/TX is cleared
*/
void SspGenericHandler(void)
{
  u32 u32Byte;
  u32 u32Timeout;
  u32 u32Current_CSR;
  
  /* Get a copy of CSR because reading it changes it */
  u32Current_CSR = SSP_psCurrentISR->pBaseAddress->US_CSR;

  /*** CS change state interrupt - only enabled on Slave SSP peripherals ***/
  if( (SSP_psCurrentISR->pBaseAddress->US_IMR & AT91C_US_CTSIC) && 
      (u32Current_CSR & AT91C_US_CTSIC) )
  {
    /* Is the CS pin asserted now? */
    if( (SSP_psCurrentISR->pCsGpioAddress->PIO_PDSR & SSP_psCurrentISR->u32CsPin) == 0)
    {
      /* Flag that CS is asserted */
      *SSP_pu32SspApplicationFlagsISR |= _SSP_CS_ASSERTED;
      *SSP_pu32SspApplicationFlagsISR &= ~(_SSP_TX_COMPLETE | _SSP_RX_COMPLETE);
    }
    else
    {
      /* Flag that CS is deasserted */
      *SSP_pu32SspApplicationFlagsISR &= ~_SSP_CS_ASSERTED;
     
      /* Make sure RCR is 1 for next transmission on Slave - no flow control devices only */
      if(SSP_psCurrentISR->eSspMode == SPI_SLAVE)
      {
        SSP_psCurrentISR->pBaseAddress->US_RCR  = 1;
      }
    }
  } /* end CS change state interrupt */

  /*** SSP ISR transmit handling for flow-control devices that do not use DMA ***/
  if( (SSP_psCurrentISR->pBaseAddress->US_IMR & AT91C_US_TXEMPTY) && 
      (u32Current_CSR & AT91C_US_TXEMPTY) )
  {
    /* Decrement counter and read the dummy byte so the SSP peripheral doesn't overrun */
    SSP_psCurrentISR->u32CurrentTxBytesRemaining--;
    u32Byte = SSP_psCurrentISR->pBaseAddress->US_RHR;
    
    if(SSP_psCurrentISR->u32CurrentTxBytesRemaining != 0)
    {
      /* Advance the pointer (non-circular buffer), load the next byte and use the callback */
      SSP_psCurrentISR->pu8CurrentTxData++;
      u32Byte = 0x000000FF & *SSP_psCurrentISR->pu8CurrentTxData;

      /* If we need LSB first, use inline assembly to flip bits with a single instruction. */
      if(SSP_psCurrentISR->eBitOrder == LSB_FIRST)
      {
        u32Byte = __RBIT(u32Byte)>>24;
      }
    
      SSP_psCurrentISR->pBaseAddress->US_THR = (u8)u32Byte; /* Clears interrupt flag */
      SSP_psCurrentISR->fnSlaveTxFlowCallback();
    }
    else
    {
      /* Done! Disable TX interrupt */
      SSP_psCurrentISR->pBaseAddress->US_IDR = AT91C_US_TXEMPTY;
      
      /* Clean up the message status and flags */
      SSP_psCurrentISR->u32PrivateFlags &= ~_SSP_PERIPHERAL_TX;  
      UpdateMessageStatus(SSP_psCurrentISR->psTransmitBuffer->u32Token, COMPLETE);
      DeQueueMessage(&SSP_psCurrentISR->psTransmitBuffer);
 
      /* Re-enable Rx interrupt and clean-up the operation */    
      SSP_psCurrentISR->pBaseAddress->US_IER = AT91C_US_RXRDY;
      *SSP_pu32SspApplicationFlagsISR |= _SSP_TX_COMPLETE; 
      SSP_psCurrentISR->fnSlaveTxFlowCallback();
    }
  } /* end AT91C_US_TXEMPTY */
  
  /*** SSP ISR handling for Slave Rx with flow control (no DMA) ***/
  if( (SSP_psCurrentISR->pBaseAddress->US_IMR & AT91C_US_RXRDY) && 
      (SSP_psCurrentISR->pBaseAddress->US_CSR & AT91C_US_RXRDY) )
  {
    /* Pull the byte out of the receive register into the Rx buffer */
    u32Byte = 0x000000FF & SSP_psCurrentISR->pBaseAddress->US_RHR;

    /* If we need LSB first, use inline assembly to flip bits with a single instruction. */
    if(SSP_psCurrentISR->eBitOrder == LSB_FIRST)
    {
      u32Byte = __RBIT(u32Byte)>>24;
    }
    
    /* DEBUG */
    if((u8)u32Byte == 0xff)
    {
      SSP_u32AntCounter++;
    }
    
    /* Send the byte to the Rx buffer; since we only do one byte at a time in this mode, then _SSP_RX_COMPLETE */
    **(SSP_psCurrentISR->ppu8RxNextByte) = (u8)u32Byte;
    *SSP_pu32SspApplicationFlagsISR |= _SSP_RX_COMPLETE;

    /* Invoke callback */
    SSP_psCurrentISR->fnSlaveRxFlowCallback();
  }

  
  /*** SSP ISR responses for non-flow-control devices that use DMA (master or slave) ***/
    
  /* ENDRX Interrupt when all requested bytes have been received */
  if( (SSP_psCurrentISR->pBaseAddress->US_IMR & AT91C_US_ENDRX) && 
      (u32Current_CSR & AT91C_US_ENDRX) )
  {
    /* Master mode and Slave mode operate differently */
    if( (SSP_psCurrentISR->eSspMode == SPI_MASTER_AUTO_CS) ||
        (SSP_psCurrentISR->eSspMode == SPI_MASTER_MANUAL_CS) ) 
    {
      /* Reset the byte counter and clear the RX flag */
      SSP_psCurrentISR->u16RxBytes = 0;
      SSP_psCurrentISR->u32PrivateFlags &= ~_SSP_PERIPHERAL_RX;
      SSP_psCurrentISR->u32PrivateFlags |=  _SSP_PERIPHERAL_RX_COMPLETE;
      SSP_u32RxCounter++;
      
      /* Deassert CS for SPI_MASTER_AUTO_CS transfers */
      if(SSP_psCurrentSsp->eSspMode == SPI_MASTER_AUTO_CS)
      {
        SSP_psCurrentISR->pCsGpioAddress->PIO_SODR = SSP_psCurrentISR->u32CsPin;
      }
     
      /* Disable the receiver and transmitter */
      SSP_psCurrentISR->pBaseAddress->US_PTCR = AT91C_PDC_RXTDIS | AT91C_PDC_TXTDIS;
      SSP_psCurrentISR->pBaseAddress->US_IDR  = AT91C_US_ENDRX;
    }
    /* Otherwise the peripheral is a Slave that just received a byte */
    /* ENDRX Interrupt when a byte has been received (RNCR is moved to RCR; RNPR is copied to RPR))*/
    else
    {
      /* Flag that a byte has arrived */
      *SSP_pu32SspApplicationFlagsISR |= _SSP_RX_COMPLETE;

      /* Update the pointer to the next valid Rx location (always leap-frogs the immediate next byte since it will be filled by the other DMA process */
      SSP_psCurrentISR->pBaseAddress->US_RNPR++;
      if(SSP_psCurrentISR->pBaseAddress->US_RPR == (u32)(SSP_psCurrentISR->pu8RxBuffer + (u32)SSP_psCurrentISR->u16RxBufferSize) )
      {
        SSP_psCurrentISR->pBaseAddress->US_RPR = (u32)SSP_psCurrentISR->pu8RxBuffer;  /* !!!!! CHECK */
      }
      
      /* Write RNCR to 1 to clear the ENDRX flag */
      SSP_psCurrentISR->pBaseAddress->US_RNCR = 1;
    }  
  } /* end ENDRX handling */


  /* ENDTX Interrupt when all requested transmit bytes have been sent (if enabled) */
  if( (SSP_psCurrentISR->pBaseAddress->US_IMR & AT91C_US_ENDTX) && 
      (u32Current_CSR & AT91C_US_ENDTX) )
  {
    /* Update this message token status and then DeQueue it */
    UpdateMessageStatus(SSP_psCurrentISR->psTransmitBuffer->u32Token, COMPLETE);
    DeQueueMessage( &SSP_psCurrentISR->psTransmitBuffer );
    SSP_psCurrentISR->u32PrivateFlags &= ~_SSP_PERIPHERAL_TX;
        
    /* Disable the transmitter and interrupt source */
    SSP_psCurrentISR->pBaseAddress->US_PTCR = AT91C_PDC_TXTDIS;
    SSP_psCurrentISR->pBaseAddress->US_IDR  = AT91C_US_ENDTX;

    /* Allow the peripheral to finish clocking out the Tx byte */
    u32Timeout = 0;
    while ( !(SSP_psCurrentISR->pBaseAddress->US_CSR & AT91C_US_TXEMPTY) && 
            u32Timeout < SSP_TXEMPTY_TIMEOUT)
    {
      u32Timeout++;
    } 
    
    if(SSP_psCurrentISR->eSspMode == SPI_MASTER_AUTO_CS)
    {
      /* Deassert chip select when the buffer and shift register are totally empty */
      if(SSP_psCurrentSsp->eSspMode == SPI_MASTER_AUTO_CS)
      {
        SSP_psCurrentISR->pCsGpioAddress->PIO_SODR = SSP_psCurrentISR->u32CsPin;
      }
      //SSP_psCurrentISR->pCsGpioAddress->PIO_SODR = SSP_psCurrentISR->u32CsPin;
    }
  } /* end ENDTX interrupt handling */

  
} /* end SspGenericHandler() */


/***********************************************************************************************************************
State Machine Function Definitions

The SSP state machine monitors messaging activity on the available SSP Master peripherals.  It manages all SSP outgoing messages and will
transmit any message that has been queued.  All configured SSP peripherals can be transmitting and receiving
simultaneously.

***********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a transmit message to be queued -- this can include a dummy transmission to receive bytes.
Half duplex transmissions are always assumed. Check one peripheral per iteration. */
void SspSM_Idle(void)
{
 static u8 au8SspErrorInvalidSsp[] = "Invalid SSP attempt\r\n";
 u32 u32Byte;
  
  /* Check all SPI/SSP peripherals for message activity or skip the current peripheral if it is already busy.
  Slave devices receive outside of the state machine.
  For Master devices sending a message, SSP_psCurrentSsp->psTransmitBuffer->pu8Message will point to the application transmit buffer.
  For Master devices receiving a message, SSP_psCurrentSsp->u16RxBytes will != 0. Dummy bytes are sent.  */
  if( ( (SSP_psCurrentSsp->psTransmitBuffer != NULL) || (SSP_psCurrentSsp->u16RxBytes !=0) ) && 
     !(SSP_psCurrentSsp->u32PrivateFlags & (_SSP_PERIPHERAL_TX | _SSP_PERIPHERAL_RX)       ) 
    )
  {
    /* For an SPI_MASTER_AUTO_CS device, start by asserting chip select 
   (SPI_MASTER_MANUAL_CS devices should already have asserted CS in the user's task) */
    if(SSP_psCurrentSsp->eSspMode == SPI_MASTER_AUTO_CS)
    {
      SSP_psCurrentSsp->pCsGpioAddress->PIO_CODR = SSP_psCurrentSsp->u32CsPin;
    }
       
    /* Check if the message is receiving based on expected byte count */
    if(SSP_psCurrentSsp->u16RxBytes !=0)
    {
      /* Receiving: flag that the peripheral is now busy */
      SSP_psCurrentSsp->u32PrivateFlags |= _SSP_PERIPHERAL_RX;    
      
      /* Clear the receive buffer so we can see (most) data changes but also so we send
      predictable dummy bytes since we'll point to this buffer to source the transmit dummies */
      memset(SSP_psCurrentSsp->pu8RxBuffer, SSP_DUMMY_BYTE, SSP_psCurrentSsp->u16RxBufferSize);

      /* Load the PDC counter and pointer registers */
      SSP_psCurrentSsp->pBaseAddress->US_RPR = (unsigned int)SSP_psCurrentSsp->pu8RxBuffer; 
      SSP_psCurrentSsp->pBaseAddress->US_TPR = (unsigned int)SSP_psCurrentSsp->pu8RxBuffer; 
      SSP_psCurrentSsp->pBaseAddress->US_RCR = SSP_psCurrentSsp->u16RxBytes;
      SSP_psCurrentSsp->pBaseAddress->US_TCR = SSP_psCurrentSsp->u16RxBytes;

      /* When RCR is loaded, the ENDRX flag is cleared so it is safe to enable the interrupt */
      SSP_psCurrentSsp->pBaseAddress->US_IER = AT91C_US_ENDRX;
      
      /* Enable the receiver and transmitter to start the transfer */
      SSP_psCurrentSsp->pBaseAddress->US_PTCR = AT91C_PDC_RXTEN | AT91C_PDC_TXTEN;
    } /* End of receive function */
    else
    {
      /* Transmitting: update the message's status and flag that the peripheral is now busy */
      UpdateMessageStatus(SSP_psCurrentSsp->psTransmitBuffer->u32Token, SENDING);
      SSP_psCurrentSsp->u32PrivateFlags |= _SSP_PERIPHERAL_TX;    
      
      /* TRANSMIT SPI_SPI_SLAVE_FLOW_CONTROL */
      /* A Slave device with flow control uses interrupt-driven single byte transfers */
      if(SSP_psCurrentSsp->eSspMode == SPI_SLAVE_FLOW_CONTROL)
      {
        /* At this point, CS is asserted and the master is waiting for flow control.
        Load in the message parameters. */
        SSP_psCurrentSsp->u32CurrentTxBytesRemaining = SSP_psCurrentSsp->psTransmitBuffer->u32Size;
        SSP_psCurrentSsp->pu8CurrentTxData = SSP_psCurrentSsp->psTransmitBuffer->pu8Message;

        /* If we need LSB first, use inline assembly to flip bits with a single instruction. */
        u32Byte = 0x000000FF & *SSP_psCurrentSsp->pu8CurrentTxData;
        if(SSP_psCurrentSsp->eBitOrder == LSB_FIRST)
        {
          u32Byte = __RBIT(u32Byte)>>24;
        }
        
        /* Reset the transmitter since we have not been managing dummy bytes and it tends to be
        in the middle of a transmission or something that causes the wrong byte to get sent (at least on startup). */
        SSP_psCurrentSsp->pBaseAddress->US_CR = (AT91C_US_RSTTX);
        SSP_psCurrentSsp->pBaseAddress->US_CR = (AT91C_US_TXEN);
        SSP_psCurrentSsp->pBaseAddress->US_THR = (u8)u32Byte;
        SSP_psCurrentSsp->pBaseAddress->US_IDR = AT91C_US_RXRDY;
        SSP_psCurrentSsp->pBaseAddress->US_IER = AT91C_US_TXEMPTY;
        SSP_psCurrentSsp->fnSlaveTxFlowCallback();
      }
      
      /* TRANSMIT SPI_MASTER_AUTO_CS, SPI_MASTER_MANUAL_CS, SPI_SLAVE NO FLOW CONTROL */
      /* A Master or Slave device without flow control uses the PDC */
      else
      {
        /* Load the PDC counter and pointer registers */
        SSP_psCurrentSsp->pBaseAddress->US_TPR = (unsigned int)SSP_psCurrentSsp->psTransmitBuffer->pu8Message; 
        SSP_psCurrentSsp->pBaseAddress->US_TCR = SSP_psCurrentSsp->psTransmitBuffer->u32Size;
   
        /* When TCR is loaded, the ENDTX flag is cleared so it is safe to enable the interrupt */
        SSP_psCurrentSsp->pBaseAddress->US_IER = AT91C_US_ENDTX;
        
        /* Enable the transmitter to start the transfer */
        SSP_psCurrentSsp->pBaseAddress->US_PTCR = AT91C_PDC_TXTEN;
      }
    } /* End of transmitting function */
  }
  
  /* Adjust to check the next peripheral next time through */
  switch (SSP_psCurrentSsp->u8PeripheralId)
  {
    case AT91C_ID_US0:
      SSP_psCurrentSsp = &SSP_Peripheral1;
      break;

    case AT91C_ID_US1:
      SSP_psCurrentSsp = &SSP_Peripheral2;
      break;

    case AT91C_ID_US2:
      SSP_psCurrentSsp = &SSP_Peripheral0;
      SSP_u32Flags &= ~_SSP_MANUAL_MODE;
      break;

    default:
      DebugPrintf(au8SspErrorInvalidSsp);
      SSP_psCurrentSsp = &SSP_Peripheral0;
      break;
  } /* end switch */
  
} /* end SspSM_Idle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error (!!!!!FUTURE) */
void SspSM_Error(void)          
{
  Ssp_pfnStateMachine = SspSM_Idle;
  
} /* end SspSM_Error() */

        
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

