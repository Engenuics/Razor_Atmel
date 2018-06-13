/***********************************************************************************************************************
File: dataflash.c

Description:
The Data flash application provides services to
Read x bytes
Write a page
Erase a page
Read device ID

API Description:
DataflashCheckStatusType DataflashCheckStatus(u32 u32Query_)
Call with an argument of 0 to check availability of the data flash.  Call with
a token value to check read/write operation status.
Function returns one of the following:
  DATAFLASH_STATUS_BUSY - the flash application is not available
  DATAFLASH_STATUS_READY - the flash application is available
  DATAFLASH_STATUS_DATA_READY - the data from a read request is now ready and should be read immediately
  DATAFLASH_STATUS_WRITE_COMPLETE - data has been written to the flash
  DATAFLASH_STATUS_ERASE_COMPLETE - the requested page erase is complete
  DATAFLASH_STATUS_INVALID_TOKEN - the token used is invalid
  DATAFLASH_STATUS_ERROR - an error has occured

An application should always first check if the dataflash is available by calling DataflashStatus() with
an argument of 0.

u32 DataflashReadBytes(u16 u16PageAddress_, u16 u16StartByte_, u16 u16Bytes_)
Sets up a read of the specified number of bytes from the main dataflash memory. The functions returns a token for the inquiry
that the client must use to check the status of the read operation.

void DataflashGetReadBytes(u16 u16Bytes_, u8* pu8Destination_)
Transfer the read bytes to the client.  This function should be called immediately after DataflashCheckStatus() returns
DATAFLASH_STATUS_DATA_READY.

u32 DataflashWritePage(u16 u16PageAddress_, u8* pu8Data_)
Writes a complete page (264 bytes) of data to the flash.  The destination page in memory must be erased first.
The functions returns a token for the inquiry that the client must use to check the status of the read operation.

u32 DataflashErasePage(u16 u16PageAddress_)
Erase a complete page (264 bytes).  The functions returns a token for the inquiry that the client must use to check
the status of the read operation.


This application relies on a system that will not have a context change while a client application
is checking the status and deciding to read or write data.  Therefore the client can assume
that if DataflashStatus() indicates the flash is ready, a call to read/write date immediately after
will work.  However, if somehow the flash because busy between those calls, it will still return
0 for a message token to indicate that the read or write could not start.  Given this, the client
could call those functions directly and use that value.

For read operations, the client must read the data within 500 ms once it is ready or it
will be discarded and the dataflash will be returned to the ready state.  As soon as the data
is read, it is discarded by the application and the flash is ready for the next operation.

For write and erase operations, the client should check that the write is complete, though it is not essential.
The write status will be available for 500ms after the write is complete before the token is invalid.


***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>DataFlash"
***********************************************************************************************************************/
/* New variables */
fnCode_type G_DataflashStateMachine;                       /* The Dataflash state machine */

u32 G_u32DataflashFlags;                                   /* Dataflash flag register */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Dataflash_" and be declared as static.
***********************************************************************************************************************/
static u32 Dataflash_u32Timeout;                                 /* Cross-state timer */
static u8 Dataflash_u8ErrorCode;                                 /* Global error code */

static DataFlashCommandType Dataflash_eCommand;                  /* Current command for data flash */
static u32 Dataflash_u32CurrentSspToken;                         /* Token of current SSP message */

static u32 Dataflash_u32Token;                                   /* Local token given to client application used to monitor the current dataflash transaction */
static DataflashCheckStatusType Dataflash_eTokenStatus;          /* Status of the message related to the current token */
static u32 Dataflash_u32TokenTimeout;                            /* Timer to track the life of a message token with a result status */

static SspConfigurationType Dataflash_sSspConfig;                /* Configuration information for SSP peripheral */
static SspPeripheralType *Dataflash_Ssp;                         /* Pointer to SSP peripheral object */

static u8 Dataflash_au8RxBuffer[DATAFLASH_RX_BUFFER_SIZE];       /* Space for incoming bytes from the dataflash */
static u8 *Dataflash_pu8RxBufferNextByte;                        /* Pointer to next spot in RxBuffer to write a byte */
static u8 *Dataflash_pu8RxBufferUnreadByte;                      /* Pointer to spot in RxBuffer to read a new byte */

static u8 Dataflash_au8TxBuffer[DATAFLASH_TX_BUFFER_SIZE];       /* Space for outgoing bytes to the dataflash */

static u8 Dataflash_u8VendorID;                                  /* Vendor ID read from connected flash */
static u8 Dataflash_u8DeviceID1;                                 /* Vendor ID read from connected flash */
static u8 Dataflash_u8DeviceID2;                                 /* Vendor ID read from connected flash */

static u16 Dataflash_u16MessageBytes;                            /* Bytes to read or write from the current client */
u16 Dataflash_u16PageAddress;                             /* Starting page address from the current client */
u16 Dataflash_u16StartByte;                               /* Starting byte address from the current client */

static u8 Dataflash_u8AddressByteHi;                             /* Calculated high byte for 24-bit address */
static u8 Dataflash_u8AddressByteMd;                             /* Calculated mid byte for 24-bit address */
static u8 Dataflash_u8AddressByteLo;                             /* Calculated low byte for 24-bit address */


static u8 Dataflash_au8BusyMsg[]         = "Dataflash busy.\n\r";
static u8 Dataflash_au8ErrorMsg[]        = "Dataflash error: ";
static u8 Dataflash_au8ErrorUnknown[]    = "UNKNOWN\n\r";
static u8 Dataflash_au8ErrorTimeoutMsg[] = "TIMEOUT\n\r";
//static u8 Dataflash_au8ErrorBadIDMsg[]   = "WRONG ID\n\r";


/***********************************************************************************************************************
* Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: DataflashCheckStatus

Description:
Allows a client to queury the state of the dataflash task.  If an argument of 0 is used, the function returns
the general DATAFLASH_STATUS_BUSY or DATAFLASH_STATUS_READY state.  If a client is looking for the status of a particular
message, it should call the function with the message token of interest and will get the current status of the message.
  DATAFLASH_STATUS_WRITE_COMPLETE - data has finished writing to the flash
  DATAFLASH_STATUS_ERASE_COMPLETE - the reqeusted erase is complete
  DATAFLASH_STATUS_INVALID_TOKEN - the token provided is not valid

Requires:
  - u32Query_ is either 0 for general status, or a dataflash message token number.
  - this function should not be called from an ISR

Promises:
  - Returns the device status or messages status
*/
DataflashCheckStatusType DataflashCheckStatus(u32 u32Query_)
{
  DataflashCheckStatusType eStatusTemp;

  /* Determine what to return based on u32Query_ */
  if(u32Query_)
  {
    /* Save a copy of the token status because it might get wiped out */
    eStatusTemp = Dataflash_eTokenStatus;

    /* If the current status isn't busy, then expect the client to do something immediately after this call */
    if(Dataflash_eTokenStatus != DATAFLASH_STATUS_BUSY)
    {
      /* Zero out the delay timer and call DataflashSM_WaitCommand state the so the data flash will be available. */
      Dataflash_u32TokenTimeout = 0;
      DataflashSM_WaitCommand();
    }

    /* Return the status of the queried message */
    return eStatusTemp;
  }

  /* Otherwise user wants just a busy status inquiry*/
  else
  {
    if(Dataflash_eCommand == DATAFLASH_COMMAND_IDLE)
    {
      return DATAFLASH_STATUS_READY;
    }

    else
    {
      return DATAFLASH_STATUS_BUSY;
    }
  }
} /* end DataflashCheckStatus() */


/*----------------------------------------------------------------------------------------------------------------------
Function: DataflashReadBytes

Description:
Sets up a read of the specified number of bytes from the dataflash memory.

Requires:
  - u16PageAddress_ is the 10-bit page address in flash of the first byte to read
  - u16StartByte_ is the 9 bit byte address within the specified page of the first byte to read
  - u16Bytes_ is the number of bytes to read

Promises:
  - Dataflash_u16PageAddress, Dataflash_u16MessageBytes and Dataflash_u16StartByte are filled with the passed parameters
  - Dataflash_eCommand is set to DATAFLASH_COMMAND_READ_BYTES
  - Returns the message token 
*/
u32 DataflashReadBytes(u16 u16PageAddress_, u16 u16StartByte_, u16 u16Bytes_)
{
  /* Check if Dataflash is available */
  if(Dataflash_eCommand == DATAFLASH_COMMAND_IDLE)
  {
    Dataflash_eCommand = DATAFLASH_COMMAND_READ_BYTES;
    Dataflash_eTokenStatus = DATAFLASH_STATUS_BUSY;

    /* Capture the data */
    Dataflash_u16PageAddress  = u16PageAddress_;
    Dataflash_u16StartByte    = u16StartByte_;
    Dataflash_u16MessageBytes = u16Bytes_;

    /* Return the current token */
    return Dataflash_u32Token;
  }

  else
  {
    DebugPrintf(strlen((char const *)Dataflash_au8BusyMsg), Dataflash_au8BusyMsg);
    return 0;
  }

} /* end DataflashReadBytes() */


/*----------------------------------------------------------------------------------------------------------------------
Function: DataflashGetReadBytes

Description:
Retrieves the read data to the application.

Requires:
  - u16Bytes_ is the number of bytes to read
  - pu8Destination points to the location to read the data from and must be large enough to read u16Bytes_ of data

Promises:
  - u16Bytes_ are transferred to the client destination
*/
void DataflashGetReadBytes(u16 u16Bytes_, u8 *pu8Destination_)
{
  for(u16 i = 0; i < u16Bytes_; i++)
  {
    *(pu8Destination_ + i) = *Dataflash_pu8RxBufferUnreadByte;
    AdvanceRxBufferUnreadBytePointer(1);
  }

} /* end DataflashGetReadBytes() */


/*----------------------------------------------------------------------------------------------------------------------
Function: DataflashWriteBytes

Description:
Sets up to write specific bytes (not a whole page) in the main dataflash memory.
To do this, the page of interest will have to be read and the changed bytes inserted.  Then the
page will need to be erased and re-written.

NOT YET IMPLEMENTED

Requires:
  - u16PageAddress_ is the 10-bit page address in flash of the first byte to read
  - u16StartByte_ is the 9 bit byte address within the specified page of the first byte to read
  - u16Bytes_ is the number of bytes to read

Promises:
  - TBD
*/

/*----------------------------------------------------------------------------------------------------------------------
Function: DataflashWritePage

Description:
Sets up a write of one page in the main dataflash memory.  It is assumed the page to write
is already erased.

Requires:
  - u16PageAddress_ is the 10-bit page address in flash of the first byte to write
  - pu8Data_ points to the data to write which should always be a complete page (264 bytes)

Promises:
  - Dataflash_u16PageAddress is filled with the passed parameters
  - Dataflash_eCommand is set to DATAFLASH_COMMAND_WRITE_PAGE
*/
u32 DataflashWritePage(u16 u16PageAddress_, u8 *pu8Data_)
{
  /* Check if Dataflash is available */
  if(Dataflash_eCommand == DATAFLASH_COMMAND_IDLE)
  {
    Dataflash_eCommand = DATAFLASH_COMMAND_WRITE_PAGE;
    Dataflash_eTokenStatus = DATAFLASH_STATUS_BUSY;

    Dataflash_u16MessageBytes = DATAFLASH_PAGE_SIZE;

    /* Capture the page and data to write to the local globals */
    Dataflash_u16PageAddress = u16PageAddress_;
    memcpy(&Dataflash_au8TxBuffer[0], pu8Data_, Dataflash_u16MessageBytes);

    /* Return the current token */
    return Dataflash_u32Token;
  }

  else
  {
    DebugPrintf(strlen((char const *)Dataflash_au8BusyMsg), Dataflash_au8BusyMsg);
    return 0;
  }

} /* end DataflashWritePage() */


/*----------------------------------------------------------------------------------------------------------------------
Function: DataflashErasePage

Description:
Sets up a page erase.

Requires:
  - u16PageAddress_ is the 10-bit page address in flash of the target page

Promises:
  - Flash page erase is queued.
  - Token number for operation is returned.

*/
u32 DataflashErasePage(u16 u16PageAddress_)
{
  /* Check if Dataflash is available */
  if(Dataflash_eCommand == DATAFLASH_COMMAND_IDLE)
  {
    Dataflash_eCommand = DATAFLASH_COMMAND_ERASE_PAGE;
    Dataflash_eTokenStatus = DATAFLASH_STATUS_BUSY;

    /* Capture the data */
    Dataflash_u16PageAddress = u16PageAddress_;

    /* Return the current token */
    return Dataflash_u32Token;
  }

  else
  {
    DebugPrintf(strlen((char const *)Dataflash_au8BusyMsg), Dataflash_au8BusyMsg);
    return 0;
  }

} /* end DataflashErasePage() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: DataflashInitialize

Description:
Sets up the application.

Requires:
  - The dataflash application is not yet running
  - The SSP resource requested should be free

Promises:
  - SSP resource Dataflash_au8RxBuffer initialized to all 0
  - Dataflash_pu8RxBufferParser set to the start of the buffer
  - G_DataflashStateMachine set to Idle
*/
void DataflashInitialize(void)
{
  u8 au8DataflashStarted[] = "Dataflash initializing... ID: ";
  u8 au8DataflashFail[]    = "Dataflash failed init\n\r";

  /* Initailze startup values and the command array */
  Dataflash_u8ErrorCode = DATAFLASH_ERROR_NONE;
  ClearReceiveBuffer();
  Dataflash_pu8RxBufferNextByte = &Dataflash_au8RxBuffer[0];
  Dataflash_pu8RxBufferUnreadByte = &Dataflash_au8RxBuffer[0];
  Dataflash_eCommand = DATAFLASH_COMMAND_IDLE;
  Dataflash_u32Token = 1;
  Dataflash_eTokenStatus = DATAFLASH_STATUS_READY;

  /* Configure the SSP resource to be used for the Dataflash application */
  Dataflash_sSspConfig.SspPeripheral      = SSP0;
  Dataflash_sSspConfig.pGpioAddress       = LPC_GPIO0;
  Dataflash_sSspConfig.u32Pin             = P0_25_SSEL0_FLASH;
  Dataflash_sSspConfig.pu8RxBufferAddress = &Dataflash_au8RxBuffer[0];
  Dataflash_sSspConfig.pu8RxNextByte      = &Dataflash_pu8RxBufferNextByte;
  Dataflash_sSspConfig.u32RxBufferSize    = DATAFLASH_RX_BUFFER_SIZE;
  Dataflash_sSspConfig.BitOrder           = MSB_FIRST;
  Dataflash_sSspConfig.SpiMode            = SPI_MODE3;

  /* Test communications */
  Dataflash_Ssp = SspRequest(&Dataflash_sSspConfig);

  /* Go to error state if the SspRequest failed */
  if(Dataflash_Ssp == NULL)
  {
    G_DataflashStateMachine = DataflashSM_Error;

  }

  /* Otherwise attempt to read the ID string from the Dataflash */
  else
  {
    /* Send the ID request command and read the necessary bytes (during init, the read
    cycle is executed completely so the data is ready after SspReadData) */
    SspAssertCS(Dataflash_Ssp);

    /* Write the command: don't care about the dummy byte */
    SspWriteByte(Dataflash_Ssp, DATAFLASH_READ_ID);

    SspReadData(Dataflash_Ssp, DATAFLASH_READ_ID_SIZE);
    SspDeAssertCS(Dataflash_Ssp);

    /* Read the received data - Since this is init, we know where the data is */
    Dataflash_u8VendorID  = Dataflash_au8RxBuffer[1];
    Dataflash_u8DeviceID1 = Dataflash_au8RxBuffer[2];
    Dataflash_u8DeviceID2 = Dataflash_au8RxBuffer[3];

    /* Catch the pointer up for the write and read */
    AdvanceRxBufferUnreadBytePointer(1 + DATAFLASH_READ_ID_SIZE);

    if(Dataflash_u8VendorID == ATMEL_VENDOR_ID)
    {
      /* If ID request is successful, the application is ready for service */
      DebugPrintf(strlen((char const *)au8DataflashStarted), &au8DataflashStarted[0]);
      PrintNumber(Dataflash_u8VendorID);
      PrintNumber(Dataflash_u8DeviceID1);
      PrintNumber(Dataflash_u8DeviceID2);
      DebugLineFeed();

      G_u32ApplicationFlags  |= _APPLICATION_FLAGS_DATAFLASH;
      G_DataflashStateMachine = DataflashSM_Idle;
    }

    else
    {
      DebugPrintf(strlen((char const *)au8DataflashFail), &au8DataflashFail[0]);
      Dataflash_u8ErrorCode = DATAFLASH_ERROR_ID;
      G_DataflashStateMachine = DataflashSM_Error;
    }
  }

  SspRelease(Dataflash_Ssp);

} /* end  DataflashInitialize() */

#if 0 /* This is sign-specific, so should not be in this driver */
/*----------------------------------------------------------------------------------------------------------------------
Function: DataflashReadSignConfig

NOT YET IMPLEMENTED

Description:
Special function to read the sign configuration values during system
initialization.  All bytes are read and populated to G_SignConfiguration
if they are present.

Requires:
  - Dataflash is initialized
  - Configuration value SIGN_CONFIGURED_VALUE is present at

Promises:
  - If the configuration values are present, loads G_SignConfiguration
    with the values read from flash and returns TRUE
  - If the configuration values are not present, returns FALSE and
    does not change G_SignConfiguration.
*/
bool DataflashReadSignConfig(void)
{
  return FALSE;

} /* end DataflashReadSignConfig() */
#endif


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: ClearReceiveBuffer()

Description:
Clears the receive buffer.

Requires:
  -

Promises:
  - Dataflash_au8RxBuffer is filled with DATAFLASH_DUMMY_BYTE
*/
static void ClearReceiveBuffer(void)
{
  for(u16 i = 0; i < DATAFLASH_RX_BUFFER_SIZE; i++)
  {
    Dataflash_au8RxBuffer[i] = DATAFLASH_DUMMY_BYTE;
  }

} /* end ClearReceiveBuffer() */


/*----------------------------------------------------------------------------------------------------------------------
Function: DataflashOperationComplete()

Description:
Clean-up functions to terminate a read, write, or erase sequence.

Requires:
  -

Promises:
  - DeAsserts CS and releases the SSP resource.
  - Initializes Dataflash_u32TokenTimeout to count down the valid data period
  - Sets the state machine to the command exitting function
*/
static void DataflashOperationComplete(void)
{
  SspDeAssertCS(Dataflash_Ssp);
  SspRelease(Dataflash_Ssp);
  Dataflash_u32TokenTimeout = G_u32SystemTime1ms;
  G_DataflashStateMachine = DataflashSM_WaitCommand;

#if 0 /* 2015-11-30 */
  G_DataflashStateMachine = DataflashSM_Idle;
#endif
  
} /* end DataflashOperationComlete() */


/*----------------------------------------------------------------------------------------------------------------------
Function: AdvanceRxBufferUnreadBytePointer()

Description:
Safely moves the Dataflash_pu8RxBufferUnreadByte pointer in the Rx Buffer

Requires:
  -

Promises:
  - Dataflash_pu8RxBufferUnreadByte is moved forward u16BytesToMove_ bytes in the circular buffer
*/
static void AdvanceRxBufferUnreadBytePointer(u16 u16BytesToMove_)
{
  for(u16 i = 0; i < u16BytesToMove_; i++)
  {
    Dataflash_pu8RxBufferUnreadByte++;

    if(Dataflash_pu8RxBufferUnreadByte == &Dataflash_au8RxBuffer[DATAFLASH_RX_BUFFER_SIZE])
    {
      Dataflash_pu8RxBufferUnreadByte = &Dataflash_au8RxBuffer[0];
    }
  }

} /* end AdvanceRxBufferUnreadBytePointer() */


/*--------------------------------------------------------------------------------------------------------------------
Function: DataflashCheckTimeout

Description:
Checks on timeout and updates the state machine if required.

Requires:
  - State machine is running through states where timeouts are frequently checked and where the result of
    a timeout should be a timeout error and redirection to the error state.
  - u32Time_ is ms count for timeout
  - Dataflash_u32Timeout is the reference time

Promises:
  - if the timeout has occured, sets the erorr code and directs the SM to Error state
*/
static void DataflashCheckTimeout(u32 u32Time_)
{
  if(IsTimeUp(&G_u32SystemTime1ms, &Dataflash_u32Timeout, u32Time_, NO_RESET_TARGET_TIMER))
  {
    Dataflash_u8ErrorCode = DATAFLASH_ERROR_TIMEOUT;
    G_DataflashStateMachine = DataflashSM_Error;
  }

} /* end SdCheckTimeout() */



/***********************************************************************************************************************
State Machine Function Declarations


***********************************************************************************************************************/

/*----------------------------------------------------------------------------*/
/* Waits for Dataflash_eCommand to trigger an action */
static void DataflashSM_Idle(void)
{
  static u8 u8SspRequestFailed[] = "Dataflash denied SSP ";
  u8 u8CommandArray[MAX_COMMAND_ARRAY_SIZE];

  /* This will monitor command inputs to the dataflash */
  if(Dataflash_eCommand != DATAFLASH_COMMAND_IDLE)
  {
    /* Request the SSP resource */
    Dataflash_Ssp = SspRequest(&Dataflash_sSspConfig);

    /* Execute the state only if the resource is free */
    if(Dataflash_Ssp == NULL)
    {
      DebugPrintf(strlen((char const *)u8SspRequestFailed), &u8SspRequestFailed[0]);
      PrintNumber(G_u32SystemTime1ms);
      DebugLineFeed();
      
      Dataflash_u32Timeout = G_u32SystemTime1ms;
      G_DataflashStateMachine = DataflashSM_WaitSSP;
    }

    else
    {
      /* Ensure the Rx buffer pointers are set correctly (will be off if client didn't read data */
      Dataflash_pu8RxBufferUnreadByte = Dataflash_pu8RxBufferNextByte;

      /* Calculate the target address as it is common to all operations */

      /* To correctly parse out the address of the page to read, u16PageAddress_ and u16StartByte_ are translated
      into a 24-bit address in the command array.  The page address is 11 bits and lives at bits 9 through 20 of the 24
      bit command.  The bottom 9 bits are the byte address of the first byte to read within the page.
      The top 4 bits are don't cares.  */
      Dataflash_u16PageAddress <<= 1;
      Dataflash_u16PageAddress  &= 0xFFFE;
      Dataflash_u8AddressByteMd  = (u8)Dataflash_u16PageAddress;
      Dataflash_u8AddressByteHi  = (u8)(Dataflash_u16PageAddress >> 8);

      SspAssertCS(Dataflash_Ssp);

      /* Carry out the command */
      switch(Dataflash_eCommand)
      {
        case DATAFLASH_COMMAND_READ_BYTES:
        {
          /* Add the byte address */
          Dataflash_u8AddressByteLo  = (u8)Dataflash_u16StartByte;
          Dataflash_u8AddressByteMd |= (u8)((Dataflash_u16StartByte >> 8) & 0x000F);

          /* Prepare the read operation by writing the read command and address to the dataflash */
          u8CommandArray[INDEX_COMMAND]    = DATAFLASH_MAIN_MEMORY_PAGE_READ;
          u8CommandArray[INDEX_ADDRESS_HI] = Dataflash_u8AddressByteHi;
          u8CommandArray[INDEX_ADDRESS_MD] = Dataflash_u8AddressByteMd;
          u8CommandArray[INDEX_ADDRESS_LO] = Dataflash_u8AddressByteLo;

          /* Need to also send 4 dummy bytes */
          for(u8 i = 0; i < 4; i++)
          {
            u8CommandArray[INDEX_READ_DUMMY0 + i] = DATAFLASH_DUMMY_BYTE;
          }

          /* Queue the message and queue a read of the expected data */
          SspWriteData(Dataflash_Ssp, DATAFLASH_COMMAND_SIZE_WITH_DUMMY, u8CommandArray);
          AdvanceRxBufferUnreadBytePointer(DATAFLASH_COMMAND_SIZE_WITH_DUMMY);

          Dataflash_u32CurrentSspToken = SspReadData(Dataflash_Ssp, Dataflash_u16MessageBytes);

          /* Wait for read to finish */
          Dataflash_u32Timeout = G_u32SystemTime1ms;
          G_DataflashStateMachine = DataflashSM_ReadData;

          break;
        } /* end DATAFLASH_COMMAND_READ_BYTES */

        case DATAFLASH_COMMAND_WRITE_PAGE:
        {
          /* Make sure the 9-bit byte address is 0 */
          Dataflash_u8AddressByteLo  = 0;
          Dataflash_u8AddressByteMd &= 0xFE;

          /* Prepare the write operation by writing the write buffer command and buffer address to the dataflash.
          The destination address for the write to main flash memory will be written later. */
          u8CommandArray[INDEX_COMMAND]    = DATAFLASH_BUFFER1_WRITE;
          u8CommandArray[INDEX_ADDRESS_HI] = 0;
          u8CommandArray[INDEX_ADDRESS_MD] = 0;
          u8CommandArray[INDEX_ADDRESS_LO] = 0;

          /* Queue the message and queue a write of the expected data; advance receive pointer past dummy bytes that will arrive */
          SspWriteData(Dataflash_Ssp, DATAFLASH_COMMAND_SIZE_NO_DUMMY, u8CommandArray);
          Dataflash_u32CurrentSspToken = SspWriteData(Dataflash_Ssp, Dataflash_u16MessageBytes, Dataflash_au8TxBuffer);
          AdvanceRxBufferUnreadBytePointer(Dataflash_u16MessageBytes + DATAFLASH_COMMAND_SIZE_NO_DUMMY);

          /* Wait for write to finish */
          Dataflash_u32Timeout = G_u32SystemTime1ms;
          G_DataflashStateMachine = DataflashSM_WriteData;

          break;
        } /* end DATAFLASH_COMMAND_WRITE_PAGE */

        case DATAFLASH_COMMAND_ERASE_PAGE:
        {
          /* Make sure the 9-bit byte address is 0 */
          Dataflash_u8AddressByteLo  = 0;
          Dataflash_u8AddressByteMd &= 0xFE;

          /* Prepare the erase page operation by writing the ommand and address to the data flash.*/
          u8CommandArray[INDEX_COMMAND]    = DATAFLASH_PAGE_ERASE;
          u8CommandArray[INDEX_ADDRESS_HI] = Dataflash_u8AddressByteHi;
          u8CommandArray[INDEX_ADDRESS_MD] = Dataflash_u8AddressByteMd;
          u8CommandArray[INDEX_ADDRESS_LO] = Dataflash_u8AddressByteLo;

          /* Queue the message */
          Dataflash_u32CurrentSspToken = SspWriteData(Dataflash_Ssp, DATAFLASH_COMMAND_SIZE_NO_DUMMY, u8CommandArray);
          AdvanceRxBufferUnreadBytePointer(DATAFLASH_COMMAND_SIZE_NO_DUMMY);

          /* Wait for write to finish */
          Dataflash_u32Timeout = G_u32SystemTime1ms;
          G_DataflashStateMachine = DataflashSM_ErasePage;

          break;
        } /* end DATAFLASH_COMMAND_ERASE_PAGE */

      } /* end switch */
    }
  }

} /* end DataflashSM_Idle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a read sequence to complete*/
static void DataflashSM_ReadData(void)
{
  if(QueryMessageStatus(Dataflash_u32CurrentSspToken) == COMPLETE)
  {
    Dataflash_eTokenStatus = DATAFLASH_STATUS_DATA_READY;
    DataflashOperationComplete();
  }

  DataflashCheckTimeout(DATAFLASH_SSP_OPERATION_TIME_MS);

} /* end DataflashSM_ReadData() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for a write sequence to complete to the data flash buffer which should be simultaneous to the SSP message.
 Still need to send the command to transfer the buffer contents to the flash. */
static void DataflashSM_WriteData(void)
{
  u8 u8CommandArray[MAX_COMMAND_ARRAY_SIZE];

  if(QueryMessageStatus(Dataflash_u32CurrentSspToken) == COMPLETE)
  {
    /* Trigger the end of the write */
    SspDeAssertCS(Dataflash_Ssp);

    /* Set up to transfer the buffer to flash */
    u8CommandArray[INDEX_COMMAND]    = DATAFLASH_BUF1_TO_MAIN;
    u8CommandArray[INDEX_ADDRESS_HI] = Dataflash_u8AddressByteHi;
    u8CommandArray[INDEX_ADDRESS_MD] = Dataflash_u8AddressByteMd;
    u8CommandArray[INDEX_ADDRESS_LO] = Dataflash_u8AddressByteLo;

    /* Queue the message and queue a write of the data */
    SspAssertCS(Dataflash_Ssp);
    Dataflash_u32CurrentSspToken = SspWriteData(Dataflash_Ssp, DATAFLASH_COMMAND_SIZE_NO_DUMMY, u8CommandArray);
    AdvanceRxBufferUnreadBytePointer(DATAFLASH_COMMAND_SIZE_NO_DUMMY);

    /* Wait for write to finish */
    Dataflash_u32Timeout = G_u32SystemTime1ms;
    G_DataflashStateMachine = DataflashSM_WaitWriteCommand;
  }

  DataflashCheckTimeout(DATAFLASH_SSP_OPERATION_TIME_MS);


} /* end DataflashSM_WriteData() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for the write command/data to reach the data flash.  Need to start the write by deasserting CS. */
static void DataflashSM_WaitWriteCommand(void)
{
  if(QueryMessageStatus(Dataflash_u32CurrentSspToken) == COMPLETE)
  {
    /* Inititate the write sequence by deasserting CS */
    SspDeAssertCS(Dataflash_Ssp);

    /* Wait for write to finish */
    G_DataflashStateMachine = DataflashSM_WaitWriteCycle;
  }

  DataflashCheckTimeout(DATAFLASH_SSP_OPERATION_TIME_MS);

} /* end DataflashSM_WaitWriteCommand() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for the dataflash write cycle.  For now we're going to trust the datasheet max write cycle time and
just wait that amount x 2 and assume the write is done.  Would be better to check the status register here.
We're also trusting that the SPI command wrote successfully, which is a fairly safe assumption if we're
at this state. */
static void DataflashSM_WaitWriteCycle(void)
{
  static u32 u32WriteCycleCheckDelay = WRITE_CYCLE_DELAY;

  u32WriteCycleCheckDelay--;

  if(u32WriteCycleCheckDelay == 0)
  {
    u32WriteCycleCheckDelay = WRITE_CYCLE_DELAY;
    Dataflash_eTokenStatus = DATAFLASH_STATUS_WRITE_COMPLETE;
    DataflashOperationComplete();
  }

} /* end DataflashSM_WaitWriteCycle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for the  erase command to reach the dataflash.  */
static void DataflashSM_ErasePage(void)
{
  if(QueryMessageStatus(Dataflash_u32CurrentSspToken) == COMPLETE)
  {
    /* Inititate the erase sequence by deasserting CS */
    SspDeAssertCS(Dataflash_Ssp);

    /* Wait for erase to finish */
    G_DataflashStateMachine = DataflashSM_WaitEraseCycle;
  }

  DataflashCheckTimeout(DATAFLASH_SSP_OPERATION_TIME_MS);

} /* end DataflashSM_ErasePage() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for the dataflash erase cycle.  For now we're going to trust the datasheet max erase cycle time and
just wait that amount x 2 and assume the erase is done.  Would be better to check the status register here.
We're also trusting that the SPI command wrote successfully, which is a fairly safe assumption if we're
at this state. */

static void DataflashSM_WaitEraseCycle(void)
{
  static u32 u32EraseCycleCheckDelay = PAGE_ERASE_CYCLE_DELAY;

  u32EraseCycleCheckDelay--;

  if(u32EraseCycleCheckDelay == 0)
  {
    u32EraseCycleCheckDelay = PAGE_ERASE_CYCLE_DELAY;
    Dataflash_eTokenStatus = DATAFLASH_STATUS_ERASE_COMPLETE;
    DataflashOperationComplete();
  }

} /* end DataflashSM_WaitEraseCycle() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* This provides the delay period to allow the client that initiated an operation to check the status and do something.
When this times out, the dataflash resource will be returned to the system for the next access.  An operation to check the status
will 0 the Dataflash_u32TokenTimeout counter so this function will exit immediately next time through the cycle even
if the original time had not elapsed. */
static void DataflashSM_WaitCommand(void)
{
  if(IsTimeUp(&G_u32SystemTime1ms, &Dataflash_u32TokenTimeout, DATAFLASH_STATUS_VALID_TIME, NO_RESET_TARGET_TIMER))
  {
    Dataflash_eTokenStatus = DATAFLASH_STATUS_INVALID_TOKEN;
    Dataflash_u32Token++;
    Dataflash_eCommand = DATAFLASH_COMMAND_IDLE;
    G_DataflashStateMachine = DataflashSM_Idle;
  }

} /* end DataflashSM_WaitCommand() */


/*-------------------------------------------------------------------------------------------------------------------*/
/* Kill time before checking SSP availability again */
static void DataflashSM_WaitSSP(void)
{
  if(IsTimeUp(&G_u32SystemTime1ms, &Dataflash_u32Timeout, DATAFLASH_SSP_WAIT_TIME_MS, NO_RESET_TARGET_TIMER))
  {
    G_DataflashStateMachine = DataflashSM_Idle;
  }

} /* end DataflashSM_WaitSSP() */


/*----------------------------------------------------------------------------------------------------------------------
Error state
*/
static void DataflashSM_Error(void)
{
  u8 *pu8ErrorMessage;
  u8 u8MessageSize;

  /* Reset the system variables */
  SspDeAssertCS(Dataflash_Ssp);
  SspRelease(Dataflash_Ssp);

  DebugPrintf( sizeof(Dataflash_au8ErrorMsg) - 1, &Dataflash_au8ErrorMsg[0]);
  switch(Dataflash_u8ErrorCode)
  {
    case DATAFLASH_ERROR_TIMEOUT:
    {
      u8MessageSize = sizeof(Dataflash_au8ErrorTimeoutMsg) - 1;
      pu8ErrorMessage = Dataflash_au8ErrorTimeoutMsg;
      break;
    }

    case DATAFLASH_ERROR_ID:
    {
      u8MessageSize = sizeof(Dataflash_au8ErrorTimeoutMsg) - 1;
      pu8ErrorMessage = Dataflash_au8ErrorTimeoutMsg;
      break;
    }

    default:
    {
      u8MessageSize = sizeof(Dataflash_au8ErrorUnknown) - 1;
      pu8ErrorMessage = Dataflash_au8ErrorUnknown;
      break;
    }
  } /* end switch */

  DebugPrintf(u8MessageSize, pu8ErrorMessage);

  /* Return to Idle state */
  G_DataflashStateMachine = DataflashSM_Idle;

} /* end DataflashSM_Error() */





/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

