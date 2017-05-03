/***********************************************************************************************************************
File: debug.h                                                                
***********************************************************************************************************************/

#ifndef __DEBUG_H
#define __DEBUG_H


/***********************************************************************************************************************
* Constants / Definitions
***********************************************************************************************************************/
#define DEBUG_RX_BUFFER_SIZE           (u32)128             /* Size of debug buffer for incoming messages */
#define DEBUG_CMD_BUFFER_SIZE          (u32)64              /* Size of debug buffer for a command */
#define DEBUG_SCANF_BUFFER_SIZE        (u8)128              /* Size of buffer for scanf messages */

/* G_u32DebugFlags */
#define _DEBUG_LED_TEST_ENABLE         (u32)0x00000001      /* Flag if LED test is enabled */
#define _DEBUG_TIME_WARNING_ENABLE     (u32)0x00000002      /* Flag if system time check is enabled */
#define _DEBUG_PASSTHROUGH             (u32)0x00000004      /* Set if Passthrough mode is enabled */

/* end of G_u32DebugFlags */

#ifdef EIE1 /* EIE1-specific G_u32DebugFlags flags */
#endif /* EIE1 */

#ifdef MPGL2 /* MPGL2-specific G_u32DebugFlags flags */
#define _DEBUG_CAPTOUCH_VALUES_ENABLE  (u32)0x00010000      /* Flag if debug should print Captouch values */
#endif /* MPGL2 */

#define _DEBUG_FLAG_ERROR              (u32)0x80000000      /* The debug Error state was reached */
/* end of G_u32DebugFlags */

#define MAX_TASK_NAME_SIZE             (u8)10               /* Maximum string size for task name reported in SystemStatusReport */

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/
typedef struct
{
  u8 *pu8CommandName;
  fnCode_type DebugFunction;
} DebugCommandType;


/***********************************************************************************************************************
* Command-Specific Definitions
***********************************************************************************************************************/
#define DEBUG_CMD_PREFIX_LENGTH   (u8)4              /* Size of command list prefix "00: " */
#define DEBUG_CMD_NAME_LENGTH     (u8)32             /* Max size for command name */
#define DEBUG_CMD_POSTFIX_LENGTH  (u8)3              /* Size of command list postfix "<CR><LF>\0" */

/* New commands must update the definitions below. Valid commands are in the range
00 - 99.  Command name string is a maximum of DEBUG_CMD_NAME_LENGTH characters. */

#ifdef EIE1
#define DEBUG_COMMANDS          8   /* Total number of debug commands */
/*                              "0123456789ABCDEF0123456789ABCDEF"  Character position reference */
#define DEBUG_CMD_NAME00        "Show debug command list         "  /* Command 0: List all commands */
#define DEBUG_CMD_NAME01        "Toggle LED test                 "  /* Command 1: Test that allows characters to toggle LEDs */
#define DEBUG_CMD_NAME02        "Toggle system timing warning    "  /* Command 2: Prints message if system tick has advanced more than 1 between main loop sleeps (i.e. tasks are taking too long) */
#define DEBUG_CMD_NAME03        "Dummy3                          "  /* Command 3: */
#define DEBUG_CMD_NAME04        "Dummy4                          "  /* Command 4: */
#define DEBUG_CMD_NAME05        "Dummy5                          "  /* Command 5: */
#define DEBUG_CMD_NAME06        "Dummy6                          "  /* Command 6: */
#define DEBUG_CMD_NAME07        "Dummy7                          "  /* Command 7: */
#endif /* EIE1 */

#ifdef MPGL2
#define DEBUG_COMMANDS          8   /* Total number of debug commands */
/*                              "0123456789ABCDEF0123456789ABCDEF"  Character position reference */
#define DEBUG_CMD_NAME00        "Show debug command list         "  /* Command 0: List all commands */
#define DEBUG_CMD_NAME01        "Toggle LED test                 "  /* Command 1: Test that allows characters to toggle LEDs */
#define DEBUG_CMD_NAME02        "Toggle system timing warning    "  /* Command 2: Prints message if system tick has advanced more than 1 between main loop sleeps (i.e. tasks are taking too long) */
#define DEBUG_CMD_NAME03        "Toggle Captouch value display   "  /* Command 2: Test that shows Captouch sense values on debug port */
#define DEBUG_CMD_NAME04        "Dummy4                          "  /* Command 4: */
#define DEBUG_CMD_NAME05        "Dummy5                          "  /* Command 5: */
#define DEBUG_CMD_NAME06        "Dummy6                          "  /* Command 6: */
#define DEBUG_CMD_NAME07        "Dummy7                          "  /* Command 7: */
#endif /* EIE1 */


#define DEBUG_UART_TIMEOUT      (u32)2000                           /* Max time in ms for a command/message to be sent */

/* Error codes */
#define DEBUG_ERROR_NONE        (u8)0                               /* No error */
#define DEBUG_ERROR_TIMEOUT     (u8)1                               /* Timeout error occured */
#define DEBUG_ERROR_MALLOC      (u8)2                               /* Dynamic memory allocation error occured */

/***********************************************************************************************************************
* Function Declarations
***********************************************************************************************************************/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/
u32 DebugPrintf(u8* u8String_);
void DebugLineFeed(void);       
void DebugPrintNumber(u32 u32Number_);

u8 DebugScanf(u8* au8Buffer_);

void DebugSetPassthrough(void);
void DebugClearPassthrough(void);

void SystemStatusReport(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void DebugInitialize(void);                   
void DebugRunActiveState(void);
void DebugRxCallback(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/
static void DebugCommandPrepareList(void);           
static void DebugCommandDummy(void);

static void DebugCommandLedTestToggle(void);
static void DebugLedTestCharacter(u8 u8Char_);
static void DebugCommandSysTimeToggle(void);

#ifdef EIE1 /* EIE1-specific debug functions */
#endif /* EIE1 */

#ifdef MPGL2 /* MPGL2-specific debug functions  */
static void DebugCommandCaptouchValuesToggle(void);
#endif /* MPGL2 */

/***********************************************************************************************************************
* State Machine Declarations
***********************************************************************************************************************/
static void DebugSM_Idle(void);                       
static void DebugSM_CheckCmd(void);                   
static void DebugSM_ProcessCmd(void);                 

static void DebugSM_Error(void);



#endif /* __DEBUG_H */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
