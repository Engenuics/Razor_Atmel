/**********************************************************************************************************************
File: sam3u_uart.h                                                                

Description:
Header file for sam3u_uart.c
**********************************************************************************************************************/

#ifndef __SAM3U_UART_H
#define __SAM3U_UART_H

#include "configuration.h"

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef enum {UART, USART0, USART1, USART2} UartNumberType;

typedef struct 
{
  UartNumberType UartPeripheral;      /* UARTx */
  u8* pu8RxBufferAddress;             /* Address to circular receive buffer */
  u32 u32RxBufferSize;                /* Size of receive buffer in bytes */
  u8** pu8RxNextByte;                 /* Pointer to buffer location where next received byte will be placed */
} UartConfigurationType;

typedef struct 
{
  AT91S_USART* pBaseAddress;          /* Base address of the associated peripheral */
  MessageType* pTransmitBuffer;       /* Pointer to the transmit message linked list */
  u8* pu8RxBuffer;                    /* Pointer to circular receive buffer in user application */
  u32 u32RxBufferSize;                /* Size of receive buffer in bytes */
  u8** pu8RxNextByte;                 /* Pointer to buffer location where next received byte will be placed */
  u32 u32Flags;                       /* Flags for peripheral */
} UartPeripheralType;

/* UARTx_u32Flags definitions in UartPeripheralType*/
#define   _UART_PERIPHERAL_BUSY         (u32)0x00000001   /* Set when the peripheral is in use */
#define   _UART_RX_BUFFER_OVERRUN       (u32)0x00000002   /* Set if the Rx FIFO overruns */
#define   _UART_STATUS_ERROR            (u32)0x00000004   /* Set if an error is flagged in LSR */


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* UART_u32Flags (UART application flags) */
#define _UART_INIT_MODE                 (u32)0x00000001   /* Set to push a transmit cycle during initialization mode */
#define _UART_U0_SENDING                (u32)0x00000002   /* Set when the first Tx byte of the simple USART0 is loaded */

#define _UART_ERROR_INVALID_UART        (u32)0x01000000   /* Set if an undefined UART is attempted to be parsed */
/* end of UART_u32Flags */

#define UART_ERROR_FLAG_MASK            (u32)0xFF000000   /* AND to UART_u32Flags to get just error flags */


#define U0RX_BUFFER_SIZE                (u16)256          /* Size of the simple receive buffer in bytes */
#define U0TX_BUFFER_SIZE                (u16)256          /* Size of the simple transmit buffer in bytes */
#define UART_TX_FIFO_SIZE               (u8)1             /* Size of the peripheral's transmit FIFO in bytes */
#define UART_RX_FIFO_SIZE               (u8)1             /* Size of the peripheral's receive FIFO in bytes */

/* The UART peripheral base addresses are essentially re-defined here because the defs in AT91SAM3U4.h can't be
casted back to integers for comparisons as far as we could tell! */
#define UART_BASE_DBGU                  (u32)0x400E0600
#define UART_BASE_US0                   (u32)0x40090000
#define UART_BASE_US1                   (u32)0x40094000
#define UART_BASE_US2                   (u32)0x40098000
#define UART_BASE_US3                   (u32)0x4009C000

#define UART_INIT_MSG_TIMEOUT           (u32)1000           /* Time in ms for init message to send */


/***********************************************************************************************************************
Constants / Definitions
***********************************************************************************************************************/
#define LED_INIT_MSG_TIMEOUT            (u32)1000     /* Time in ms for init message to send */


/**********************************************************************************************************************
* Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions */
/*--------------------------------------------------------------------------------------------------------------------*/
bool Uart_putc(u8 u8Char_);
u8 Uart_getc(void);
bool UartCheckForNewChar(void);

UartPeripheralType* UartRequest(UartConfigurationType* psUartConfig_);
void UartRelease(UartPeripheralType* psUartPeripheral_);

u32 UartWriteByte(UartPeripheralType* psUartPeripheral_, u8 u8Byte_);
u32 UartWriteData(UartPeripheralType* psUartPeripheral_, u32 u32Size_, u8* u8Data_);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void UartInitialize(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/
static void UartFillTxBuffer(UartPeripheralType* UartPeripheral_);
static void UartReadRxBuffer(UartPeripheralType* psTargetUart_);
static void UartManualMode(void);

void UART_IRQHandler(void);
;void USART0_IrqHandler(void);
void UART1_IRQHandler(void);
void UART2_IRQHandler(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
void UartSM_Idle(void);
void UartSM_Transmitting(void);
void UartSM_Error(void);         


#endif /* __SAM3U_UART_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
