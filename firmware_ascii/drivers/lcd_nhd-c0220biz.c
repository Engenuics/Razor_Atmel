/*!**********************************************************************************************************************
@file lcd_nhd-c0220biz.c                                                                
@brief Driver for Newhaven Display NHD-C0220BiZ ASCII LCD.

This application requires an IIC (TWI) resource to output data.

The displayable area of the screen is 20 characters x 2 lines, though the LCD RAM will accomodate
40 characters per line (so can be used for scrolling text applications).
Each character has a 1-byte address. Mnemonics are defined for the main locations

Line #      Left most address             Last printed char           Right most address
  1       0x00 (LINE1_START_ADDR)       0x13 (LINE1_END_ADDR)       0x27 (LINE1_END_ABSOLUTE)      
  2       0x40 (LINE2_START_ADDR)       0x53 (LINE2_END_ADDR)       0x67 (LINE2_END_ABSOLUTE)      

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- See "LCD Commands" in lcd_nhd-c0220biz.h

TYPES
- NONE

PUBLIC FUNCTIONS
- void LcdCommand(u8 u8Command_)
- void LcdMessage(u8 u8Address_, u8 *u8Message_)
- void LcdClearChars(u8 u8Address_, u8 u8CharactersToClear_)

PROTECTED FUNCTIONS
- void LcdInitialize(void)


***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>Lcd"
***********************************************************************************************************************/
/* New variables */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;          /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;           /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;            /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;       /*!< @brief From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Lcd_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Lcd_pfnStateMachine;         /*!< @brief The application state machine */

static u32 Lcd_u32Timer;                        /*!< @brief Timeout counter used across states */
//static u32 Lcd_u32Flags;                        /*!< @brief Application flags */


/***********************************************************************************************************************
* Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!---------------------------------------------------------------------------------------------------------------------
@fn void LcdCommand(u8 u8Command_)

@brief Queues a command char to be sent to the LCD using the TWI messaging function. 

Some common commands are shown below.
LCD_CLEAR_CMD				Writes spaces to all chars
LCD_HOME_CMD				Puts cursor at 0x00

LCD_DISPLAY_CMD		  	Root literal for managing display
| LCD_DISPLAY_ON				OR with LCD_DISPLAY_CMD to turn display on
| LCD_DISPLAY_CURSOR		OR with LCD_DISPLAY_CMD to turn cursor on
| LCD_DISPLAY_BLINK		OR with LCD_DISPLAY_CMD to turn cursor blink on

e.g. Turn display on with a solid (non-blinking) cursor

LcdCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR);

Requires:
@param u8Command_ is an acceptable command value for the LCD as taken from the
"LCD Commands" list in lcd_nhd_c0220biz.h header file.  The command must
be complete and include any optional bits.

Promises:
- The command is queued and will be sent to the LCD at the next
  available time.

*/
void LcdCommand(u8 u8Command_)
{
  static u8 au8LCDWriteCommand[] = {LCD_CONTROL_COMMAND, 0x00};

  /* Update the command paramter into the command array */
  au8LCDWriteCommand[1] = u8Command_;
    
  /* Queue the command to the I²C application */
  TwiWriteData(U8_LCD_ADDRESS, sizeof(au8LCDWriteCommand), &au8LCDWriteCommand[0], TWI_STOP);

  /* Add a delay during initialization to let the command send properly */
  if(G_u32SystemFlags & _SYSTEM_INITIALIZING )
  {
    for(u32 i = 0; i < 100000; i++);
  }
  
} /* end LcdCommand() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void LcdMessage(u8 u8Address_, u8 *u8Message_)

@brief Sends a text message to the LCD to be printed at the address specified.  

The message to display is no more than (40 - the selected display location) 
characters in length.  Any characters not desired on screen that will not be 
overwritten need to be erased first.

e.g. 
u8 au8Message[] = "Hello world!";
LcdMessage(LINE1_START_ADDR, au8Message);

Requires:
- LCD is initialized

@param u8Message_ is a pointer to a NULL-terminated C-string

Promises:
- Message to set cursor address in the LCD is queued, then message data 
  is queued to the LCD to be displayed. 

*/
void LcdMessage(u8 u8Address_, u8 *u8Message_)
{ 
  u8 u8Index; 
  static u8 au8LCDMessage[U8_LCD_MESSAGE_OVERHEAD_SIZE + U8_LCD_MAX_MESSAGE_SIZE] = {LCD_CONTROL_DATA};
  
  /* Set the cursor to the correct address */
  LcdCommand(LCD_ADDRESS_CMD | u8Address_);
  
  /* Fill the message */
  u8Index = 1;
  while(*u8Message_ != '\0')
  {
    au8LCDMessage[u8Index++] = *u8Message_++;
  }
    
  /* Queue the message */
  TwiWriteData(U8_LCD_ADDRESS, u8Index, au8LCDMessage, TWI_STOP);

} /* end LcdMessage() */


/*!---------------------------------------------------------------------------------------------------------------------
@fn void LcdClearChars(u8 u8Address_, u8 u8CharactersToClear_)

@brief Clears a number of chars starting from the address specified.  This function is
not meant to span rows.

Requires:
- LCD is initialized

@param u8Address_ is the starting address where the first character will be cleared
@param u8CharactersToClear_ is the number of characters to clear and does not cause 
the cursor to go past the available data RAM.

Promises:
- Message to set cursor address in the LCD is queued, then message data 
  consisting of all ' ' characters is queued to the LCD to be displayed. 

*/
void LcdClearChars(u8 u8Address_, u8 u8CharactersToClear_)
{ 
  u8 u8Index; 
  static u8 au8LCDMessage[U8_LCD_MESSAGE_OVERHEAD_SIZE + U8_LCD_MAX_MESSAGE_SIZE] =  {LCD_CONTROL_DATA};
  
  /* Set the cursor to the correct address */
  LcdCommand(LCD_ADDRESS_CMD | u8Address_);
  
  /* Fill the message characters with ' ' */
  for(u8Index = 0; u8Index < u8CharactersToClear_; u8Index++)
  {
    au8LCDMessage[u8Index + 1] = ' ';
  }
      
  /* Queue the message */
  TwiWriteData(U8_LCD_ADDRESS, u8CharactersToClear_ + 1, au8LCDMessage, TWI_STOP);
      	
} /* end LcdClearChars() */


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void LcdInitialize(void)

@brief Initializes the LCD task and manually sends a message to the LCD

Requires:
- NONE

Promises:
- LCD task Setup and LCD functions can now be called

*/
void LcdInitialize(void)
{
  u8 u8Byte;
  u8 au8Commands[] = 
  {
    LCD_FUNCTION_CMD, LCD_FUNCTION2_CMD, LCD_BIAS_CMD, 
    LCD_CONTRAST_CMD, LCD_DISPLAY_SET_CMD, LCD_FOLLOWER_CMD 
  };
                 /* "012345567890123456789" */
  u8 au8Welcome[] = "RAZOR SAM3U2 ASCII   ";
  
  /* State to Idle */
  Lcd_pfnStateMachine = LcdSM_Idle;
  
  /* Turn on LCD wait 40 ms for it to setup */
  AT91C_BASE_PIOB->PIO_SODR = PB_09_LCD_RST;
  Lcd_u32Timer = G_u32SystemTime1ms;
  while( !IsTimeUp(&Lcd_u32Timer, U8_LCD_STARTUP_DELAY_MS) );
  
  /* Send Control Command */
  u8Byte = LCD_CONTROL_COMMAND;
  TwiWriteData(U8_LCD_ADDRESS, 1, &u8Byte, TWI_NO_STOP);
  
  /* Send Control Commands */
  TwiWriteData(U8_LCD_ADDRESS, sizeof(au8Commands), &au8Commands[0], TWI_NO_STOP);
  
  /* Wait for 200 ms */
  Lcd_u32Timer = G_u32SystemTime1ms;
  while( !IsTimeUp(&Lcd_u32Timer, U8_LCD_CONTROL_COMMAND_DELAY_MS) );
  
  /* Send Final Command to turn it on */
  u8Byte = (LCD_DISPLAY_CMD | LCD_DISPLAY_ON);
  TwiWriteData(U8_LCD_ADDRESS, 1, &u8Byte, TWI_STOP);

  /* Blacklight - White */
  LedOn(LCD_RED);
  LedOn(LCD_GREEN);
  LedOn(LCD_BLUE);
  
  u8Byte = LCD_CONTROL_DATA;
  TwiWriteData(U8_LCD_ADDRESS, 1,  &u8Byte, TWI_NO_STOP);
  TwiWriteData(U8_LCD_ADDRESS, 20, &au8Welcome[0], TWI_STOP);
   
  Lcd_u32Timer = G_u32SystemTime1ms;
  G_u32ApplicationFlags |= _APPLICATION_FLAGS_LCD;

} /* end LcdInitialize */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void LcdRunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void LcdRunActiveState(void)
{
  Lcd_pfnStateMachine();

} /* end LcdRunActiveState */


/***********************************************************************************************************************
State Machine Function Declarations
***********************************************************************************************************************/

/*!-------------------------------------------------------------------------------------------------------------------
@fn LcdSM_Idle

@brief Placeholder for some fancier functionality to come later.

*/
static void LcdSM_Idle(void)
{
  
} /* end LcdSM_Idle() */