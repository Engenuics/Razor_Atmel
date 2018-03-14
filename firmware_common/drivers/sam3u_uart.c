/*!**********************************************************************************************************************
@file sam3u_uart.c                                                                
@brief Provides a driver to use UART peripherals on the SAM3U processor to send and receive data using interrupts. 

This driver covers both the dedicated UART peripheral and the three USART peripherals (assuming they are
running in asynchronous (UART) mode).

INITIALIZATION (should take place in application's initialization function):
1. Create a variable of UartConfigurationType in your application and initialize it to the desired UART peripheral,
the address of the receive buffer for the application, and the size in bytes of the receive buffer.

2. Call UartRequest() with pointer to the configuration variable created in step 1.  The returned pointer is the
UartPeripheralType object created that will be used by your application and should be assigned to a variable
accessible to your application.

3. If the application no longer needs the UART resource, call UartRelease().  

DATA TRANSFER:
1. Received bytes on the allocated peripheral will be dropped into the application's designated receive
buffer.  The buffer is written circularly, with no provision to monitor bytes that are overwritten.  The 
application is responsible for processing all received data.  The application must provide its own parsing
pointer to read the receive buffer and properly wrap around.  This pointer will not be impacted by the interrupt
service routine that may add additional characters at any time.

2. Transmitted data is queued using one of two functions, UartWriteByte() and UartWriteData().  Once the data
is queued, it is sent as soon as possible.  Each UART resource has a transmit queue, but only one UART resource
will send data at any given time from this state machine.  However, all UART resources may receive data simultaneously
through their respective interrupt handlers based on interrupt priority.

All receive functionality is automatic. Incoming bytes are deposited to the 
buffer specified in psUartConfig_

Both Tx and Rx use the peripheral DMA controller, though received bytes
are always received one at a time to allow use of a circular buffer by the
client task.

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- G_u32UartApplicationFlags
- G_u32Uart0ApplicationFlags
- G_u32Uart1ApplicationFlags
- G_u32Uart2ApplicationFlags

CONSTANTS
- NONE

TYPES
- UartConfigurationType
- UartPeripheralType

PUBLIC FUNCTIONS
- UartPeripheralType* UartRequest(UartConfigurationType* psUartConfig_)
- void UartRelease(UartPeripheralType* psUartPeripheral_)
- u32 UartWriteByte(UartPeripheralType* psUartPeripheral_, u8 u8Byte_)
- u32 UartWriteData(UartPeripheralType* psUartPeripheral_, u32 u32Size_, u8* u8Data_)

PROTECTED FUNCTIONS
- void UartInitialize(void);
- void UartRunActiveState(void);
- static void UartManualMode(void);


**********************************************************************************************************************/

#include "configuration.h"


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>Uart"
***********************************************************************************************************************/
/* New variables */
u32 G_u32UartApplicationFlags;                   /*!< @brief Status flags meant for application using this UART peripheral */
u32 G_u32Uart0ApplicationFlags;                  /*!< @brief Status flags meant for application using this UART peripheral */
u32 G_u32Uart1ApplicationFlags;                  /*!< @brief Status flags meant for application using this UART peripheral */
u32 G_u32Uart2ApplicationFlags;                  /*!< @brief Status flags meant for application using this UART peripheral */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;          /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;           /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;            /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;       /*!< @brief From main.c */



/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Uart_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Uart_pfnStateMachine;         /*!< @brief The UART application state machine function pointer */

static u32 Uart_u32Timer;                        /*!< @brief Counter used across states */
static u32 Uart_u32Flags;                        /*!< @brief Application flags for UART */
static u8 Uart_u8ActiveUarts = 0;                /*!< @brief Counting semaphore for # of active UARTs */

static UartPeripheralType Uart_sPeripheral;      /*!< @brief UART peripheral object */
static UartPeripheralType Uart_sPeripheral0;     /*!< @brief USART0 peripheral object (used as UART) */
static UartPeripheralType Uart_sPeripheral1;     /*!< @brief USART1 peripheral object (used as UART) */
static UartPeripheralType Uart_sPeripheral2;     /*!< @brief USART2 peripheral object (used as UART) */

static UartPeripheralType* Uart_psCurrentUart;   /*!< @brief Current UART peripheral being processed */
static UartPeripheralType* Uart_psCurrentISR;    /*!< @brief Current UART peripheral being processed in ISR */
static u32* Uart_pu32ApplicationFlagsISR;        /*!< @brief Current UART application status flags in ISR */

/* Buffers for simple UART implementation */
#if USE_SIMPLE_USART0
static u8  Uart_au8U0RxBuffer[U16_U0RX_BUFFER_SIZE]; /* Receive buffer for basic UART0 */
static u8* Uart_pu8U0RxBufferNextChar;           /* Pointer to location where next incoming char should be written */
static u8* Uart_pu8U0RxBufferUnreadChar;         /* Pointer to location of next char that has not yet been read */

static u8  Uart_au8U0TxBuffer[U16_U0TX_BUFFER_SIZE]; /* Transmit buffer for basic UART0 */
static u8* Uart_pu8U0TxBufferNextChar;           /* Pointer to location where next outgoing char should be written */
static u8* Uart_pu8U0TxBufferUnsentChar;         /* Pointer to location of next char that has not yet been sent */
#endif /* USE_SIMPLE_USART0 */

static u32 Uart_u32IntCount  = 0;                /*!< @brief Debug counter for UART interrupts */
static u32 Uart_u32Int0Count = 0;                /*!< @brief Debug counter for USART0 interrupts */
static u32 Uart_u32Int1Count = 0;                /*!< @brief Debug counter for USART1 interrupts */
static u32 Uart_u32Int2Count = 0;                /*!< @brief Debug counter for USART2 interrupts */


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

#if USE_SIMPLE_USART0

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
  - If there is at least 1 new character, the oldest character from the UART is loaded to pu8Byte and function returns TRUE
  - If there are no new characters, pu8Byte_ is untouched and function returns FALSE.
*/
bool Uart_getc(u8* pu8Byte_)
{
  /* Check if there is at least one new character in the buffer */
  if( UartCheckForNewChar() )
  {
    /* Read the oldest unread character */
    *pu8Byte_ = *Uart_pu8U0RxBufferUnreadChar;

    /* Safely advance the pointer */
    Uart_pu8U0RxBufferUnreadChar++;
    if(Uart_pu8U0RxBufferUnreadChar == &Uart_au8U0RxBuffer[U16_U0RX_BUFFER_SIZE])
    {
      Uart_pu8U0RxBufferUnreadChar = &Uart_au8U0RxBuffer[0];
    }
  }
  else
  {
    return FALSE;
  }
  
  return TRUE;
  
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
  if(Uart_pu8U0RxBufferNextChar == Uart_pu8U0RxBufferUnreadChar)
  {
    return(FALSE);
  }
  else
  {
    return(TRUE);
  }
  
} /* end UartCheckForNewChar() */

#endif /* USE_SIMPLE_USART0 */


/*!---------------------------------------------------------------------------------------------------------------------
@fn UartPeripheralType* UartRequest(UartConfigurationType* psUartConfig_)

@brief Requests access to a UART resource.  

If the resource is available, the transmit and receive parameters are set up
and the peripheral is made ready to use in the application.  

Requires:
- Uart_sPeripheralx perihperal objects have been initialized
- USART Peripheralx registers are not write-protected (WPEN)
- UART peripheral register initialization values in configuration.h must be set correctly
- UART/USART peripheral registers configured here are at the same address offset regardless of the peripheral. 

@param psUartConfig_ has the UART peripheral number, address of the RxBuffer, and the RxBuffer size and the calling
       application is ready to start using the peripheral.

Promises:
- Returns NULL if a resource cannot be assigned; OR
- Returns a pointer to the requested UART peripheral object if the resource is available
- Peripheral is configured and enabled 
- Peripheral interrupts are enabled.

*/
UartPeripheralType* UartRequest(UartConfigurationType* psUartConfig_)
{
  UartPeripheralType* psRequestedUart;
  u32 u32TargetCR;
  u32 u32TargetMR;
  u32 u32TargetIER;
  u32 u32TargetIDR;
  u32 u32TargetBRGR;
  
  /* Set-up is peripheral-specific */
  switch(psUartConfig_->UartPeripheral)
  {
    case UART:
    {
      psRequestedUart = &Uart_sPeripheral; 

      u32TargetCR   = UART_US_CR_INIT;
      u32TargetMR   = UART_US_MR_INIT; 
      u32TargetIER  = UART_US_IER_INIT; 
      u32TargetIDR  = UART_US_IDR_INIT;
      u32TargetBRGR = UART_US_BRGR_INIT;
      break;
    } 

    case USART0:
    {
      psRequestedUart = &Uart_sPeripheral0; 

      u32TargetCR   = USART0_US_CR_INIT;
      u32TargetMR   = USART0_US_MR_INIT; 
      u32TargetIER  = USART0_US_IER_INIT; 
      u32TargetIDR  = USART0_US_IDR_INIT;
      u32TargetBRGR = USART0_US_BRGR_INIT;
      break;
    } 

    case USART1:
    {
      psRequestedUart = &Uart_sPeripheral1; 

      u32TargetCR   = USART1_US_CR_INIT;
      u32TargetMR   = USART1_US_MR_INIT; 
      u32TargetIER  = USART1_US_IER_INIT; 
      u32TargetIDR  = USART1_US_IDR_INIT;
      u32TargetBRGR = USART1_US_BRGR_INIT;
      break;
    } 
    
    case USART2:
    {
      psRequestedUart = &Uart_sPeripheral2; 
      
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
      break;
    } 
  } /* end switch */

  /* If the requested peripheral is already assigned, return NULL now */
  if(psRequestedUart->u32PrivateFlags & _UART_PERIPHERAL_ASSIGNED)
  {
    return(NULL);
  }
  
  /* Activate and configure the peripheral */
  AT91C_BASE_PMC->PMC_PCER |= (1 << psRequestedUart->u8PeripheralId);

  psRequestedUart->pu8RxBuffer     = psUartConfig_->pu8RxBufferAddress;
  psRequestedUart->u16RxBufferSize = psUartConfig_->u16RxBufferSize;
  psRequestedUart->pu8RxNextByte   = psUartConfig_->pu8RxNextByte;
  psRequestedUart->fnRxCallback    = psUartConfig_->fnRxCallback;
  psRequestedUart->u32PrivateFlags |= _UART_PERIPHERAL_ASSIGNED;
  
  psRequestedUart->pBaseAddress->US_CR   = u32TargetCR;
  psRequestedUart->pBaseAddress->US_MR   = u32TargetMR;
  psRequestedUart->pBaseAddress->US_IER  = u32TargetIER;
  psRequestedUart->pBaseAddress->US_IDR  = u32TargetIDR;
  psRequestedUart->pBaseAddress->US_BRGR = u32TargetBRGR;

  /* Preset the receive PDC pointers and counters; the receive buffer must be starting from [0] and be at least 2 bytes long)*/
  psRequestedUart->pBaseAddress->US_RPR  = (unsigned int)psUartConfig_->pu8RxBufferAddress;
  psRequestedUart->pBaseAddress->US_RNPR = (unsigned int)((psUartConfig_->pu8RxBufferAddress) + 1);
  psRequestedUart->pBaseAddress->US_RCR  = 1;
  psRequestedUart->pBaseAddress->US_RNCR = 1;
  
  /* Enable the receiver and transmitter requests */
  psRequestedUart->pBaseAddress->US_PTCR = AT91C_PDC_RXTEN | AT91C_PDC_TXTEN;

  /* Enable UART interrupts */
  NVIC_ClearPendingIRQ( (IRQn_Type)psRequestedUart->u8PeripheralId );
  NVIC_EnableIRQ( (IRQn_Type)psRequestedUart->u8PeripheralId );
  
  return(psRequestedUart);
  
} /* end UartRequest() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void UartRelease(UartPeripheralType* psUartPeripheral_)

@brief Releases a UART resource.  

Requires:
@param psUartPeripheral_ has the UART peripheral number, address of the RxBuffer, and the RxBuffer size and the calling
       application is ready to start using the peripheral.

Promises:
- Resets peripheral object's pointers and data to safe values
- Peripheral is disabled
- Peripheral interrupts are disabled.

*/
void UartRelease(UartPeripheralType* psUartPeripheral_)
{
  /* Check to see if the peripheral is already released */
  if(psUartPeripheral_->pu8RxBuffer == NULL)
  {
    return;
  }
  
  /* First disable the interrupts */
  NVIC_DisableIRQ( (IRQn_Type)(psUartPeripheral_->u8PeripheralId) );
  NVIC_ClearPendingIRQ( (IRQn_Type)(psUartPeripheral_->u8PeripheralId) );
 
  /* Now it's safe to release all of the resources in the target peripheral */
  psUartPeripheral_->pu8RxBuffer   = NULL;
  psUartPeripheral_->pu8RxNextByte = NULL;
  psUartPeripheral_->fnRxCallback  = NULL;
  psUartPeripheral_->u32PrivateFlags = 0;

  /* Empty the transmit buffer if there were leftover messages */
  while(psUartPeripheral_->psTransmitBuffer != NULL)
  {
    UpdateMessageStatus(psUartPeripheral_->psTransmitBuffer->u32Token, ABANDONED);
    DeQueueMessage(&psUartPeripheral_->psTransmitBuffer);
  }
  
  /* Ensure the SM is in the Idle state */
  Uart_pfnStateMachine = UartSM_Idle;
 
} /* end UartRelease() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn u32 UartWriteByte(UartPeripheralType* psUartPeripheral_, u8 u8Byte_)

@brief Queues a single byte for transfer on the target UART peripheral.  

Requires:
@param psUartPeripheral_ has been requested.
@param u8Byte_ is the byte to send

Promises:
- Creates a 1-byte message at psUartPeripheral_->pTransmitBuffer that will be sent by the UART application
  when it is available.
- Returns the message token assigned to the message

*/
u32 UartWriteByte(UartPeripheralType* psUartPeripheral_, u8 u8Byte_)
{
  u32 u32Token;
  u8 u8Data = u8Byte_;
  
  /* Attempt to queue message and get a response token */
  u32Token = QueueMessage(&psUartPeripheral_->psTransmitBuffer, 1, &u8Data);
  
  if( u32Token != NULL )
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


/*!---------------------------------------------------------------------------------------------------------------------
@fn u32 UartWriteData(UartPeripheralType* psUartPeripheral_, u32 u32Size_, u8* u8Data_)

@brief Queues an array of bytes for transfer on the target UART peripheral.  

Requires:
@param psUartPeripheral_ has been requested and holds a valid pointer to a transmit buffer; even if a transmission is
       in progress, the node in the buffer that is currently being sent will not be destroyed during this function.
@param u32Size_ is the number of bytes in the data array; should not be 0
@param u8Data_ points to the first byte of the data array

Promises:
- adds the data message at psUartPeripheral_->pTransmitBuffer that will be sent by the UART application
  when it is available.
- Returns the message token assigned to the message; 0 is returned if the message cannot be queued in which case
  G_u32MessagingFlags can be checked for the reason

*/
u32 UartWriteData(UartPeripheralType* psUartPeripheral_, u32 u32Size_, u8* u8Data_)
{
  u32 u32Token;
  
  /* Check for a valid size */
  if(u32Size_ == 0)
  {
    return NULL;
  }

  /* Attempt to queue message and get a response token */
  u32Token = QueueMessage(&psUartPeripheral_->psTransmitBuffer, u32Size_, u8Data_);
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
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void UartInitialize(void)

@brief Initializes the UART application and its variables.  

UART0 is setup to be available immediately for debugging purposes.  The generic peripherals 
themselves are not fully configured until requested by a calling application.

Requires:
- NONE 

Promises:
- UART peripheral objects are ready 
- UART application set to Idle

*/
void UartInitialize(void)
{
  Uart_u32Flags = 0;

#ifdef USE_SIMPLE_USART0
  /* Setup USART0 for use as a basic debug port */
  
  /* Initialize pointers and clear the receive buffer */
  Uart_pu8U0RxBufferNextChar   = &Uart_au8U0RxBuffer[0];
  Uart_pu8U0RxBufferUnreadChar = &Uart_au8U0RxBuffer[0];
  for(u16 i = 0; i < U16_U0RX_BUFFER_SIZE; i++)
  {
    Uart_au8U0RxBuffer[i] = 0;
  }

  Uart_pu8U0TxBufferNextChar   = &Uart_au8U0TxBuffer[0];
  Uart_pu8U0TxBufferUnsentChar = &Uart_au8U0TxBuffer[0];
  for(u16 i = 0; i < U16_U0TX_BUFFER_SIZE; i++)
  {
    Uart_au8U0TxBuffer[i] = 0;
  }
  
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
  Uart_u32Timer = G_u32SystemTime1ms;
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
    if( IsTimeUp(&Uart_u32Timer, Uart_INIT_MSG_TIMEOUT) )
    {
      break;
    }
  }

#if 0
  DebugPrintf(au8Uart0StartupMsg);
  while(Uart_pu8U0TxBufferUnsentChar != Uart_pu8U0TxBufferNextChar)
  {
    UartManualMode();
  }
#endif
  
#endif /* USE_SIMPLE_USART0 */
   
  /* Initialize all the UART peripheral structures */
  Uart_sPeripheral.pBaseAddress     = (AT91S_USART*)AT91C_BASE_DBGU;
  Uart_sPeripheral.psTransmitBuffer = NULL;
  Uart_sPeripheral.pu8RxBuffer      = NULL;
  Uart_sPeripheral.u16RxBufferSize  = 0;
  Uart_sPeripheral.pu8RxNextByte    = NULL;
  Uart_sPeripheral.u32PrivateFlags  = 0;
  Uart_sPeripheral.u8PeripheralId   = AT91C_ID_DBGU;

  Uart_sPeripheral0.pBaseAddress     = AT91C_BASE_US0;
  Uart_sPeripheral0.psTransmitBuffer = NULL;
  Uart_sPeripheral0.pu8RxBuffer      = NULL;
  Uart_sPeripheral0.u16RxBufferSize  = 0;
  Uart_sPeripheral0.pu8RxNextByte    = NULL;
  Uart_sPeripheral0.u32PrivateFlags  = 0;
  Uart_sPeripheral0.u8PeripheralId   = AT91C_ID_US0;

  Uart_sPeripheral1.pBaseAddress     = AT91C_BASE_US1;
  Uart_sPeripheral1.psTransmitBuffer = NULL;
  Uart_sPeripheral1.pu8RxBuffer      = NULL;
  Uart_sPeripheral1.u16RxBufferSize  = 0;
  Uart_sPeripheral1.pu8RxNextByte    = NULL;
  Uart_sPeripheral1.u32PrivateFlags  = 0;
  Uart_sPeripheral1.u8PeripheralId   = AT91C_ID_US1;

  Uart_sPeripheral2.pBaseAddress     = AT91C_BASE_US2;
  Uart_sPeripheral2.psTransmitBuffer = NULL;
  Uart_sPeripheral2.pu8RxBuffer      = NULL;
  Uart_sPeripheral2.u16RxBufferSize  = 0;
  Uart_sPeripheral2.pu8RxNextByte    = NULL;
  Uart_sPeripheral2.u32PrivateFlags  = 0;
  Uart_sPeripheral2.u8PeripheralId   = AT91C_ID_US2;
  
  Uart_psCurrentUart               = &Uart_sPeripheral;

  /* Set application pointer */
  Uart_pfnStateMachine = UartSM_Idle;
  
} /* end UartInitialize() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void UartRunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void UartRunActiveState(void)
{
  Uart_pfnStateMachine();

} /* end UartRunActiveState */


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*!----------------------------------------------------------------------------------------------------------------------
@fn static void UartManualMode(void)

@brief Runs a transmit cycle of the UART application to clock out a message.  This function is used only during
initialization.

Requires:
- G_u32SystemFlags _SYSTEM_INITIALIZING is set
- UART application has been initialized.

Promises:
- Runs the UART task until no UART messages are queued for transmission.

*/
static void UartManualMode(void)
{
  Uart_u32Flags |=_UART_MANUAL_MODE;
  Uart_psCurrentUart = &Uart_sPeripheral;
  
  while(Uart_u32Flags &_UART_MANUAL_MODE)
  {
    Uart_pfnStateMachine();
    Uart_u32Timer = G_u32SystemTime1ms;
    while( !IsTimeUp(&Uart_u32Timer, 1) );
  }
      
} /* end UartManualMode() */


#ifdef USE_SIMPLE_USART0
/*----------------------------------------------------------------------------------------------------------------------
Interrupt Service Routine: UART0_IRQHandler

Description:
Handles the enabled UART0 interrupts. 
Receive: The UART peripheral is always enabled and ready to receive data.  Receive interrupts will occur when a
new byte has been read by the peripheral. All incoming data is dumped into the circular receive data buffer Uart_au8U0RxBuffer.
No processing is done on the data - it is up to the processing application to parse incoming data to find useful information
and to manage dummy bytes.

Note that if the Rx buffer is not read and U16_U0RX_BUFFER_SIZE characters come in, all data will be lost because of the popinter wrap.

Transmit: All data bytes in the transmit buffer are immediately sent as fast as the interrupt can process them.

Requires:
  - Only TXEMPTY and RXRDY interrupts are ever enabled
  - Transmit and receive buffers should be correctly configured 

Promises:
  - If ENDRX interrupt occurs, the received character is deposited in Uart_au8U0RxBuffer
  - If ENDTX interrupt occurs, the transmit buffer is checked to see if another character should be queued to the peripheral
*/

void USART0_IrqHandler(void)
{
  /* Check which interrupt has occurred */
  if(AT91C_BASE_US0->US_CSR & AT91C_US_RXRDY)
  {
    /* Move the received character into the buffer - clears the RXRDY flag */
    *Uart_pu8U0RxBufferNextChar = (u8)(AT91C_BASE_US0->US_RHR);
    
    /* Safely advance the pointer */
    Uart_pu8U0RxBufferNextChar++;
    if(Uart_pu8U0RxBufferNextChar == &Uart_au8U0RxBuffer[U16_U0RX_BUFFER_SIZE])
    {
      Uart_pu8U0RxBufferNextChar = &Uart_au8U0RxBuffer[0];
    }
  }
#if 0
  if(AT91C_BASE_US0->US_CSR & AT91C_US_TXEMPTY)
  {
    /* If the transmit buffer has more data to send, then queue the next byte */
    if(Uart_pu8U0TxBufferUnsentChar != Uart_pu8U0TxBufferNextChar)
    {
      /* Queue the next byte */
      AT91C_BASE_US0->US_THR = *Uart_pu8U0TxBufferUnsentChar;

      /* Safely advance the pointer */
      Uart_pu8U0TxBufferUnsentChar++;
      if(Uart_pu8U0TxBufferUnsentChar == &Uart_au8U0TxBuffer[U16_U0TX_BUFFER_SIZE])
      {
        Uart_pu8U0TxBufferUnsentChar = &Uart_au8U0TxBuffer[0];
      }
    }
    /* Otherwise all data has been sent */
    else
    {
      Uart_u32Flags &= _Uart_U0_SENDING;
      AT91C_BASE_US0->US_IER = AT91C_US_TXEMPTY;
    }
  }
#endif
} /* end UART0_IRQHandler() */
#endif /* USE_SIMPLE_USART0 */



/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void UART_IRQHandler(void)

@brief Handles the enabled UART interrupts for the basic UART. 

Requires:
- NONE

Promises:
- Gets the current interrupt context and proceeds to the Generic handler

*/
void UART_IRQHandler(void)
{
  /* Set the current ISR pointers to UART targets */
  Uart_psCurrentISR = &Uart_sPeripheral;                       
  Uart_pu32ApplicationFlagsISR = &G_u32UartApplicationFlags;   
  Uart_u32IntCount++;

  /* Go to common UART interrupt using Uart_psCurrentISR since the SSP cannot interrupt itself */
  UartGenericHandler();

} /* end Uart_IRQHandler() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void UART0_IRQHandler(void)

@brief Handles the enabled UART interrupts for UART0. 

Requires:
- NONE

Promises:
- Gets the current interrupt context and proceeds to the Generic handler

*/
void UART0_IRQHandler(void)
{
  /* Set the current ISR pointers to UART0 targets */
  Uart_psCurrentISR = &Uart_sPeripheral0;                         
  Uart_pu32ApplicationFlagsISR = &G_u32Uart0ApplicationFlags;     
  Uart_u32Int0Count++;

  /* Go to common interrupt */
  UartGenericHandler();

} /* end UART0_IRQHandler() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void UART1_IRQHandler(void)

@brief Handles the enabled UART interrupts for UART1. 

Requires:
- NONE

Promises:
- Gets the current interrupt context and proceeds to the Generic handler

*/
void UART1_IRQHandler(void)
{
  /* Set the current ISR pointers to UART1 targets */
  Uart_psCurrentISR = &Uart_sPeripheral1;                          
  Uart_pu32ApplicationFlagsISR = &G_u32Uart1ApplicationFlags;     
  Uart_u32Int1Count++;

  /* Go to common interrupt */
  UartGenericHandler();

} /* end UART1_IRQHandler() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn ISR void UART2_IRQHandler(void)

@brief Handles the enabled interrupts for UART2. 

Requires:
- NONE

Promises:
- Gets the current interrupt context and proceeds to the Generic handler

*/
void UART2_IRQHandler(void)
{
  /* Set the current ISR pointers to UART2 targets */
  Uart_psCurrentISR = &Uart_sPeripheral2;                          
  Uart_pu32ApplicationFlagsISR = &G_u32Uart2ApplicationFlags;     
  Uart_u32Int2Count++;

  /* Go to common interrupt */
  UartGenericHandler();

} /* end UART2_IRQHandler() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void UartGenericHandler(void)

@brief Common handler for all expected UART interrupts regardless of base peripheral

Receive: A requested UART peripheral is always enabled and ready to receive data.  Receive interrupts will occur when a
new byte has been read by the peripheral. All incoming data is dumped into the circular receive data buffer configured.
No processing is done on the data - it is up to the processing application to parse incoming data to find useful information
and to manage dummy bytes.  All data reception is done with DMA, but only 1 byte at a time.  Receiving is done by using
the two reception pointers to ensure no data is missed.

Transmit: All data bytes in the transmit buffer are sent using DMA and interrupts. Once the full message has been sent,
the message status is updated.

*/
void UartGenericHandler(void)
{
  /* ENDRX Interrupt when a byte has been received (RNCR is moved to RCR; RNPR is copied to RPR) */
  if( (Uart_psCurrentISR->pBaseAddress->US_IMR & AT91C_US_ENDRX) && 
      (Uart_psCurrentISR->pBaseAddress->US_CSR & AT91C_US_ENDRX) )
  {
    /* Flag that a byte has arrived */
    *Uart_pu32ApplicationFlagsISR |= _UART_RX_COMPLETE;

    /* Update the "next" DMA pointer to the next valid Rx location with wrap-around check */
    Uart_psCurrentISR->pBaseAddress->US_RNPR++;
    if(Uart_psCurrentISR->pBaseAddress->US_RNPR == (u32)(Uart_psCurrentISR->pu8RxBuffer + ( (u32)(Uart_psCurrentISR->u16RxBufferSize) & 0x0000FFFF ) ) )
    {
      Uart_psCurrentISR->pBaseAddress->US_RNPR = (u32)Uart_psCurrentISR->pu8RxBuffer;  
    }

    /* Invoke the callback */
    Uart_psCurrentISR->fnRxCallback();
    
    /* Write RNCR to 1 to clear the ENDRX flag */
    Uart_psCurrentISR->pBaseAddress->US_RNCR = 1;
    
  } /* end of ENDRX interrupt processing */

  
  /* ENDTX Interrupt when all requested transmit bytes have been sent (if enabled) */
  if( (Uart_psCurrentISR->pBaseAddress->US_IMR & AT91C_US_ENDTX) && 
      (Uart_psCurrentISR->pBaseAddress->US_CSR & AT91C_US_ENDTX) )
  {
    /* Update this message's token status and then DeQueue it */
    UpdateMessageStatus(Uart_psCurrentISR->psTransmitBuffer->u32Token, COMPLETE);
    DeQueueMessage( &Uart_psCurrentISR->psTransmitBuffer );
    Uart_psCurrentISR->u32PrivateFlags &= ~_UART_PERIPHERAL_TX;
        
    /* Disable the transmitter and interrupt source */
    Uart_psCurrentISR->pBaseAddress->US_PTCR = AT91C_PDC_TXTDIS;
    Uart_psCurrentISR->pBaseAddress->US_IDR  = AT91C_US_ENDTX;
    
    /* Decrement # of active UARTs */
    if(Uart_u8ActiveUarts != 0)
    {
      Uart_u8ActiveUarts--;
    }
    else
    {
      /* If Uart_u8ActiveUarts is already 0, then we are not properly synchronized */
      DebugPrintf("\n\rUART counter out of sync\n\r");
      Uart_u32Flags |= _UART_NO_ACTIVE_UARTS;
    }
    
  } /* end of ENDTX interrupt processing */
  
} /* end SspGenericHandler() */


/***********************************************************************************************************************
State Machine Function Definitions

The UART state machine monitors messaging activity on the available UART peripherals.  It manages outgoing messages and will
transmit any bytes that has been queued.  Only one message will be transferred at once (e.g. UART1 and UART2 will not both 
send at the same time, though both peripherals will continue to receive simultaneously).
Since all transmit and receive bytes are transferred using interrupts, the SM does not have to worry about prioritizing.

Transmitting on USART 0:
When Uart_pu8U0TxBufferUnsentChar doesn't match Uart_pu8U0TxBufferNextChar, then we know that there is data to send.
Data transfer is initiated by writing the first byte and setting the _Uart_U0_SENDING flag to keep the USART state machine 
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
#if USE_SIMPLE_USART0
  u8 u8Temp;

  /* Send on the debug UART */
  if( (Uart_pu8U0TxBufferUnsentChar != Uart_pu8U0TxBufferNextChar) && !(Uart_u32Flags & _Uart_U0_SENDING) )
  {
    /* Flag that a send on U0 is in progress */
    Uart_u32Flags |= _Uart_U0_SENDING;
    
    /* Get a copy of the next byte and safely advance the pointer */
    u8Temp = *Uart_pu8U0TxBufferUnsentChar;
    Uart_pu8U0TxBufferUnsentChar++;
    if(Uart_pu8U0TxBufferUnsentChar == &Uart_au8U0TxBuffer[U16_U0TX_BUFFER_SIZE])
    {
      Uart_pu8U0TxBufferUnsentChar = &Uart_au8U0TxBuffer[0];
    }
    
    /* Queue the next byte and enable the Tx interrupt */
    AT91C_BASE_US0->US_THR = u8Temp;
    AT91C_BASE_US0->US_IER = AT91C_US_TXEMPTY;
  }
#endif /* USE_SIMPLE_USART0 */

  /* Check all UART peripherals for message activity or skip the current peripheral if it is already busy sending.
  All receive functions take place outside of the state machine.
  Devices sending a message will have Uart_psCurrentSsp->psTransmitBuffer->pu8Message pointing to the message to send. */
  if( (Uart_psCurrentUart->psTransmitBuffer != NULL) && 
     !(Uart_psCurrentUart->u32PrivateFlags & _UART_PERIPHERAL_TX ) )
  {
    /* Transmitting: update the message's status and flag that the peripheral is now busy */
    UpdateMessageStatus(Uart_psCurrentUart->psTransmitBuffer->u32Token, SENDING);
    Uart_psCurrentUart->u32PrivateFlags |= _UART_PERIPHERAL_TX;    
      
    /* Load the PDC counter and pointer registers */
    Uart_psCurrentUart->pBaseAddress->US_TPR = (unsigned int)Uart_psCurrentUart->psTransmitBuffer->pu8Message; /* CHECK */
    Uart_psCurrentUart->pBaseAddress->US_TCR = Uart_psCurrentUart->psTransmitBuffer->u32Size;

    /* When TCR is loaded, the ENDTX flag is cleared so it is safe to enable the interrupt */
    Uart_psCurrentUart->pBaseAddress->US_IER = AT91C_US_ENDTX;
    
    /* Update active UART count and enable the transmitter to start the transfer */
    Uart_u8ActiveUarts++;
    if(Uart_u8ActiveUarts > U8_MAX_NUM_UARTS)
    {
      /* Alert that the number of actual UARTs has been exceeded */
      DebugPrintf("\n\rToo many UARTs!\n\r");
      Uart_u32Flags |= _UART_TOO_MANY_UARTS;
    }
    Uart_psCurrentUart->pBaseAddress->US_PTCR = AT91C_PDC_TXTEN;
  }
  
  /* Adjust to check the next peripheral next time through */
  switch (Uart_psCurrentUart->u8PeripheralId)
  {
    case AT91C_ID_DBGU:
      Uart_psCurrentUart = &Uart_sPeripheral0;
      break;

    case AT91C_ID_US0:
      Uart_psCurrentUart = &Uart_sPeripheral1;
      break;

    case AT91C_ID_US1:
      Uart_psCurrentUart = &Uart_sPeripheral2;
      break;

    case AT91C_ID_US2:
      Uart_psCurrentUart = &Uart_sPeripheral;
      
      /* Only clear _UART_MANUAL_MODE if all UARTs are done sending to ensure messages are sent during initialization */
      if( (G_u32SystemFlags & _SYSTEM_INITIALIZING) && !Uart_u8ActiveUarts)
      {
        Uart_u32Flags &= ~_UART_MANUAL_MODE;
      }
      break;

    default:
      Uart_psCurrentUart = &Uart_sPeripheral;
      break;
  } /* end switch */
  
} /* end UartSM_Idle() */


#ifdef USE_SIMPLE_USART0        
/*-------------------------------------------------------------------------------------------------------------------*/
/* Transmit in progress until current bytes have reached 0.  On exit, the transmit message must be dequeued.
*/
void UartSM_Transmitting(void)
{
  /* Check if all of the message bytes have completely finished sending */
  if( (Uart_u32CurrentTxBytesRemaining == 0) && 
      (Uart_psCurrentUart->pBaseAddress->US_IER & AT91C_US_TXEMPTY) )
  {
    /* Update the status queue and then dequeue the message */
    UpdateMessageStatus(Uart_psCurrentUart->pTransmitBuffer->u32Token, COMPLETE);
    DeQueueMessage(&Uart_psCurrentUart->pTransmitBuffer);

    /* Make sure _Uart_INIT_MODE flag is clear in case this was a manual cycle */
    Uart_u32Flags &= ~_Uart_INIT_MODE;
    Uart_pfnStateMachine = UartSM_Idle;
  }
    
} /* end UartSM_Transmitting() */
#endif /* USE_SIMPLE_USART0 */

/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
void UartSM_Error(void)          
{
  Uart_u32Flags &= ~UART_ERROR_FLAG_MASK;
  
  Uart_pfnStateMachine = UartSM_Idle;
  
} /* end UartSM_Error() */

          
          
          
          
        
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

