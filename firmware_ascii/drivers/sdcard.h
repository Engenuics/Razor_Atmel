/**********************************************************************************************************************
File: sdcard.h                                                                

Description:
Header file for sdcard.c
**********************************************************************************************************************/

#ifndef __SDCARD_H
#define __SDCARD_H

//#define ENABLE_SD

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef enum {SD_NO_CARD, SD_CARD_ERROR, SD_IDLE, SD_READING, SD_DATA_READY, SD_WRITING} SdCardStateType;


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* SD_u32Flags */
#define _SD_CARD_INSERTED         (u32)0x00000001      /* Set when card detect switch shows card inserted */
#define _SD_CARD_GOOD             (u32)0x00000002      /* Set when communication to the card is successful */
#define _SD_CARD_HC               (u32)0x00000004      /* Set if card is high capacity type (block access) */
#define _SD_TYPE_SD1		          (u32)0x00000008   	 /* SD ver 1 */
#define _SD_TYPE_SD2		          (u32)0x00000010		   /* SD ver 2 */
#define _SD_TYPE_MMC		          (u32)0x00000020	     /* SD ver 3 */
#define _SD_TYPE_BLOCK		        (u32)0x00000040		   /* Block addressing */
//#define _SD_TYPE_SDSC             (u32)0x00000000      /* Standard Capacity SD Memory Card (SDSC): Up to and including 2 GB */
//#define _SD_TYPE_SDHC             (u32)0x00000000      /* High Capacity SD Memory Card (SDHC): More than 2GB and up to and including 32GB */
//#define _SD_TYPE_SDXC             (u32)0x00000000      /* Extended Capacity SD Memory Card (SDXC): More than 32GB and up to and including 2TB */
/* end of SD_u32Flags */

#define SD_CLEAR_CARD_TYPE_BITS  ~(_SD_CARD_HC | _SD_TYPE_MMC | _SD_TYPE_SD1 | _SD_TYPE_SD2 |_SD_TYPE_BLOCK)
#define _SD_TYPE_SDC		          (_SD_TYPE_SD1 | _SD_TYPE_SD2)	

#define SDCARD_RX_BUFFER_SIZE     (u32)548             /* Size of buffer for incoming SD data */

#define SD_RESPONSE_TIMEOUT       (u32)100             /* Time in ms for the SD card to respond to a command */
#define SD_WAIT_TIME              (u32)1000            /* Time in ms for waiting for SD stuff to occur */
#define SD_WAKEUP_BYTES           (u32)20              /* Number of dummy bytes sent to wake up new SD card */
#define SD_CMD_RETRIES            (u8)20               /* Number of polls to retry a command response */

#define SD_CMD_SIZE               (u8)6                /* Size of an SD card command */

#define SD_SPI_WAIT_TIME_MS	      (u32)(500)           /* Time to wait for the SPI resource to become available */
#define SD_READ_TOKEN_MS		      (u32)(200)
#define SD_INIT_TIMEOUT_MS		    (u32)(1000)
#define SD_SECTOR_READ_TIMEOUT_MS	(u32)(1000)
#define SD_ERASE_TIMEOUT_MS	      (u32)(30000)


/* SD Commands support in SPI mode */
#define SD_CMD0		                (u8)(0)			  /* GO_IDLE_STATE */
#define SD_CMD1		                (u8)(1)			  /* SEND_OP_COND (SD) */
#define SD_CMD8		                (u8)(8)			  /* SEND_IF_COND */
#define SD_CMD8_INDEX_VHS         (u8)(2)       /* Index of VHS information in response to CMD 8 */
#define SD_CMD8_INDEX_CHECK       (u8)(3)       /* Index of check pattern information in response to CMD 8 */
#define SD_CMD9		                (u8)(9)			  /* SEND_CSD */
#define SD_CMD10		              (u8)(10)			/* SEND_CID */
#define SD_CMD12		              (u8)(12)			/* STOP_TRANSMISSION */
#define SD_ACMD13	                (u8)(13)	    /* SD_STATUS (SDC) */
#define SD_CMD16		              (u8)(16)			/* SET_BLOCKLEN */
#define SD_CMD17		              (u8)(17)			/* READ_SINGLE_BLOCK */
#define SD_CMD18		              (u8)(18)			/* READ_MULTIPLE_BLOCK */
#define SD_CMD23		              (u8)(23)			/* SET_BLOCK_COUNT (SD) */
#define SD_ACMD23	                (u8)(23)   		/* SET_WR_BLK_ERASE_COUNT (SDC) */
#define SD_CMD24		              (u8)(24)			/* WRITE_BLOCK */
#define SD_CMD25		              (u8)(25)			/* WRITE_MULTIPLE_BLOCK */
#define SD_CMD32		              (u8)(32)			/* ERASE_ER_BLK_START */
#define SD_CMD33		              (u8)(33)			/* ERASE_ER_BLK_END */
#define SD_CMD38		              (u8)(38)			/* ERASE */
#define SD_ACMD41	                (u8)(41)	    /* SEND_OP_COND (SDC) */
#define SD_CMD55		              (u8)(55)			/* APP_CMD */
#define SD_CMD58		              (u8)(58)			/* READ_OCR */
#define SD_CMD59                  (u8)(59)      /* Toggle CRC mode */

#define SD_HOST_CMD               (u8)0x40      /* Set bit 6 to indicate command is from host */
#define SD_NO_CRC                 (u8)0x01      /* Placeholder value when a CRC byte is not required */
#define SD_CMD0_CRC               (u8)0x95      /* CRC always for CMD0 */
#define SD_CMD8_CRC               (u8)0x87      /* CRC always for CMD8 */
#define SD_VHS_VALUE              (u8)0x01      /* Value in CMD8 argument for 2.7-3.6V supply */
#define SD_CHECK_PATTERN          (u8)0xAA      /* Recommended bit pattern for check in CMD8 */

#define _SD_OCR_CCS_BIT           (u8)0x40      /* Bit in OCR register that indicates card capacity (high == high capacity) */

#define SD_R1_LEN	                (u8)(1)
#define SD_R2_LEN	                (u8)(2)
#define SD_R3_LEN	                (u8)(SD_R1_LEN + 4)	// 4-byte OCR
#define SD_R7_LEN	                (u8)(SD_R1_LEN + 4)

/* Card type flags (CardType) */
#define SD_CT_MMC		              (u8)0x01  	  /* SD ver 3 */
#define SD_CT_SD1		              (u8)0x02		  /* SD ver 1 */
#define SD_CT_SD2		              (u8)0x04		  /* SD ver 2 */
#define SD_CT_SDC		              (u8)(SD_CT_SD1 | SD_CT_SD2)	/* SD */
#define SD_CT_BLOCK		            (u8)0x08		  /* Block addressing */

/* Card status bits (in Response R1) */
#define SD_STATUS_READY           (u8)0x00      /* Response R1 when card is READY */
#define SD_STATUS_IDLE            (u8)0x01      /* Response R1 when card is IDLE */
#define SD_STATUS_ERASE_RESET     (u8)0x02      /* Response bit in R1 */
#define SD_STATUS_ILLEGAL_CMD     (u8)0x04      /* Response bit in R1 */
#define SD_STATUS_COM_CRC_ERR     (u8)0x08      /* Response bit in R1 */
#define SD_STATUS_ERASE_SEQ_ERR   (u8)0x10      /* Response bit in R1 */
#define SD_STATUS_ADDRESS_ERR     (u8)0x20      /* Response bit in R1 */
#define SD_STATUS_PARAMETER_ERR   (u8)0x40      /* Response bit in R1 */

/* Data tokens */
#define TOKEN_START_BLOCK         (u8)0xFE      /* First byte of a single block read or write, or multiple block read */
#define TOKEN_START_BLOCK_MULT    (u8)0xFC      /* First byte of each block in multiple block write */
#define TOKEN_STOP_BLOCK_MULT     (u8)0xFD      /* Stop transmission request token for multi-block write */

/* SD Error Codes */
#define SD_ERROR_NONE             (u8)0x00      /* No error */
#define SD_ERROR_TIMEOUT          (u8)0x01      /* SSP application did not deliver expected response */
#define SD_ERROR_CARD_VOLTAGE     (u8)0x02      /* Card voltage not supported */
#define SD_ERROR_BAD_RESPONSE     (u8)0x03      /* Unexpected or no response to a command */
#define SD_ERROR_NO_TOKEN         (u8)0x04      /* Got '0' for a message token => message task is broken */
#define SD_ERROR_NO_SD_TOKEN      (u8)0x05      /* Expected a token from the SD card but didn't get it */


/**********************************************************************************************************************
* Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Macros */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions */
/*--------------------------------------------------------------------------------------------------------------------*/
SdCardStateType SdGetStatus(void);
bool SdReadBlock(u32 u32BlockAddress_);
bool SdWriteBlock(u32 u32BlockAddress_);             
bool SdGetReadData(u8* pu8Destination_);
void CheckTimeout(u32 u32Time_);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void SdCardInitialize(void);
void SdCardRunActiveState(void);

bool SdIsCardInserted(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/
static void SdCommand(u8* pau8Command_);
//static void AdvanceSD_pu8RxBufferParser(u32 u32NumBytes_);
//static void FlushSdRxBuffer(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
#ifndef ENABLE_SD 
static void SdCardSM_Disabled(void);
#endif /* ENABLE_SD */

static void SdCardSM_IdleNoCard(void);     
static void SdCardSM_Dummies(void);
static void SdCardSM_ResponseCMD0(void);
static void SdCardSM_ResponseCMD8(void);
static void SdCardSM_ReadCMD8(void);
static void SdCardSM_ResponseCMD55(void);
static void SdCardSM_ResponseACMD41(void);
static void SdCardSM_ResponseCMD58(void);
static void SdCardSM_ResponseCMD16(void);
static void SdCardSM_ReadCMD58(void);

static void SdCardSM_ReadyIdle(void);          
static void SdCardSM_ResponseCMD17(void);
static void SdCardSM_WaitStartToken(void);          
static void SdCardSM_DataTransfer(void);
static void SdCardSM_FailedDataTransfer(void);

//static void SdCardSM_WaitReady(void);
static void SdCardSM_WaitCommand(void);
static void SdCardSM_WaitResponse(void);
static void SdCardSM_WaitSSP(void);

static void SdCardSM_Error(void);         


#endif /* __SDCARD_H */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
