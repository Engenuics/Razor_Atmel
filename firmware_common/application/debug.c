/***********************************************************************************************************************
File: debug.c                                                                

Description:
Debugging functions and state machine.  Since the system is small, debugger commands
will be strictly numerical, though each command will have a string name that can
be requested by the user.  The debugger will print a list of these commands if 
requested using en+c00.  Commands range from 01 to 99 (must include the leading 0
for single-digit commands) and all commands must have the prefix en+c. 
The current command list can be quickly checked in debug_x.h (where x is application-specific)

This application requires a UART resource for input/output data.

The terminal program used to interface to the debugger should be set to:
- no local echo
- send "CR" for new line
- 115200-8-N-1


DISCLAIMER: THIS CODE IS PROVIDED WITHOUT ANY WARRANTY OR GUARANTEES.  USERS MAY
USE THIS CODE FOR DEVELOPMENT AND EXAMPLE PURPOSES ONLY.  ENGENUICS TECHNOLOGIES
INCORPORATED IS NOT RESPONSIBLE FOR ANY ERRORS, OMISSIONS, OR DAMAGES THAT COULD
RESULT FROM USING THIS FIRMWARE IN WHOLE OR IN PART.

------------------------------------------------------------------------------------------------------------------------
API:
Types: none

Globals:
G_au8DebugScanfBuffer[] is the DebugScanf() input buffer that can be read directly
G_u8DebugScanfCharCount holds number of characters in Debug_au8ScanfBuffer
Both of these variables are cleared whenever DebugScanf() is called.

Constants:
DEBUG_SCANF_BUFFER_SIZE is the size of G_au8DebugScanfBuffer and thus the max of G_u8DebugScanfCharCount

Public:
u32 DebugPrintf(u8* u8String_)
Queues the string pointed to by u8String_ to the Debug port.  The string must be
null-terminated.  It may also contain control charactesr like newline (\n) and line feed (\f)
e.g.
u8 u8String[] = "A string to print.\n\r"
DebugPrintf(u8String);

void DebugLineFeed(void)
Queues a <CR><LF> sequence to the debug UART.
e.g.
DebugLineFeed();

void DebugPrintNumber(u32 u32Number_)
Formats a long into an ASCII string and queues to print.  Leading zeros are not printed.
e.g.
u32 u32Number = 1234567;
DebugPrintNumber(u32Number);

u8 DebugScanf(u8* au8Buffer_)
Copies the current input buffer to au8Buffer_ and returns the number of new characters.
Everytime DebugScanf is called, the 
e.g.
u8 u8MyBuffer[SCANF_BUFFER_SIZE]
u8 u8NumChars;
u8NumChars = DebugScanf(u8MyBuffer);


void DebugSetPassthrough(void)
Puts the Debug task in Passthrough mode so ALL characters received are put in to the Scanf buffer and
the Debug task does not look for input for the menu system. 


void DebugClearPassthrough(void)
Takes the Debug task out of Passthrough mode.


***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_Debug"
***********************************************************************************************************************/
/* New variables */
u32 G_u32DebugFlags;                                     /* Debug flag register */

u8 G_au8DebugScanfBuffer[DEBUG_SCANF_BUFFER_SIZE]; /* Space to latch characters for DebugScanf() */
u8 G_u8DebugScanfCharCount = 0;                    /* Counter for # of characters in Debug_au8ScanfBuffer */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                    /* From main.c */
extern volatile u32 G_u32ApplicationFlags;               /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                  /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                   /* From board-specific source file */

extern u8 G_au8MessageOK[];                              /* From utilities.c */
extern u8 G_au8MessageFAIL[];                            /* From utilities.c */
extern u8 G_au8MessageON[];                              /* From utilities.c */
extern u8 G_au8MessageOFF[];                             /* From utilities.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Debug_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Debug_pfnStateMachine;                /* The Debug state machine function pointer */

static UartPeripheralType* Debug_Uart;                   /* Pointer to debug UART peripheral object */
static u32 Debug_u32CurrentMessageToken;                 /* Token for current message */
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
#ifdef EIE1
DebugCommandType Debug_au8Commands[DEBUG_COMMANDS] = { {DEBUG_CMD_NAME00, DebugCommandPrepareList},
                                                       {DEBUG_CMD_NAME01, DebugCommandLedTestToggle},
                                                       {DEBUG_CMD_NAME02, DebugCommandSysTimeToggle},
                                                       {DEBUG_CMD_NAME03, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME04, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME05, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME06, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME07, DebugCommandDummy} 
                                                     };

static u8 Debug_au8StartupMsg[] = "\n\n\r*** RAZOR SAM3U2 ASCII LCD DEVELOPMENT BOARD ***\n\rDebug ready\n\r";
#endif /* EIE1 */

#ifdef MPGL2
DebugCommandType Debug_au8Commands[DEBUG_COMMANDS] = { {DEBUG_CMD_NAME00, DebugCommandPrepareList},
                                                       {DEBUG_CMD_NAME01, DebugCommandLedTestToggle},
                                                       {DEBUG_CMD_NAME02, DebugCommandSysTimeToggle},
                                                       {DEBUG_CMD_NAME03, DebugCommandCaptouchValuesToggle},
                                                       {DEBUG_CMD_NAME04, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME05, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME06, DebugCommandDummy},
                                                       {DEBUG_CMD_NAME07, DebugCommandDummy} 
                                                     };

static u8 Debug_au8StartupMsg[] = "\n\n\r*** RAZOR SAM3U2 DOT MATRIX DEVELOPMENT BOARD ***\n\rDebug ready\n\r";
#endif /* MPGL2 */


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
    Debug_pfnStateMachine = DebugSM_Error;
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
Function: DebugScanf

Description:
Copies G_u8DebugScanfCharCount characters from G_au8DebugScanfBuffer to a target array 
so the input can be saved.  Once copied, G_au8DebugScanfBuffer is cleared and
G_u8DebugScanfCharCount is zeroed.

Requires:
  - G_u8DebugScanfCharCount holds the number of characters in the G_au8DebugScanfBuffer
  - au8Buffer_ points to an array large enough to hold G_u8DebugScanfCharCount characters
  - Debug task is blocked here so new characters are not added

Promises:
  - G_u8DebugScanfCharCount characters copied to *au8Buffer_
  - G_au8DebugScanfBuffer[i] = '\0', where 0 <= i <= DEBUG_SCANF_BUFFER_SIZE
  - G_u8DebugScanfCharCount = 0
*/
u8 DebugScanf(u8* au8Buffer_)
{
  u8 u8Temp = G_u8DebugScanfCharCount;
  
  /* Copy the characters, clearing as we go */
  for(u8 i = 0; i < G_u8DebugScanfCharCount; i++)
  {
    *(au8Buffer_ + i) = G_au8DebugScanfBuffer[i];
    G_au8DebugScanfBuffer[i] = '\0';
  }
  
  G_u8DebugScanfCharCount = 0;
  return u8Temp;
  
} /* end DebugScanf() */



/*----------------------------------------------------------------------------------------------------------------------
Function: DebugSetPassthrough

Description:
Puts the Debug task in Passthrough mode so ALL characters received are put in to the Scanf buffer and
the Debug task does not look for input for the menu system. This allows task to have full access to 
terminal input without the Debug task printing messages or stealing Backspace characters!

Passthrough mode does NOT disable any other Debug functions that have already been enabled.  For example,
if you want the 1ms timing violation warning you can enable this and then enable Passthrough mode.

Requires:
  -

Promises:
  - G_u32DebugFlags _DEBUG_PASSTHROUGH is set
*/
void DebugSetPassthrough(void)
{
  G_u32DebugFlags |= _DEBUG_PASSTHROUGH;
  
  DebugPrintf("\n\n\r***Debug Passthrough enabled***\n\n\r");

} /* end DebugSetPassthrough */


/*----------------------------------------------------------------------------------------------------------------------
Function: DebugClearPassthrough

Description:
Takes the Debug task out of Passthrough mode.

Requires:
  -

Promises:
  - G_u32DebugFlags _DEBUG_PASSTHROUGH is cleared
*/
void DebugClearPassthrough(void)
{
  G_u32DebugFlags &= ~_DEBUG_PASSTHROUGH;
  
  DebugPrintf("\n\n\r***Debug Passthrough disabled***\n\n\r");
  
} /* end DebugClearPassthrough */


/*----------------------------------------------------------------------------------------------------------------------
Function: SystemStatusReport

Description:
Reports if system is good or not.

Requires:
  - G_u32SystemFlags up to date with system status
  - New tasks should be added to the check list below including in the message string for the task name
  - The system is in initialization state so MsgSenderForceSend() is used
    to output each meassage after it is queued.

Promises:
  - Prints out messages for any system tests that failed
  - Prints out overall good message if all tests passed
*/
void SystemStatusReport(void)
{
  u8 au8SystemPassed[] = "No failed tasks.";
  u8 au8SystemReady[] = "\n\rInitialization complete. Type en+c00 for debug menu.  Failed tasks:\n\r";
  u32 u32TaskFlagMaskBit = (u32)0x01;
  bool bNoFailedTasks = TRUE;

#ifdef EIE1
  u8 aau8AppShortNames[NUMBER_APPLICATIONS][MAX_TASK_NAME_SIZE] = {"LED", "BUTTON", "DEBUG", "LCD", "ANT", "TIMER", "ADC", "SD"};
#endif /* EIE1 */

#ifdef MPGL2
  u8 aau8AppShortNames[NUMBER_APPLICATIONS][MAX_TASK_NAME_SIZE] = {"LED", "BUTTON", "DEBUG", "LCD", "ANT", "TIMER", "ADC", "CAPTOUCH"};
#endif /* MPGL2 */

  /* Announce init complete then report any tasks that failed init */
  DebugPrintf(au8SystemReady);
    
  for(u8 i = 0; i < NUMBER_APPLICATIONS; i++)
  {
    if( !(u32TaskFlagMaskBit & G_u32ApplicationFlags) )
    {
      bNoFailedTasks = FALSE;
      DebugPrintf(&aau8AppShortNames[i][0]);
      DebugLineFeed();
    }
    
    u32TaskFlagMaskBit <<= 1;
  }     
        
  if( bNoFailedTasks)
  {
    DebugPrintf(au8SystemPassed);
  }
  
  DebugLineFeed();
  
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
  - Debug_pfnStateMachine set to Idle
*/
void DebugInitialize(void)
{
  UartConfigurationType sUartConfig;  

  /* Clear the receive buffer */
  for (u16 i = 0; i < DEBUG_RX_BUFFER_SIZE; i++)
  {
    Debug_au8RxBuffer[i] = 0;
  }

  /* Clear the scanf buffer and counter */
  G_u8DebugScanfCharCount = 0;
  for (u8 i = 0; i < DEBUG_SCANF_BUFFER_SIZE; i++)
  {
    G_au8DebugScanfBuffer[i] = 0;
  }

  /* Initailze startup values and the command array */
  Debug_pu8RxBufferParser    = &Debug_au8RxBuffer[0];
  Debug_pu8RxBufferNextChar  = &Debug_au8RxBuffer[0]; 
  Debug_pu8CmdBufferNextChar = &Debug_au8CommandBuffer[0]; 

  /* Request the UART resource to be used for the Debug application */
  sUartConfig.UartPeripheral     = DEBUG_UART;
  sUartConfig.pu8RxBufferAddress = &Debug_au8RxBuffer[0];
  sUartConfig.pu8RxNextByte      = &Debug_pu8RxBufferNextChar;
  sUartConfig.u16RxBufferSize    = DEBUG_RX_BUFFER_SIZE;
  sUartConfig.fnRxCallback       = DebugRxCallback;
  
  Debug_Uart = UartRequest(&sUartConfig);
  
  /* Go to error state if the UartRequest failed */
  if(Debug_Uart == NULL)
  {
    Debug_pfnStateMachine = DebugSM_Error;

  }
  /* Otherwise send the first message, set "good" flag and head to Idle */
  else
  {
    DebugPrintf(Debug_au8StartupMsg);   
    G_u32ApplicationFlags |= _APPLICATION_FLAGS_DEBUG;
    Debug_pfnStateMachine = DebugSM_Idle;
  }
  
} /* end  DebugInitialize() */


/*----------------------------------------------------------------------------------------------------------------------
Function DebugRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void DebugRunActiveState(void)
{
  Debug_pfnStateMachine();

} /* end DebugRunActiveState */


/*----------------------------------------------------------------------------------------------------------------------
Function DebugRxCallback()

Description:
Call back function used when character received.

Requires:
  - None

Promises:
  - Safely advances Debug_pu8RxBufferNextChar.
*/
void DebugRxCallback(void)
{
  /* Safely advance the NextChar pointer */
  Debug_pu8RxBufferNextChar++;
  if(Debug_pu8RxBufferNextChar == &Debug_au8RxBuffer[DEBUG_RX_BUFFER_SIZE])
  {
    Debug_pu8RxBufferNextChar = &Debug_au8RxBuffer[0];
  }
  
} /* end DebugRxCallback() */


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
  au8CommandLine[DEBUG_CMD_PREFIX_LENGTH + DEBUG_CMD_NAME_LENGTH + 2] = '\0';

  /* Prepare a nicely formatted list of commands */
  DebugPrintf(au8ListHeading);
  
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
    DebugPrintf(au8CommandLine);
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
  
  DebugPrintf(au8DummyCommand);
  
} /* end DebugCommandDummy() */


/*----------------------------------------------------------------------------------------------------------------------
Function: DebugCommandLedTestToggle

Description:
Toggles the active state of the LED test which allows typed characters corresponding to LED colors
to toggle those LEDs on or off.  LEDs are started all ON.  They are left in their current state when
the function exits.
*/
static void DebugCommandLedTestToggle(void)
{
  u8 au8LedTestMessage[] = "\n\rLed Test ";
  
  /* Print message and toggle the flag */
  DebugPrintf(au8LedTestMessage);
  if(G_u32DebugFlags & _DEBUG_LED_TEST_ENABLE)
  {
    G_u32DebugFlags &= ~_DEBUG_LED_TEST_ENABLE;
    DebugPrintf(G_au8MessageOFF);
  }
  else
  {
    G_u32DebugFlags |= _DEBUG_LED_TEST_ENABLE;
    DebugPrintf(G_au8MessageON);
    
#ifdef EIE1
    LedOn(WHITE);
    LedOn(PURPLE);
    LedOn(BLUE);
    LedOn(CYAN);
    LedOn(GREEN);
    LedOn(YELLOW);
    LedOn(ORANGE);
    LedOn(RED);
#endif /* EIE1 */   
    
#ifdef MPGL2
#ifdef MPGL2_R01
    LedOn(BLUE);
    LedOn(GREEN);
    LedOn(YELLOW);
    LedOn(RED);
#else
    LedOn(BLUE0);
    LedOn(BLUE1);
    LedOn(BLUE2);
    LedOn(BLUE3);
    LedOn(RED0);
    LedOn(RED1);
    LedOn(RED2);
    LedOn(RED3);
    LedOn(GREEN0);
    LedOn(GREEN1);
    LedOn(GREEN2);
    LedOn(GREEN3);
#endif /* MPGL2_R01 */
#endif /* MPGL2 */
  }
  
} /* end DebugCommandLedTestToggle() */


/*----------------------------------------------------------------------------------------------------------------------
Function: DebugLedTestCharacter

Description:
Checks the character and toggles associated LED if applicable.
This implementation is specific to the target hardware.

Requires:
  - u8Char_ is the character to check

Promises:
  - If u8Char_ is a valid toggling character, the corresponding LED will be toggled.
*/
static void DebugLedTestCharacter(u8 u8Char_)
{
  /* Check the char to see if an LED should be toggled */  
#ifdef EIE1
  if(u8Char_ == 'W')
  {
    LedToggle(WHITE);
  }  

  if(u8Char_ == 'P')
  {
    LedToggle(PURPLE);
  } 

  if(u8Char_ == 'B')
  {
    LedToggle(BLUE);
  } 

  if(u8Char_ == 'C')
  {
    LedToggle(CYAN);
  } 

  if(u8Char_ == 'G')
  {
    LedToggle(GREEN);
  } 

  if(u8Char_ == 'Y')
  {
    LedToggle(YELLOW);
  } 

  if(u8Char_ == 'O')
  {
    LedToggle(ORANGE);
  } 

  if(u8Char_ == 'R')
  {
    LedToggle(RED);
  } 

#endif /* EIE1 */
  
#ifdef MPGL2
  
#ifdef MPGL2_R01
  if(u8Char_ == 'B')
  {
    LedToggle(BLUE);
  } 

  if(u8Char_ == 'G')
  {
    LedToggle(GREEN);
  } 

  if(u8Char_ == 'Y')
  {
    LedToggle(YELLOW);
  } 

  if(u8Char_ == 'R')
  {
    LedToggle(RED);
  } 
  
#else

  if(u8Char_ == 'B')
  {
    LedToggle(BLUE0);
    LedToggle(BLUE1);
    LedToggle(BLUE2);
    LedToggle(BLUE3);
  }  

  if(u8Char_ == 'R')
  {
    LedToggle(RED0);
    LedToggle(RED1);
    LedToggle(RED2);
    LedToggle(RED3);
  }  
  
  if(u8Char_ == 'G')
  {
    LedToggle(GREEN0);
    LedToggle(GREEN1);
    LedToggle(GREEN2);
    LedToggle(GREEN3);
  }  
 
#endif /* MPGL2_R01 */
#endif /* MPGL2 */
  
} /* end DebugCommandLedTestToggle() */


/*----------------------------------------------------------------------------------------------------------------------
Function: DebugCommandSysTimeToggle

Description:
Toggles the active state of the LED test which allows typed characters corresponding to LED colors
to toggle those LEDs on or off.
*/
static void DebugCommandSysTimeToggle(void)
{
  u8 au8SysTimeTestMessage[] = "\n\rSystem time violation reporting ";
  
  /* Print message and toggle the flag */
  DebugPrintf(au8SysTimeTestMessage);
  if(G_u32DebugFlags & _DEBUG_TIME_WARNING_ENABLE)
  {
    G_u32DebugFlags &= ~_DEBUG_TIME_WARNING_ENABLE;
    DebugPrintf(G_au8MessageOFF);
  }
  else
  {
    G_u32DebugFlags |= _DEBUG_TIME_WARNING_ENABLE;
    DebugPrintf(G_au8MessageON);
  }
  
} /* end DebugCommandSysTimeToggle() */

#ifdef MPGL2 /* MPGL2 only tests */
/*----------------------------------------------------------------------------------------------------------------------
Function: DebugCommandCaptouchValuesToggle

Description:
Toggles printing the current Captouch horizontal and vertical values.
*/
static void DebugCommandCaptouchValuesToggle(void)
{
  u8 au8CaptouchDisplayMessage[] = "\n\rDisplay Captouch values ";
  u8 au8CaptouchOnMessage[] = "No values displayed if Captouch is OFF\n\r";
  
  /* Print message and toggle the flag */
  DebugPrintf(au8CaptouchDisplayMessage);
  if(G_u32DebugFlags & _DEBUG_CAPTOUCH_VALUES_ENABLE)
  {
    G_u32DebugFlags &= ~_DEBUG_CAPTOUCH_VALUES_ENABLE;
    DebugPrintf(G_au8MessageOFF);
  }
  else
  {
    G_u32DebugFlags |= _DEBUG_CAPTOUCH_VALUES_ENABLE;
    DebugPrintf(G_au8MessageON);
    DebugPrintf(au8CaptouchOnMessage);
  }
  
} /* end DebugCommandCaptouchValuesToggle() */
#endif /* MPGL2 only tests */


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
  while( (Debug_pu8RxBufferParser != Debug_pu8RxBufferNextChar) && (bCommandFound == FALSE) )
  {
    /* Grab a copy of the current byte and echo it back */
    u8CurrentByte = *Debug_pu8RxBufferParser;
        
    /* Process the character */
    switch (u8CurrentByte)
    {
      /* Backspace: update command buffer pointer and send sequence to delete the char on the terminal */
      case(ASCII_BACKSPACE): 
      {
        /* Process for scanf as long as we are not in Passthrough mode */
        if( G_u32DebugFlags & _DEBUG_PASSTHROUGH )
        {        
          if(G_u8DebugScanfCharCount < DEBUG_SCANF_BUFFER_SIZE)
          {
            G_au8DebugScanfBuffer[G_u8DebugScanfCharCount] = u8CurrentByte;
            G_u8DebugScanfCharCount++;
          }
        }
        else
        {
          if(G_u8DebugScanfCharCount != 0)
          {
            G_u8DebugScanfCharCount--;
            G_au8DebugScanfBuffer[G_u8DebugScanfCharCount] = '\0';
          }

          /* Process for command */
          if(Debug_pu8CmdBufferNextChar != &Debug_au8CommandBuffer[0])
          {
            Debug_pu8CmdBufferNextChar--;
            Debug_u16CommandSize--;
          }
        }
                
        /* Send the Backspace sequence to clear the character on the terminal */
        DebugPrintf(au8BackspaceSequence);
        break;
      }

      /* Carriage return: change states to process new command and fall through to echo character */
      case(ASCII_CARRIAGE_RETURN): 
      {
        if( !( G_u32DebugFlags & _DEBUG_PASSTHROUGH) )
        {
          bCommandFound = TRUE;
          Debug_pfnStateMachine = DebugSM_CheckCmd;
        }
        
        /* Fall through to default */        
      }
        
      /* Add to command buffer and echo */
      default: 
      {
        /* Process for scanf */
        if(G_u8DebugScanfCharCount < DEBUG_SCANF_BUFFER_SIZE)
        {
          G_au8DebugScanfBuffer[G_u8DebugScanfCharCount] = u8CurrentByte;
          G_u8DebugScanfCharCount++;
        }
        
        /* Echo the character back to the terminal */
        UartWriteByte(Debug_Uart, u8CurrentByte);
        
        /* As long as Passthrough mode is not active, then update the command buffer */
        if( !( G_u32DebugFlags & _DEBUG_PASSTHROUGH) )
        {
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

            Debug_u32CurrentMessageToken = DebugPrintf(au8CommandOverflow);
          }
        }
        break;
      }

    } /* end switch (u8RxChar) */

    /* If the LED test is active, toggle LEDs based on characters */
    if(G_u32DebugFlags & _DEBUG_LED_TEST_ENABLE)
    {
      DebugLedTestCharacter(u8CurrentByte);
    }
    
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
    Debug_pfnStateMachine = DebugSM_ProcessCmd;
  }
  /* Otherwise print an error message and return to Idle */
  else
  { 
    DebugPrintf(au8InvalidCommand);
    Debug_pfnStateMachine = DebugSM_Idle;
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
  Debug_pfnStateMachine = DebugSM_Idle;

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
  G_u32DebugFlags |= _DEBUG_FLAG_ERROR;
  DebugPrintf(au8DebugErrorMsg);
  DebugPrintNumber( (u32)(Debug_u8ErrorCode) );
  DebugLineFeed();
  
  /* Return to Idle state */
  Debug_u16CommandSize = 0;
  Debug_pu8CmdBufferNextChar = &Debug_au8CommandBuffer[0];
  Debug_pfnStateMachine = DebugSM_Idle;

} /* end DebugSM_Error() */
             

          
             
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/

