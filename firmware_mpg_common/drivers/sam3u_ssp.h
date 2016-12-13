/**********************************************************************************************************************
File: sam3u_ssp.h                                                                

Description:
Header file for sam3u_ssp.c
**********************************************************************************************************************/

#ifndef __SAM3U_SSP_H
#define __SAM3U_SSP_H

#include "configuration.h"

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef enum {MSB_FIRST, LSB_FIRST} SspeBitOrderType;
typedef enum {SSP_FULL_DUPLEX, SSP_HALF_DUPLEX} SspDuplexModeType;
typedef enum {SPI_MASTER_AUTO_CS, SPI_MASTER_MANUAL_CS, SPI_SLAVE, SPI_SLAVE_FLOW_CONTROL} SspModeType;
typedef enum {SSP_RX_EMPTY = 0, SSP_RX_WAITING, SSP_RX_RECEIVING, SSP_RX_COMPLETE, SSP_RX_TIMEOUT} SspRxStatusType;

typedef struct 
{
  PeripheralType SspPeripheral;       /* Easy name of peripheral */
  AT91PS_PIO pCsGpioAddress;          /* Base address for GPIO port for chip select line */
  u32 u32CsPin;                       /* Pin location for SSEL line */
  SspeBitOrderType eBitOrder;         /* MSB_FIRST or LSB_FIRST: this is only available in SPI_SLAVE_FLOW_CONTROL mode */
  SspModeType eSspMode;               /* Type of SPI configured */
  fnCode_type fnSlaveTxFlowCallback;  /* Callback function for SPI_SLAVE_FLOW_CONTROL transmit */
  fnCode_type fnSlaveRxFlowCallback;  /* Callback function for SPI_SLAVE_FLOW_CONTROL receive */
  u8* pu8RxBufferAddress;             /* Address to circular receive buffer */
  u8** ppu8RxNextByte;                /* Location of pointer to next byte to write in buffer for SPI_SLAVE_FLOW_CONTROL only */
  u16 u16RxBufferSize;                /* Size of receive buffer in bytes */
} SspConfigurationType;

typedef struct 
{
  AT91PS_USART pBaseAddress;          /* Base address of the associated peripheral */
  AT91PS_PIO pCsGpioAddress;          /* Base address for GPIO port for chip select line */
  u32 u32CsPin;                       /* Pin location for SSEL line */
  SspeBitOrderType eBitOrder;         /* MSB_FIRST or LSB_FIRST: this is only available in SPI_SLAVE_FLOW_CONTROL mode */
  SspModeType eSspMode;               /* Type of SPI configured */
  u16 u16Pad;                         /* Preserve 4-byte alignment */
  u32 u32PrivateFlags;                /* Private peripheral flags */
  fnCode_type fnSlaveTxFlowCallback;  /* Callback function for SPI SLAVE transmit that uses flow control */
  fnCode_type fnSlaveRxFlowCallback;  /* Callback function for SPI SLAVE receive that uses flow control */
  u8* pu8RxBuffer;                    /* Pointer to receive buffer in user application */
  u8** ppu8RxNextByte;                /* Pointer to buffer location where next received byte will be placed (SPI_SLAVE_FLOW_CONTROL only) */
  u16 u16RxBufferSize;                /* Size of receive buffer in bytes */
  u16 u16RxBytes;                     /* Number of bytes to receive (DMA transfers) */
  u8 u8PeripheralId;                  /* Simple peripheral ID number */
//  u8 u8Pad;                           /* Preserve 4-byte alignment */
  MessageType* psTransmitBuffer;      /* Pointer to the transmit message struct linked list */
//  MessageType* psReceiveBuffer;       /* Pointer to the transmit message struct linked list */
  u32 u32CurrentTxBytesRemaining;     /* Counter for bytes remaining in current transfer */
  u8* pu8CurrentTxData;               /* Pointer to current location in the Tx buffer */
} SspPeripheralType;

/* u32PrivateFlags */
#define _SSP_PERIPHERAL_ASSIGNED      (u32)0x00100000    /* Set when the peripheral is in use */
#define _SSP_PERIPHERAL_TX            (u32)0x00200000    /* Set when the peripheral is transmitting */
#define _SSP_PERIPHERAL_RX            (u32)0x00400000    /* Set when the peripheral is receiving */
#define _SSP_PERIPHERAL_RX_COMPLETE   (u32)0x00800000    /* Set when the peripheral is finished receiving */


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* G_u32SspxApplicationFlags */
#define _SSP_CS_ASSERTED              (u32)0x00000001    /* INTERRUPT CONTROLLED ONLY: mirrors the CS line status to the application */
#define _SSP_TX_COMPLETE              (u32)0x00000002    /* Set when expected bytes have been transmitted; cleared automatically when new message begins or can be cleared by application */
#define _SSP_RX_COMPLETE              (u32)0x00000004    /* Set when expected bytes have been received; cleared automatically on CS or can be cleared by application */
#define _SSP_RX_OVERFLOW              (u32)0x00000008    /* Set if receiver overflows; cleared by application */
/* end G_u32SspxApplicationFlags */

/* SSP_u32Flags (local SSP application flags) */
#define _SSP_MANUAL_MODE              (u32)0x00000001   /* Set to push a cycle during initialization mode */

#define _SSP_ERROR_INVALID_SSP        (u32)0x01000000   /* Set if a function case switches to default */

#define SSP_ERROR_FLAG_MASK           (u32)0xFF000000   /* AND to SSP_u32Flags to get just error flags */
/* end of SSP_u32Flags flags */

#define SSP_DUMMY_BYTE                (u8)0x00          /* Byte to send for dummy */

#define SSP_TXEMPTY_TIMEOUT           (u32)100           /* Instruction cycles of a while loop that waits for a register to clear */


/**********************************************************************************************************************
* Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions */
/*--------------------------------------------------------------------------------------------------------------------*/
SspPeripheralType* SspRequest(SspConfigurationType* psSspConfig_);
void SspRelease(SspPeripheralType* psSspPeripheral_);

void SspAssertCS(SspPeripheralType* psSspPeripheral_);
void SspDeAssertCS(SspPeripheralType* psSspPeripheral_);

u32 SspWriteByte(SspPeripheralType* psSspPeripheral_, u8 u8Byte_);
u32 SspWriteData(SspPeripheralType* psSspPeripheral_, u32 u32Size_, u8* u8Data_);

bool SspReadData(SspPeripheralType* psSspPeripheral_, u16 u16Size_);
bool SspReadByte(SspPeripheralType* psSspPeripheral_);
SspRxStatusType SspQueryReceiveStatus(SspPeripheralType* psSspPeripheral_);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void SspInitialize(void);
void SspRunActiveState(void);

void SspManualMode(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void SSP0_IRQHandler(void);
void SSP1_IRQHandler(void);
void SSP2_IRQHandler(void);
void SspGenericHandler(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
void SspSM_Idle(void);
void SspSM_Error(void);         


#endif /* __SAM3U_SSP_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
