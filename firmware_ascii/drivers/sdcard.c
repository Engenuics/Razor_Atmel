/**********************************************************************************************************************
File: sdcard.c                                                                

Description:
SD card interface application.  This task looks for an SD card and initializes it so it is ready
to receive data read / write commands.  

API
Client applications may use the following functions to access this driver:

SdGetStatus() - returns a variable of type SdCardStateType which may have the following value:
  SD_NO_CARD: no card is inserted.
  SD_CARD_ERROR: an inserted card has an error.
  SD_IDLE: card is ready and waiting for a read or a write to be requested.
  SD_READING: the card is being read and is not available for anything else 
  SD_DATA_READY: a sector of data has been requested from the SD card is ready for the client.
  SD_WRITING: the card is being written and is not available for anything else

bool SdReadBlock(u32 u32SectorAddress_) - initiates read of one 512 byte block of memory from the SD card.
Returns TRUE if the card is available and can start reading. 
User must use SdGetStatus() and wait until the card status is SD_DATA_READY which means the read is done.

bool SdWriteBlock(u32 u32BlockAddress_) - not yet implemented

bool SdGetReadData(u8* pu8Destination_) - transfers the read data to the client.  The card state will return to SD_IDLE.


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32SdCardFlags;                         /* Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "SD_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type SD_pfStateMachine;              /* The Debug state machine function pointer */

static u32 SD_u32Flags;                            /* Application flags for SD card */
static SdCardStateType SD_CardState;               /* Card state variable */

static u8  SD_u8ErrorCode;                         /* Error code */
static fnCode_type SD_pfWaitReturnState;           /* The saved state to return after a wait period */
//static u8* SD_NextCommand;                         /* Saved command to be executed next */

static SspConfigurationType SD_sSspConfig;         /* Configuration information for SSP peripheral */
static SspPeripheralType* SD_Ssp;                  /* Pointer to SSP peripheral object */

static u8 SD_au8RxBuffer[SDCARD_RX_BUFFER_SIZE];   /* Space for incoming bytes from the SD card */
static u8 *SD_pu8RxBufferNextByte;                 /* Pointer to next spot in RxBuffer to write a byte */
//static u8 *SD_pu8RxBufferParser;                   /* Pointer to loop through the Rx buffer to read bytes */

static u32 SD_u32Timeout;                          /* Timeout counter used across states */
static u32 SD_u32CurrentMsgToken;                  /* Token of message currently being sent */
static u32 SD_u32Address;                          /* Current read/write sector address */

static u8 SD_au8CardInMessage[]    = "SD card inserted\n\r";
static u8 SD_au8SspRequestFailed[] = "SdCard denied SSP\n\r";
static u8 SD_au8CardReady[]        = "SD ready\n\r";
static u8 SD_au8CardError[]        = "SD error: ";
static u8 SD_au8CardError0[]       = "UNKNOWN\n\r";
static u8 SD_au8CardError1[]       = "TIMEOUT\n\r";
static u8 SD_au8CardError2[]       = "CARD_VOLTAGE\n\r ";
static u8 SD_au8CardError3[]       = "BAD_RESPONSE\n\r ";
static u8 SD_au8CardError4[]       = "NO_TOKEN\n\r";
static u8 SD_au8CardError5[]       = "NO_SD_TOKEN\n\r";


static u8 SD_au8CMD0[]   = {SD_HOST_CMD | SD_CMD0,  0, 0, 0, 0, SD_CMD0_CRC};
static u8 SD_au8CMD8[]   = {SD_HOST_CMD | SD_CMD8,  0, 0, SD_VHS_VALUE, SD_CHECK_PATTERN, SD_CMD8_CRC};
static u8 SD_au8CMD16[]  = {SD_HOST_CMD | SD_CMD16, 0, 0, 0x02, 0x00, SD_NO_CRC};
static u8 SD_au8CMD17[]  = {SD_HOST_CMD | SD_CMD17, 0, 0, 0, 0, SD_NO_CRC};
static u8 SD_au8CMD55[]  = {SD_HOST_CMD | SD_CMD55, 0, 0, 0 ,0, SD_NO_CRC};
static u8 SD_au8CMD58[]  = {SD_HOST_CMD | SD_CMD58, 0, 0, 0 ,0, SD_NO_CRC};

static u8 SD_au8ACMD41[] = {SD_HOST_CMD | SD_ACMD41,0, 0, 0, 0, SD_NO_CRC};


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: SdIsCardInserted

Description:
Updates and indicates whether or not an SD card is currently inserted.

Requires:
  - 

Promises:
  - Returns TRUE if card is inserted; otherwise returns FALSE 
  - _SD_CARD_INSERTED is updated (SET if card is in)
*/
bool SdIsCardInserted(void)
{
  if( SD_CARD_INSERTED() )
  {
    SD_u32Flags |= _SD_CARD_INSERTED;
    return TRUE;
  }
  else
  {
    SD_u32Flags &= ~_SD_CARD_INSERTED;
    SD_CardState = SD_NO_CARD;
    return FALSE;
  }
  
} /* end SdIsCardInserted() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SdGetStatus

Description:
Reports the current status of the SD card.

SD_NO_CARD: no card is inserted.
SD_CARD_ERROR: an inserted card has an error.
SD_IDLE: card is ready and waiting for a read or a write to be requested.
SD_READING: the card is being read and is not available for anything else 
SD_DATA_READY: a sector of data has been requested from the SD card is ready for the client.
SD_WRITING: the card is being written and is not available for anything else

Requires:
  - SD_CardState up to date.

Promises:
  - Returns SD_CardState
*/
SdCardStateType SdGetStatus(void)
{
  return SD_CardState;
  
} /* end SdGetStatus() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SdReadBlock

Description:
Reads a block at the sector address provided.
Byte-addressable cards are automatically converted appropriately so user does not have to distinguish
and can always read by 512 byte block.

Requires:
  - _SD_TYPE_SD1, _SD_TYPE_SD2, _SD_CARD_HC are correctly set/clear to indicate card type.
  - u32SectorAddress_ is a valid SD card address

Promises:
  - If the card is currently SD_IDLE, initiates the read, changes card state to "SD_READING" and returns TRUE.
*/
bool SdReadBlock(u32 u32SectorAddress_)
{
  if(SD_CardState == SD_IDLE)
  {
    /* Capture the card address of interest with adjustment for byte-accessed cards as required */
    SD_u32Address = u32SectorAddress_;
    if( !(SD_u32Flags & _SD_CARD_HC) )
    {
      SD_u32Address *= 512;
    }
    
    /* Update the card state which will trigger the start of the read sequence */
    SD_CardState = SD_READING;
    return TRUE;
  }
  
  return FALSE;
  
} /* end SdReadBlock() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SdWriteBlock

Description:
Writes a block at the address provided.

Requires:
  - 

Promises:
  - 
*/
bool SdWriteBlock(u32 u32BlockAddress_)
{
  return FALSE;
    
} /* end SdWriteBlock() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SdGetReadData

Description:
Transfers the data that was just read from the card.

Requires:
  - pu8Destination points to the start of a 512 byte buffer where the data will be read.
  - The 512 bytes of data that was just read is at SD_au8RxBuffer[0] thru SD_au8RxBuffer[511] -
    this will set SD_CardState to SD_DATA_READY.

Promises:
  - if SD_CardState = SD_DATA_READY, loads 512 bytes to pu8Destination_ and returns TRUE
  - else returns FALSE
*/
bool SdGetReadData(u8* pu8Destination_)
{
  /* To ensure data integrity, card state must be SD_DATA_READY */
  if(SD_CardState == SD_DATA_READY)
  {
    SD_CardState = SD_IDLE;

    for(u16 i = 0; i < 512; i++)
    {
      *pu8Destination_ = SD_au8RxBuffer[i];
      pu8Destination_++;
    }
    
    return TRUE;
  }
  /* Otherwise return FALSE */
  else
  {
    return FALSE;
  }
    
} /* end SdGetReadData() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: SdCardInitialize

Description:
Initializes the State Machine and its variables.

Requires:
  -

Promises:
  - 
*/
void SdCardInitialize(void)
{
#ifdef ENABLE_SD
  u8 au8SdCardStartedMsg[] = "SdCard task ready\n\r";

  /* Reset the receive buffer to dummies for a known starting state */
  memset(SD_au8RxBuffer, SSP_DUMMY_BYTE, SDCARD_RX_BUFFER_SIZE);

  /* Initailze startup values and the command array */
  SD_pu8RxBufferNextByte = &SD_au8RxBuffer[0];

  /* Configure the SSP resource to be used for the SD Card application */
  SD_sSspConfig.SspPeripheral      = SD_SSP;
  SD_sSspConfig.pCsGpioAddress     = SD_BASE_PORT;
  SD_sSspConfig.u32CsPin           = SD_CS_PIN;
  SD_sSspConfig.pu8RxBufferAddress = SD_au8RxBuffer;
  SD_sSspConfig.ppu8RxNextByte     = &SD_pu8RxBufferNextByte;
  SD_sSspConfig.u16RxBufferSize    = SDCARD_RX_BUFFER_SIZE;
  SD_sSspConfig.eBitOrder          = MSB_FIRST;
  SD_sSspConfig.eSspMode           = SPI_MASTER_MANUAL_CS;
  
  /* Always start in SdCardSM_IdleNoCard but display different message if card is already in */
  SD_pfStateMachine = SdCardSM_IdleNoCard;
  DebugPrintf(au8SdCardStartedMsg);

  if( SdIsCardInserted() )
  {
    DebugPrintf(SD_au8CardInMessage);
  }

  G_u32ApplicationFlags |= _APPLICATION_FLAGS_SDCARD;
#else /* ENABLE_SD */
  u8 au8SdCardStartedMsg[] = "SdCard task disabled\n\r";

  G_u32ApplicationFlags |= _APPLICATION_FLAGS_SDCARD;
  SD_pfStateMachine = SdCardSM_Disabled;
  DebugPrintf(au8SdCardStartedMsg);
#endif /* ENABLE_SD */
} /* end SdCardInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function SdCardRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void SdCardRunActiveState(void)
{
  SD_pfStateMachine();

} /* end SdCardRunActiveState */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------
Function: SdCommand

Description:
Queues a command and sets up the application to read the response when it arrives.

Requires:
  - No other commands should be queued for the SSP peripheral being used.
  - All commands have the same size, SD_CMD_SIZE which include an extra byte which is the first
    read back for the response.
  - pau8Command_ is a pointer to the first byte of the command byte array

Promises:
  - Requested command is queued to the SSP peripheral
  - SD_u32CurrentMsgToken updated with the corresponding message token
  - SD_u32Timeout loaded to start counting the timeout period for the command
  - State machine set to wait command
*/
void SdCommand(u8* pau8Command_)
{
  /* Queue the transmit message with this command */
  SD_u32CurrentMsgToken = SspWriteData(SD_Ssp, SD_CMD_SIZE, pau8Command_);
  if(SD_u32CurrentMsgToken)
  {
    SspAssertCS(SD_Ssp);

    /* Set up time-outs and next state */
    SD_u32Timeout = G_u32SystemTime1ms;
    SD_pfStateMachine = SdCardSM_WaitCommand;
  }
  else
  {
    /* We didn't get a return token, so abort */
    SD_u8ErrorCode = SD_ERROR_NO_TOKEN;
    SD_pfStateMachine = SdCardSM_Error;
  }
    
} /* end SdCommand() */


/*--------------------------------------------------------------------------------------------------------------------
Function: CheckTimeout

Description:
Checks on timeout and updates the state machine if required.

Requires:
  - State machine is running through states where timeouts are frequently checked and where the result of
    a timeout should be a timeout error and redirection to the error state.
  - u32Time_ is ms count for timeout
  - SD_u32Timeout is the reference time

Promises:
  - if the timeout has occured, sets the erorr code and directs the SM to SdCardSM_Error state
*/
void CheckTimeout(u32 u32Time_)
{
  if( IsTimeUp(&SD_u32Timeout, u32Time_) )
  {
    SD_u8ErrorCode = SD_ERROR_TIMEOUT;
    SD_pfStateMachine = SdCardSM_Error;
  }

} /* end CheckTimeout() */

#if 0
/*--------------------------------------------------------------------------------------------------------------------
Function: AdvanceSD_pu8RxBufferParser

Description:
Safely advances SD_pu8RxBufferParser by the number of bytes required.

Requires:
  - u32NumBytes_ is the number of bytes that the buffer pointer will be advanced

Promises:
  - SD_pu8RxBufferParser moved u32NumBytes_ with wrap-around protection.
*/
void AdvanceSD_pu8RxBufferParser(u32 u32NumBytes_)
{
  for(u32 i = 0; i < u32NumBytes_; i++)
  {
    SD_pu8RxBufferParser++;
    if(SD_pu8RxBufferParser == &SD_au8RxBuffer[SDCARD_RX_BUFFER_SIZE])
    {
      SD_pu8RxBufferParser = &SD_au8RxBuffer[0];
    }
  }
  
} /* end AdvanceSD_pu8RxBufferParser() */


/*--------------------------------------------------------------------------------------------------------------------
Function: FlushSdRxBuffer

Description:
Skips all new bytes currently in the RxBuffer.

Requires:
  - 

Promises:
  - SD_pu8RxBufferParser set to SD_pu8RxBufferNextByte
*/
void FlushSdRxBuffer(void)
{
  SD_pu8RxBufferParser = SD_pu8RxBufferNextByte;
  
} /* end FlushSdRxBuffer() */
#endif

/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a card to be inserted */
static void SdCardSM_IdleNoCard(void)
{

  if( SdIsCardInserted() )
  {
    /* Request the SSP resource to talk to the card */
    SD_Ssp = SspRequest(&SD_sSspConfig);
    if(SD_Ssp == NULL)
    {
      /* Go to wait state if SSP is not available */
      DebugPrintf(SD_au8SspRequestFailed);
      SD_u32Timeout = G_u32SystemTime1ms;
      SD_pfWaitReturnState = SdCardSM_IdleNoCard;
      SD_pfStateMachine = SdCardSM_WaitSSP;
    }
    else
    {
      /* If card is in, set flag and then try to talk to card.  Note that the SSP peripheral will 
      be allocated to the SD card for this whole initialization process. */
      SD_u32Flags &= SD_CLEAR_CARD_TYPE_BITS;

      /* CS is NOT asserted for initial dummy clocks */
      SspDeAssertCS(SD_Ssp);
      
      /* Queue up a set of dummy transfers to make sure the card is awake; */
      if(SspReadData(SD_Ssp, SD_WAKEUP_BYTES))
      {
        SD_pfStateMachine = SdCardSM_Dummies;
      }
      else
      {
        /* We didn't get a return token, so abort */
        SD_u8ErrorCode = SD_ERROR_NO_TOKEN;
        SD_pfStateMachine = SdCardSM_Error;
      }
    }
  }  
  
} /* end SdCardSM_IdleNoCard() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for the dummies to be sent to wake up card. Data that will clock in to RxBuffer is ignored. */
static void SdCardSM_Dummies(void)
{
  if( SspQueryReceiveStatus(SD_Ssp) == SSP_RX_COMPLETE )
  { 
    /* Queue CMD0 to be sent. SdCommand sets SD_pfStateMachine for the next state. */
    SdCommand(&SD_au8CMD0[0]);
    SD_pfWaitReturnState = SdCardSM_ResponseCMD0;
  }
} /* end SdCardSM_Dummies() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Check the response to CMD0. SD_pu8RxBufferParser is pointing to the RxBuffer where a response R1 is sitting */
/* This is not a state but more a call-back after CMD0 */

static void SdCardSM_ResponseCMD0(void)
{
  /* CS is deasserted after CMD0 */
  SspDeAssertCS(SD_Ssp);

  /* Process the received byte */
  if(SD_au8RxBuffer[0] == SD_STATUS_IDLE)
  {
    /* Card is in Idle state, so issue CMD8 */
    SdCommand(&SD_au8CMD8[0]);
    SD_pfWaitReturnState = SdCardSM_ResponseCMD8;
  }
  else
  {
    /* Unexpected response, go to error */
    SD_u8ErrorCode = SD_ERROR_BAD_RESPONSE;
    SD_pfStateMachine = SdCardSM_Error;
  }
  
  /* In any case, advance the buffer pointer */
  //AdvanceSD_pu8RxBufferParser(1);
       
} /* end SdCardSM_ResponseCMD0() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Queue a read to get the CMD8 data. */
/* This is not a state but more a call-back after CMD8 */
static void SdCardSM_ResponseCMD8(void)
{
  /* Check the response byte (response R1) */
  if(SD_au8RxBuffer[0] == SD_STATUS_IDLE)
  {
    /* Command is good which means the card is at least SDv2 so we can read 4 more bytes of the CMD8 response. */
    SD_u32Flags |= _SD_TYPE_SD2;
    
    /* CS is still asserted since we are reading data */
    if(SspReadData(SD_Ssp, 4))
    {
      SD_pfStateMachine = SdCardSM_ReadCMD8;
    }
    else
    {
      /* SSP read error - we'll just abort */
      SD_u8ErrorCode = SD_ERROR_NO_TOKEN;
      SD_pfStateMachine = SdCardSM_Error;
    }
  }
  /* CMD8 not supported => not SDv2 */
  else
  {
    /* The card does not support CMD8 so Deassert CS and go directly to ACMD41 */
    SD_u32Flags &= ~_SD_TYPE_SD2;
    SspDeAssertCS(SD_Ssp);
    /* Do we need a delay here since CS will be asserted almost immediately in SdCommand? */
    
    SdCommand(&SD_au8ACMD41[0]);
    SD_pfWaitReturnState = SdCardSM_ResponseCMD55;
  }
    
  /* In either case, advance the buffer pointer */
  //AdvanceSD_pu8RxBufferParser(1);
   
} /* end SdCardSM_ResponseCMD8() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for the 4 payload bytes from CMD8. */
static void SdCardSM_ReadCMD8(void)
{
  /* Check to see if the SSP peripheral has sent the data request */
//  if( QueryMessageStatus(SD_u32CurrentMsgToken) == COMPLETE )
  if(SspQueryReceiveStatus(SD_Ssp) == SSP_RX_COMPLETE)
  {
    /* CMD8 is finished so deassert CS */
    SspDeAssertCS(SD_Ssp);
    
    /* Process the four response bytes (only the last two matter) */
    //SD_pu8RxBufferParser = ;
    if(SD_au8RxBuffer[SD_CMD8_INDEX_VHS] == SD_VHS_VALUE)
    {
      //AdvanceSD_pu8RxBufferParser(1);
      if(SD_au8RxBuffer[SD_CMD8_INDEX_CHECK] == SD_CHECK_PATTERN)
      {
        /* Card supports VCC 2.7 - 3.6V so we're good to go */
        /* Next we need to send CMD55 so we can send ACMD41 */
        SdCommand(&SD_au8CMD55[0]);
        SD_pfWaitReturnState = SdCardSM_ResponseCMD55;
      }
      //AdvanceSD_pu8RxBufferParser(1);
    }
    else
    {
      /* The card does not support the voltage range so is not usable */
      SD_u8ErrorCode = SD_ERROR_CARD_VOLTAGE;
      SD_pfStateMachine = SdCardSM_Error;
    }
  }
  
  /* Watch for SSP timeout */
  CheckTimeout(SD_SPI_WAIT_TIME_MS);
     
} /* end SdCardSM_ReadCMD8() */

     
/*-------------------------------------------------------------------------------------------------------------------*/
/* Check the response to CMD55. */
/* This is really a call-back to CMD55 */
static void SdCardSM_ResponseCMD55(void)
{
  /* CMD55 is finished so deassert CS */
  SspDeAssertCS(SD_Ssp);

  /* Process the received byte from CMD55 */
  if(SD_au8RxBuffer[0] == SD_STATUS_IDLE)
  {
    /* Card is ready for ACMD41 to configure Host Capacity Support (HCS) bit */
    if(SD_u32Flags & _SD_TYPE_SD2)
    {
      SD_au8ACMD41[1] |= BIT6;
    }
    SdCommand(&SD_au8ACMD41[0]);
    SD_pfWaitReturnState = SdCardSM_ResponseACMD41;
  }
  else
  {
    /* Unexpected response, go to error */
    SD_u8ErrorCode = SD_ERROR_BAD_RESPONSE;
    SD_pfStateMachine = SdCardSM_Error;
  }
  
  /* In any case, advance the buffer pointer */
  //AdvanceSD_pu8RxBufferParser(1);
       
} /* end SdCardSM_ResponseCMD55() */
     
     
/*-------------------------------------------------------------------------------------------------------------------*/
/* Check the response to ACMD41 which is waiting for the card to NOT be in idle (repeat CMD55 + ACMD41 sequence).
Once the card is ready, can send CMD58.  The next step is different for version 1 and version 2 cards.  MMC will not be supported. */
static void SdCardSM_ResponseACMD41(void)
{
  /* ACMD41 is finished so deassert CS */
  SspDeAssertCS(SD_Ssp);

  /* Process the received byte based on card type */
  if(SD_au8RxBuffer[0] == SD_STATUS_READY)
  {
    /* Card is ready for next command */
    if(SD_u32Flags & _SD_TYPE_SD2)
    {
      /* SDv2 cards use CMD58 */
      SdCommand(&SD_au8CMD58[0]);
      SD_pfWaitReturnState = SdCardSM_ResponseCMD58;
    }
    else
    {
      /* SDv1 card: set flag and block access size */
      SD_u32Flags |= _SD_TYPE_SD1;
      
      /* SDv1 cards are always low capacity, but can have variable block access.   
      Set to 512 to match SDv2. */
      SdCommand(&SD_au8CMD16[0]);
      SD_pfWaitReturnState = SdCardSM_ResponseCMD16;
    }
  }
  else 
  {
    /* Card is not idle yet, so repeat */
    SdCommand(&SD_au8CMD55[0]);
    SD_pfWaitReturnState = SdCardSM_ResponseCMD55;
  }
  
  /* In any case, advance the buffer pointer */
  //AdvanceSD_pu8RxBufferParser(1);
       
} /* end SdCardSM_ResponseCMD55() */     


/*-------------------------------------------------------------------------------------------------------------------*/
/* Queue a read to get the CMD58 data. RxBuffer has R1 response byte. */
static void SdCardSM_ResponseCMD58(void)
{
  /* Check the response byte (response R1) */
  if(SD_au8RxBuffer[0] == SD_STATUS_READY)
  {
    /* Command is good so we can read 4 more bytes of the CMD58 response */
    /* CS is still asserted since we are reading data */
    if(SspReadData(SD_Ssp, 4))
    {
      SD_pfStateMachine = SdCardSM_ReadCMD58;
    }
    else
    {
      /* SSP read error - we'll just abort */
      SD_u8ErrorCode = SD_ERROR_NO_TOKEN;
      SD_pfStateMachine = SdCardSM_Error;
    }
  }
  else
  {
    SD_u8ErrorCode = SD_ERROR_BAD_RESPONSE;
    SD_pfStateMachine = SdCardSM_Error;
  }
    
  /* In either case, advance the buffer pointer */
  //AdvanceSD_pu8RxBufferParser(1);
       
} /* end SdCardSM_ResponseCMD8() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Process response to CMD16. */
/* Not a state, just a call back for CMD16 */
static void SdCardSM_ResponseCMD16(void)
{
  /* Check the response byte (response R1) */
  if(SD_au8RxBuffer[0] == SD_STATUS_READY)
  {
    /* Success! Card is ready for read/write operations.  We can release the SSP resource for now. */
    SspDeAssertCS(SD_Ssp);
    SspRelease(SD_Ssp);

    SD_CardState = SD_IDLE;
    DebugPrintf(SD_au8CardReady);

    SD_pfStateMachine = SdCardSM_ReadyIdle;
  }
  else
  {
    SD_u8ErrorCode = SD_ERROR_BAD_RESPONSE;
    SD_pfStateMachine = SdCardSM_Error;
  }
    
  /* In either case, advance the buffer pointer */
  //AdvanceSD_pu8RxBufferParser(1);
       
} /* end SdCardSM_ResponseCMD8() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a data for CMD58. RxBuffer will have four response bytes when SSP is complete. */
static void SdCardSM_ReadCMD58(void)
{
  /* Check to see if the SSP peripheral has sent the command */
//  if( QueryMessageStatus(SD_u32CurrentMsgToken) == COMPLETE )
  if(SspQueryReceiveStatus(SD_Ssp) == SSP_RX_COMPLETE)
  {
    /* CMD58 is finished so deassert CS */
    SspDeAssertCS(SD_Ssp);

    /* Determine card capacity */
    SD_u32Flags &= ~_SD_CARD_HC;
    if(SD_au8RxBuffer[0] & _SD_OCR_CCS_BIT)
    {
      SD_u32Flags |= _SD_CARD_HC;
      
      /* Success! Card is ready for read/write operations */
      SspRelease(SD_Ssp);
  
      SD_CardState = SD_IDLE;
      DebugPrintf(SD_au8CardReady);
    
      SD_pfStateMachine = SdCardSM_ReadyIdle;
    }
    /* For standard capacity, make sure block size is 512 */
    else
    {
      SdCommand(&SD_au8CMD16[0]);
      SD_pfWaitReturnState = SdCardSM_ResponseCMD16;
    }
    
    /* Ignore the other 3 response bytes */
    //AdvanceSD_pu8RxBufferParser(4);
  }
  
  /* Watch for SSP timeout */
  CheckTimeout(SD_SPI_WAIT_TIME_MS);
     
} /* end SdCardSM_ReadCMD58() */
           
#if 0     
/*-------------------------------------------------------------------------------------------------------------------*/
/* Kill time waiting for the SD card to indicate it is ready after CS.
SD_NextCommand is queue with the command that will be sent once
the card is ready. */
     
static void SdCardSM_WaitReady(void)
{
  if( QueryMessageStatus(SD_u32CurrentMsgToken) == COMPLETE )
  {  
    if( *SD_pu8RxBufferParser != 0xFF )
    {
      if(!SspReadByte(SD_Ssp))
      {
        /* We didn't get a return token, so abort */
        SD_u8ErrorCode = SD_ERROR_NO_TOKEN;
        SD_pfStateMachine = SdCardSM_Error;
      }
    }
    /* The card is ready for the command */
    else
    {
      SD_u32CurrentMsgToken = SspWriteData(SD_Ssp, SD_CMD_SIZE, SD_NextCommand);
      
      /* Pre-emptively move RxBufferParser so it will point to command response */
      //AdvanceSD_pu8RxBufferParser(SD_CMD_SIZE);
    
      /* Set up time-outs and next state */
      SD_u32Timeout = G_u32SystemTime1ms;
      SD_pfStateMachine = SdCardSM_WaitCommand;
    }
  }
  
  /* Watch for SSP timeout */
  CheckTimeout(SD_SPI_WAIT_TIME_MS);
     
} /* end SdCardSM_WaitReady() */
#endif

/*-------------------------------------------------------------------------------------------------------------------*/
/* Kill time waiting for a command to finish sending; the first byte from all completed commands
is response R1 which has BIT7 clear.
     
REQUIRES: 
  - SD_u32CurrentMsgToken references the message that is being sent
     
PROMISES: 
  - State machine set to either SdCardSM_WaitResponse or SdCardSM_Error
*/
static void SdCardSM_WaitCommand(void)
{
  /* Check to see if the SSP peripheral has sent the command */
  if( QueryMessageStatus(SD_u32CurrentMsgToken) == COMPLETE )
  {
    /* Request 1 byte (response byte from card) */  
    if( SspReadByte(SD_Ssp) )
    {
      SD_pfStateMachine = SdCardSM_WaitResponse;
    }
    else
    {
      /* We didn't get a return token, so abort */
      SD_u8ErrorCode = SD_ERROR_NO_TOKEN;
      SD_pfStateMachine = SdCardSM_Error;
    }
  }

  /* Monitor time out */
  if( IsTimeUp(&SD_u32Timeout, SD_WAIT_TIME) )
  {
    SD_u8ErrorCode = SD_ERROR_TIMEOUT;
    SD_pfStateMachine = SdCardSM_Error;
  }
     
} /* end SdCardSM_WaitCommand() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Kill time waiting for a response read to finish; the first byte from all completed commands
is response R1 which has BIT7 clear.  This response will be at SD_au8RxBuffer[0].
     
REQUIRES: 
  - SD_pfWaitReturnState points to the function that should be accessed next.
     
PROMISES: 
  - State machine set to either SD_pfWaitReturnState or SdCardSM_Error
  - RxBuffer holds response byte 
*/
static void SdCardSM_WaitResponse(void)
{
  static u8 u8Retries = SD_CMD_RETRIES;
  
  /* Check to see if the SSP peripheral has sent the read request */
  if(SspQueryReceiveStatus(SD_Ssp) == SSP_RX_COMPLETE)
  {
    /* If no response but retries left, queue another read */
    if( (SD_au8RxBuffer[0] & BIT7) && (u8Retries != 0) )
    {
      u8Retries--;
      
      if(u8Retries == 0)
      {
        SD_u8ErrorCode = SD_ERROR_BAD_RESPONSE;
        SD_pfStateMachine = SdCardSM_Error;
      }
      else if( !SspReadByte(SD_Ssp) )
      {
        /* We didn't get a return token, so abort */
        SD_u8ErrorCode = SD_ERROR_NO_TOKEN;
        SD_pfStateMachine = SdCardSM_Error;
      }
    }
    else
    {
      /* Otherwise we have a good response, so return now */
      u8Retries = SD_CMD_RETRIES;
      SD_pfStateMachine = SD_pfWaitReturnState;
    }
  }

  /* Monitor time out */
  if( IsTimeUp(&SD_u32Timeout, SD_WAIT_TIME) )
  {
    u8Retries = SD_CMD_RETRIES;
    SD_u8ErrorCode = SD_ERROR_TIMEOUT;
    SD_pfStateMachine = SdCardSM_Error;
  }
     
} /* end SdCardSM_WaitResponse() */

  
/*-------------------------------------------------------------------------------------------------------------------*/
/* Kill time before checking SSP availability again */
static void SdCardSM_WaitSSP(void)          
{
  if( IsTimeUp(&SD_u32Timeout, SD_SPI_WAIT_TIME_MS) )
  {
    SD_pfStateMachine = SD_pfWaitReturnState;
  }
  
} /* end SdCardSM_WaitSSP() */
     
     
/*-------------------------------------------------------------------------------------------------------------------*/
/* SD card is initialized: wait for action request. */
static void SdCardSM_ReadyIdle(void)          
{
  /* Check if the card is still in; if not return through WaitSSP to allow some debounce time */
  if( !SdIsCardInserted() )
  {
    SD_u32Flags &= SD_CLEAR_CARD_TYPE_BITS;
    
    /* Exit through a wait state for effective debouncing */
    SD_u32Timeout = G_u32SystemTime1ms;
    SD_pfWaitReturnState = SdCardSM_IdleNoCard;
    SD_pfStateMachine = SdCardSM_WaitSSP;
  }
  else
  {
    /* Look for a request to read or write file data */
    if( (SD_CardState == SD_WRITING) || (SD_CardState == SD_READING) )
    {
      /* Request the SSP resource to talk to the card */
      SD_Ssp = SspRequest(&SD_sSspConfig);
      if(SD_Ssp == NULL)
      {
        /* Go to wait state if SSP is not available */
        DebugPrintf(SD_au8SspRequestFailed);
        SD_u32Timeout = G_u32SystemTime1ms;
        SD_pfWaitReturnState = SdCardSM_ReadyIdle;
        SD_pfStateMachine = SdCardSM_WaitSSP;
      }
      else
      {
        /* Got SSP, so start read or write */
        if(SD_CardState == SD_WRITING)
        {
          /* Not yet implemented */
          SD_pfStateMachine = SdCardSM_ReadyIdle;
          SD_CardState = SD_IDLE;
        }
        else
        {
          /* Parse out the bytes of the address into the command array */
          SD_au8CMD17[1] = (u8)(SD_u32Address >> 24);
          SD_au8CMD17[2] = (u8)(SD_u32Address >> 16);
          SD_au8CMD17[3] = (u8)(SD_u32Address >> 8);
          SD_au8CMD17[4] = (u8)SD_u32Address;
          
          SdCommand(&SD_au8CMD17[0]);
          SD_pfWaitReturnState = SdCardSM_ResponseCMD17;
        }
      }
    }
  }
} /* end SdCardSM_ReadyIdle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Start read sequence */
static void SdCardSM_ResponseCMD17(void)
{
  /* Check the response byte (response R1) */
  if(SD_au8RxBuffer[0] == SD_STATUS_READY)
  {
    /* Queue a read looking to get TOKEN_START_BLOCK back from the card */
    //SD_u32CurrentMsgToken = SspReadByte(SD_Ssp);
    if(SspReadByte(SD_Ssp))
    {
      SD_u32Timeout = G_u32SystemTime1ms;
      SD_pfStateMachine = SdCardSM_WaitStartToken;
    }
    else
    {
      /* SSP read error - we'll just abort */
      SD_u8ErrorCode = SD_ERROR_NO_TOKEN;
      SD_pfStateMachine = SdCardSM_Error;
    }
  }
  else
  {
    /* Incorrect response from the SD card, so abort */
    SD_u8ErrorCode = SD_ERROR_BAD_RESPONSE;
    SD_pfStateMachine = SdCardSM_FailedDataTransfer;
  }

  /* Either way, advance the RxBuffer pointer */  
  //AdvanceSD_pu8RxBufferParser(1);

} /* end SdCardSM_ResponseCMD17() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Look for the returned token that indicates the read or write process has begun */
static void SdCardSM_WaitStartToken(void)          
{
  /* Check if the SSP peripheral has sent the data request */
//  if( QueryMessageStatus(SD_u32CurrentMsgToken) == COMPLETE )
  if(SspQueryReceiveStatus(SD_Ssp) == SSP_RX_COMPLETE)
  {
    /* Check the response byte */
    if(SD_au8RxBuffer[0] == TOKEN_START_BLOCK)
    {
      /* Set the RxBuffer pointers to the start of the RxBuffer so the sector data occupies the beginning */
      //SD_pu8RxBufferNextByte = &SD_au8RxBuffer[0];
      //SD_pu8RxBufferParser   = &SD_au8RxBuffer[0];
      
      /* Queue a read for the entire sector plus two checksum bytes */
      /* CS is still asserted since we are reading data */
      if(SspReadData(SD_Ssp, 514))
      {
        SD_pfStateMachine = SdCardSM_DataTransfer;
      }
      else
      {
        /* SSP read error - we'll just abort */
        SD_u8ErrorCode = SD_ERROR_NO_TOKEN;
        SD_pfStateMachine = SdCardSM_Error;
      }
    }
    else
    {
      /* Didn't get the start token so retry */
      if(SspReadByte(SD_Ssp))
      {
        SD_pfStateMachine = SdCardSM_WaitStartToken;
      }
      else
      {
        /* SSP read error - we'll just abort */
        SD_u8ErrorCode = SD_ERROR_NO_TOKEN;
        SD_pfStateMachine = SdCardSM_Error;
      }
    }
  }
  
  /* Monitor time */
  if(IsTimeUp(&SD_u32Timeout, SD_READ_TOKEN_MS))
  {
    SD_u8ErrorCode = SD_ERROR_TIMEOUT;
    SD_pfStateMachine = SdCardSM_FailedDataTransfer;
  }
  
} /* end SdCardSM_WaitStartToken() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Read the sector */
static void SdCardSM_DataTransfer(void)
{
  /* Check if the SSP peripheral is finished with the data request */
  if(SspQueryReceiveStatus(SD_Ssp) == SSP_RX_COMPLETE)
  {
    SD_CardState = SD_DATA_READY;

    SspDeAssertCS(SD_Ssp);
    SspRelease(SD_Ssp);

    /* Reset the RxBuffer pointers to the start of the RxBuffer */
    //SD_pu8RxBufferNextByte = &SD_au8RxBuffer[0];
    //SD_pu8RxBufferParser   = &SD_au8RxBuffer[0];

    SD_pfStateMachine = SdCardSM_ReadyIdle;
  }

  /* Monitor time */
  if(IsTimeUp(&SD_u32Timeout, SD_SECTOR_READ_TIMEOUT_MS))
  {
    SD_u8ErrorCode = SD_ERROR_TIMEOUT;
    SD_pfStateMachine = SdCardSM_Error;
  }

} /* end SdCardSM_DataTransfer() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle a failed data transfer */
static void SdCardSM_FailedDataTransfer(void)
{
  /* Reset the system variables */
  SspDeAssertCS(SD_Ssp);
  SspRelease(SD_Ssp);
  //FlushSdRxBuffer();
  SD_CardState = SD_CARD_ERROR;
  
  SD_u32Timeout = G_u32SystemTime1ms;
  SD_pfStateMachine = SdCardSM_WaitSSP;
  
} /* end SdCardSM_FailedDataTransfer() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void SdCardSM_Error(void)          
{
  u8* pu8ErrorMessage;
  //u8 u8MessageSize;
  
  /* Reset the system variables */
  SspDeAssertCS(SD_Ssp);
  SspRelease(SD_Ssp);
  //FlushSdRxBuffer();

  /* Indicate error and return through the SSP delay state to give the system some recovery time */
  DebugPrintf(SD_au8CardError);
  switch (SD_u8ErrorCode)
  {
    case SD_ERROR_TIMEOUT:
    {
      //u8MessageSize = sizeof(SD_au8CardError1) - 1;
      pu8ErrorMessage = SD_au8CardError1;
      break;
    }
    
    case SD_ERROR_CARD_VOLTAGE:
    {
      //u8MessageSize = sizeof(SD_au8CardError2) - 1;
      pu8ErrorMessage = SD_au8CardError2;
      break;
    }

    case SD_ERROR_BAD_RESPONSE:
    {
      //u8MessageSize = sizeof(SD_au8CardError3) - 1;
      pu8ErrorMessage = SD_au8CardError3;
      break;
    }

    case SD_ERROR_NO_TOKEN:
    {
      //u8MessageSize = sizeof(SD_au8CardError4) - 1;
      pu8ErrorMessage = SD_au8CardError4;
      break;
    }

    case SD_ERROR_NO_SD_TOKEN:
    {
      //u8MessageSize = sizeof(SD_au8CardError5) - 1;
      pu8ErrorMessage = SD_au8CardError5;
      break;
    }
    
   default:
   {
    //u8MessageSize = sizeof(SD_au8CardError0) - 1;
    pu8ErrorMessage = SD_au8CardError0;
    break;
  }
   
  } /* end switch */
  
  DebugPrintf(pu8ErrorMessage);
  
  SD_CardState = SD_NO_CARD;
  SD_u32Timeout = G_u32SystemTime1ms;
  SD_pfWaitReturnState = SdCardSM_IdleNoCard;
  SD_pfStateMachine = SdCardSM_WaitSSP;
  
} /* end SdCardSM_Error() */


#ifndef ENABLE_SD 
/*-------------------------------------------------------------------------------------------------------------------*/
/* SD card disbled state */
static void SdCardSM_Disabled(void)
{
  
} /* end SdCardSM_Disabled */
#endif /* ENABLE_SD */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
