/***********************************************************************************************************************
File: dataflash.h
***********************************************************************************************************************/

#ifndef __DATAFLASH_H
#define __DATAFLASH_H

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef enum {
  DATAFLASH_COMMAND_IDLE,
  DATAFLASH_COMMAND_READ_BYTES,
  DATAFLASH_COMMAND_READ_PAGE,
  DATAFLASH_COMMAND_WRITE_PAGE,
  DATAFLASH_COMMAND_ERASE_PAGE
} DataFlashCommandType;


typedef enum {
  DATAFLASH_STATUS_READY,
  DATAFLASH_STATUS_BUSY,
  DATAFLASH_STATUS_DATA_READY,
  DATAFLASH_STATUS_WRITE_COMPLETE,
  DATAFLASH_STATUS_ERASE_COMPLETE,
  DATAFLASH_STATUS_INVALID_TOKEN,
  DATAFLASH_STATUS_ERROR
} DataflashCheckStatusType;

/***********************************************************************************************************************
* Constants / Definitions
***********************************************************************************************************************/
#define DATAFLASH_RX_BUFFER_SIZE     (u32)264             /* Size of buffer for incoming messages */
#define DATAFLASH_TX_BUFFER_SIZE     (u32)264             /* Size of buffer for outgoing messages */
#define DATAFLASH_MAX_BYTES          (u32)264             /* Maximum # of bytes a client may request to write */
#define DATAFLASH_DATA_PAGE_BYTES    (u16)256             /* Data bytes per data page */

/* The following definitions come from Dataflash Allocation Map.xlsx */
#define DATAFLASH_FIRMWARE_START_PAGE 512

/* End of Dataflash Allocation Map.xlsx definitions */

/* !!!!! NEEDS TO BE REPLACED BY DEFINITIONS IN Dataflash Allocation.xlsx */
/*  define the msg info page ,from page 0 to 255   */
//#define SECTOR_N_PAGE_BASE(n)    (u16)(n*256) 			  /*  */

#define CONFIG_PAGE_BASE             ( 16)      
#define MSG_INDEX_TM_PAGE            CONFIG_PAGE_BASE        
#define MSG_TYPE_BASE_PAGE           (CONFIG_PAGE_BASE + 1)  
#define CONFIG_PAGE			             (CONFIG_PAGE_BASE + 2)   /* !!!! These should not be hard coded as "GPRS" */
#define GPRSPARACONFIG_PAGE		       (CONFIG_PAGE_BASE + 3) 
#define SIGNID_PAGE				           (CONFIG_PAGE_BASE + 4) 
#define SCREENCONFIG_PAGE	           (CONFIG_PAGE_BASE + 5) 
#define WIFIPARACONFIG_PAGE		       (CONFIG_PAGE_BASE + 6) 
#define MESSAGE_TYPE_BASE_PAGE       (CONFIG_PAGE_BASE + 32)
#define MESSAGE_BASE_PAGE            (CONFIG_PAGE_BASE + 48)

/* G_u32DataflashFlags */
#define _DATAFLASH_ERROR_ID          (u32)0x02000000      /* Set if vendor ID request does not return expected value */

#define DATAFLASH_SSP_WAIT_TIME_MS   (u32)100             /* Time in ms to wait after being denied SSP access */
#define DATAFLASH_SSP_OPERATION_TIME_MS (u32)200          /* Time in ms to wait for an SSP operation to complete */
#define DATAFLASH_STATUS_VALID_TIME  (u32)500             /* Time in ms for the status of a completed message to remain valid */
#define	WRITE_CYCLE_DELAY   	       (u32)10              /* Time in ms expected for a write cycle to complete */
#define	PAGE_ERASE_CYCLE_DELAY   	   (u32)50              /* Time in ms expected for a write cycle to complete */

#define MAX_COMMAND_ARRAY_SIZE       (u8)8                /* Maximum size needed for a command array */
#define INDEX_COMMAND                (u8)0                /* Index in a command array of the command byte */
#define INDEX_ADDRESS_HI             (u8)1                /* Index in a command array of the high address byte */
#define INDEX_ADDRESS_MD             (u8)2                /* Index in a command array of the mid address byte */
#define INDEX_ADDRESS_LO             (u8)3                /* Index in a command array of the low address byte */
#define INDEX_READ_DUMMY0            (u8)4                /* Index of first dummy byte in a read array */

/* Dataflash_u8ErrorCode Error Codes */
#define DATAFLASH_ERROR_NONE         (u8)0x00             /* No error */
#define DATAFLASH_ERROR_ID           (u8)0x01             /* Reported ID is not what was expected */
#define DATAFLASH_ERROR_TIMEOUT      (u8)0x02             /* SSP application did not deliver expected response on time */



/**********************************************************************************************************************
Macros
**********************************************************************************************************************/
#define GET_MSG_TYPE_PAGE(no)        (MESSAGE_TYPE_BASE_PAGE + no)     //48 - 58
#define GET_MESSAGE_PAGE(MessageNo,CurrentLanguage,CurrentPage)        (MESSAGE_BASE_PAGE + (MessageNo * 16) + (CurrentLanguage * 4) + CurrentPage)  //64 - 240


/***********************************************************************************************************************
* Command-Specific Definitions
***********************************************************************************************************************/


/***********************************************************************************************************************
* Function Declarations
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/
DataflashCheckStatusType DataflashCheckStatus(u32 u32Query_);
u32 DataflashReadBytes(u16 u16PageAddress_, u16 u16StartByte_, u16 u16Bytes_);
void DataflashGetReadBytes(u16 u16Bytes_, u8 *pu8Destination_);
u32 DataflashWritePage(u16 u16PageAddress_, u8 *pu8Data_);
u32 DataflashErasePage(u16 u16PageAddress_);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void DataflashInitialize(void);
bool DataflashReadSignConfig(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/
static void ClearReceiveBuffer(void);
static void DataflashOperationComplete(void);
static void AdvanceRxBufferUnreadBytePointer(u16 u16BytesToMove_);
static void DataflashCheckTimeout(u32 u32Time_);


/***********************************************************************************************************************
* State Machine Declarations
***********************************************************************************************************************/
static void DataflashSM_Idle(void);
static void DataflashSM_ReadData(void);

static void DataflashSM_WriteData(void);
static void DataflashSM_WaitWriteCommand(void);
static void DataflashSM_WaitWriteCycle(void);

static void DataflashSM_ErasePage(void);
static void DataflashSM_WaitEraseCycle(void);

static void DataflashSM_WaitCommand(void);
static void DataflashSM_WaitSSP(void);
static void DataflashSM_Error(void);



/***********************************************************************************************************************
* Data flash memory organization
************************************************************************************************************************
This file defines the space in a 512kB (4MBit) Atmel Flash AT45DB041D
Page size is 264 bytes.

The dataflash is used for configuration, message storage, one firmware upgrade, and one base firmware.
The Flash is organized per the following page table.  The maximum page address is 0x7ff (11 bits for 2048 pages) to cover the entire
4Mbit (512kByte) address space

0x000 - 0x0FF (Pages 0 - 255): Message storage
0x100 - 0x103 (Pages 256 - 259): Configuration information
0x104 - 0x1FF (Pages 260 - 511): Error log data (first page is bookmarks: read until 0xFF, then back one)
0x200 - 0x4FF (Pages 512 - 1279): Firmware 1 space (OTA destination)
0x500 - 0x7FF (Pages 1280 - 2047): Firmware 2 space (backup factory load)
*/




#endif /* __DATAFLASH_H */
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
