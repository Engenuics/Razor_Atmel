/*!**********************************************************************************************************************
@file sam3u_ssp.c                                                                
@brief Provides a driver to use a USART peripheral in SPI/SSP mode to send and 
receive data using interrupts and peripheral DMA.

Note that Master SPI devices keep CS as a GPIO so it can be managed manually to 
more easily work with the multitude of variants in Slave device CS requirements.

This driver should work for SPI Slaves with or without flow control, though you may 
need to make adjustments to how data is timed.  A Slave with flow control is 
implemented with callback functions to manage flow control lines.

When a USART peripheral on the SAM3U2 is configured in SPI mode, the bit that controls
MSB vs. LSB first is used for CPOL and therefore not available.  Sending MSB vs. LSB 
first is currently only selectable in Slave mode with Flow Control which does not use 
DMA and thus the byte can be processed (flipped) as it is loaded.  If LSB-first data 
is required for other modes, data arrays will have to be pre-flipped by the application.


INITIALIZATION 
This should take place in application's initialization function.

1. Create a variable of SspConfigurationType in your application and initialize it 
to the desired SSP peripheral, and the size & address of the receive buffer in the 
application.

2. Call SspRequest() with pointer to the configuration variable created in step 1.  
The returned pointer is the SspPeripheralType object created that will be used by 
your application for all API calls.

3. If the application no longer needs the SSP resource, call SspRelease().  
Note: if multiple tasks share an SSP peripheral & bus, SspRequest() and 
SspRelease() will be used frequently to allow traffic to both devices. 


MASTER MODE DATA TRANSFER
Transmitted data is queued using one of two functions, SspWriteByte() and 
SspWriteData() which both return a message token unique to the queued message.  
Once the data is queued, it is sent by the SSP as soon as possible.  Different 
SSP resources may transmit and receive data simultaneously.  

The SPI protocol always receives a byte with every transmitted byte.  This may 
be a defined dummy byte, or it may be 0xFF or 0x00 depending on the idle state 
of the MISO line.  Your application must process the received bytes and determine 
if they are dummy bytes or useful data.

To receive data from an SSP Slave, call SspReadByte() for a single 
byte or SspReadData() for multiple bytes.  These functions will automatically 
queue SSP_DUMMY bytes to transmit and activate the clock.

Received bytes on the allocated peripheral will be dropped into the application's 
designated receive buffer.  The buffer is written circularly, with no provision 
to monitor bytes that are overwritten.  The application is responsible for 
processing all received data.  The application must provide its own parsing
pointer to read the receive buffer and properly wrap around.  This pointer 
will not be impacted by the interrupt service routine.  Data may be added to
the application's receive buffer at any time.


SLAVE MODE DATA TRANSFER (NO FLOW CONTROL)
In Slave mode, the peripheral is always ready to receive bytes from the Master. 
Since SPI always sends and receives with every transaction, the Slave receiver
must always examine data received even if the transaction was expected to be a
transmit from Slave to Master.

Received bytes on the allocated peripheral will be dropped into the receive
buffer that the application specifies upon requesting the SPI peripheral.  The buffer 
is written circularly, with no provision to monitor bytes that are overwritten.  The 
application is responsible for processing all received data.  The application must 
provide its own parsing pointer to read the receive buffer and properly wrap around.  
This pointer will not be impacted by the interrupt service routine that may add 
additional characters at any time.

Transmitting data from a Slave to Master without flow control is slightly awkward 
in this system. If a Master requests data, it must allow at least 2ms for the Slave
to process the data request and queue the required data response.
Transmitted data is queued using one of two functions, SspWriteByte() and 
SspWriteData().  Once the data is queued, the SSP is ready to send via DMA
with the expected number of bytes.  The Master should clock all of these bytes.

If the correct number of clocks are provided, the ENDTX interrupt will fire

If not enough clocks are sent and the Master de-asserts CS, then the transmission
is aborted and the message is flushed out.

If too many clocks are provided, the Slave will resort to sending dummy bytes.


SLAVE DATA TRANSFER WITH FLOW CONTROL
Flow control is offered in Slave mode through callback functions. Data transfer is
managed by the peripheral DMA controller byte-by-byte so the system can run
the callbacks and manage flow control lines.  

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- G_u32Ssp0ApplicationFlags
- G_u32Ssp1ApplicationFlags
- G_u32Ssp2ApplicationFlags

CONSTANTS
- NONE

TYPES
- SspBitOrderType
- SspModeType
- SspRxStatusType
- SspConfigurationType
- SspPeripheralType

PUBLIC FUNCTIONS
- SspPeripheralType* SspRequest(SspConfigurationType* psSspConfig_)
- void SspRelease(SspPeripheralType* psSspPeripheral_)
- void SspAssertCS(SspPeripheralType* psSspPeripheral_)
- void SspDeAssertCS(SspPeripheralType* psSspPeripheral_)
- u32 SspWriteByte(SspPeripheralType* psSspPeripheral_, u8 u8Byte_)
- u32 SspWriteData(SspPeripheralType* psSspPeripheral_, u32 u32Size_, u8* pu8Data_)

Master mode only:
- bool SspReadByte(SspPeripheralType* psSspPeripheral_)
- bool SspReadData(SspPeripheralType* psSspPeripheral_, u16 u16Size_)
- SspRxStatusType SspQueryReceiveStatus(SspPeripheralType* psSspPeripheral_)

PROTECTED FUNCTIONS
- void SspInitialize(void)
- void SspRunActiveState(void)
- void SspManualMode(void)
- void SSP0_IRQHandler(void)
- void SSP1_IRQHandler(void)
- void SSP2_IRQHandler(void)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>Ssp"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32Ssp0ApplicationFlags;          /*!< @brief Status flags meant for application using this SSP peripheral */
volatile u32 G_u32Ssp1ApplicationFlags;          /*!< @brief Status flags meant for application using this SSP peripheral */
volatile u32 G_u32Ssp2ApplicationFlags;          /*!< @brief Status flags meant for application using this SSP peripheral */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;          /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;           /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;            /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;       /*!< @brief From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "SSP_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type SSP_pfnStateMachine;          /*!< @brief The SSP application state machine */

static u32 SSP_u32Timer;                         /*!< @brief Timeout counter used across states */
static u32 SSP_u32Flags;                         /*!< @brief Application flags for SSP */

static SspPeripheralType SSP_Peripheral0;        /*!< @brief SSP0 peripheral object */
static SspPeripheralType SSP_Peripheral1;        /*!< @brief SSP1 peripheral object */
static SspPeripheralType SSP_Peripheral2;        /*!< @brief SSP2 peripheral object */

static SspPeripheralType* SSP_psCurrentSsp;      /*!< @brief Current SSP peripheral being processed task */
static SspPeripheralType* SSP_psCurrentISR;      /*!< @brief Current SSP peripheral being processed in ISR */
static u32* SSP_pu32SspApplicationFlagsISR;      /*!< @brief Current SSP application status flags in ISR */

static u8 SSP_u8Dummies = SSP_DUMMY_BYTE;        /*!< @brief Dummy source byte */

/*! @cond DOXYGEN_EXCLUDE */
static u32 SSP_u32Int0Count = 0;                 /* Debug counter for SSP0 interrupts */
static u32 SSP_u32Int1Count = 0;                 /* Debug counter for SSP1 interrupts */
static u32 SSP_u32Int2Count = 0;                 /* Debug counter for SSP2 interrupts */
static u32 SSP_u32AntCounter = 0;                /* Debug counter */
static u32 SSP_u32RxCounter = 0;                 /* Debug counter */
/*! @endcond */


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!---------------------------------------------------------------------------------------------------------------------
@fn SspPeripheralType* SspRequest(SspConfigurationType* psSspConfig_)

@brief Requests access to an SSP resource.  

If the resource is available, the transmit and receive parameters are set up
and the peripheral is made ready to use in the application. The peripheral will be 
configured in different ways for different SSP modes.  The following modes are supported:

SPI_MASTER: transmit and receive using peripheral DMA controller; transmit occurs 
through the Message API

SSP_SLAVE: transmit through Message Task; receive set up per-byte using current 
and next DMA pointers and managed into circular buffer.

SSP_SLAVE_FLOW_CONTROL: transmit through interrupt-driven single byte transfers 
and call-back; receive using peripheral DMA controller direct to task buffer.

Requires:
- SSP peripheral register initialization values in configuration.h must be set 
  correctly; currently this does not support different SSP configurations 
  for multiple slaves on the same bus - all peripherals on the bus must work with
  the same setup.

@param psSspConfig_ has the SSP peripheral number, address of the RxBuffer and the RxBuffer size

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
  
  psRequestedSsp->pCsGpioAddress   = psSspConfig_->pCsGpioAddress;
  psRequestedSsp->u32CsPin         = psSspConfig_->u32CsPin;
  psRequestedSsp->eBitOrder        = psSspConfig_->eBitOrder;
  psRequestedSsp->eSspMode         = psSspConfig_->eSspMode;
  psRequestedSsp->pu8RxBuffer      = psSspConfig_->pu8RxBufferAddress;
  psRequestedSsp->ppu8RxNextByte   = psSspConfig_->ppu8RxNextByte;
  psRequestedSsp->u16RxBufferSize  = psSspConfig_->u16RxBufferSize;
  psRequestedSsp->u32PrivateFlags |= _SSP_PERIPHERAL_ASSIGNED;
  psRequestedSsp->fnSlaveTxFlowCallback = psSspConfig_->fnSlaveTxFlowCallback;
  psRequestedSsp->fnSlaveRxFlowCallback = psSspConfig_->fnSlaveRxFlowCallback;
   
  psRequestedSsp->pBaseAddress->US_CR   = u32TargetCR;
  psRequestedSsp->pBaseAddress->US_MR   = u32TargetMR;
  psRequestedSsp->pBaseAddress->US_IER  = u32TargetIER;
  psRequestedSsp->pBaseAddress->US_IDR  = u32TargetIDR;
  psRequestedSsp->pBaseAddress->US_BRGR = u32TargetBRGR;
  
  /* Special considerations for SPI Slaves */
  if(psRequestedSsp->eSspMode == SSP_SLAVE)
  {
    /* Preset the PDC receive pointers and counters; the receive buffer must be starting 
    from [0] and be at least 2 bytes long)*/
    psRequestedSsp->pBaseAddress->US_RPR  = (u32)psSspConfig_->pu8RxBufferAddress;
    psRequestedSsp->pBaseAddress->US_RNPR = (u32)(psSspConfig_->pu8RxBufferAddress + 1);
    psRequestedSsp->pBaseAddress->US_RCR  = 1;
    psRequestedSsp->pBaseAddress->US_RNCR = 1;
    psRequestedSsp->ppu8RxNextByte = NULL; /* not used for SSP_SLAVE */
    
    /* Preset the PDC transmit registers to return predictable SPI dummy bytes
    if the Slave is receiving. These will be changed if the Slave transmit is queued
    by the application.  */
    psRequestedSsp->pBaseAddress->US_TPR  = (u32)&SSP_u8Dummies; 
    psRequestedSsp->pBaseAddress->US_TNPR = (u32)&SSP_u8Dummies; 
    psRequestedSsp->pBaseAddress->US_TCR = 1;
    psRequestedSsp->pBaseAddress->US_TNCR = 1;

    /* Enable the receiver and transmitter so they are ready to go if the Master starts clocking */
    psRequestedSsp->pBaseAddress->US_PTCR = (AT91C_PDC_RXTEN | AT91C_PDC_TXTEN);
    psRequestedSsp->pBaseAddress->US_IER  = (AT91C_US_CTSIC | AT91C_US_ENDRX | AT91C_US_ENDTX);
  }

  /* Special considerations for SPI Slaves with Flow Control */
  if(psRequestedSsp->eSspMode == SSP_SLAVE_FLOW_CONTROL)
  {
    /* Enable the CS interrupt */
    psRequestedSsp->pBaseAddress->US_IER = AT91C_US_CTSIC;
  }
  
  /* Enable SSP interrupts */
  NVIC_ClearPendingIRQ( (IRQn_Type)psRequestedSsp->u8PeripheralId );
  NVIC_EnableIRQ( (IRQn_Type)psRequestedSsp->u8PeripheralId );
  
  return(psRequestedSsp);
  
} /* end SspRequest() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void SspRelease(SspPeripheralType* psSspPeripheral_)

@brief Releases an SSP resource.  

Requires:
- Receive operation is not in progress

@param psSspPeripheral_ has the SSP peripheral number, address of the RxBuffer, 
and the RxBuffer.

Promises:
- Resets peripheral object's pointers and data to safe values
- Peripheral is disabled
- Peripheral interrupts are disabled.

*/
void SspRelease(SspPeripheralType* psSspPeripheral_)
{
  /* Check to see if the peripheral is already released */
  if( !(psSspPeripheral_->u32PrivateFlags) & _SSP_PERIPHERAL_ASSIGNED )
  {
    return;
  }
  
  /* Disable interrupts */
  NVIC_DisableIRQ( (IRQn_Type)(psSspPeripheral_->u8PeripheralId) );
  NVIC_ClearPendingIRQ( (IRQn_Type)(psSspPeripheral_->u8PeripheralId) );
 
  /* Now it's safe to release all of the resources in the target peripheral */
  psSspPeripheral_->pCsGpioAddress  = NULL;
  psSspPeripheral_->pu8RxBuffer     = NULL;
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
  SSP_pfnStateMachine = SspSM_Idle;
  
} /* end SspRelease() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void SspAssertCS(SspPeripheralType* psSspPeripheral_)

@brief Master mode manual CS only. Asserts (CLEARS) the CS line on the target SSP peripheral.  

Requires:
- eSspMode should be SSP_MASTER_MANUAL_CS or function does nothing.

@param psSspPeripheral_ is the SSP peripheral to use and it has already been requested.

Promises:
- Target's CS line is LOW

*/
void SspAssertCS(SspPeripheralType* psSspPeripheral_)
{
  if( psSspPeripheral_->eSspMode == SSP_MASTER_MANUAL_CS )
  {
    psSspPeripheral_->pCsGpioAddress->PIO_CODR = psSspPeripheral_->u32CsPin;
  }
  
} /* end SspAssertCS() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void SspDeAssertCS(SspPeripheralType* psSspPeripheral_)

@brief Master mode only. Deasserts (SETS) the CS line on the target SSP peripheral.  

Requires:
- eSspMode should be SSP_MASTER_MANUAL_CS or function does nothing.

@param psSspPeripheral_ is the SSP peripheral to use and it has already been requested.

Promises:
- Target's CS line is HIGH

*/
void SspDeAssertCS(SspPeripheralType* psSspPeripheral_)
{
  if( psSspPeripheral_->eSspMode == SSP_MASTER_MANUAL_CS )
  {
    psSspPeripheral_->pCsGpioAddress->PIO_SODR = psSspPeripheral_->u32CsPin;
  } 
  
} /* end SspDessertCS() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn u32 SspWriteByte(SspPeripheralType* psSspPeripheral_, u8 u8Byte_)

@brief Queues a single byte for transfer on the target SSP peripheral.  

Requires:
- A receive request cannot be in progress

@param psSspPeripheral_ is the SSP peripheral to use and it has already been requested.
@param u8Byte_ is the byte to send

Promises:
- Creates a 1-byte message at psSspPeripheral_->psTransmitBuffer that will be sent 
  by the SSP application when it is available.
- Returns the message token assigned to the message; 0 is returned if the message 
  cannot be queued in which case G_u32MessagingFlags can be checked for the reason

*/
u32 SspWriteByte(SspPeripheralType* psSspPeripheral_, u8 u8Byte_)
{
  u32 u32Token;
  u8 u8Data = u8Byte_;

  /* Make sure no receive function is already in progress based on the bytes in the buffer */
  if( psSspPeripheral_->u16RxBytes != 0)
  {
    return(0);
  }

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


/*!--------------------------------------------------------------------------------------------------------------------
@fn u32 SspWriteData(SspPeripheralType* psSspPeripheral_, u32 u32Size_, u8* pu8Data_)

@brief Queues a data array for transfer on the target SSP peripheral.  

Requires:
- A receive request cannot be in progress

@param psSspPeripheral_ is the SSP peripheral to use and it has already been requested.
@param u32Size_ is the number of bytes in the data array
@param u8Data_ points to the first byte of the data array

Promises:
- adds the data message at psSspPeripheral_->psTransmitBuffer that will be sent by the SSP application
  when it is available.
- Returns the message token assigned to the message; 0 is returned if the message 
  cannot be queued in which case G_u32MessagingFlags can be checked for the reason

*/
u32 SspWriteData(SspPeripheralType* psSspPeripheral_, u32 u32Size_, u8* pu8Data_)
{
  u32 u32Token;

  /* Make sure no receive function is already in progress based on the bytes in the buffer */
  if( psSspPeripheral_->u16RxBytes != 0)
  {
    return(0);
  }

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


/*!--------------------------------------------------------------------------------------------------------------------
@fn bool SspReadByte(SspPeripheralType* psSspPeripheral_)

@brief Master mode only.  Queues a request for a single byte from the Slave on 
the target SSP peripheral.  There cannot be pending writes.

Requires:
- Master mode 

@param psSspPeripheral_ is the SSP peripheral to use and it has already been requested.

Promises:
- Creates a message with one SSP_DUMMY_BYTE at psSspPeripheral_->psTransmitBuffer that will be sent by the SSP application
  when it is available and thus clock in a received byte to the target receive buffer.
- Returns TRUE and loads the target SSP u16RxBytes

*/
bool SspReadByte(SspPeripheralType* psSspPeripheral_)
{
  /* Confirm Master Mode */
  if( (psSspPeripheral_->eSspMode == SSP_SLAVE) || 
      (psSspPeripheral_->eSspMode == SSP_SLAVE_FLOW_CONTROL) )
  {
    return FALSE;
  }

  /* Make sure no Tx or Rx function is already in progress */
  if( (psSspPeripheral_->u16RxBytes != 0) || (psSspPeripheral_->psTransmitBuffer != NULL) )
  {
    return FALSE;
  }
  
  /* Load the counter and return success */
  psSspPeripheral_->u16RxBytes = 1;
  return TRUE;
  
} /* end SspReadByte() */


/*!--------------------------------------------------------------------------------------------------------------------
@fn bool SspReadData(SspPeripheralType* psSspPeripheral_, u16 u16Size_)

@brief Master mode only. Prepares to get multiple bytes from the Slave on the target SSP peripheral.  

Requires:
- Master mode 

@param psSspPeripheral_ is the SSP peripheral to use and it has already been requested.
@param u32Size_ is the number of bytes to receive

Promises:
- Returns FALSE if the message is too big, or the peripheral already has a read request
- Returns TRUE and loads the target SSP u16RxBytes

*/
bool SspReadData(SspPeripheralType* psSspPeripheral_, u16 u16Size_)
{
  /* Confirm Master Mode */
  if( (psSspPeripheral_->eSspMode == SSP_SLAVE) || 
      (psSspPeripheral_->eSspMode == SSP_SLAVE_FLOW_CONTROL) )
  {
    return FALSE;
  }

  /* Make sure no Tx or Rx function is already in progress */
  if( (psSspPeripheral_->u16RxBytes != 0) || (psSspPeripheral_->psTransmitBuffer != NULL) )
  {
    return FALSE;
  }

  /* Do not allow if requested size is too large */
  if(u16Size_ > U16_MAX_TX_MESSAGE_LENGTH)
  {
    DebugPrintf("\r\nSSP message too large\n\r");
    return FALSE;
  }
  
  /* Load the counter and return success */
  psSspPeripheral_->u16RxBytes = u16Size_;
  return TRUE;
    
} /* end SspReadData() */


/*!--------------------------------------------------------------------------------------------------------------------
@fn SspRxStatusType SspQueryReceiveStatus(SspPeripheralType* psSspPeripheral_)

@brief Master mode only.  Returns status of currently requested receive data.

Requires:
@param psSspPeripheral_ is the SSP peripheral to use and it has already been requested.

Promises:
- Returns the SspRxStatusType status of the currently receiving message.

*/
SspRxStatusType SspQueryReceiveStatus(SspPeripheralType* psSspPeripheral_)
{
  /* Confirm Master Mode */
  if( (psSspPeripheral_->eSspMode == SSP_SLAVE) || 
      (psSspPeripheral_->eSspMode == SSP_SLAVE_FLOW_CONTROL) )
  {
    return SSP_RX_INVALID;
  }

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
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void SspInitialize(void)

@brief Initializes the SSP application and its variables.  

The peripherals themselves are not configured until requested by a calling application.

Requires:
- NONE

Promises:
- SSP peripheral objects are ready 
- SSP application set to Idle

*/
void SspInitialize(void)
{
  /* Initialize the SSP peripheral structures */
  SSP_Peripheral0.pBaseAddress     = AT91C_BASE_US0;
  SSP_Peripheral0.u8PeripheralId   = AT91C_ID_US0;
  SSP_Peripheral0.pCsGpioAddress   = NULL;
  SSP_Peripheral0.psTransmitBuffer = NULL;
  SSP_Peripheral0.pu8RxBuffer      = NULL;
  SSP_Peripheral0.u16RxBufferSize  = 0;
  SSP_Peripheral0.ppu8RxNextByte   = NULL;
  SSP_Peripheral0.u32PrivateFlags  = 0;
  
  SSP_Peripheral1.pBaseAddress     = AT91C_BASE_US1;
  SSP_Peripheral1.u8PeripheralId   = AT91C_ID_US1;
  SSP_Peripheral1.pCsGpioAddress   = NULL;
  SSP_Peripheral1.psTransmitBuffer = NULL;
  SSP_Peripheral1.pu8RxBuffer      = NULL;
  SSP_Peripheral1.u16RxBufferSize  = 0;
  SSP_Peripheral1.ppu8RxNextByte   = NULL;
  SSP_Peripheral1.u32PrivateFlags  = 0;

  SSP_Peripheral2.pBaseAddress     = AT91C_BASE_US2;
  SSP_Peripheral2.u8PeripheralId   = AT91C_ID_US2;
  SSP_Peripheral2.pCsGpioAddress   = NULL;
  SSP_Peripheral2.psTransmitBuffer = NULL;
  SSP_Peripheral2.pu8RxBuffer      = NULL;
  SSP_Peripheral2.u16RxBufferSize  = 0;
  SSP_Peripheral2.ppu8RxNextByte   = NULL;
  SSP_Peripheral2.u32PrivateFlags  = 0;

  /* Init starting SSP and clear all flags */
  SSP_psCurrentSsp = &SSP_Peripheral0;
  SSP_u32Flags = 0;
  G_u32Ssp0ApplicationFlags = 0;
  G_u32Ssp1ApplicationFlags = 0;
  G_u32Ssp2ApplicationFlags = 0;
  
  /* Set application pointer */
  SSP_pfnStateMachine = SspSM_Idle;
  DebugPrintf("SSP Peripherals Ready\n\r");

} /* end SspInitialize() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void SspRunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void SspRunActiveState(void)
{
  SSP_pfnStateMachine();

} /* end SspRunActiveState */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void SspManualMode(void)

@brief Runs a transmit cycle of the SSP application to clock out a message.  
This function is used only during initialization.

Requires:
- G_u32SystemFlags _SYSTEM_INITIALIZING is set
- SSP application has been initialized.

Promises:
- All currently queued SSP Master transmit and receive operations are completed.

*/
void SspManualMode(void)
{
  u32 u32Timer;
  
  /* Set up for manual mode */
  SSP_u32Flags |= _SSP_MANUAL_MODE;
  SSP_psCurrentSsp = &SSP_Peripheral0;

  /* Run the SSP state machine so all SSP peripherals send their current message */  
  while(SSP_u32Flags & _SSP_MANUAL_MODE)
  {
    SSP_pfnStateMachine();
    MessagingRunActiveState();
    
    u32Timer = G_u32SystemTime1ms;
    while( !IsTimeUp(&u32Timer, 1) );
  }
      
} /* end SspManualMode() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void SSP0_IRQHandler(void)

@brief Initial handling of enabled SSP0 interrupts before generic handler. 

Requires:
- NONE

Promises:
- Global SSP ISR variables updated for SSP0
- SSP_u32Int0Count incremented for debug purposes

*/
void SSP0_IRQHandler(void)
{
  /* Set the current ISR pointers to SSP0 targets */
  SSP_psCurrentISR = &SSP_Peripheral0;                         
  SSP_pu32SspApplicationFlagsISR = (u32*)&G_u32Ssp0ApplicationFlags; 
  SSP_u32Int0Count++;

  /* Go to common SSP interrupt using psCurrentSspISR since the SSP cannot interrupt itself */
  SspGenericHandler();
  
} /* end SSP0_IRQHandler() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void SSP1_IRQHandler(void)

@brief Initial handling of enabled SSP1 interrupts before generic handler. 

Requires:
- NONE

Promises:
- Global SSP ISR variables updated for SSP1
- SSP_u32Int1Count incremented for debug purposes

*/
void SSP1_IRQHandler(void)
{
  /* Set the current ISR pointers to SSP1 targets */
  SSP_psCurrentISR = &SSP_Peripheral1;                      
  SSP_pu32SspApplicationFlagsISR = (u32*)&G_u32Ssp1ApplicationFlags; 
  SSP_u32Int1Count++;

  /* Go to common SSP interrupt using psCurrentSspISR since the SSP cannot interrupt itself */
  SspGenericHandler();
  
} /* end SSP1_IRQHandler() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void SSP2_IRQHandler(void)

@brief Initial handling of enabled SSP2 interrupts before generic handler. 

Requires:
- NONE

Promises:
- Global SSP ISR variables updated for SSP2
- SSP_u32Int2Count incremented for debug purposes

*/
void SSP2_IRQHandler(void)
{
  /* Set the current ISR pointers to SSP2 targets */
  SSP_psCurrentISR = &SSP_Peripheral2;                      
  SSP_pu32SspApplicationFlagsISR = (u32*)&G_u32Ssp2ApplicationFlags;
  SSP_u32Int2Count++;

  /* Go to common SSP interrupt using psCurrentSspISR since the SSP cannot interrupt itself */
  SspGenericHandler();
 
} /* end SSP2_IRQHandler() */


/*----------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*----------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn static void SspGenericHandler(void)

@brief Common handler for all expected SSP interrupts regardless of base peripheral

CHIP SELECT: only enabled for SLAVE peripherals.  A Slave peripheral needs this 
signal to know it is communicating. If it is supposed to be transmitting and does 
not have any flow control, the data should already be queued to send.

TXEMPTY: Transmit for Flow Control Slaves
RXRDY: Receive for Flow Control Slaves

ENDTX: An End Transmit interrupt will occur when the PDC has finished sending all 
of the bytes for Master or Slave.

ENDRX: An End Receive interrupt will occur when the PDC has finished receiving all 
of the expected bytes for Master or a single byte for Slave.


Requires:
- SSP_psCurrentISR points to the SSP peripheral that has triggered the interrupt
- SSP_pu32SspApplicationFlagsISR points to the application flags from the SSP 
  peripheral that triggered the interrupt
- A different SSP peripheral cannot interrupt this ISR

Promises:
- Each interrupt has different outcomes for the system depending on the SSP mode.  
  See each section for more details.

*/
static void SspGenericHandler(void)
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
      /* Flag that CS is asserted and make sure TX and RX COMPLETE flags are clear */
      *SSP_pu32SspApplicationFlagsISR |= _SSP_CS_ASSERTED;
      *SSP_pu32SspApplicationFlagsISR &= ~(_SSP_TX_COMPLETE | _SSP_RX_COMPLETE);
      
      /* If a no flow control Slave is receiving, then it should be ready to respond with dummy bytes */
      if(SSP_psCurrentISR->eSspMode == SSP_SLAVE)
      {
        if(SSP_psCurrentISR->psTransmitBuffer == NULL)
        {
          SSP_psCurrentISR->pBaseAddress->US_THR = SSP_DUMMY_BYTE;
        }
      }

      /* Flow control Slaves should have their RXRDY interrupt enabled */
      if(SSP_psCurrentISR->eSspMode == SSP_SLAVE_FLOW_CONTROL)
      {
        SSP_psCurrentISR->pBaseAddress->US_IER |= AT91C_US_RXRDY;
      }

    }
    else
    {
      /* Flag that CS is deasserted */
      *SSP_pu32SspApplicationFlagsISR &= ~_SSP_CS_ASSERTED;
                 
      /* If a transmit was interrupted, then the message needs to be aborted. */
      if(SSP_psCurrentISR->u32PrivateFlags & _SSP_PERIPHERAL_TX)
      {
        /* Clean up the message status and flags */
        SSP_psCurrentISR->pBaseAddress->US_IDR = AT91C_US_ENDTX;
        
        SSP_psCurrentISR->u32PrivateFlags &= ~_SSP_PERIPHERAL_TX;  
        UpdateMessageStatus(SSP_psCurrentISR->psTransmitBuffer->u32Token, ABANDONED);
        DeQueueMessage(&SSP_psCurrentISR->psTransmitBuffer);
   
        *SSP_pu32SspApplicationFlagsISR |= _SSP_TX_COMPLETE; 
        
      }

      /* Make sure PDC counters are reset for next transmission on Slave - no flow control devices only */
      if(SSP_psCurrentISR->eSspMode == SSP_SLAVE)
      {
        SSP_psCurrentISR->pBaseAddress->US_RCR = 1;
        SSP_psCurrentISR->pBaseAddress->US_RNCR = 1;
      }

      /* Special case for an interrupted flow control mode */
      if(SSP_psCurrentISR->eSspMode == SSP_SLAVE_FLOW_CONTROL)
      {
        /* Re-enable Rx interrupt, clean-up the operation */    
        SSP_psCurrentISR->pBaseAddress->US_IER = AT91C_US_RXRDY;
      }
      
    } /* end of CS de-asserted */
    
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
      if(SSP_psCurrentISR->eBitOrder == SSP_LSB_FIRST)
      {
        u32Byte = __RBIT(u32Byte) >> 24;
      }
    
      /* Clear interrupt flag */
      SSP_psCurrentISR->pBaseAddress->US_THR = (u8)u32Byte; 
    }
    else
    {
      /* Done! Disable TX interrupt */
      SSP_psCurrentISR->pBaseAddress->US_IDR = AT91C_US_TXEMPTY;
      
      /* Clean up the message status and flags */
      *SSP_pu32SspApplicationFlagsISR |= _SSP_TX_COMPLETE; 
      UpdateMessageStatus(SSP_psCurrentISR->psTransmitBuffer->u32Token, COMPLETE);
      DeQueueMessage(&SSP_psCurrentISR->psTransmitBuffer);
      SSP_psCurrentISR->u32PrivateFlags &= ~_SSP_PERIPHERAL_TX;  
 
      /* Re-enable Rx interrupt and make final call to callback */    
      SSP_psCurrentISR->pBaseAddress->US_IER = AT91C_US_RXRDY;
    }
    
    /* Both cases use the callback */
    SSP_psCurrentISR->fnSlaveTxFlowCallback();
    
  } /* end AT91C_US_TXEMPTY */
  
  
  /*** SSP ISR handling for Slave Rx with flow control (no DMA) ***/
  if( (SSP_psCurrentISR->pBaseAddress->US_IMR & AT91C_US_RXRDY) && 
      (SSP_psCurrentISR->pBaseAddress->US_CSR & AT91C_US_RXRDY) )
  {
    /* Pull the byte out of the receive register into the Rx buffer */
    u32Byte = 0x000000FF & SSP_psCurrentISR->pBaseAddress->US_RHR;

    /* If we need LSB first, use inline assembly to flip bits with a single instruction. */
    if(SSP_psCurrentISR->eBitOrder == SSP_LSB_FIRST)
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
  } /* end of receive with flow control */

  
  /*** SSP ISR responses for non-flow control devices that use DMA (Master or Slave) ***/
    
  /* ENDRX Interrupt when all requested bytes have been received */
  if( (SSP_psCurrentISR->pBaseAddress->US_IMR & AT91C_US_ENDRX) && 
      (u32Current_CSR & AT91C_US_ENDRX) )
  {
    /* Master mode and Slave mode operate differently */
    if( (SSP_psCurrentISR->eSspMode == SSP_MASTER_AUTO_CS) ||
        (SSP_psCurrentISR->eSspMode == SSP_MASTER_MANUAL_CS) ) 
    {
      /* Reset the byte counter and clear the RX flag */
      SSP_psCurrentISR->u16RxBytes = 0;
      SSP_psCurrentISR->u32PrivateFlags &= ~_SSP_PERIPHERAL_RX;
      SSP_psCurrentISR->u32PrivateFlags |=  _SSP_PERIPHERAL_RX_COMPLETE;
      SSP_u32RxCounter++;
      
      /* Deassert CS for SSP_MASTER_AUTO_CS transfers */
      if(SSP_psCurrentSsp->eSspMode == SSP_MASTER_AUTO_CS)
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

      /* Update the pointer to the next valid Rx location (always leap-frogs the immediate 
      next byte since it will be filled by the other DMA process) */
      SSP_psCurrentISR->pBaseAddress->US_RNPR++;
      if(SSP_psCurrentISR->pBaseAddress->US_RPR == (u32)(SSP_psCurrentISR->pu8RxBuffer + (u32)SSP_psCurrentISR->u16RxBufferSize) )
      {
        SSP_psCurrentISR->pBaseAddress->US_RPR = (u32)SSP_psCurrentISR->pu8RxBuffer;  
      }
      
      /* Write RNCR to 1 to clear the ENDRX flag */
      SSP_psCurrentISR->pBaseAddress->US_RNCR = 1;
    }  
  } /* end ENDRX handling */


  /* ENDTX Interrupt when all requested transmit bytes have been sent (if enabled) */
  if( (SSP_psCurrentISR->pBaseAddress->US_IMR & AT91C_US_ENDTX) && 
      (u32Current_CSR & AT91C_US_ENDTX) )
  {
    /* If this was a non-dummy transmit... */
    if(SSP_psCurrentISR->u32PrivateFlags & _SSP_PERIPHERAL_TX)
    {
      /* Update this message token status and then DeQueue it */
      UpdateMessageStatus(SSP_psCurrentISR->psTransmitBuffer->u32Token, COMPLETE);
      DeQueueMessage(&SSP_psCurrentISR->psTransmitBuffer);
      SSP_psCurrentISR->u32PrivateFlags &= ~_SSP_PERIPHERAL_TX;
    }
 
    /* Master devices: Disable the transmitter and interrupt source. 
    No action for Slave devices as the PDC pointers are already reset back to 
    SSP_u8Dummies due to the "Next" PDC registers and the transmitter stays on.
    Flow control Slaves do not use PDC and thus will not generate this interrupt. */
    if( (SSP_psCurrentISR->eSspMode == SSP_MASTER_AUTO_CS) ||
        (SSP_psCurrentISR->eSspMode == SSP_MASTER_MANUAL_CS) )
    {
      SSP_psCurrentISR->pBaseAddress->US_PTCR = AT91C_PDC_TXTDIS;
      SSP_psCurrentISR->pBaseAddress->US_IDR  = AT91C_US_ENDTX;
    }

    /* Allow the peripheral to finish clocking out the Tx byte */
    u32Timeout = 0;
    while ( !(SSP_psCurrentISR->pBaseAddress->US_CSR & AT91C_US_TXEMPTY) && 
            u32Timeout < SSP_TXEMPTY_TIMEOUT)
    {
      u32Timeout++;
    } 
    
    /* Deassert chip select when the buffer and shift register are totally empty */
    if(SSP_psCurrentSsp->eSspMode == SSP_MASTER_AUTO_CS)
    {
      SSP_psCurrentISR->pCsGpioAddress->PIO_SODR = SSP_psCurrentISR->u32CsPin;
    }
    
  } /* end ENDTX interrupt handling */

  
} /* end SspGenericHandler() */


/***********************************************************************************************************************
State Machine Function Definitions

The SSP state machine monitors messaging activity on the available SSP Master peripherals.  
It manages all SSP outgoing  messages and will transmit any message that has been queued.  
All configured SSP peripherals can be transmitting and receiving simultaneously.
***********************************************************************************************************************/

/*!-------------------------------------------------------------------------------------------------------------------
@fn static void SspSM_Idle(void)

@brief Wait for a transmit message to be queued -- this can include a dummy transmission to 
receive bytes.

Half duplex transmissions are always assumed. Check one peripheral per iteration. 

*/
static void SspSM_Idle(void)
{
  u32 u32Byte;
  
  /* Check all SPI/SSP peripherals for message activity or skip the current peripheral 
  if it is already busy.
  Slave devices receive outside of the state machine.
  For Master devices sending a message, SSP_psCurrentSsp->psTransmitBuffer->pu8Message will 
  point to the application transmit buffer.
  For Master devices receiving a message, SSP_psCurrentSsp->u16RxBytes will != 0. Dummy bytes 
  are sent. */
  if( ( (SSP_psCurrentSsp->psTransmitBuffer != NULL) || (SSP_psCurrentSsp->u16RxBytes !=0) ) && 
     !(SSP_psCurrentSsp->u32PrivateFlags & (_SSP_PERIPHERAL_TX | _SSP_PERIPHERAL_RX)       ) 
    )
  {
    /* For an SSP_MASTER_AUTO_CS device, start by asserting chip select 
   (SSP_MASTER_MANUAL_CS devices should already have asserted CS in the user's task) */
    if(SSP_psCurrentSsp->eSspMode == SSP_MASTER_AUTO_CS)
    {
      SSP_psCurrentSsp->pCsGpioAddress->PIO_CODR = SSP_psCurrentSsp->u32CsPin;
    }
       
    /* Check if the message is receiving based on expected byte count */
    if(SSP_psCurrentSsp->u16RxBytes !=0)
    {
      /* Receiving: flag that the peripheral is now busy */
      SSP_psCurrentSsp->u32PrivateFlags |= _SSP_PERIPHERAL_RX;    
      
      /* Initialize the receive buffer so we can see data changes but also so we send
      predictable dummy bytes since we'll point to this buffer to source the transmit dummies */
      memset(SSP_psCurrentSsp->pu8RxBuffer, SSP_DUMMY_BYTE, SSP_psCurrentSsp->u16RxBufferSize);

      /* Load the PDC counter and pointer registers */
      SSP_psCurrentSsp->pBaseAddress->US_RPR = (u32)SSP_psCurrentSsp->pu8RxBuffer; 
      SSP_psCurrentSsp->pBaseAddress->US_TPR = (u32)SSP_psCurrentSsp->pu8RxBuffer; 
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
      
      /* TRANSMIT SPI_SSP_SLAVE_FLOW_CONTROL */ 
      if(SSP_psCurrentSsp->eSspMode == SSP_SLAVE_FLOW_CONTROL)
      {
        /* A Slave device with flow control uses interrupt-driven single byte transfers.
        CS must be asserted for the Slave to have queued data to get to here. */

        /* Load in the message parameters. */
        SSP_psCurrentSsp->u32CurrentTxBytesRemaining = SSP_psCurrentSsp->psTransmitBuffer->u32Size;
        SSP_psCurrentSsp->pu8CurrentTxData = SSP_psCurrentSsp->psTransmitBuffer->pu8Message;

        /* If we need LSB first, use inline assembly to flip bits with a single instruction. */
        u32Byte = 0x000000FF & *SSP_psCurrentSsp->pu8CurrentTxData;
        if(SSP_psCurrentSsp->eBitOrder == SSP_LSB_FIRST)
        {
          u32Byte = __RBIT(u32Byte)>>24;
        }
        
        /* This driver assumes half-duplex comms, so disable RX interrupt for now */
        SSP_psCurrentSsp->pBaseAddress->US_IDR = AT91C_US_RXRDY;
        
        /* Reset the transmitter since we have not been managing dummy bytes and it tends to be
        in the middle of a transmission or something that causes the wrong byte to get sent (at least on startup). */
        SSP_psCurrentSsp->pBaseAddress->US_CR = (AT91C_US_RSTTX);
        SSP_psCurrentSsp->pBaseAddress->US_CR = (AT91C_US_TXEN);
        SSP_psCurrentSsp->pBaseAddress->US_THR = (u8)u32Byte;
        SSP_psCurrentSsp->pBaseAddress->US_IER = AT91C_US_TXEMPTY;
        
        /* Trigger the callback which should provide flow-control to start transmitting */
        SSP_psCurrentSsp->fnSlaveTxFlowCallback();
      }
      
      /* TRANSMIT SSP_MASTER_AUTO_CS, SSP_MASTER_MANUAL_CS, SSP_SLAVE (no flow control) */
      /* A Master or Slave device without flow control uses the PDC */
      else
      {
        /* Load the PDC counter and pointer registers.  The "Next" pointers are never changed and will
        always point to SSP_u8Dummies with length 1.  */
        SSP_psCurrentSsp->pBaseAddress->US_TPR = (u32)SSP_psCurrentSsp->psTransmitBuffer->pu8Message; 
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
      DebugPrintf("Invalid SSP attempt\r\n");
      SSP_psCurrentSsp = &SSP_Peripheral0;
      break;
  } /* end switch */
  
} /* end SspSM_Idle() */


#if 0
/*!-------------------------------------------------------------------------------------------------------------------
@fn static void SspSM_Error(void)          

@brief Handle an error 

*/
static void SspSM_Error(void)          
{
  SSP_pfnStateMachine = SspSM_Idle;
  
} /* end SspSM_Error() */
#endif
        




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

