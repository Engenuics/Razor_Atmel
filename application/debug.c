/***********************************************************************************************************************
File: debug.c                                                                

Description:
Debugging functions and state machine.  Since the system is small, debugger commands
will be strictly numerical, though each command will have a string name that can
be requested by the user.  The debugger will print a list of these commands if 
requested using mpg+c00.  Commands range from 01 to 99 (must include the leading 0
for single-digit commands) and all commands must have the prefix mpg+c. 
The current command list can be quickly checked in debug_x.h (where x is application-specific)

This application requires a UART resource for input/output data.

The terminal program used to interface to the debugger should be set to:
- no local echo
- send "CR" for new line
- 38400-8-N-1

***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
fnCode_type G_DebugStateMachine;                       /* The Debug state machine */

u32 G_u32DebugFlags;                                   /* Debug flag register */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                    /* From main.c */
extern volatile u32 G_u32ApplicationFlags;               /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                  /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                   /* From board-specific source file */

extern const u8 G_au8MessageOK[];                        /* From utilities.c */
extern const u8 G_au8MessageFAIL[];                      /* From utilities.c */
extern const u8 G_au8MessageON[];                        /* From utilities.c */
extern const u8 G_au8MessageOFF[];                       /* From utilities.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Debug_" and be declared as static.
***********************************************************************************************************************/
static UartPeripheralType* Debug_Uart;                   /* Pointer to debug UART peripheral object */
static u32 Debug_u32CurrentMessageToken;                 /* Token for current message */
//static u32 Debug_u32Timer;                               /* Cross-state timer for Debug task */
static u8 Debug_u8ErrorCode;                             /* Error code */

static u8 Debug_au8RxBuffer[DEBUG_RX_BUFFER_SIZE];       /* Space for incoming characters of debug commands */
static u8 *Debug_pu8RxBufferNextChar;                    /* Pointer to next spot in the Rxbuffer */
static u8 *Debug_pu8RxBufferParser;                      /* Pointer to loop through the Rx buffer */

static u8 Debug_au8CommandBuffer[DEBUG_CMD_BUFFER_SIZE]; /* Space to store chars as they build up to the next command */ 
static u8 *Debug_pu8CmdBufferNextChar;                   /* Pointer to incoming char location in the command buffer */
static u16 Debug_u16CommandSize;                         /* Number of characters in the command buffer */

static u8 Debug_u8Command;                               /* A validated command number */

/* Add commands by updating debug.h in the Command-Specific Definitions section, then update this list
with the function name to call for the corresponding command: */
DebugCommandType Debug_au8Commands[DEBUG_COMMANDS] = { {DEBUG_CMD_NAME00, DebugCommandPrepareList},
                                                       {DEBUG_CMD_NAME01, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME02, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME03, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME04, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME05, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME06, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME07, DebugCommandDummy} 
                                                     };


/***********************************************************************************************************************
* Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: DebugPrintf

Description:
Sends a text string to the debug UART.

Requires:
  - u8String_ is a NULL-terminated C-string
  - The debug UART resource has been setup for the debug application.

Promises:
  - The string is queued to the debug UART.
  - The message token is returned
*/
u32 DebugPrintf(u8* u8String_)
{
  u8* pu8Parser = u8String_;
  u32 u32Size = 0;
  
  while(*pu8Parser != NULL)
  {
    u32Size++;
    pu8Parser++;
  }
  return( UartWriteData(Debug_Uart, u32Size, u8String_) );
 
} /* end DebugPrintf() */


/*----------------------------------------------------------------------------------------------------------------------
Function: DebugLineFeed

Description:
Queues a <CR><LF> sequence to the debug UART.

Requires:
  -

Promises:
  - <CR><LF> sequence to the debug UART
*/
void DebugLineFeed(void)
{
  u8 au8Linefeed[] = {ASCII_LINEFEED, ASCII_CARRIAGE_RETURN};
  
  UartWriteData(Debug_Uart, sizeof(au8Linefeed), &au8Linefeed[0]);

} /* end DebugLineFeed() */


/*-----------------------------------------------------------------------------/
Function: DebugPrintNumber

Description:
Formats a long into an ASCII string and queues to print

Requires:
  - Enough space is available on the heap to temporarily store the number array

Promises:
  - The number is converted to an array of ascii without leading zeros and sent to UART
*/
void DebugPrintNumber(u32 u32Number_)
{
  bool bFoundDigit = FALSE;
  u8 au8AsciiNumber[10];
  u8 u8CharCount = 0;
  u32 u32Temp, u32Divider = 1000000000;
  u8 *pu8Data;

  /* Parse out all the digits, start counting after leading zeros */
  for(u8 index = 0; index < 10; index++)
  {
    au8AsciiNumber[index] = (u32Number_ / u32Divider) + 0x30;
    if(au8AsciiNumber[index] != '0')
    {
      bFoundDigit = TRUE;
    }
    if(bFoundDigit)
    {
      u8CharCount++;
    }
    u32Number_ %= u32Divider;
    u32Divider /= 10;
  }
  
  /* Handle special case where u32Number == 0 */
  if(!bFoundDigit)
  {
    u8CharCount = 1;
  }
  
  /* Allocate memory for the right number and copy the array */
  pu8Data = malloc(u8CharCount);
  if (pu8Data == NULL)
  {
    Debug_u8ErrorCode = DEBUG_ERROR_MALLOC;
    G_DebugStateMachine = DebugSM_Error;
  }
  
  u32Temp = 9;
  for(u8 index = u8CharCount; index != 0; index--)
  {
    pu8Data[index - 1] = au8AsciiNumber[u32Temp--];
  }
    
  /* Print the ascii string and free the memory */
  UartWriteData(Debug_Uart, u8CharCount, pu8Data);
  free(pu8Data);
  
} /* end DebugDebugPrintNumber() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SystemStatusReport

Description:
Reports if system is good or not.

Requires:
  - GGu32SystemFlags up to date with system status
  - The system is in initialization state so MsgSenderForceSend() is used
    to output each meassage after it is queued.

Promises:
  - Prints out messages for any system tests that failed
  - Prints out overall good message if all tests passed
*/
void SystemStatusReport(void)
{
#if 0
  u8 au8SystemPassed[] = "Self tests passed:\n\r";
  u8 au8SystemFailed[] = "Self tests failed:\n\r";
  u8 au8SystemReady[] = "\n\rSystem ready\n\rType 'en+c00' for debug commands\n\r*************************************\n\n\r";


  if( (GGu32SystemFlags & SYSTEM_FLAGS_TESTS_MASK) == SYSTEM_FLAGS_TESTS_MASK )
  {
    QueueTxMessage(DEBUG_MSG_SRC, sizeof(au8SystemPassed), &au8SystemPassed[0]);
    MsgSenderForceSend();
  }
  /* This is brute force for now: */
  else
  {
    QueueTxMessage(DEBUG_MSG_SRC, sizeof(au8SystemFailed), &au8SystemFailed[0]);
    MsgSenderForceSend();
    
    if( !(GGu32SystemFlags & _SYSTEM_FLAGS_LCD_GOOD) )
    {
      QueueTxMessage(DEBUG_MSG_SRC, sizeof(au8LCDFailed), &au8LCDFailed[0]);
      MsgSenderForceSend();
    }
    
    if( !(GGu32SystemFlags & _SYSTEM_FLAGS_ANT_GOOD) )
    {
      QueueTxMessage(DEBUG_MSG_SRC, sizeof(au8AntFailed), &au8AntFailed[0]);
      MsgSenderForceSend();
    }
  }

  QueueTxMessage(DEBUG_MSG_SRC, sizeof(au8SystemReady), &au8SystemReady[0]);
  MsgSenderForceSend();
#endif
  
} /* end SystemStatusReport() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: DebugInitialize

Description:
Sets up the debug command list and activates the debug functionality.

Requires:
  - The debug application is not yet running
  - The UART resource requested should be free

Promises:
  - UART resource Debug_au8RxBuffer initialized to all 0
  - Buffer pointers Debug_pu8CmdBufferCurrentChar and Debug_pu8RxBufferParser set to the start of the buffer
  - G_DebugStateMachine set to Idle
*/
void DebugInitialize(void)
{
  UartConfigurationType sUartConfig;  
  u8 au8DebugStarted[] = "Debug task initialized\n\r";

  /* Clear the receive buffer */
  for (u16 i = 0; i < DEBUG_RX_BUFFER_SIZE; i++)
  {
    Debug_au8RxBuffer[i] = 0;
  }

  /* Initailze startup values and the command array */
  Debug_pu8RxBufferParser    = &Debug_au8RxBuffer[0];
  Debug_pu8RxBufferNextChar  = &Debug_au8RxBuffer[0]; 
  Debug_pu8CmdBufferNextChar = &Debug_au8CommandBuffer[0]; 

  /* Request the UART resource to be used for the Debug application */
  sUartConfig.UartPeripheral     = DEBUG_UART;
  sUartConfig.pu8RxBufferAddress = &Debug_au8RxBuffer[0];
  sUartConfig.pu8RxNextByte      = &Debug_pu8RxBufferNextChar;
  sUartConfig.u32RxBufferSize    = DEBUG_RX_BUFFER_SIZE;
  
  Debug_Uart = UartRequest(&sUartConfig);
  
  /* Go to error state if the UartRequest failed */
  if(Debug_Uart == NULL)
  {
    G_DebugStateMachine = DebugSM_Error;

  }
  /* Otherwise send the first message, set "good" flag and head to Idle */
  else
  {
    DebugLineFeed();
    DebugLineFeed();
    UartWriteData(Debug_Uart, sizeof(au8DebugStarted) - 1, &au8DebugStarted[0]);
    
    G_u32ApplicationFlags |= _APPLICATION_FLAGS_DEBUG;
    G_DebugStateMachine = DebugSM_Idle;
  }
  
} /* end  DebugInitialize() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function DebugCommandPrepareList

Description:
Queues the entire list of debug commands available in the system so they will
be sent out the debug UART for the user to view.

Requires:
  - Message Sender application is running

Promises:
  - Command numbers and names of all installed commands are queued to messagesender.
*/
static void DebugCommandPrepareList(void)
{
  u8 au8ListHeading[] = "\n\n\rAvailable commands:\n\r";
  u8 au8CommandLine[DEBUG_CMD_PREFIX_LENGTH + DEBUG_CMD_NAME_LENGTH + DEBUG_CMD_POSTFIX_LENGTH];
  
  /* Write static characters to command list line */
  au8CommandLine[2] = ':';
  au8CommandLine[3] = ' ';
  au8CommandLine[DEBUG_CMD_PREFIX_LENGTH + DEBUG_CMD_NAME_LENGTH] = '\n';
  au8CommandLine[DEBUG_CMD_PREFIX_LENGTH + DEBUG_CMD_NAME_LENGTH + 1] = '\r';

  /* Prepare a nicely formatted list of commands */
  UartWriteData(Debug_Uart, sizeof(au8ListHeading) - 1, &au8ListHeading[0]);
  
  /* Loop through the array of commands parsing out the command number
  and printing it along with the command name. */  
  for(u8 i = 0; i < DEBUG_COMMANDS; i++)
  {
    /* Get the command number in ASCII */
    if(i >= 10)
    {
      au8CommandLine[0] = (i / 10) + 0x30;
    }
    else
    {
      au8CommandLine[0] = 0x30;
    }
    
    au8CommandLine[1] = (i % 10) + 0x30;
    
    /* Read the command name */
    for(u8 j = 0; j < DEBUG_CMD_NAME_LENGTH; j++)
    {
      au8CommandLine[DEBUG_CMD_PREFIX_LENGTH + j] = Debug_au8Commands[i].pu8CommandName[j];
    }
    
    /* Queue the command name to the UART */
    UartWriteData(Debug_Uart, sizeof(au8CommandLine), &au8CommandLine[0]);
  }

  DebugLineFeed();
  
} /* end DebugCommand0PrepareList() */



/*----------------------------------------------------------------------------------------------------------------------
Function: DebugCommandDummy

Description:
A command place-holder.
*/
static void DebugCommandDummy(void)
{
  u8 au8DummyCommand[] = "\n\rDummy!\n\n\r";
  
  UartWriteData(Debug_Uart, sizeof(au8DummyCommand) - 1, &au8DummyCommand[0]);
  
} /* end DebugCommandDummy() */


/***********************************************************************************************************************
State Machine Function Declarations

The debugger state machine monitors the receive buffer to grab characters as they come in
from the interrupt-driven receiver and store them in the debug command buffer.  Nothing happens 
until the user sends a CR indicating they think they've entered a valid command.  The command is
checked and reacted to accordingly.
***********************************************************************************************************************/

/*----------------------------------------------------------------------------------------------------------------------
Waits for a byte to appear in the Rx buffer.  The BufferParser is always moved
through all new characters placing them into the command buffer until it hits a CR or there are no new
characters to read. If there is no CR in this iteration, nothing else occurs.

Backspace: Echo the backspace and a space character to clear the character on screen; move Debug_pu8BufferCurrentChar back.
CR: Advance states to process the command.
Any other character: Echo it to the UART Tx and place a copy in Debug_au8CommandBuffer.
*/
void DebugSM_Idle(void)               
{
  bool bCommandFound = FALSE;
  u8 u8CurrentByte;
  static u8 au8BackspaceSequence[] = {ASCII_BACKSPACE, ' ', ASCII_BACKSPACE};
  static u8 au8CommandOverflow[] = "\r\n*** Command too long ***\r\n\n";
  
  /* Parse any new characters that have come in until no more chars or a command is found */
  while( (Debug_pu8RxBufferParser != *Debug_Uart->pu8RxNextByte) && (bCommandFound == FALSE) )
  {
    /* Grab a copy of the current byte and echo it back */
    u8CurrentByte = *Debug_pu8RxBufferParser;
    
    /* Process the character */
    switch (u8CurrentByte)
    {
      /* Backspace: update command buffer pointer and send sequence to delete the char on the terminal */
      case(ASCII_BACKSPACE): 
      {
        if(Debug_pu8CmdBufferNextChar != &Debug_au8CommandBuffer[0])
        {
          Debug_pu8CmdBufferNextChar--;
          Debug_u16CommandSize--;
        }
        
        UartWriteData(Debug_Uart, sizeof(au8BackspaceSequence), &au8BackspaceSequence[0]);
        break;
      }

      /* Carriage return: change states to process new command and fall through to echo character */
      case(ASCII_CARRIAGE_RETURN): 
      {
        bCommandFound = TRUE;
        
        G_DebugStateMachine = DebugSM_CheckCmd;
        
        /* Fall through to default */        
      }
        
      /* Add to command buffer and echo */
      default: 
      {
        /* Echo the character and place it in the command buffer */
        UartWriteByte(Debug_Uart, u8CurrentByte);
        *Debug_pu8CmdBufferNextChar = u8CurrentByte;
        Debug_pu8CmdBufferNextChar++;
        Debug_u16CommandSize++;

        /* If the command buffer is now full but the last character was not ASCII_CARRIAGE_RETURN, throw out the whole
        buffer and report an error message */
        if( (Debug_pu8CmdBufferNextChar >= &Debug_au8CommandBuffer[DEBUG_CMD_BUFFER_SIZE]) &&
            (u8CurrentByte != ASCII_CARRIAGE_RETURN) )
        {
          Debug_pu8CmdBufferNextChar = &Debug_au8CommandBuffer[0];
          Debug_u16CommandSize = 0;

          Debug_u32CurrentMessageToken = UartWriteData(Debug_Uart, sizeof(au8CommandOverflow), au8CommandOverflow);
        }
        break;
      }

    } /* end switch (u8RxChar) */
      
    /* In all cases, advance the RxBufferParser pointer safely */
    Debug_pu8RxBufferParser++;
    if(Debug_pu8RxBufferParser >= &Debug_au8RxBuffer[DEBUG_RX_BUFFER_SIZE])
    {
      Debug_pu8RxBufferParser = &Debug_au8RxBuffer[0];
    }
    
  } /* end while */
  
  /* Clear out any completed messages */
  if(Debug_u32CurrentMessageToken != 0)
  {
    QueryMessageStatus(Debug_u32CurrentMessageToken);
  }
    
} /* end DebugSM_Idle() */


/*----------------------------------------------------------------------------------------------------------------------
At the start of this state, the command buffer has a candidate command terminated in CR.
There is a strict rule that commands are of the form
en+cxx where xx is any number from 0 to DEBUG_COMMANDS, so parsing can be done based
on that rule.  All other strings are invalid.  Debug interrupts remain off
until the command is processed.
*/
void DebugSM_CheckCmd(void)        
{
  static u8 au8CommandHeader[] = "en+c";
  static u8 au8InvalidCommand[] = "\nInvalid command\n\n\r"; 
  bool bGoodCommand = TRUE;
  u8 u8Index;
  s8 s8Temp;
  
  /* Verify that the command starts with en+c */
  u8Index = 0;
  do
  {
    if(Debug_au8CommandBuffer[u8Index] != au8CommandHeader[u8Index])
    {
      bGoodCommand = FALSE;
    }

    u8Index++;
  } while ( bGoodCommand && (u8Index < 4) );
  
  /* On good header, read the command number */
  if(bGoodCommand)
  {
    /* Make an assumption */
    bGoodCommand = FALSE;

    /* Verify the next char is a digit */
    s8Temp = Debug_au8CommandBuffer[u8Index++] - 0x30;
  
    if( (s8Temp >= 0) && (s8Temp <= 9) )
    {
      Debug_u8Command = s8Temp * 10;
  
      /* Verify the next char is a digit */
      s8Temp = Debug_au8CommandBuffer[u8Index++] - 0x30;
      if( (s8Temp >= 0) && (s8Temp <= 9) )
      {
        Debug_u8Command += s8Temp;
        
        /* Check that the command number is within the range of commands available and the last char is CR */
        if( (Debug_u8Command < DEBUG_COMMANDS) && (Debug_au8CommandBuffer[u8Index] == ASCII_CARRIAGE_RETURN) )
        {
          bGoodCommand = TRUE;
        }
      }
    }
  }
           
  /* If still good command */
  if( bGoodCommand )
  {
    G_DebugStateMachine = DebugSM_ProcessCmd;
  }
  /* Otherwise print an error message and return to Idle */
  else
  { 
    UartWriteData(Debug_Uart, sizeof(au8InvalidCommand) - 1, &au8InvalidCommand[0]);
    G_DebugStateMachine = DebugSM_Idle;
  }

  /* Reset the command buffer */
  Debug_pu8CmdBufferNextChar = &Debug_au8CommandBuffer[0];

} /* end DebugSM_CheckCmd() */


/*----------------------------------------------------------------------------------------------------------------------
Carry out the debug instruction. 
*/
void DebugSM_ProcessCmd(void)         
{
  /* Setup for return to Idle state */
  G_DebugStateMachine = DebugSM_Idle;

  /* Call the command function in the function array (may change next state ) */
  Debug_au8Commands[Debug_u8Command].DebugFunction();
  
} /* end DebugSM_ProcessCmd() */


/*----------------------------------------------------------------------------------------------------------------------
Error state 
Attempt to print an error message (even though if the Debug UART has failed, then it obviously cannot print
a message to tell you that!)
*/
void DebugSM_Error(void)         
{
  static u8 au8DebugErrorMsg[] = "\n\nDebug task error: ";
  
  /* Flag an error and report it (if possible) */
  G_u32DebugFlags |= DEBUG_FLAG_ERROR;
  UartWriteData(Debug_Uart, sizeof(au8DebugErrorMsg) - 1, au8DebugErrorMsg);
  DebugPrintNumber( (u32)(Debug_u8ErrorCode) );
  DebugLineFeed();
  
  /* Return to Idle state */
  Debug_u16CommandSize = 0;
  Debug_pu8CmdBufferNextChar = &Debug_au8CommandBuffer[0];
  G_DebugStateMachine = DebugSM_Idle;

} /* end DebugSM_Error() */
             

             
             
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

