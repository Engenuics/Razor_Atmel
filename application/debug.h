/***********************************************************************************************************************
File: debug.h                                                                
***********************************************************************************************************************/

#ifndef __DEBUG_H
#define __DEBUG_H


/***********************************************************************************************************************
* Constants / Definitions
***********************************************************************************************************************/
#define DEBUG_RX_BUFFER_SIZE     (u32)128             /* Size of debug buffer for incoming messages */
#define DEBUG_CMD_BUFFER_SIZE    (u32)64              /* Size of debug buffer for a command */

/* G_u32DebugFlags */
#define DEBUG_FLAG_NEW_COMMAND   (u32)0x00000001      /* A command has been entered by the user */
#define DEBUG_FLAG_ERROR         (u32)0x00000002      /* The debug Error state was reached */


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
#define DEBUG_CMD_POSTFIX_LENGTH  (u8)2              /* Size of command list postfix "<CR><LF>" */

/* New commands must update the definitions below. Valid commands are in the range
00 - 99.  Command name string is a maximum of DEBUG_CMD_NAME_LENGTH characters. */

#define DEBUG_COMMANDS          8   /* Total number of debug commands */
/*                              "0123456789ABCDEF0123456789ABCDEF"  Character position reference */
#define DEBUG_CMD_NAME00        "Show debug command list         "  /* Command 0: List all commands */
#define DEBUG_CMD_NAME01        "Toggle LED test mode            "  /* Command 1: Toggle LED test mode on/off */
#define DEBUG_CMD_NAME02        "Dummy2                          "  /* Command 2: */
#define DEBUG_CMD_NAME03        "Dummy3                          "  /* Command 3: */
#define DEBUG_CMD_NAME04        "Dummy4                          "  /* Command 4: */
#define DEBUG_CMD_NAME05        "Dummy5                          "  /* Command 5: */
#define DEBUG_CMD_NAME06        "Dummy6                          "  /* Command 6: */
#define DEBUG_CMD_NAME07        "Dummy7                          "  /* Command 7: */


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

void SystemStatusReport(void);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void DebugInitialize(void);                   


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/
static void DebugCommandPrepareList(void);           
static void DebugCommandDummy(void);


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
