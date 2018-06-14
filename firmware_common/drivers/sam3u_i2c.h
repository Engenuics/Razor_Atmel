/*!**********************************************************************************************************************
@file sam3u_i2c.h                                                                
@brief Header file for sam3u_TWI.c
**********************************************************************************************************************/

#ifndef __SAM3U_TWI_H
#define __SAM3U_TWI_H

#include "configuration.h"

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/

/*! 
@enum TwiStopType
@brief Type of behaviour for STOP condition during function operations 
*/
typedef enum {TWI_STOP, TWI_NO_STOP, TWI_NA} TwiStopType;


/*! 
@enum SspBitOrderType
@brief Controlled list to specify data transfer bit order. 
*/
typedef enum {TWI_WRITE, TWI_READ} TwiMessageType;


/*! 
@struct TwiPeripheralType
@brief User-defined TWI configuration information 
*/
typedef struct 
{
  AT91PS_TWI pBaseAddress;             /*!< @brief Base address of the associated peripheral */
  MessageType* pTransmitBuffer;        /*!< @brief Pointer to the transmit message struct linked list */
  u8* pu8RxBuffer;                     /*!< @brief Pointer to receive buffer in user application */
  u32 u32PrivateFlags;                 /*!< @brief Private peripheral flags */
} TwiPeripheralType;

/* u32PrivateFlags definitions in TwiPeripheralType */
#define   _TWI_STATUS_ERROR            (u32)0x00000001   /* Set if an error is flagged in LSR */
#define   _TWI_TRANSMITTING            (u32)0x00000002   /* Peripheral is Transmitting */
#define   _TWI_TRANS_NOT_COMP          (u32)0x00000004   /* Tx Transmit hasn't been completed */
#define   _TWI_RECEIVING               (u32)0x00000008   /* Peripheral is Receiving */
/* end u32PrivateFlags */


/*! 
@struct TwiMessageQueueType
@brief Message-specific information 
*/
typedef struct
{
  TwiMessageType Direction;            /*!< @brief Tx/Rx Message Type */
  u32 u32Size;                         /*!< @brief Size of the transfer */
  u8 u8Address;                        /*!< @brief Slave address */
  u8 u8Attempts;                       /*!< @brief Number of attempts taken to send msg */
  TwiStopType Stop;                    /*!< @brief WRITE ONLY: STOP condition behaviour */               
  u8* pu8RxBuffer;                     /*!< @brief READ ONLY: Pointer to receive buffer in user application */
} TwiMessageQueueType;



/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* TWI_u32Flags */
#define _TWI_INIT_MODE                 (u32)0x00000001     /*!< @brief Set to push a transmit cycle during initialization mode */

#define _TWI_ERROR_NACK                (u32)0x01000000     /*!< @brief Set if a NACK is received */
#define _TWI_ERROR_INTERRUPT           (u32)0x02000000     /*!< @brief Set if an unexpected interrupt occurs */

#define TWI_ERROR_FLAG_MASK            (u32)0xFF000000     /*!< @brief AND to TWI_u32Flags to get just error flags */
/* end of TWI_u32Flags */



#define U8_MAX_TWI_MSG_ATTEMPTS        (u8)3               /*!< @brief Number of attempts to send TWI msg */
#define U8_TWI_TX_FIFO_SIZE            (u8)1               /*!< @brief Size of the peripheral's transmit FIFO in bytes */



/*! @cond DOXYGEN_EXCLUDE */
#define TWI_MMR_ADDRESS_SHIFT          (u8)0x10            /* Used with << to shift address to correct position in MMR */

/*! @endcond */

#if 0
#define U32_TWI_INIT_MSG_TIMEOUT       (u32)1000           /* Time in ms for init message to send */
#define U8_TWI_RX_FIFO_SIZE            (u8)1               /* Size of the peripheral's receive FIFO in bytes */


#define _TWI_CR_START_BIT              (u32)(1 << 0)       /* Start Condition Control Bit */
#define _TWI_CR_STOP_BIT               (u32)(1 << 1)       /* Stop Condition Control Bit */
#define _TWI_CR_MSEN_BIT               (u32)(1 << 2)       /* Master Enable Bit */
#define _TWI_CR_SWRST_BIT              (u32)(1 << 7)       /* Software Reset Bit */

#define _TWI_MMR_MREAD_BIT             (u32)(1 << 12)      /* Read/Write Bit Or with MMR to set Read */
#define _TWI_MMR_MREAD_MASK            (u32)0xFFFFEFFF     /* And with MMR to set Write */
#define _TWI_MMR_DADR_MASK             (u32)0xFF80FFFF     /* And with MMR to Clear DADR (address) */

#define _TWI_SR_TXCOMP                 (u32)(1<<0)         /* Transmission Complete used for both TX/RX */
#define _TWI_SR_RXRDY                  (u32)(1<<1)         /* Receive Holding register ready Bit */
#define _TWI_SR_TXRDY                  (u32)(1<<2)         /* Transmit Holding register ready Bit */
#define _TWI_SR_OVRE                   (u32)(1<<6)         /* Rx Holding Buffer Overflow Bit */
#define _TWI_SR_NACK                   (u32)(1<<8)         /* NACK Received */
#endif


/**********************************************************************************************************************
* Function Declarations
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
bool TWI0ReadByte(u8 u8SlaveAddress_, u8* pu8RxBuffer_);
bool TWI0ReadData(u8 u8SlaveAddress_, u8* pu8RxBuffer_, u32 u32Size_);
u32 TWI0WriteByte(u8 u8SlaveAddress_, u8 u8Byte_, TwiStopType Send_);
u32 TWI0WriteData(u8 u8SlaveAddress_, u32 u32Size_, u8* u8Data_, TwiStopType Send_);


/*-------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
void TWIInitialize(void);
void TWIRunActiveState(void);
void TWIManualMode(void);
void TWI0_IRQHandler(void);


/*-------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
static void TWI0FillTxBuffer(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
void TwiSM_Idle(void);
void TwiSM_Transmitting(void);
void TwiSM_Receiving(void);

void TwiSM_Error(void);         



#endif /* __SAM3U_TWI_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
