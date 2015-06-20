/**********************************************************************************************************************
File: ant.h                                                                

Description:
ANT implementation for Cortex-M3 / AP2 SPI
**********************************************************************************************************************/

#ifndef __ANT_H
#define __ANT_H


/*******************************************************************************
* Type definitions
*******************************************************************************/

typedef struct 
{
  u8 AntChannel;
  u8 AntChannelType;
  u8 AntNetwork;
  u8 AntSerialLo;
  u8 AntSerialHi;
  u8 AntDeviceType;
  u8 AntTransmissionType;
  u8 AntChannelPeriodLo;
  u8 AntChannelPeriodHi;
  u8 AntFrequency;
  u8 AntTxPower;
} AntSetupDataType;

/* Message struct for outgoing data messages */
typedef struct
{
  u32 u32TimeStamp;                     /* Current G_u32SystemTime1s */
  u8 au8MessageData[MESG_MAX_SIZE];     /* Array for message data */
  void *psNextMessage;                  /* Pointer to AntDataMessageStructType */
} AntOutgoingMessageListType;


/*******************************************************************************
* Constants
*******************************************************************************/
/* Configuration */
#define ANT_MASTER  TRUE
#define ANT_SLAVE   FALSE

#define ANT_ACTIVITY_LOOP_CYCLES  (u32)4       /* Number of instruction cycles in an ANT activity loop */
#define ANT_MAX_ACTIVITY_TIME     (u32)500     /* Target time in microseconds for max time to allow the ANT task to do something */
#define MICRO_PER_SECOND          (u32)1000000 /* Microseconds per second */
//#define ANT_ACTIVITY_TIME_COUNT   (u32)(CCLK_VALUE * ANT_MAX_ACTIVITY_TIME / (MICRO_PER_SECOND * ANT_ACTIVITY_LOOP_CYCLES) )  /* Value used in a while loop that is waiting for an activity to be completed */
#define ANT_ACTIVITY_TIME_COUNT   (u32)1000000    /* Value used in a while loop that is waiting for an activity to be completed */
#define ANT_SRDY_DELAY            (u32)300

#define ANT_TX_TIMEOUT            (u32)100      /* Time in ms max to wait for Tx to ANT */

/* Symbols for bytes in messages to help readability */
#define CH                        (u8)0         /* Channel */
#define D0                        (u8)0         /* Data */
#define D1                        (u8)0         /* Data */
#define D2                        (u8)0         /* Data */
#define D3                        (u8)0         /* Data */
#define D4                        (u8)0         /* Data */
#define D5                        (u8)0         /* Data */
#define D6                        (u8)0         /* Data */
#define D7                        (u8)0         /* Data */
#define CS                        (u8)0         /* Checksum */


/*******************************************************************************
* Macros 
*******************************************************************************/
#define IS_SEN_ASSERTED()      (ANT_SSP_FLAGS & _SSP_CS_ASSERTED)          
#define ACK_SEN_ASSERTED()     (ANT_SSP_FLAGS &= ~_SSP_CS_ASSERTED)          
#define IS_SEN_DEASSERTED()    (ANT_SSP_FLAGS & _SSP_CS_DEASSERTED)          
#define ACK_SEN_DEASSERTED()   (ANT_SSP_FLAGS &= ~_SSP_CS_DEASSERTED)          

#define IS_MRDY_ASSERTED()     (ANT_MRDY_READ_REG == 0) 
#define SYNC_MRDY_ASSERT()     (ANT_MRDY_CLEAR_REG)     
#define SYNC_MRDY_DEASSERT()   (ANT_MRDY_SET_REG)

#define SYNC_SRDY_ASSERT()     (ANT_SRDY_CLEAR_REG)            
#define SYNC_SRDY_DEASSERT()   (ANT_SRDY_SET_REG)

#define ANT_RESET_ASSERT()     (ANT_RESET_CLEAR_REG)
#define ANT_RESET_DEASSERT()   (ANT_RESET_SET_REG)


/*******************************************************************************
* Application Values
*******************************************************************************/

/* G_u32AntFlags */
/* Error / event flags */
#define _ANT_FLAGS_LENGTH_MISMATCH    (u32)0x00000001        /* Set when an Acked data message gets acked */
#define _ANT_FLAGS_CMD_ERROR          (u32)0x00000002        /* A command received an error response  */
#define _ANT_FLAGS_UNEXPECTED_EVENT   (u32)0x00000004        /* The message parser handled an unexpected message */
#define _ANT_FLAGS_UNEXPECTED_MSG     (u32)0x00000008        /* The message parser handled an unexpected message */

#define ANT_ERROR_FLAGS_MASK          (u32)0x0000FFFF        /* Mask out all error flags */
#define ANT_ERROR_FLAGS_COUNT         (u8)4                  /* Current number of error flags */

/* Status flags */
#define _ANT_FLAGS_RESTART            (u32)0x00010000        /* An ANT restart message was received */
#define _ANT_FLAGS_CHANNEL_ASSIGNED   (u32)0x00020000        /* Set when the ANT channel is open */
#define _ANT_FLAGS_CHANNEL_CONFIGURED (u32)0x00040000        /* Set when the ANT channel is open */
#define _ANT_FLAGS_CHANNEL_OPEN       (u32)0x00080000        /* Set when the ANT channel is open */

/* Control flags */
#define _ANT_FLAGS_GOT_ACK            (u32)0x01000000        /* Set when an Acked data message gets acked */
#define _ANT_FLAGS_SEN                (u32)0x02000000        /* The SEN line has been asserted (set and cleared by ISR) */
#define _ANT_FLAGS_RX_IN_PROGRESS     (u32)0x04000000        /* Set when an ANT frame reception starts */
#define _ANT_FLAGS_TX_IN_PROGRESS     (u32)0x08000000        /* Set when an ANT frame transmission starts */
#define _ANT_FLAGS_TX_INTERRUPTED     (u32)0x10000000        /* An attempt to transmit was interrupted */
/* end G_u32AntFlags */

/* Default Channel ID parameters */
#define	ANT_SERIAL_LO_DEFAULT			        (u8)0xFF
#define ANT_SERIAL_HI_DEFAULT			        (u8)0xFF
#define	ANT_DEVICE_TYPE_DEFAULT					  (u8)0x01
#define	ANT_TRANSMISSION_TYPE_DEFAULT		  (u8)0x10

/* Default channel configuration parameters */
#define ANT_CHANNEL_DEFAULT               (u8)0
#define ANT_CHANNEL_TYPE_DEFAULT          CHANNEL_TYPE_MASTER
#define ANT_NETWORK_DEFAULT               (u8)0
#define ANT_CHANNEL_PERIOD_LO_DEFAULT		  (u8)0xcd
#define ANT_CHANNEL_PERIOD_HI_DEFAULT		  (u8)0x0c
#define ANT_FREQUENCY_DEFAULT						  (55)
#define ANT_TX_POWER_DEFAULT						  RADIO_TX_POWER_0DBM

/* Data buffer size: minimum size is 2 bytes */
//#define ANT_TX_BUFFER_SIZE                (u16)100
#define ANT_RX_BUFFER_SIZE                (u16)256

#define ANT_RESET_WAIT_MS                 (u32)100
#define ANT_RESTART_DELAY_MS              (u32)1000

#define SEN_TIMEOUT_MS                    (u32)100      
#define ANT_MSG_TIMEOUT_MS                (u32)5000

#define ANT_RESET_DDR_MASK                (u32)0xFF0FFFFF
#define ANT_RESET_INPUT_MASK              (u32)0x00400000
#define ANT_RESET_OUTPUT_MASK             (u32)0x00200000


/*******************************************************************************
* Function prototypes
*******************************************************************************/

/* ANT Private Serial-layer Functions */
static void AntSyncSerialInitialize(void);
static void AntRxMessage(void);
static void AntAbortMessage(void);
static void AdvanceAntRxBufferCurrentChar(void);
static void AdvanceAntRxBufferUnreadMsgPointer(void);
//static void AdvanceAntRxBufferNextCharPointer(void);


/* ANT Protected Interface-layer Functions */
void AntInitialize(void);
void AntRunActiveState(void);
bool AntTxMessage(u8 *pu8AntTxMessage_);
u8 AntExpectResponse(u8 u8ExpectedMessageID_, u32 u32TimeoutMS_);
void AntTxFlowControlCallback(void);
void AntRxFlowControlCallback(void);
u8 AntCalculateTxChecksum(u8* pu8Message_);
bool AntQueueOutgoingMessage(u8 *pu8Message_);
void AntDeQueueApplicationMessage(void);

/* ANT private Interface-layer Functions */
static u8 AntProcessMessage(void);
static void AntTick(u8 u8Code_);
static bool AntQueueApplicationMessage(AntApplicationMessageType eMessageType_, u8 *pu8DataSource_);
static void AntDeQueueOutgoingMessage(void);


/* ANT State Machine Definition */
void AntSM_Idle(void);
void AntSM_ReceiveMessage(void);
void AntSM_TransmitMessage(void);
void AntSM_NoResponse(void);

#endif /* __ANT_H */