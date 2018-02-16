/**********************************************************************************************************************
File: LPC17xx_ssp.h                                                                

Description:
Header file for LPC17xx_ssp.c
**********************************************************************************************************************/

#ifndef __LPC17XX_SSP_H
#define __LPC17XX_SSP_H

#include "configuration.h"

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef enum {SSP0, SSP1} SspNumberType;
typedef enum {MSB_FIRST, LSB_FIRST} SspBitOrderType;
typedef enum {SAVE_READ_BYTES, DISCARD_READ_BYTES} SspWriteBehaviourType;

typedef enum
{                            /* CPHA	CPOL in SSP_CR0 */
	SPI_MODE0 = 0x00000000,    /*  0     0              */
	SPI_MODE1 = 0x00000040,    /*  0     1              */
	SPI_MODE2 = 0x00000080,    /*  1     0              */
	SPI_MODE3 = 0x000000C0,    /*  1     1              */
	NUM_SPI_MODES = 4
} SpiModeType;

typedef struct 
{
  SspNumberType SspPeripheral;        /* SSP0 or SSP1 */
  LPC_GPIO_TypeDef* pGpioAddress;     /* Base address for GPIO port for chip select line */
  u32 u32Pin;                         /* Pin location for SSEL line */
  u8* pu8RxBufferAddress;             /* Address to circular receive buffer */
  u32 u32RxBufferSize;                /* Size of receive buffer in bytes */
  u8** pu8RxNextByte;                 /* Location of pointer to next byte to write in buffer */
  SspBitOrderType BitOrder;           /* MSB_FIRST or LSB_FIRST */
  SpiModeType SpiMode;                /* Signal polarity selection */
} SspConfigurationType;

typedef struct 
{
  LPC_SSP_TypeDef* pBaseAddress;      /* Base address of the associated peripheral */
  MessageType* pTransmitBuffer;       /* Pointer to the transmit message linked list */
  u8* pu8RxBuffer;                    /* Pointer to circular receive buffer in user application */
  u32 u32RxBufferSize;                /* Size of receive buffer in bytes */
  u8** pu8RxNextByte;                 /* Pointer to buffer location where next received byte will be placed */
  u32 u32Flags;                       /* Flags for peripheral */
  LPC_GPIO_TypeDef* pGpioAddress;     /* Base address for GPIO port for chip select line */
  u32 u32Pin;                         /* Pin location for SSEL line */
} SspPeripheralType;

/* SspPeripheralType u32Flags definitions */
#define   _SSP_PERIPHERAL_BUSY          (u32)0x00000001   /* Set when the peripheral is in use */
#define   _SSP_RX_BUFFER_OVERRUN        (u32)0x00000002   /* Set if the Rx FIFO overruns */



/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* SSP_u32Flags (SSP application flags) */
#define _SSP_INIT_MODE                 (u32)0x00000001   /* Set to push a transmit cycle during initialization mode */
#define _SSP_ERROR_INVALID_SSP         (u32)0x01000000   /* Set if a function case switches to default */

#define SSP_ERROR_FLAG_MASK            (u32)0xFF000000   /* AND to SSP_u32Flags to get just error flags */


#define SSP_RX_BUFFER_SIZE             (u32)256    /* Size of SSP receive buffer in bytes (4-byte aligned) */
#define SSP_SENT_TOKEN_LIST_SIZE       (u32)256    /* Size of SSP receive buffer in bytes (4-byte aligned) */
#define SSP_DUMMY_BYTE                 (u8)0xFF

#define _SSP0_PCONP_BIT                (u32)(1 << 21)
#define _SSP1_PCONP_BIT                (u32)(1 << 10)

#define _SSP0_NVIC_BIT                 (u32)(1 << 14)
#define _SSP1_NVIC_BIT                 (u32)(1 << 15)

#define _SSP_RORMIS_BIT                (u32)(1 << 0)
#define _SSP_RTMIS_BIT                 (u32)(1 << 1)
#define _SSP_RXMIS_BIT                 (u32)(1 << 2)
#define _SSP_TXMIS_BIT                 (u32)(1 << 3)

#define _SSP_RORMIC_BIT                (u32)(1 << 0)
#define _SSP_RTIC_BIT                  (u32)(1 << 1)

#define _SSP_RORMI_BIT                 (u32)(1 << 0)
#define _SSP_RTMI_BIT                  (u32)(1 << 1)
#define _SSP_RXMI_BIT                  (u32)(1 << 2)
#define _SSP_TXMI_BIT                  (u32)(1 << 3)

#define _SSP_SR_TFE_BIT                (u32)(1 << 0)
#define _SSP_SR_TNF_BIT                (u32)(1 << 1)
#define _SSP_SR_RNE_BIT                (u32)(1 << 2)
#define _SSP_SR_RFF_BIT                (u32)(1 << 3)
#define _SSP_SR_BSY_BIT                (u32)(1 << 4)


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

u32 SspReadByte(SspPeripheralType* psSspPeripheral_);
u32 SspReadData(SspPeripheralType* psSspPeripheral_, u32 u32Size_);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void SspInitialize(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/
static void SspFillTxBuffer(SspPeripheralType* psSspPeripheral_); 
static void SspReadRxBuffer(SspPeripheralType* psTargetSsp_);

static void SspManualMode(void);

void SSP0_IRQHandler(void);
void SSP1_IRQHandler(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
void SspIdle(void);
void SspTransmitting(void);
void SspError(void);         


#endif /* __LPC17XX_SSP_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
