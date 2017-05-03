/**********************************************************************************************************************
File: sam3u_TWI.h                                                                

Description:
Header file for sam3u_TWI.c
**********************************************************************************************************************/

#ifndef __SAM3U_TWI_H
#define __SAM3U_TWI_H

#include "configuration.h"

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef enum {STOP, NO_STOP, NA} TWIStopType;
typedef enum {WRITE, READ} TWIMessageType;

typedef struct 
{
  AT91PS_TWI pBaseAddress;            /* Base address of the associated peripheral */
  MessageType* pTransmitBuffer;       /* Pointer to the transmit message linked list */
  u8* pu8RxBuffer;                    /* Pointer to receive buffer in user application */
  u32 u32Flags;                       /* Flags for peripheral */
} TWIPeripheralType;

typedef struct
{
  TWIMessageType Direction;           /* Tx/Rx Message Type */
  u32 u32Size;                        /* Size of the transfer */
  u8 u8Address;                       /* Slave address */
  u8 u8Attempts;                      /* Number of attempts taken to send msg */
  
  /* Only Applicable to Write Operations */
  TWIStopType Stop;                   
  
  /* Only Applicable to Read Operations */
  u8* pu8RxBuffer;                    /* Pointer to receive buffer in user application */
}TWIMessageQueueType;

/* TWIx_u32Flags definitions in TWIPeripheralType*/
#define   _TWI_STATUS_ERROR            (u32)0x00000001   /* Set if an error is flagged in LSR */
#define   _TWI_TRANSMITTING            (u32)0x00000002   /* Peripheral is Transmitting */
#define   _TWI_TRANS_NOT_COMP          (u32)0x00000004   /* Tx Transmit hasn't been completed */
#define   _TWI_RECEIVING               (u32)0x00000008   /* Peripheral is Receiving */


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* TWI_u32Flags (TWI application flags) */
#define _TWI_INIT_MODE                 (u32)0x00000001   /* Set to push a transmit cycle during initialization mode */

#define _TWI_ERROR_NACK                (u32)0x01000000   /* Set if a NACK is received */
#define _TWI_ERROR_INTERRUPT           (u32)0x02000000   /* Set if an unexpected interrupt occurs */

/* end of TWI_u32Flags */

#define TWI_ERROR_FLAG_MASK            (u32)0xFF000000   /* AND to TWI_u32Flags to get just error flags */

#define MAX_ATTEMPTS                   (u8)3             /* Number of attempts to send TWI msg */

#define TWI_TX_FIFO_SIZE               (u8)1             /* Size of the peripheral's transmit FIFO in bytes */
#define TWI_RX_FIFO_SIZE               (u8)1             /* Size of the peripheral's receive FIFO in bytes */

#define TWI_INIT_MSG_TIMEOUT           (u32)1000           /* Time in ms for init message to send */

#define _TWI_CR_START_BIT              (u32)(1 << 0)       /* Start Condition Control Bit */
#define _TWI_CR_STOP_BIT               (u32)(1 << 1)       /* Stop Condition Control Bit */
#define _TWI_CR_MSEN_BIT               (u32)(1 << 2)       /* Master Enable Bit */
#define _TWI_CR_SWRST_BIT              (u32)(1 << 7)       /* Software Reset Bit */

#define _TWI_MMR_MREAD_BIT             (u32)(1 << 12)      /* Read/Write Bit Or with MMR to set Read */
#define _TWI_MMR_MREAD_MASK            (u32)0xFFFFEFFF     /* And with MMR to set Write */
#define _TWI_MMR_DADR_MASK             (u32)0xFF80FFFF     /* And with MMR to Clear DADR (address) */
#define _TWI_MMR_ADDRESS_SHIFT         (u8)0x10            /* Used with << to shift address to correct position in MMR */

#define _TWI_SR_TXCOMP                 (u32)(1<<0)         /* Transmission Complete used for both TX/RX */
#define _TWI_SR_RXRDY                  (u32)(1<<1)         /* Receive Holding register ready Bit */
#define _TWI_SR_TXRDY                  (u32)(1<<2)         /* Transmit Holding register ready Bit */
#define _TWI_SR_OVRE                   (u32)(1<<6)         /* Rx Holding Buffer Overflow Bit */
#define _TWI_SR_NACK                   (u32)(1<<8)         /* NACK Received */


/**********************************************************************************************************************
* Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions */
/*--------------------------------------------------------------------------------------------------------------------*/
bool TWI0ReadByte(u8 u8SlaveAddress_, u8* pu8RxBuffer_);
bool TWI0ReadData(u8 u8SlaveAddress_, u8* pu8RxBuffer_, u32 u32Size_);
u32 TWI0WriteByte(u8 u8SlaveAddress_, u8 u8Byte_, TWIStopType Send_);
u32 TWI0WriteData(u8 u8SlaveAddress_, u32 u32Size_, u8* u8Data_, TWIStopType Send_);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void TWIInitialize(void);
void TWIRunActiveState(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/
static void TWI0FillTxBuffer(void);
static void TWIManualMode(void);
void TWI0_IRQHandler(void);
void TWI1_IRQHandler(void);

/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
void TWISM_Idle(void);
void TWISM_Transmitting(void);
void TWISM_Receiving(void);
void TWISM_Error(void);         

#endif /* __SAM3U_TWI_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
