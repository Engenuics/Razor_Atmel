/**********************************************************************************************************************
File: sam3u_uart.c                                                                

Description:
Provides a driver to use UART peripherals to send and receive data using interrupts. 
This driver covers both the dedicated UART peripheral and the three USART peripherals (assuming they are
running in asynchronous (UART) mode).

UART0 (38,400 8-N-1) gets special treatment to allow it to run very simply since it is only a debug interface.  The transmit buffer is
owned by this source file and is accessed through the API.


------------------------------------------------------------------------------------------------------------------------
API:
Simple Public Debug UART functions:
void Uart_putc(u8 u8Char_)
bool UartCheckForNewChar(void)
u8 Uart_getc(void)

Generic Public UART functions
UartPeripheralType* UartRequest(UartConfigurationType* psUartConfig_);
void UartRelease(UartPeripheralType* psUartPeripheral_);
u32 UartWriteByte(UartPeripheralType* psUartPeripheral_, u8 u8Byte_);
u32 UartWriteData(UartPeripheralType* psUartPeripheral_, u32 u32Size_, u8* u8Data_);
All receive functionality is automatic. Incoming bytes are deposited to the 
buffer specified in psUartConfig_

Private

INITIALIZATION (should take place in application's initialization function):
1. Create a variable of UartConfigurationType in your application and initialize it to the desired UART peripheral,
the address of the receive buffer for the application and the size in bytes of the receive buffer.

2. Call UartRequest() with pointer to the configuration variable created in step 1.  The returned pointer is the
UartPeripheralType object created that will be used by your application and should be assigned to a variable
accessible to your application.

3. If the application no longer needs the UART resource, call UartRelease().  

DATA TRANSFER:
1. Received bytes on the allocated peripheral will be dropped into the application's designated received
buffer.  The buffer is written circularly, with no provision to monitor bytes that are overwritten.  The 
application is responsible for processing all received data.  The application must provide its own parsing
pointer to read the receive buffer and properly wrap around.  This pointer will not be impacted by the interrupt
service routine that may add additional characters at any time.

2. Transmitted data is queued using one of two functions, UartWriteByte() and UartWriteData().  Once the data
is queued, it is sent as soon as possible.  Each UART resource has a transmit queue, but only one UART resource
will send data at any given time from this state machine.  However, all UART resources may receive data simultaneously
through their respective interrupt handlers based on interrupt priority.

**********************************************************************************************************************/

#include "configuration.h"

#define USE_SIMPLE_USART0 1  /* Define to use USART0 as a very simple byte-wise UART for debug purposes */


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile fnCode_type G_UartStateMachine;       /* The UART application state machine */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;        /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;         /* From board-specific source file */

extern volatile u32 G_u32SystemFlags;          /* From main.c */
extern volatile u32 G_u32ApplicationFlags;     /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UART_" and be declared as static.
***********************************************************************************************************************/
static u32 UART_u32Timer;                       /* Counter used across states */
static u32 UART_u32Flags;                       /* Application flags for UART */

static UartPeripheralType UART_Peripheral;      /* UART peripheral object */
static UartPeripheralType UART_Peripheral0;     /* USART0 peripheral object (used as UART) */
static UartPeripheralType UART_Peripheral1;     /* USART1 peripheral object (used as UART) */
static UartPeripheralType UART_Peripheral2;     /* USART2 peripheral object (used as UART) */

static UartPeripheralType* UART_psCurrentUart;  /* Current UART peripheral being processed */
static u32 UART_u32CurrentTxBytesRemaining;     /* Down counter for number of bytes being clocked out */
static u8* UART_pu8CurrentTxData;               /* Pointer to current message being clocked out */

static u8 UART_au8U0RxBuffer[U0RX_BUFFER_SIZE]; /* Receive buffer for basic UART0 */
static u8* UART_pu8U0RxBufferNextChar;          /* Pointer to location where next incoming char should be written */
static u8* UART_pu8U0RxBufferUnreadChar;        /* Pointer to location of next char that has not yet been read */

#if 0
static u8 UART_au8U0TxBuffer[U0TX_BUFFER_SIZE]; /* Transmit buffer for basic UART0 */
static u8* UART_pu8U0TxBufferNextChar;          /* Pointer to location where next outgoing char should be written */
static u8* UART_pu8U0TxBufferUnsentChar;        /* Pointer to location of next char that has not yet been sent */
#endif

/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: Uart_putc

Description:
Writes a char directly to the debug UART transmit buffer.

Requires:
  - Debug UART is set up

Promises:
  - Character is written directly to the UART buffer if transmitter is ready
  - Returns TRUE if character queued to transmit buffer; else returns FALSE
*/
bool Uart_putc(u8 u8Char_)
{
  /* Check if the transmitter is available */
  if(AT91C_BASE_US0->US_CSR & AT91C_US_TXRDY)
  {
    /* Load the character to send it */
    AT91C_BASE_US0->US_THR = u8Char_;    
    return TRUE;
  }
  else
  {
    /* Transmitter not ready: return FALSE */
    return FALSE;
  }
  
} /* end Uart_putc() */


/*----------------------------------------------------------------------------------------------------------------------
Function: Uart_getc

Description:
Reads a char directly to the debug UART transmit buffer.
Recommended that user first calls UartCheckForNewChar to ensure there is a valid character ready.

Requires:
  - Debug UART is set up

Promises:
  - Oldest character is returned from the UART 
  - If there are no new characters, NULL is returned but NULL could be a valid
    character (though technically should not be if all chars are input ASCII)
*/
u8 Uart_getc(void)
{
  u8 u8Char;
    
  /* Check if there is at least one new character in the buffer */
  if( UartCheckForNewChar() )
  {
    /* Read the oldest unread character */
    u8Char = *UART_pu8U0RxBufferUnreadChar;

    /* Safely advance the pointer */
    UART_pu8U0RxBufferUnreadChar++;
    if(UART_pu8U0RxBufferUnreadChar == &UART_au8U0RxBuffer[U0RX_BUFFER_SIZE])
    {
      UART_pu8U0RxBufferUnreadChar = &UART_au8U0RxBuffer[0];
    }
  }
  else
  {
    u8Char = 0x0;
  }
  
  return u8Char;
  
} /* end Uart_getc() */


/*----------------------------------------------------------------------------------------------------------------------
Function: UartCheckForNewChar

Description:
Checks to see if any new characters are present in the UART debug receive buffer.

Requires:
  - Debug UART is set up

Promises:
  - Returns TRUE if there are new chars ready in the debug UART rx buffer; otherwise
    returns FALSE
*/
bool UartCheckForNewChar(void)
{
  if(UART_pu8U0RxBufferNextChar == UART_pu8U0RxBufferUnreadChar)
  {
    return(FALSE);
  }
  else
  {
    return(TRUE);
  }
  
} /* end UartCheckForNewChar() */


/*----------------------------------------------------------------------------------------------------------------------
Function: UartRequest

Description:
Requests access to a UART resource.  If the resource is available, the transmit and receive parameters are set up
and the peripheral is made ready to use in the application.  

Requires:
  - UART_Peripheralx perihperal objects have been initialized
  - USART Peripheralx registers are not write-protected (WPEN)
  - UART peripheral register initialization values in configuration.h must be set correctly
  - psUartConfig_ has the UART peripheral number, address of the RxBuffer, and the RxBuffer size and the calling
    application is ready to start using the peripheral.
  - UART/USART peripheral registers configured here are available and at the same address offset regardless of the peripheral. 

Promises:
  - Returns a pointer to the requested UART peripheral object if the resource is available; otherwise returns NULL
  - Peripheral is configured and enabled 
  - Peripheral interrupts are enabled.
*/
UartPeripheralType* UartRequest(UartConfigurationType* psUartConfig_)
{
  UartPeripheralType* psRequestedUart;
  u32 u32TargetCR, u32TargetMR, u32TargetIER, u32TargetIDR, u32TargetBRGR;
  u32 u32TargetPerpipheralNumber;
  
  switch(psUartConfig_->UartPeripheral)
  {
    case(UART):
    {
      psRequestedUart = &UART_Peripheral; 

      u32TargetPerpipheralNumber = AT91C_ID_US0;
      psRequestedUart->pBaseAddress = AT91C_BASE_US0;
      
      u32TargetCR   = USART0_US_CR_INIT;
      u32TargetMR   = USART0_US_MR_INIT; 
      u32TargetIER  = USART0_US_IER_INIT; 
      u32TargetIDR  = USART0_US_IDR_INIT;
      u32TargetBRGR = USART0_US_BRGR_INIT;
      break;
    } 

    case(USART0):
    {
      psRequestedUart = &UART_Peripheral0; 

      u32TargetPerpipheralNumber = AT91C_ID_US0;
      psRequestedUart->pBaseAddress = AT91C_BASE_US0;
      
      u32TargetCR   = USART0_US_CR_INIT;
      u32TargetMR   = USART0_US_MR_INIT; 
      u32TargetIER  = USART0_US_IER_INIT; 
      u32TargetIDR  = USART0_US_IDR_INIT;
      u32TargetBRGR = USART0_US_BRGR_INIT;
      break;
    } 
#if 0    
    case(USART1):
    {
      psRequestedUart = &UART_Peripheral1; 

      u32TargetPerpipheralNumber = AT91C_ID_US1;
      psRequestedUart->pBaseAddress = AT91C_BASE_US1;
      
      u32TargetCR   = USART1_US_CR_INIT;
      u32TargetMR   = USART1_US_MR_INIT; 
      u32TargetIER  = USART1_US_IER_INIT; 
      u32TargetIDR  = USART1_US_IDR_INIT;
      u32TargetBRGR = USART1_US_BRGR_INIT;
      break;
    } 
    
    case(USART2):
    {
      psRequestedUart = &UART_Peripheral2; 

      u32TargetPerpipheralNumber = AT91C_ID_US2;
      psRequestedUart->pBaseAddress = AT91C_BASE_US2;
      
      u32TargetCR   = USART2_US_CR_INIT;
      u32TargetMR   = USART2_US_MR_INIT; 
      u32TargetIER  = USART2_US_IER_INIT; 
      u32TargetIDR  = USART2_US_IDR_INIT;
      u32TargetBRGR = USART2_US_BRGR_INIT;
      break;
    } 
#endif    
    default:
    {
      return(NULL);
      break;
    } 
  } /* end switch */

  /* If the requested peripheral is already assigned, return NULL now */
  if(psRequestedUart->u32Flags & _UART_PERIPHERAL_BUSY)
  {
    return(NULL);
  }
  
  /* Apply the parameters if the resource is free */
  psRequestedUart->pu8RxBuffer     = psUartConfig_->pu8RxBufferAddress;
  psRequestedUart->u32RxBufferSize = psUartConfig_->u32RxBufferSize;
  psRequestedUart->pu8RxNextByte   = psUartConfig_->pu8RxNextByte;
  psRequestedUart->u32Flags       |= _UART_PERIPHERAL_BUSY;
  
  /* Activate and configure the peripheral */
  AT91C_BASE_PMC->PMC_PCER |= (1 << u32TargetPerpipheralNumber);

  psRequestedUart->pBaseAddress->US_CR   = u32TargetCR;
  psRequestedUart->pBaseAddress->US_MR   = u32TargetMR;
  psRequestedUart->pBaseAddress->US_IER  = u32TargetIER;
  psRequestedUart->pBaseAddress->US_IDR  = u32TargetIDR;
  psRequestedUart->pBaseAddress->US_BRGR = u32TargetBRGR;
  
  /* Enable UART interrupts */
  NVIC_ClearPendingIRQ( (IRQn_Type)u32TargetPerpipheralNumber );
  NVIC_EnableIRQ( (IRQn_Type)u32TargetPerpipheralNumber );
  
  return(psRequestedUart);
  
} /* end UartRequest() */


/*----------------------------------------------------------------------------------------------------------------------
Function: UartRelease

Description:
Releases a UART resource.  

Requires:
  - psUartPeripheral_ has the UART peripheral number, address of the RxBuffer, and the RxBuffer size and the calling
    application is ready to start using the peripheral.

Promises:
  - Resets peripheral object's pointers and data to safe values
  - Peripheral is disabled
  - Peripheral interrupts are disabled.
*/
void UartRelease(UartPeripheralType* psUartPeripheral_)
{
  u32 u32TargetPerpipheralNumber;

  /* Disable interrupts, deactivate the peripheral */
  u32TargetPerpipheralNumber = (u32)(psUartPeripheral_->pBaseAddress);
  switch( u32TargetPerpipheralNumber )
  {
    case( UART_BASE_US0 ):
    {
      u32TargetPerpipheralNumber = AT91C_ID_US0;
      break;
    } 
    
    default:
    {
      return;
      break;
    } 
  } /* end switch */

  NVIC_DisableIRQ( (IRQn_Type)u32TargetPerpipheralNumber );
  NVIC_ClearPendingIRQ( (IRQn_Type)u32TargetPerpipheralNumber );
 
  /* Now it's safe to release all of the resources in the target peripheral */
  psUartPeripheral_->pu8RxBuffer     = NULL;
  psUartPeripheral_->u32RxBufferSize = 0;
  psUartPeripheral_->pu8RxNextByte   = NULL;
  psUartPeripheral_->u32Flags        = 0;
  
} /* end UartRelease() */


/*----------------------------------------------------------------------------------------------------------------------
Function: UartWriteByte

Description:
Queues a single byte for transfer on the target UART peripheral.  

Requires:
  - psUartPeripheral_ has been requested.

Promises:
  - Creates a 1-byte message at psUartPeripheral_->pTransmitBuffer that will be sent by the UART application
    when it is available.
  - Returns the message token assigned to the message
*/
u32 UartWriteByte(UartPeripheralType* psUartPeripheral_, u8 u8Byte_)
{
  u32 u32Token;
  u8 u8Data = u8Byte_;
  
  u32Token = QueueMessage(1, &u8Data, &psUartPeripheral_->pTransmitBuffer);
  if(u32Token)
  {
    /* If the system is initializing, we want to manually cycle the UART task through one iteration
    to send the message */
    if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
    {
      UartManualMode();
    }
  }
  
  return(u32Token);
  
} /* end UartWriteByte() */


/*----------------------------------------------------------------------------------------------------------------------
Function: UartWriteData

Description:
Queues a data array for transfer on the target UART peripheral.  

Requires:
  - psUartPeripheral_ has been requested and holds a valid pointer to a transmit buffer; even if a transmission is
    in progress, the node in the buffer that is currently being sent will not be destroyed during this function.
  - u32Size_ is the number of bytes in the data array
  - u8Data_ points to the first byte of the data array

Promises:
  - adds the data message at psUartPeripheral_->pTransmitBuffer that will be sent by the UART application
    when it is available.
  - Returns the message token assigned to the message; 0 is returned if the message cannot be queued in which case
    G_u32MessagingFlags can be checked for the reason
*/
u32 UartWriteData(UartPeripheralType* psUartPeripheral_, u32 u32Size_, u8* u8Data_)
{
  u32 u32Token;

  u32Token = QueueMessage(u32Size_, u8Data_, &psUartPeripheral_->pTransmitBuffer);
  if(u32Token)
  {
    /* If the system is initializing, manually cycle the UART task through one iteration to send the message */
    if(G_u32SystemFlags & _SYSTEM_INITIALIZING)
    {
      UartManualMode();
    }
  }
  
  return(u32Token);
  
} /* end UartWriteData() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: UartInitialize

Description:
Initializes the UART application and its variables.  UART0 is setup to be available immediately for debugging
purposes.  The generic peripherals themselves are not fully configured until requested by a calling application.

Requires:
  - 

Promises:
  - UART peripheral objects are ready 
  - UART application set to Idle
*/
void UartInitialize(void)
{
  u8 au8Uart0StartupMsg[] = "\n\n\r*** MPG SAM3U2 DEVELOPMENT BOARD ***\n\rDebug UART ready\n\r";
  u8* pu8Parser;
  UART_u32Flags = 0;

#ifdef USE_SIMPLE_USART0
  /* Setup USART0 for use as a basic debug port */
  
  /* Initialize pointers and clear the receive buffer */
  UART_pu8U0RxBufferNextChar   = &UART_au8U0RxBuffer[0];
  UART_pu8U0RxBufferUnreadChar = &UART_au8U0RxBuffer[0];
  for(u16 i = 0; i < U0RX_BUFFER_SIZE; i++)
  {
    UART_au8U0RxBuffer[i] = 0;
  }

#if 0  
  UART_pu8U0TxBufferNextChar   = &UART_au8U0TxBuffer[0];
  UART_pu8U0TxBufferUnsentChar = &UART_au8U0TxBuffer[0];
  for(u16 i = 0; i < U0TX_BUFFER_SIZE; i++)
  {
    UART_au8U0TxBuffer[i] = 0;
  }
#endif
  
  /* Activate the US0 clock and set peripheral configuration registers */
  AT91C_BASE_PMC->PMC_PCER |= (1 << AT91C_ID_US0);
  
  AT91C_BASE_US0->US_CR   = USART0_US_CR_INIT;
  AT91C_BASE_US0->US_MR   = USART0_US_MR_INIT;
  AT91C_BASE_US0->US_IER  = USART0_US_IER_INIT;
  AT91C_BASE_US0->US_IDR  = USART0_US_IDR_INIT;
  AT91C_BASE_US0->US_BRGR = USART0_US_BRGR_INIT;

  /* Enable U0 interrupts */
  NVIC_ClearPendingIRQ(IRQn_US0);
  NVIC_EnableIRQ(IRQn_US0);
  
  /* Print the startup message */
  UART_u32Timer = G_u32SystemTime1ms;
  pu8Parser = &au8Uart0StartupMsg[0];
  while(*pu8Parser != NULL)
  {
    /* Attempt to queue the character */
    if( Uart_putc(*pu8Parser) )
    {
      /* Advance only if character has been sent */
      pu8Parser++;
    }
       
    /* Watch for timeout */
    if( IsTimeUp(&UART_u32Timer, UART_INIT_MSG_TIMEOUT) )
    {
      break;
    }
  }

#if 0
  DebugPrintf(au8Uart0StartupMsg);
  while(UART_pu8U0TxBufferUnsentChar != UART_pu8U0TxBufferNextChar)
  {
    UartManualMode();
  }
#endif
  
#endif /* USE_SIMPLE_USART0 */
  
  /* Setup generic UARTs */
  
  /* Initialize the UART peripheral structures */
  UART_Peripheral.pBaseAddress    = (AT91S_USART*)AT91C_BASE_DBGU;
  UART_Peripheral.pTransmitBuffer = NULL;
  UART_Peripheral.pu8RxBuffer     = NULL;
  UART_Peripheral.u32RxBufferSize = 0;
  UART_Peripheral.pu8RxNextByte   = NULL;
  UART_Peripheral.u32Flags        = 0;

  UART_Peripheral0.pBaseAddress    = AT91C_BASE_US0;
  UART_Peripheral0.pTransmitBuffer = NULL;
  UART_Peripheral0.pu8RxBuffer     = NULL;
  UART_Peripheral0.u32RxBufferSize = 0;
  UART_Peripheral0.pu8RxNextByte   = NULL;
  UART_Peripheral0.u32Flags        = 0;

  UART_Peripheral1.pBaseAddress    = AT91C_BASE_US1;
  UART_Peripheral1.pTransmitBuffer = NULL;
  UART_Peripheral1.pu8RxBuffer     = NULL;
  UART_Peripheral1.u32RxBufferSize = 0;
  UART_Peripheral1.pu8RxNextByte   = NULL;
  UART_Peripheral1.u32Flags        = 0;

  UART_Peripheral2.pBaseAddress    = AT91C_BASE_US2;
  UART_Peripheral2.pTransmitBuffer = NULL;
  UART_Peripheral2.pu8RxBuffer     = NULL;
  UART_Peripheral2.u32RxBufferSize = 0;
  UART_Peripheral2.pu8RxNextByte   = NULL;
  UART_Peripheral2.u32Flags        = 0;
  
  UART_psCurrentUart               = &UART_Peripheral;
  UART_u32CurrentTxBytesRemaining  = 0;
  UART_pu8CurrentTxData            = NULL;

  /* Set application pointer */
  G_UartStateMachine = UartSM_Idle;
  
} /* end UartInitialize() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: UartFillTxBuffer

Description:
Fills the UART peripheral buffer with bytes from the current messsage that is sending.  
This function can be called from the UART ISR!
Note: if the implemented processor does not have a FIFO, this function can still be used but will only ever
add one byte to the transmitter.

Requires:
  - The TxBuffer is empty
  - psUartPeripheral_ points to the UART peripheral being used.  
  - UART_pu8CurrentTxData points to the next byte in the message to be sent
  - UART_u32CurrentTxBytesRemaining has an accurate count of the bytes remaining in the message data to be sent
  - Transmit interrupts are off

Promises:
  - Data from *UART_pu8CurrentTxData is added to the UART peripheral Tx FIFO until the FIFO is full or there
    is no more data to send.
*/
static void UartFillTxBuffer(UartPeripheralType* psUartPeripheral_)
{
  u8 u8ByteCount = UART_TX_FIFO_SIZE;
  
  /* Use the active global variables to fill up the transmit FIFO */
  while( (u8ByteCount != 0) && (UART_u32CurrentTxBytesRemaining != 0) )
  {
    psUartPeripheral_->pBaseAddress->US_THR = *UART_pu8CurrentTxData;
    UART_pu8CurrentTxData++;
    UART_u32CurrentTxBytesRemaining--;
    u8ByteCount--;
  }
    
  /* If there are no remaining bytes to load to the TX FIFO, disable the UART transmit 
  FIFO empty interrupt */
  if(UART_u32CurrentTxBytesRemaining == 0)
  {
    psUartPeripheral_->pBaseAddress->US_IDR = AT91C_US_TXEMPTY;
  }
  /* Otherwise make sure transmit interrupts are enabled */
  else
  {
    psUartPeripheral_->pBaseAddress->US_IER = AT91C_US_TXEMPTY;
  }
  
} /* end UartFillTxBuffer() */


/*----------------------------------------------------------------------------------------------------------------------
Function: UartReadRxBuffer

Description:
Reads all bytes from the UART peripheral Rx FIFO and places them in the application receive buffer.  
This function is only called from the UART ISR so interrupts will be off.

Requires:
  - UART_pu8CurrentTxData points to the next byte in the message to be sent
  - UART_u32CurrentTxBytesRemaining has an accurate count of the bytes remaining in the message data to be sent
  - Transmit interrupts are off

Promises:
  - All bytes currently in the UART Rx FIFO are read out to the application receive circular buffer.
*/
static void UartReadRxBuffer(UartPeripheralType* psTargetUart_) 
{
  u8 u8Test;
  
  /* Read all the bytes in the Rx FIFO */
  while(psTargetUart_->pBaseAddress->US_CSR & AT91C_US_RXRDY)
  {
    u8Test = psTargetUart_->pBaseAddress->US_RHR;
    **(psTargetUart_->pu8RxNextByte) = u8Test; 

    /* Safely advance the pointer in the circular buffer */
    (*psTargetUart_->pu8RxNextByte)++;
    if( *psTargetUart_->pu8RxNextByte >= ( psTargetUart_->pu8RxBuffer + psTargetUart_->u32RxBufferSize ) )
    {
      *psTargetUart_->pu8RxNextByte = psTargetUart_->pu8RxBuffer; 
    }
    /* Always zero the current char in the buffer */
    **(psTargetUart_->pu8RxNextByte) = 0;
  }
      
} /* end UartReadRxBuffer() */


/*----------------------------------------------------------------------------------------------------------------------
Function: UartManualMode

Description:
Runs a transmit cycle of the UART application to clock out a message.  This function is used only during
initialization.

Requires:
  - UART application has been initialized.

Promises:
  - All bytes currently in the UART Rx FIFO are read out to the application receive circular buffer.
*/
static void UartManualMode(void)
{
  UART_u32Flags |=_UART_INIT_MODE;
  UART_u32Timer  = G_u32SystemTime1ms;
  
  while(UART_u32Flags &_UART_INIT_MODE)
  {
    G_UartStateMachine();
    IsTimeUp(&UART_u32Timer, 1);
  }
      
} /* end UartManualMode() */


#ifdef USE_SIMPLE_USART0
/*----------------------------------------------------------------------------------------------------------------------
Interrupt Service Routine: UART0_IRQHandler

Description:
Handles the enabled UART0 interrupts. 
Receive: The UART peripheral is always enabled and ready to receive data.  Receive interrupts will occur when a
new byte has been read by the peripheral. All incoming data is dumped into the circular receive data buffer UART_au8U0RxBuffer.
No processing is done on the data - it is up to the processing application to parse incoming data to find useful information
and to manage dummy bytes.

Note that if the Rx buffer is not read and U0RX_BUFFER_SIZE characters come in, all data will be lost because of the popinter wrap.

Transmit: All data bytes in the transmit buffer are immediately sent as fast as the interrupt can process them.

Requires:
  - Only TXEMPTY and RXRDY interrupts are ever enabled
  - Transmit and receive buffers should be correctly configured 

Promises:
  - If ENDRX interrupt occurs, the received character is deposited in UART_au8U0RxBuffer
  - If ENDTX interrupt occurs, the transmit buffer is checked to see if another character should be queued to the peripheral
*/

void USART0_IrqHandler(void)
{
  /* Check which interrupt has occurred */
  if(AT91C_BASE_US0->US_CSR & AT91C_US_RXRDY)
  {
    /* Move the received character into the buffer - clears the RXRDY flag */
    *UART_pu8U0RxBufferNextChar = (u8)(AT91C_BASE_US0->US_RHR);
    
    /* Safely advance the pointer */
    UART_pu8U0RxBufferNextChar++;
    if(UART_pu8U0RxBufferNextChar == &UART_au8U0RxBuffer[U0RX_BUFFER_SIZE])
    {
      UART_pu8U0RxBufferNextChar = &UART_au8U0RxBuffer[0];
    }
  }
#if 0
  if(AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY)
  {
    /* If the transmit buffer has more data to send, then queue the next byte */
    if(UART_pu8U0TxBufferUnsentChar != UART_pu8U0TxBufferNextChar)
    {
      /* Queue the next byte */
      AT91C_BASE_US0->US_THR = *UART_pu8U0TxBufferUnsentChar;

      /* Safely advance the pointer */
      UART_pu8U0TxBufferUnsentChar++;
      if(UART_pu8U0TxBufferUnsentChar == &UART_au8U0TxBuffer[U0TX_BUFFER_SIZE])
      {
        UART_pu8U0TxBufferUnsentChar = &UART_au8U0TxBuffer[0];
      }
    }
    /* Otherwise all data has been sent */
    else
    {
      UART_u32Flags &= _UART_U0_SENDING;
      AT91C_BASE_US0->US_IER = AT91C_US_TXEMPTY;
    }
  }
#endif
} /* end UART0_IRQHandler() */
#endif /* USE_SIMPLE_USART0 */


#ifndef USE_SIMPLE_USART0
/*----------------------------------------------------------------------------------------------------------------------
Interrupt Service Routine: UART0_IRQHandler

Description:
Handles the enabled UART0 interrupts. 
Receive: The UART peripheral is always enabled and ready to receive data.  Receive interrupts will occur when a
new byte has been read by the peripheral. All incoming data is dumped into the circular receive data buffer configured in UART_Peripheral0.
No processing is done on the data - it is up to the processing application to parse incoming data to find useful information
and to manage dummy bytes.

Transmit: All data bytes in the transmit buffer are 

Requires:
  - Transmit and receive buffers should be correctly configured 

Promises:
  - Any transmit data will be filled in the Tx FIFO
  - Any received data will be flushed to the receiver's circular buffer
*/

void UART_IRQHandler(void)
{
  u8 u8LSR;
  u32 u32UARTInterruptID;

  /* Read IIR to get the interrupt source ID: clears interrupt flag */
  u32UARTInterruptID = LPC_UART0->IIR;
  u32UARTInterruptID &= UART_INT_ID_MASK;

  /*  Receive Line Status (RLS) is highest priority interrupt.  Check the status code if this interrupt
  is currently enabled */
  if( (LPC_UART0->US_IER & _UART_INT_ENABLE_RLS) && (u32UARTInterruptID == UART_INT_ID_RLS) )
  {
    /* Get the current status register (clears flag) and check for error bits */
    u8LSR = LPC_UART0->LSR;
	  if (u8LSR & UART_ERROR_BITS)
    {
      /* Flag the error for the application to use and exit */
      UART_Peripheral0.u32Flags |= _UART_STATUS_ERROR;
      
      return;
    }
  }

  /*  Receive Data Available (RDA) / Character Timeout Indicator (CTI) are second level of priority. 
  Data should be read if this interrupt is currently enabled */
  if( (LPC_UART0->IER & _UART_INT_ENABLE_RBR) && 
      ( (u32UARTInterruptID == UART_INT_ID_RDA) || (u32UARTInterruptID == UART_INT_ID_CTI) ) )
  {
    /* Read the data until the Rx FIFO is empty (reading RBR clears interrupt flag)*/
    UartReadRxBuffer(&UART_Peripheral0);
  }

  /*  Transmit FIFO empty (3rd level of priority). Fill the transmit buffer if this interrupt is currently enabled */
  if( (LPC_UART0->IER & _UART_INT_ENABLE_THRE) && (u32UARTInterruptID == UART_INT_ID_THRE)  )
  {
    /* Reload the UART transmit buffer */
    UartFillTxBuffer(&UART_Peripheral0);
  }

} /* end UART_IRQHandler() */
#endif /* USE_SIMPLE_USART0 */


/***********************************************************************************************************************
State Machine Function Definitions

The UART state machine monitors messaging activity on the available UART peripherals.  It manages outgoing messages and will
transmit any bytes that has been queued.  Only one message will be transferred at once (e.g. UART1 and UART2 will not both 
send at the same time, though both peripherals will continue to receive simultaneously).
Since all transmit and receive bytes are transferred using interrupts, the SM does not have to worry about prioritizing.

Transmitting on USART 0:
When UART_pu8U0TxBufferUnsentChar doesn't match UART_pu8U0TxBufferNextChar, then we know that there is data to send.
Data transfer is initiated by writing the first byte and setting the _UART_U0_SENDING flag to keep the USART state machine 
busy sending all of the current data on the USART.  The interrupt service routine will be responsible for clearing the
bit which will allow the SM to return to Idle.  

Receiving on USART 0:
Since the UART can only talk to one device, we will hard-code some of the functionality.  Reception of bytes will
simply dump into the UartRxBuffer and the task interested in those bytes can read the data.  In this case, we
know that this is only the Debug / User interface task.  Though other tasks could also access the buffer, we
assume they won't.

***********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a transmit message to be queued.  Received data is handled in interrupts. */
void UartSM_Idle(void)
{
#if 0 /* Debating to do this for the debug UART, but for now, send characters directly */
  u8 u8Temp;

  /* Send on the debug UART */
  if( (UART_pu8U0TxBufferUnsentChar != UART_pu8U0TxBufferNextChar) && !(UART_u32Flags & _UART_U0_SENDING) )
  {
    /* Flag that a send on U0 is in progress */
    UART_u32Flags |= _UART_U0_SENDING;
    
    /* Get a copy of the next byte and safely advance the pointer */
    u8Temp = *UART_pu8U0TxBufferUnsentChar;
    UART_pu8U0TxBufferUnsentChar++;
    if(UART_pu8U0TxBufferUnsentChar == &UART_au8U0TxBuffer[U0TX_BUFFER_SIZE])
    {
      UART_pu8U0TxBufferUnsentChar = &UART_au8U0TxBuffer[0];
    }
    
    /* Queue the next byte and enable the Tx interrupt */
    AT91C_BASE_US0->US_THR = u8Temp;
    AT91C_BASE_US0->US_IER = AT91C_US_TXEMPTY;
  }
#endif 
  
  /* Check which peripheral is processed next */
  if(UART_psCurrentUart->pBaseAddress == (AT91S_USART*)UART_BASE_DBGU)
  {
    UART_psCurrentUart = &UART_Peripheral1;
  }
  else if(UART_psCurrentUart->pBaseAddress == AT91C_BASE_US1)
  {
    UART_psCurrentUart = &UART_Peripheral2;
  }
  else if(UART_psCurrentUart->pBaseAddress == AT91C_BASE_US2)
  {
    UART_psCurrentUart = &UART_Peripheral;
  }
  else 
  {
    UART_u32Flags |= _UART_ERROR_INVALID_UART;
  }

  /* Check if a message has been queued on the current UART */
  if(UART_psCurrentUart->pTransmitBuffer != NULL)
  {
    /* Set up to transmit the message */
    UART_u32CurrentTxBytesRemaining = UART_psCurrentUart->pTransmitBuffer->u32Size;
    UART_pu8CurrentTxData = UART_psCurrentUart->pTransmitBuffer->pu8Message;
    UartFillTxBuffer(UART_psCurrentUart);
    
    /* Update the message's status */
    UpdateMessageStatus(UART_psCurrentUart->pTransmitBuffer->u32Token, SENDING);

    /* Proceed to next state to let the current message send */
    G_UartStateMachine = UartSM_Transmitting;
  }
  
  /* Check for errors */
  if(UART_u32Flags & UART_ERROR_FLAG_MASK)
  {
    /* Reset peripheral parser and go to error state */
    UART_psCurrentUart = &UART_Peripheral;
    G_UartStateMachine = UartSM_Error;
  }
  
} /* end UartSM_Idle() */

        
/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit in progress until current bytes have reached 0.  On exit, the transmit message must be dequeued.
*/
void UartSM_Transmitting(void)
{
  /* Check if all of the message bytes have completely finished sending */
  if( (UART_u32CurrentTxBytesRemaining == 0) && 
      (UART_psCurrentUart->pBaseAddress->US_IER & AT91C_US_TXEMPTY) )
  {
    /* Update the status queue and then dequeue the message */
    UpdateMessageStatus(UART_psCurrentUart->pTransmitBuffer->u32Token, COMPLETE);
    DeQueueMessage(&UART_psCurrentUart->pTransmitBuffer);

    /* Make sure _UART_INIT_MODE flag is clear in case this was a manual cycle */
    UART_u32Flags &= ~_UART_INIT_MODE;
    G_UartStateMachine = UartSM_Idle;
  }
    
} /* end UartSM_Transmitting() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
void UartSM_Error(void)          
{
  UART_u32Flags &= ~UART_ERROR_FLAG_MASK;
  
  G_UartStateMachine = UartSM_Idle;
  
} /* end UartSM_Error() */

          
          
          
          
        
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

