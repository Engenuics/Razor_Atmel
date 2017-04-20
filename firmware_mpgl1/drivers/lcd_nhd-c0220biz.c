/***********************************************************************************************************************
File: lcd_nhd-c0220biz.c.c                                                                

Description:
Driver for Newhaven Display NHD-C0220BiZ ASCII LCD.

This application requires an I²C resource to output data.

The displayable area of the screen is 20 characters x 2 lines, though the LCD RAM will accomodate
40 characters per line (so can be used for scrolling text applications).
Each character has a 1-byte address. Nmemonics are defined for the main locations

Line #      Left most address             Last printed char           Right most address
  1       0x00 (LINE1_START_ADDR)       0x13 (LINE1_END_ADDR)       0x27 (LINE1_END_ABSOLUTE)      
  2       0x40 (LINE2_START_ADDR)       0x53 (LINE2_END_ADDR)       0x67 (LINE2_END_ABSOLUTE)      

------------------------------------------------------------------------------------------------------------------------
API
void LcdInitialize(void)
Initializes the LCD task and manually sends a message to the LCD.
This function must be run during the startup section of main.
e.g.
LcdInitialize();

void LCDCommand(u8 u8Command_)
Queues a command code to be sent to the LCD.  See the full command list in the header file.
Some common commands are shown below.
LCD_CLEAR_CMD				Writes spaces to all chars
LCD_HOME_CMD				Puts cursor at 0x00

LCD_DISPLAY_CMD			Root literal for managing display
LCD_DISPLAY_ON				OR with LCD_DISPLAY_CMD to turn display on
LCD_DISPLAY_CURSOR		OR with LCD_DISPLAY_CMD to turn cursor on
LCD_DISPLAY_BLINK			OR with LCD_DISPLAY_CMD to turn cursor blink on

e.g. Turn display on with a solid (non-blinking) cursor
LCDCommand(LCD_DISPLAY_CMD | LCD_DISPLAY_ON | LCD_DISPLAY_CURSOR);

void LCDMessage(u8 u8Address_, u8 *u8Message_)
Sends a text message to the LCD to be printed at the address specified.  
e.g. 
u8 au8Message[] = "Hello world!";
LCDMessage(LINE1_START_ADDR, au8Message);

void LCDClearChars(u8 u8Address_, u8 u8CharactersToClear_)
Clears a number of chars starting from the address specified.  This function does not span rows.
e.g. Clear "world!" from the screen after the above example.
LCDClearChars(LINE1_START_ADDR + 5, 6);

***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>Lcd"
***********************************************************************************************************************/
/* New variables */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Lcd_" and be declared as static.
***********************************************************************************************************************/
static fnCode_type Lcd_StateMachine;

static u32 Lcd_u32Timer;


/***********************************************************************************************************************
* Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function LCDCommand

Description:
Queues a command char to be sent to the LCD using the TWI messaging function. 

Requires:
	- u8Command_ is an acceptable command value for the LCD as taken from the
    "LCD Commands" list in lcd_nhd_c0220biz.h header file.  The command must
    be complete and include any optional bits.

Promises:
  - The command is queued and will be sent to the LCD at the next
    available time.
*/
void LCDCommand(u8 u8Command_)
{
  static u8 au8LCDWriteCommand[] = {LCD_CONTROL_COMMAND, 0x00};

  /* Update the command paramter into the command array */
  au8LCDWriteCommand[1] = u8Command_;
    
  /* Queue the command to the I²C application */
  TWI0WriteData(LCD_ADDRESS, sizeof(au8LCDWriteCommand), &au8LCDWriteCommand[0], STOP);

  /* Add a delay during initialization to let the command send properly */
  if(G_u32SystemFlags & _SYSTEM_INITIALIZING )
  {
    for(u32 i = 0; i < 100000; i++);
  }
  
} /* end LCDCommand() */

/*------------------------------------------------------------------------------
Function: LCDMessage

Description:
Sends a text message to the LCD to be printed at the address specified.  

Requires:
  - LCD is initialized
  - u8Message_ is a pointer to a NULL-terminated C-string
	- The message to display is no more than (40 - the selected display location) 
    characters in length
  - Any characters not desired on screen that will not be overwritten need to 
    be erased first

Promises:
  - Message to set cursor address in the LCD is queued, then message data 
    is queued to the LCD to be displayed. 
*/
void LCDMessage(u8 u8Address_, u8 *u8Message_)
{ 
  u8 u8Index; 
  static u8 au8LCDMessage[LCD_MESSAGE_OVERHEAD_SIZE + LCD_MAX_MESSAGE_SIZE] = 
                           {LCD_CONTROL_DATA};
  
  /* Set the cursor to the correct address */
  LCDCommand(LCD_ADDRESS_CMD | u8Address_);
  
  /* Fill the message */
  u8Index = 1;
  while(*u8Message_ != '\0')
  {
    au8LCDMessage[u8Index++] = *u8Message_++;
  }
    
  /* Queue the message */
  TWI0WriteData(LCD_ADDRESS, u8Index, au8LCDMessage, STOP);

} /* end LCDMessage() */


/*------------------------------------------------------------------------------
Function: LCDClearChars

Description:
Clears a number of chars starting from the address specified.  This function is
not meant to span rows.

Requires:
  - LCD is initialized
  - u8Address_ is the starting address where the first character will be cleared
	- u8CharactersToClear_ is the number of characters to clear and does not cause 
    the cursor to go past the available data RAM.

Promises:
  - Message to set cursor address in the LCD is queued, then message data 
    consisting of all ' ' characters is queued to the LCD to be displayed. 
*/
void LCDClearChars(u8 u8Address_, u8 u8CharactersToClear_)
{ 
  u8 u8Index; 
  static u8 au8LCDMessage[LCD_MESSAGE_OVERHEAD_SIZE + LCD_MAX_MESSAGE_SIZE] =  {LCD_CONTROL_DATA};
  
  /* Set the cursor to the correct address */
  LCDCommand(LCD_ADDRESS_CMD | u8Address_);
  
  /* Fill the message characters with ' ' */
  for(u8Index = 0; u8Index < u8CharactersToClear_; u8Index++)
  {
    au8LCDMessage[u8Index + 1] = ' ';
  }
      
  /* Queue the message */
  TWI0WriteData(LCD_ADDRESS, u8CharactersToClear_ + 1, au8LCDMessage, STOP);
      	
} /* end LCDClearChars() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------
Function: LcdInitialize

Description:
Initializes the LCD task and manually sends a message to the LCD

Requires:
  - 

Promises:
  - LCD task Setup and LCD functions can now be called
*/
void LcdInitialize(void)
{
  u8 au8Commands[] = 
  {
    LCD_FUNCTION_CMD, LCD_FUNCTION2_CMD, LCD_BIAS_CMD, 
    LCD_CONTRAST_CMD, LCD_DISPLAY_SET_CMD, LCD_FOLLOWER_CMD 
  };
                 /* "012345567890123456789" */
  u8 au8Welcome[] = "RAZOR SAM3U2 ASCII   ";
  
  /* State to Idle */
  Lcd_StateMachine = LcdSM_Idle;
  
  /* Turn on LCD wait 40 ms for it to setup */
  AT91C_BASE_PIOB->PIO_SODR = PB_09_LCD_RST;
  Lcd_u32Timer = G_u32SystemTime1ms;
  while( !IsTimeUp(&Lcd_u32Timer, LCD_STARTUP_DELAY) );
  
  /* Send Control Command */
  TWI0WriteByte(LCD_ADDRESS, LCD_CONTROL_COMMAND, NO_STOP);
  
  /* Send Control Commands */
  TWI0WriteData(LCD_ADDRESS, NUM_CONTROL_CMD, &au8Commands[0], NO_STOP);
  
  /* Wait for 200 ms */
  Lcd_u32Timer = G_u32SystemTime1ms;
  while( !IsTimeUp(&Lcd_u32Timer, LCD_CONTROL_COMMAND_DELAY) );
  
  /* Send Final Command to turn it on */
  TWI0WriteByte(LCD_ADDRESS, LCD_DISPLAY_CMD | LCD_DISPLAY_ON, STOP);

  /* Blacklight - White */
  LedOn(LCD_RED);
  LedOn(LCD_GREEN);
  LedOn(LCD_BLUE);
  
  TWI0WriteByte(LCD_ADDRESS, LCD_CONTROL_DATA, NO_STOP);
  TWI0WriteData(LCD_ADDRESS, 20, &au8Welcome[0], STOP);
   
  Lcd_u32Timer = G_u32SystemTime1ms;
  G_u32ApplicationFlags |= _APPLICATION_FLAGS_LCD;

} /* end LcdInitialize */


/*----------------------------------------------------------------------------------------------------------------------
Function LcdRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void LcdRunActiveState(void)
{
  Lcd_StateMachine();

} /* end LcdRunActiveState */


/***********************************************************************************************************************
State Machine Function Declarations
***********************************************************************************************************************/

/*------------------------------------------------------------------------------
Function: LcdSM_Idle

Description:
Placeholder for some fancier functionality to come later.

Requires:
  - LCD is initialized

Promises:
  - 
*/
void LcdSM_Idle(void)
{
  
}