/*!**********************************************************************************************************************
@file sam3u_spi.h                                                                
@brief Header file for sam3u_spi.c
**********************************************************************************************************************/

#ifndef __SAM3U_SPI_H
#define __SAM3U_SPI_H

#include "configuration.h"

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/

/*! 
@enum SpiBitOrderType
@brief Controlled list to specify data transfer bit order. 
*/
typedef enum {SPI_MSB_FIRST, SPI_LSB_FIRST} SpiBitOrderType;

/*! 
@enum SpiModeType
@brief Controlled list of SPI modes. 
*/
typedef enum {SPI_MASTER, SPI_SLAVE} SpiModeType;

/*! 
@enum SpiRxStatusType
@brief Controlled list of SPI peripheral Rx status. 
*/
typedef enum {SPI_RX_EMPTY = 0, SPI_RX_WAITING, SPI_RX_RECEIVING, SPI_RX_COMPLETE, SPI_RX_TIMEOUT, SPI_RX_INVALID} SpiRxStatusType;


/*! 
@struct SpiConfigurationType
@brief User-defined SPI configuration information 
*/
typedef struct 
{
  PeripheralType SpiPeripheral;       /*!< @brief Easy name of peripheral */
  AT91PS_PIO pCsGpioAddress;          /*!< @brief Base address for GPIO port for chip select line */
  u32 u32CsPin;                       /*!< @brief Pin location for SSEL line */
  SpiBitOrderType eBitOrder;          /*!< @brief MSB_FIRST or LSB_FIRST: this is only available in SPI_SLAVE_FLOW_CONTROL mode */
  SpiModeType eSpiMode;               /*!< @brief Type of SPI configured */
  u16 u16RxBufferSize;                /*!< @brief Size of receive buffer in bytes */
  u8* pu8RxBufferAddress;             /*!< @brief Address to circular receive buffer */
  u8** ppu8RxNextByte;                /*!< @brief Location of pointer to next byte to write in buffer for SPI_SLAVE_FLOW_CONTROL only */
} SpiConfigurationType;


/*! 
@struct SpiPeripheralType
@brief Full definition of SPI peripheral 
*/
typedef struct 
{
  AT91PS_SPI pBaseAddress;            /*!< @brief Base address of the associated peripheral */
  AT91PS_PIO pCsGpioAddress;          /*!< @brief Base address for GPIO port for chip select line */
  u32 u32CsPin;                       /*!< @brief Pin location for SSEL line */
  SpiBitOrderType eBitOrder;          /*!< @brief MSB_FIRST or LSB_FIRST: this is only available in SPI_SLAVE_FLOW_CONTROL mode */
  SpiModeType eSpiMode;               /*!< @brief Type of SPI configured */
  u8 u8PeripheralId;                  /*!< @brief Simple peripheral ID number */
  u8 u8Pad;                           /*!< @brief Preserve 4-byte alignment */
  u32 u32PrivateFlags;                /*!< @brief Private peripheral flags */
  u8* pu8RxBuffer;                    /*!< @brief Pointer to receive buffer in user application */
  u8** ppu8RxNextByte;                /*!< @brief Pointer to buffer location where next received byte will be placed (SPI_SLAVE_FLOW_CONTROL only) */
  u16 u16RxBufferSize;                /*!< @brief Size of receive buffer in bytes */
  u16 u16RxBytes;                     /*!< @brief Number of bytes to receive */
  MessageType* psTransmitBuffer;      /*!< @brief Pointer to the transmit message struct linked list */
  u32 u32CurrentTxBytesRemaining;     /*!< @brief Counter for bytes remaining in current transfer */
  u8* pu8CurrentTxData;               /*!< @brief Pointer to current location in the Tx buffer */
} SpiPeripheralType;

/* u32PrivateFlags in SpiPeripheralType */
#define _SPI_PERIPHERAL_ASSIGNED      (u32)0x00100000    /*!< @brief Set when the peripheral is in use */
#define _SPI_PERIPHERAL_TX            (u32)0x00200000    /*!< @brief Set when the peripheral is transmitting */
#define _SPI_PERIPHERAL_RX            (u32)0x00400000    /*!< @brief Set when the peripheral is receiving */
#define _SPI_PERIPHERAL_RX_COMPLETE   (u32)0x00800000    /*!< @brief Set when the peripheral is finished receiving */
/* end u32PrivateFlags */


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* G_u32Spi0ApplicationFlags */
#define _SPI_CS_ASSERTED              (u32)0x00000001    /*!< @brief INTERRUPT CONTROLLED ONLY: mirrors the CS line status to the application */
#define _SPI_TX_COMPLETE              (u32)0x00000002    /*!< @brief Set when expected bytes have been transmitted; cleared automatically when new message begins or can be cleared by application */
#define _SPI_RX_COMPLETE              (u32)0x00000004    /*!< @brief Set when expected bytes have been received; cleared automatically on CS or can be cleared by application */
/* end G_u32Spi0ApplicationFlags */


/* SPI_u32Flags (local SPI application flags) */
#define _SPI_MANUAL_MODE              (u32)0x00000001    /*!< @brief Set to push a cycle during initialization mode */

#define _SPI_ERROR_INVALID_SPI        (u32)0x01000000    /*!< @brief Set if a function case switches to default */

#define SPI_ERROR_FLAG_MASK           (u32)0xFF000000    /*!< @brief AND to SPI_u32Flags to get just error flags */
/* end of SPI_u32Flags flags */

#define SPI_DUMMY                     (u8)0xAA           /*!< @brief Byte to send for dummy */

#define SPI_TXEMPTY_TIMEOUT           (u32)100           /*!< @brief Instruction cycles of a while loop that waits for a register to clear */


/**********************************************************************************************************************
* Function Declarations
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
SpiPeripheralType* SpiRequest(SpiConfigurationType* psSpiConfig_);
void SpiRelease(SpiPeripheralType* psSpiPeripheral_);

u32 SpiWriteByte(SpiPeripheralType* psSpiPeripheral_, u8 u8Byte_);
u32 SpiWriteData(SpiPeripheralType* psSpiPeripheral_, u32 u32Size_, u8* pu8Data_);

bool SpiReadByte(SpiPeripheralType* psSpiPeripheral_);
bool SpiReadData(SpiPeripheralType* psSpiPeripheral_, u16 u16Size_);
SpiRxStatusType SpiQueryReceiveStatus(SpiPeripheralType* psSpiPeripheral_);


/*-------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
void SpiInitialize(void);
void SpiRunActiveState(void);

void SpiManualMode(void);

void SPI0_IrqHandler(void);


/*-------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void SpiSM_Idle(void);
static void SpiSM_Error(void);         


#endif /* __SAM3U_SPI_H */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
