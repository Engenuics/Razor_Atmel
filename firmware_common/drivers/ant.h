/*!********************************************************************************************************************
@file ant.h                                                                
@brief Header file for ANT implementation for Cortex-M3 / nRF51422 with AP2 network processor 
SPI EiE Razor development board
**********************************************************************************************************************/

#ifndef __ANT_H
#define __ANT_H

/**********************************************************************************************************************
Run time switches
**********************************************************************************************************************/
//#define ANT_VERBOSE                 /*!< @brief Define to enable Debug reporting of ANT Events */

/**********************************************************************************************************************
Type definitions
**********************************************************************************************************************/
/*! 
@enum AntChannelStatusType
@brief Enum to define the status of an ANT channel 
*/
typedef enum {ANT_UNCONFIGURED = 0, ANT_CONFIGURED = 1, ANT_OPENING = 2, 
              ANT_OPEN = 3, ANT_CLOSING = 4, ANT_CLOSED = 1} AntChannelStatusType;

/*! 
@enum AntApplicationMessageType
@brief Enum to define the type of message in the ANT API information buffer 
*/
typedef enum {ANT_EMPTY, ANT_DATA, ANT_TICK} AntApplicationMessageType;

/*! 
@enum AntChannelNumberType
@brief Enum to specify an ANT channel 
*/
typedef enum {ANT_CHANNEL_0 = 0, ANT_CHANNEL_1, ANT_CHANNEL_2, ANT_CHANNEL_3,
              ANT_CHANNEL_4, ANT_CHANNEL_5, ANT_CHANNEL_6, ANT_CHANNEL_7,
              ANT_CHANNEL_SCANNING = 0} AntChannelNumberType;

/*! 
@struct AntAssignChannelInfoType
@brief Data struct to fully configure an ANT channel */
typedef struct 
{
  AntChannelNumberType AntChannel;         /*!< @brief The ANT channel number */
  u8 AntChannelType;                       /*!< @brief ANT channel type from antdefines.h line 75 */
  u8 AntNetwork;                           /*!< @brief Network number */
  u8 AntNetworkKey[8];                     /*!< @brief Network key assigned to AntNetwork number */
  u8 AntDeviceIdLo;                        /*!< @brief Device ID low byte */
  u8 AntDeviceIdHi;                        /*!< @brief Device ID high byte */
  u8 AntDeviceType;                        /*!< @brief Device type byte */
  u8 AntTransmissionType;                  /*!< @brief Transmission type byte */
  u8 AntChannelPeriodLo;                   /*!< @brief Low byte of Channel Period */
  u8 AntChannelPeriodHi;                   /*!< @brief High byte of Channel Period */
  u8 AntFrequency;                         /*!< @brief RF frequency value */
  u8 AntTxPower;                           /*!< @brief RF power level from antdefines.h line 40 */
  u8 AntFlags;                             /*!< @brief Flag byte for tracking status */
} AntAssignChannelInfoType;

/* Channel flags used for AntFlags in AntAssignChannelInfoType */
#define _ANT_FLAGS_CHANNEL_CONFIGURED     (u8)0x01               /* Set when the ANT channel is configured and ready to be opened */
#define _ANT_FLAGS_CHANNEL_OPEN_PENDING   (u8)0x02               /* Set when the ANT channel open request has been made */
#define _ANT_FLAGS_CHANNEL_OPEN           (u8)0x04               /* Set when the ANT channel is open */
#define _ANT_FLAGS_CHANNEL_CLOSE_PENDING  (u8)0x08               /* Set when a request to close the ANT channel has been sent */
#define _ANT_FLAGS_GOT_ACK                (u8)0x10               /* Set when an Acked data message gets acked */


/*! 
@struct AntExtendedDataType
@brief Data struct for extended data information 
*/
typedef struct
{
  u8 u8Channel;                            /*!< @brief ANT channel number byte */
  u8 u8Flags;                              /*!< @brief Extended data flags from the received message */
  u16 u16DeviceID;                         /*!< @brief Device ID from the received message */
  u8 u8DeviceType;                         /*!< @brief Device Type from the received message */
  u8 u8TransType;                          /*!< @brief Transmission Type from the received message */
  s8 s8RSSI;                               /*!< @brief RSSI from the received message */
  u8 u8Dummy;                              /*!< @brief Pad for 4-byte alignment */
} AntExtendedDataType;


#define ANT_APPLICATION_MESSAGE_BYTES       (u8)8

/*! 
@struct AntApplicationMsgListType
@brief Data struct for the ANT application API message information 
*/
typedef struct
{
  u32 u32TimeStamp;                                  /*!< @brief Current G_u32SystemTime1s */
  AntApplicationMessageType eMessageType;            /*!< @brief Type of data */
  u8 u8Channel;                                      /*!< @brief Channel to which the data applies */
  u8 au8MessageData[ANT_APPLICATION_MESSAGE_BYTES];  /*!< @brief Array for message data */
  AntExtendedDataType sExtendedData;                 /*!< @brief Struct of extended message data */
  void *psNextMessage;                               /*!< @brief Pointer to next AntApplicationMsgListType */
} AntApplicationMsgListType;


/*! 
@struct AntOutgoingMessageListType
@brief Data struct for outgoing data messages */
typedef struct
{
  u32 u32TimeStamp;                        /*!< @brief Current G_u32SystemTime1s */
  u8 au8MessageData[MESG_MAX_SIZE];        /*!< @brief Array for message data */
  void *psNextMessage;                     /*!< @brief Pointer to next AntOutgoingMessageListType */
} AntOutgoingMessageListType;   


/*! 
@struct AntMessageResponseType
@brief Data struct for an ANT response message */
typedef struct
{
  u8 u8Channel;                            /*!< @brief The ANT channel number */
  u8 u8MessageNumber;                      /*!< @brief The message number to which the response refers */
  u8 u8ResponseCode;                       /*!< @brief The associated response code / event code */
} AntMessageResponseType;



/**********************************************************************************************************************
Macros 
**********************************************************************************************************************/
#define IS_SEN_ASSERTED()      (ANT_SSP_FLAGS & _SSP_CS_ASSERTED)   /*!< @brief Macro returns TRUE if SEN is asserted */
#define ACK_SEN_ASSERTED()     (ANT_SSP_FLAGS &= ~_SSP_CS_ASSERTED) /*!< @brief Macro to clear the _SSP_CS_ASSERTED flag */         

#define IS_MRDY_ASSERTED()     (ANT_MRDY_READ_REG == 0) /*!< @brief Macro returns TRUE if MRDY is asserted */
#define SYNC_MRDY_ASSERT()     (ANT_MRDY_CLEAR_REG)     /*!< @brief Macro to assert MRDY */   
#define SYNC_MRDY_DEASSERT()   (ANT_MRDY_SET_REG)       /*!< @brief Macro to deassert MRDY */

#define SYNC_SRDY_ASSERT()     (ANT_SRDY_CLEAR_REG)     /*!< @brief Macro to assert SRDY */       
#define SYNC_SRDY_DEASSERT()   (ANT_SRDY_SET_REG)       /*!< @brief Macro to deassert SRDY */

#define ANT_RESET_ASSERT()     (ANT_RESET_CLEAR_REG)    /*!< @brief Macro to assert the ANT RESET signal */
#define ANT_RESET_DEASSERT()   (ANT_RESET_SET_REG)      /*!< @brief Macro to deassert the ANT RESET signal */

#define ANT_BOOT_DISABLE()     !(ANT_DISABLE_BUTTON)    /*!< @brief Macro to check if BUTTON3 is pressed which will disable ANT boot */


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
#define ANT_NUM_CHANNELS                  (u8)8                  /*!< @brief Maximum number of ANT channels in the system */
#define ANT_RX_BUFFER_SIZE                (u16)256               /*!< @brief ANT incoming data buffer size */

#define ANT_CONFIGURE_TIMEOUT_MS          (u32)2000              /*!< @brief Maximum time to send all channel configuration messages */
#define ANT_INFINITE_SEARCH_TIMEOUT       (u8)0xFF               /*!< @brief Value for Set Search Timeout for infinite timeout */

/*!@cond DOXYGEN_EXCLUDE */
#define ANT_RESET_WAIT_MS                 (u32)100
#define ANT_RESTART_DELAY_MS              (u32)1000
#define ANT_MSG_TIMEOUT_MS                (u32)1000

/* G_u32AntFlags */
/* Error / event flags */
#define _ANT_FLAGS_LENGTH_MISMATCH        (u32)0x00000001        /* Set when if counted Rx bytes != Length byte */
#define _ANT_FLAGS_CMD_ERROR              (u32)0x00000002        /* A command received an error response  */
#define _ANT_FLAGS_UNEXPECTED_EVENT       (u32)0x00000004        /* The message parser handled an unexpected event */
#define _ANT_FLAGS_UNEXPECTED_MSG         (u32)0x00000008        /* The message parser handled an unexpected message */

#define ANT_ERROR_FLAGS_MASK              (u32)0x0000FFFF        /* Mask out all error flags */
#define ANT_ERROR_FLAGS_COUNT             (u8)4                  /* Current number of error flags */

/* Status flags */
#define _ANT_FLAGS_RESTART                (u32)0x00010000        /* An ANT restart message was received */

/* Control flags */
#define _ANT_FLAGS_RX_IN_PROGRESS         (u32)0x01000000        /* Set when an ANT frame reception starts */
#define _ANT_FLAGS_TX_IN_PROGRESS         (u32)0x02000000        /* Set when an ANT frame transmission starts */
#define _ANT_FLAGS_TX_INTERRUPTED         (u32)0x04000000        /* An attempt to transmit was interrupted */
/* end G_u32AntFlags */


/* #### Default channel configuration parameters #### */
#define	ANT_DEVICE_ID_LO_DEFAULT		      (u8)0x00
#define ANT_DEVICE_ID_HI_DEFAULT			    (u8)0x20
#define	ANT_DEVICE_TYPE_DEFAULT					  (u8)0x01
#define	ANT_TRANSMISSION_TYPE_DEFAULT		  (u8)0x01

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

#define ANT_ACTIVITY_TIME_COUNT   (u32)10000    /* Value used in a while loop that is waiting for an activity to be completed */
#define ANT_SRDY_DELAY            (u32)200      /* A loop-kill delay to provide guaranteed minimum space for SRDY messages */
#define ANT_SRDY_PERIOD           (u32)20       /* A loop-kill delay to stretch the SRDY pulse out */


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


/*!@endcond */


/**********************************************************************************************************************
Function prototypes
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
/* ANT Public Interface-layer Functions */
u8 AntCalculateTxChecksum(u8* pu8Message_);
bool AntQueueOutgoingMessage(u8 *pu8Message_);
void AntDeQueueApplicationMessage(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
/* ANT Protected Interface-layer Functions */
void AntInitialize(void);
void AntRunActiveState(void);

void AntTxFlowControlCallback(void);
void AntRxFlowControlCallback(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
/* ANT Private Interface-layer Functions */
static bool AntTxMessage(u8 *pu8AntTxMessage_);
static void AntRxMessage(void);
static void AntAbortMessage(void);
static void AdvanceAntRxBufferCurrentChar(void);
static void AdvanceAntRxBufferUnreadMsgPointer(void);

static u8 AntExpectResponse(u8 u8ExpectedMessageID_, u32 u32TimeoutMS_);
static u8 AntProcessMessage(void);
static bool AntParseExtendedData(u8* pu8SourceMessage_, AntExtendedDataType* psExtDataTarget_);
static bool AntQueueExtendedApplicationMessage(AntApplicationMessageType eMessageType_, u8* pu8DataSource_, AntExtendedDataType* psExtData_);
static void AntTickExtended(u8* pu8AntMessage_);
static void AntDeQueueOutgoingMessage(void);

/* ANT Private Serial-layer Functions */
static void AntSyncSerialInitialize(void);
static void AntSrdyPulse(void);

/* ANT State Machine Definition */
static void AntSM_Idle(void);
static void AntSM_ReceiveMessage(void);
static void AntSM_TransmitMessage(void);
static void AntSM_NoResponse(void);

#endif /* __ANT_H */