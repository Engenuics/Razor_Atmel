/**********************************************************************************************************************
File: ant.h                                                                

Description:
ANT implementation for Cortex-M3 / AP2 SPI

Search "####" for ANT Channel ID defaults

**********************************************************************************************************************/

#ifndef __ANT_H
#define __ANT_H


/*******************************************************************************
* Constants
*******************************************************************************/
/* Configuration */
#define ANT_MASTER  TRUE
#define ANT_SLAVE   FALSE

#define ANT_ACTIVITY_LOOP_CYCLES  (u32)4        /* Number of instruction cycles in an ANT activity loop */
#define ANT_MAX_ACTIVITY_TIME     (u32)500      /* Target time in microseconds for max time to allow the ANT task to do something */
#define MICRO_PER_SECOND          (u32)1000000  /* Microseconds per second */
#define ANT_ACTIVITY_TIME_COUNT   (u32)10000    /* Value used in a while loop that is waiting for an activity to be completed */
#define ANT_SRDY_DELAY            (u32)200      /* A loop-kill delay to provide guaranteed minimum space for SRDY messages */
#define ANT_SRDY_PERIOD           (u32)20       /* A loop-kill delay to stretch the SRDY pulse out */

#define ANT_TX_TIMEOUT            (u32)100      /* Time in ms max to wait for Tx to ANT */

#define ANT_APPLICATION_MESSAGE_BYTES       (u8)8

/* Network number */
#define ANT_NETWORK_NUMBER_BYTES  (u8)8
#define ANT_DEFAULT_NETWORK_KEY   (u8)0
#define N_0                       (u8)0         
#define N_1                       (u8)0         
#define N_2                       (u8)0         
#define N_3                       (u8)0         
#define N_4                       (u8)0         
#define N_5                       (u8)0         
#define N_6                       (u8)0         
#define N_7                       (u8)0         

/* Symbols for bytes in messages to help readability */
#define CH                        (u8)0         /* Channel */
#define D_0                       (u8)0         /* Data */
#define D_1                       (u8)0         /* Data */
#define D_2                       (u8)0         /* Data */
#define D_3                       (u8)0         /* Data */
#define D_4                       (u8)0         /* Data */
#define D_5                       (u8)0         /* Data */
#define D_6                       (u8)0         /* Data */
#define D_7                       (u8)0         /* Data */
#define CS                        (u8)0         /* Checksum */


/*******************************************************************************
* Type definitions
*******************************************************************************/
typedef enum {ANT_UNCONFIGURED = 0, ANT_CONFIGURED = 1, ANT_OPENING = 2, 
              ANT_OPEN = 3, ANT_CLOSING = 4, ANT_CLOSED = 1} AntChannelStatusType;
typedef enum {ANT_EMPTY, ANT_DATA, ANT_TICK} AntApplicationMessageType;
typedef enum {ANT_GENERIC_MSG_READY, ANT_GENERIC_MSG_BUSY, ANT_GENERIC_MSG_OK, ANT_GENERIC_MSG_FAIL} AntApplicationGenericMsgStatus;
typedef enum {ANT_CHANNEL_0 = 0, ANT_CHANNEL_1, ANT_CHANNEL_2, ANT_CHANNEL_3,
              ANT_CHANNEL_4, ANT_CHANNEL_5, ANT_CHANNEL_6, ANT_CHANNEL_7,
              ANT_CHANNEL_SCANNING = 0} AntChannelNumberType;

/* Data struct for extended data information */
typedef struct
{
  u8 u8Channel;                            /* Channel number */
  u8 u8Flags;                              /* Extended data flags from the received message */
  u16 u16DeviceID;                         /* Device ID from the received message */
  u8 u8DeviceType;                         /* Device Type from the received message */
  u8 u8TransType;                          /* Transmission Type from the received message */
  s8 s8RSSI;                               /* RSSI from the received message */
  u8 u8Dummy;                              /* Pad for 4-byte alignment */
} AntExtendedDataType;

typedef struct
{
  u32 u32TimeStamp;                                  /* Current G_u32SystemTime1s */
  AntApplicationMessageType eMessageType;            /* Type of data */
  u8 u8Channel;                                      /* Channel to which the data applies */
  u8 au8MessageData[ANT_APPLICATION_MESSAGE_BYTES];  /* Array for message data */
  AntExtendedDataType sExtendedData;                 /* Struct of extended message data */
  void *psNextMessage;                               /* Pointer to AntDataMessageStructType */
} AntApplicationMsgListType;

#if 0
typedef struct 
{
  u8 AntChannel;                           /* The ANT channel number */
  u8 AntChannelType;                       /* ANT channel type from antdefines.h line 75 */
  u8 AntNetwork;                           /* Network number */
  u8 AntSerialLo;                          /* Device ID low byte */
  u8 AntSerialHi;                          /* Device ID high byte */
  u8 AntDeviceType;                        /* Device type byte */
  u8 AntTransmissionType;                  /* Transmission type byte */
  u8 AntChannelPeriodLo;                   /* Low byte of Channel Period */
  u8 AntChannelPeriodHi;                   /* High byte of Channel Period */
  u8 AntFrequency;                         /* RF frequency value */
  u8 AntTxPower;                           /* RF power level from antdefines.h line 40 */
} AntSetupDataType;
#endif

typedef struct 
{
  AntChannelNumberType AntChannel;         /* The ANT channel number */
  u8 AntChannelType;                       /* ANT channel type from antdefines.h line 75 */
  u8 AntNetwork;                           /* Network number */
  u8 AntNetworkKey[8];                     /* Network key assigned to AntNetwork number */
  u8 AntDeviceIdLo;                        /* Device ID low byte */
  u8 AntDeviceIdHi;                        /* Device ID high byte */
  u8 AntDeviceType;                        /* Device type byte */
  u8 AntTransmissionType;                  /* Transmission type byte */
  u8 AntChannelPeriodLo;                   /* Low byte of Channel Period */
  u8 AntChannelPeriodHi;                   /* High byte of Channel Period */
  u8 AntFrequency;                         /* RF frequency value */
  u8 AntTxPower;                           /* RF power level from antdefines.h line 40 */
  u8 AntFlags;                             /* Flag byte for tracking status */
} AntAssignChannelInfoType;

typedef struct
{
  u8 u8Channel;
  u8 u8MessageNumber;
  u8 u8ResponseCode;
} AntMessageResponseType;

/* Message struct for outgoing data messages */
typedef struct
{
  u32 u32TimeStamp;                        /* Current G_u32SystemTime1s */
  u8 au8MessageData[MESG_MAX_SIZE];        /* Array for message data */
  void *psNextMessage;                     /* Pointer to AntDataMessageStructType */
} AntOutgoingMessageListType;   


/*******************************************************************************
* Macros 
*******************************************************************************/
#define IS_SEN_ASSERTED()      (ANT_SSP_FLAGS & _SSP_CS_ASSERTED)          
#define ACK_SEN_ASSERTED()     (ANT_SSP_FLAGS &= ~_SSP_CS_ASSERTED)          

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
#define _ANT_FLAGS_LENGTH_MISMATCH        (u32)0x00000001        /* Set when an Acked data message gets acked */
#define _ANT_FLAGS_CMD_ERROR              (u32)0x00000002        /* A command received an error response  */
#define _ANT_FLAGS_UNEXPECTED_EVENT       (u32)0x00000004        /* The message parser handled an unexpected message */
#define _ANT_FLAGS_UNEXPECTED_MSG         (u32)0x00000008        /* The message parser handled an unexpected message */

#define ANT_ERROR_FLAGS_MASK              (u32)0x0000FFFF        /* Mask out all error flags */
#define ANT_ERROR_FLAGS_COUNT             (u8)4                  /* Current number of error flags */

/* Status flags */
#define _ANT_FLAGS_RESTART                (u32)0x00010000        /* An ANT restart message was received */

/* Control flags */
#define _ANT_FLAGS_FIRST_BYTE             (u32)0x02000000        /* The first byte in an ANT transmission is coming in */
#define _ANT_FLAGS_RX_IN_PROGRESS         (u32)0x04000000        /* Set when an ANT frame reception starts */
#define _ANT_FLAGS_TX_IN_PROGRESS         (u32)0x08000000        /* Set when an ANT frame transmission starts */
#define _ANT_FLAGS_TX_INTERRUPTED         (u32)0x10000000        /* An attempt to transmit was interrupted */
/* end G_u32AntFlags */


/* Channel flags used for AntFlags in AntAssignChannelInfoType */
#define _ANT_FLAGS_CHANNEL_CONFIGURED     (u8)0x01               /* Set when the ANT channel is configured and ready to be opened */
#define _ANT_FLAGS_CHANNEL_OPEN_PENDING   (u8)0x02               /* Set when the ANT channel open request has been made */
#define _ANT_FLAGS_CHANNEL_OPEN           (u8)0x04               /* Set when the ANT channel is open */
#define _ANT_FLAGS_CHANNEL_CLOSE_PENDING  (u8)0x08               /* Set when a request to close the ANT channel has been sent */
#define _ANT_FLAGS_GOT_ACK                (u8)0x10               /* Set when an Acked data message gets acked */



/* Default Channel ID parameters */
//#define	ANT_SERIAL_LO_DEFAULT			        (u8)0x00
//#define ANT_SERIAL_HI_DEFAULT			        (u8)0x20
#define	ANT_DEVICE_ID_LO_DEFAULT		      (u8)0x00
#define ANT_DEVICE_ID_HI_DEFAULT			    (u8)0x20
#define	ANT_DEVICE_TYPE_DEFAULT					  (u8)0x01
#define	ANT_TRANSMISSION_TYPE_DEFAULT		  (u8)0x01

/* #### Default channel configuration parameters #### */
#define ANT_CHANNEL_DEFAULT               (u8)0
#define ANT_CHANNEL_TYPE_DEFAULT          CHANNEL_TYPE_MASTER
#define ANT_NETWORK_DEFAULT               (u8)0

#define ANT_CHANNEL_PERIOD_DEC_DEFAULT    (u16)8192
#define ANT_CHANNEL_PERIOD_HEX_DEFAULT    (u16)0x2000
#define ANT_CHANNEL_PERIOD_HI_DEFAULT		  (u8)0x20
#define ANT_CHANNEL_PERIOD_LO_DEFAULT		  (u8)0x00

#define ANT_FREQUENCY_DEFAULT						  (u8)55
#define ANT_TX_POWER_DEFAULT						  RADIO_TX_POWER_4DBM
/* #### end of default channel configuration parameters ####*/

#define ANT_NUM_CHANNELS                  (u8)8
#define ANT_RX_BUFFER_SIZE                (u16)256

#define ANT_RESET_WAIT_MS                 (u32)100
#define ANT_RESTART_DELAY_MS              (u32)1000

#define SEN_TIMEOUT_MS                    (u32)100      
#define ANT_MSG_TIMEOUT_MS                (u32)1000

#define ANT_RESET_DDR_MASK                (u32)0xFF0FFFFF
#define ANT_RESET_INPUT_MASK              (u32)0x00400000
#define ANT_RESET_OUTPUT_MASK             (u32)0x00200000


/*******************************************************************************
* Function prototypes
*******************************************************************************/

/* ANT Private Serial-layer Functions */
static void AntSyncSerialInitialize(void);
static void AntSrdyPulse(void);
static void AntRxMessage(void);
static void AntAbortMessage(void);
static void AdvanceAntRxBufferCurrentChar(void);
static void AdvanceAntRxBufferUnreadMsgPointer(void);
static bool AntParseExtendedData(u8* pu8SourceMessage, AntExtendedDataType* psExtDataTarget_);


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

static void AntTickExtended(u8* pu8AntMessage_);
static bool AntQueueExtendedApplicationMessage(AntApplicationMessageType eMessageType_, u8* pu8DataSource_, AntExtendedDataType* psExtData_);
static void AntDeQueueOutgoingMessage(void);


/* ANT State Machine Definition */
void AntSM_Idle(void);
void AntSM_ReceiveMessage(void);
void AntSM_TransmitMessage(void);
void AntSM_NoResponse(void);

#endif /* __ANT_H */