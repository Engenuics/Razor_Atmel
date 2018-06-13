/*!**********************************************************************************************************************
@file sam3u_ssp.h                                                                
@brief Header file for sam3u_ssp.c
**********************************************************************************************************************/

#ifndef __SAM3U_SSP_H
#define __SAM3U_SSP_H

#include "configuration.h"

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/

/*! 
@enum SspBitOrderType
@brief Controlled list to specify data transfer bit order. 
*/
typedef enum {SSP_MSB_FIRST, SSP_LSB_FIRST} SspBitOrderType;

/*! 
@enum SspModeType
@brief Controlled list of SSP modes. 
*/
typedef enum {SSP_MASTER_AUTO_CS, SSP_MASTER_MANUAL_CS, SSP_SLAVE, SSP_SLAVE_FLOW_CONTROL} SspModeType;

/*! 
@enum SspRxStatusType
@brief Controlled list of SSP Rx peripheral status. 
*/
typedef enum {SSP_RX_EMPTY = 0, SSP_RX_WAITING, SSP_RX_RECEIVING, SSP_RX_COMPLETE, SSP_RX_TIMEOUT, SSP_RX_INVALID} SspRxStatusType;

/*! 
@struct SspConfigurationType
@brief User-defined SSP configuration information 
*/
typedef struct 
{
  PeripheralType SspPeripheral;       /*!< @brief Easy name of peripheral */
  AT91PS_PIO pCsGpioAddress;          /*!< @brief Base address for GPIO port for chip select line */
  u32 u32CsPin;                       /*!< @brief Pin location for SSEL line */
  SspBitOrderType eBitOrder;          /*!< @brief SSP_MSB_FIRST or SSP_LSB_FIRST: this is only available in SSP_SLAVE_FLOW_CONTROL mode */
  SspModeType eSspMode;               /*!< @brief Type of SPI configured */
  fnCode_type fnSlaveTxFlowCallback;  /*!< @brief Callback function for SSP_SLAVE_FLOW_CONTROL transmit */
  fnCode_type fnSlaveRxFlowCallback;  /*!< @brief Callback function for SSP_SLAVE_FLOW_CONTROL receive */
  u8* pu8RxBufferAddress;             /*!< @brief Address to circular receive buffer */
  u8** ppu8RxNextByte;                /*!< @brief Location of pointer to next byte to write in buffer for SSP_SLAVE_FLOW_CONTROL only */
  u16 u16RxBufferSize;                /*!< @brief Size of receive buffer in bytes */
  u16 u16Pad;                         /*!< @brief Preserve 4-byte alignment */
} SspConfigurationType;


/*! 
@struct SspPeripheralType
@brief Full definition of SSP peripheral 
*/
typedef struct 
{
  AT91PS_USART pBaseAddress;          /*!< @brief Base address of the associated peripheral */
  AT91PS_PIO pCsGpioAddress;          /*!< @brief Base address for GPIO port for chip select line */
  u32 u32CsPin;                       /*!< @brief Pin location for SSEL line */
  SspBitOrderType eBitOrder;          /*!< @brief SSP_MSB_FIRST or SSP_LSB_FIRST: this is only available in SSP_SLAVE_FLOW_CONTROL mode */
  SspModeType eSspMode;               /*!< @brief Type of SPI configured */
  u32 u32PrivateFlags;                /*!< @brief Private peripheral flags */
  fnCode_type fnSlaveTxFlowCallback;  /*!< @brief Callback function for SPI SLAVE transmit that uses flow control */
  fnCode_type fnSlaveRxFlowCallback;  /*!< @brief Callback function for SPI SLAVE receive that uses flow control */
  u8* pu8RxBuffer;                    /*!< @brief Pointer to receive buffer in user application */
  u8** ppu8RxNextByte;                /*!< @brief Pointer to buffer location where next received byte will be placed (SSP_SLAVE_FLOW_CONTROL only) */
  u16 u16RxBufferSize;                /*!< @brief Size of receive buffer in bytes */
  u16 u16RxBytes;                     /*!< @brief Number of bytes to receive (DMA transfers) */
  u8 u8PeripheralId;                  /*!< @brief Simple peripheral ID number */
  u8 u8Pad;                           /*!< @brief Preserve 4-byte alignment */
  u16 u16Pad;                         /*!< @brief Preserve 4-byte alignment */
  MessageType* psTransmitBuffer;      /*!< @brief Pointer to the transmit message struct linked list */
  u32 u32CurrentTxBytesRemaining;     /*!< @brief Counter for bytes remaining in current transfer */
  u8* pu8CurrentTxData;               /*!< @brief Pointer to current location in the Tx buffer */
} SspPeripheralType;

/* u32PrivateFlags in SspPeripheralType */
#define _SSP_PERIPHERAL_ASSIGNED      (u32)0x00100000    /*!< @brief Set when the peripheral is in use */
#define _SSP_PERIPHERAL_TX            (u32)0x00200000    /*!< @brief Set when the peripheral is transmitting */
#define _SSP_PERIPHERAL_RX            (u32)0x00400000    /*!< @brief Set when the peripheral is receiving */
#define _SSP_PERIPHERAL_RX_COMPLETE   (u32)0x00800000    /*!< @brief Set when the peripheral is finished receiving */
/* end u32PrivateFlags */


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* G_u32SspxApplicationFlags */
#define _SSP_CS_ASSERTED              (u32)0x00000001    /*!< @brief INTERRUPT CONTROLLED ONLY: mirrors the CS line status to the application */
#define _SSP_TX_COMPLETE              (u32)0x00000002    /*!< @brief Set when expected bytes have been transmitted; cleared automatically when new message begins or can be cleared by application */
#define _SSP_RX_COMPLETE              (u32)0x00000004    /*!< @brief Set when expected bytes have been received; cleared automatically on CS or can be cleared by application */
/* end G_u32SspxApplicationFlags */

/* SSP_u32Flags (local SSP application flags) */
#define _SSP_MANUAL_MODE              (u32)0x00000001    /*!< @brief Set to push a cycle during initialization mode */

#define _SSP_ERROR_INVALID_SSP        (u32)0x01000000    /*!< @brief Set if a function case switches to default */

#define SSP_ERROR_FLAG_MASK           (u32)0xFF000000    /*!< @brief AND to SSP_u32Flags to get just error flags */
/* end of SSP_u32Flags flags */

#define SSP_DUMMY_BYTE                (u8)0x00           /*!< @brief Byte to send for dummy */

#define SSP_TXEMPTY_TIMEOUT           (u32)100           /*!< @brief Instruction cycles of a while loop that waits for a register to clear */


/**********************************************************************************************************************
* Function Declarations
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
SspPeripheralType* SspRequest(SspConfigurationType* psSspConfig_);
void SspRelease(SspPeripheralType* psSspPeripheral_);

void SspAssertCS(SspPeripheralType* psSspPeripheral_);
void SspDeAssertCS(SspPeripheralType* psSspPeripheral_);

u32 SspWriteByte(SspPeripheralType* psSspPeripheral_, u8 u8Byte_);
u32 SspWriteData(SspPeripheralType* psSspPeripheral_, u32 u32Size_, u8* u8Data_);

bool SspReadData(SspPeripheralType* psSspPeripheral_, u16 u16Size_);
bool SspReadByte(SspPeripheralType* psSspPeripheral_);
SspRxStatusType SspQueryReceiveStatus(SspPeripheralType* psSspPeripheral_);


/*-------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
void SspInitialize(void);
void SspRunActiveState(void);

void SspManualMode(void);

void SSP0_IRQHandler(void);
void SSP1_IRQHandler(void);
void SSP2_IRQHandler(void);


/*-------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
static void SspGenericHandler(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void SspSM_Idle(void);
static void SspSM_Error(void);         


#endif /* __SAM3U_SSP_H */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
