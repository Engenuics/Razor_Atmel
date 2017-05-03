/******************************************************************************
File: NHD-C0220BiZ_LCD.h                                                               

Description:
Header file for Newhaven Display NHD-C0220BiZ-FS(RGB)-FBW-3VM LCD.

DISCLAIMER: THIS CODE IS PROVIDED WITHOUT ANY WARRANTY OR GUARANTEES.  USERS MAY
USE THIS CODE FOR DEVELOPMENT AND EXAMPLE PURPOSES ONLY.  ENGENUICS TECHNOLOGIES
INCORPORATED IS NOT RESPONSIBLE FOR ANY ERRORS, OMISSIONS, OR DAMAGES THAT COULD
RESULT FROM USING THIS FIRMWARE IN WHOLE OR IN PART.

******************************************************************************/

#ifndef __LCD_C0220BIZ_H
#define __LCD_C0220BIZ_H


#define LCD_ADDRESS                       (u8)0x3C              /* Address of the On board chip of the LCD */
                                                                /* Careful!!!! - the documentation for the LCD says 0x78 however,
                                                                   that includes the Read/Write Bit */
/* Flag Bits */
#define _LCD_FLAG_NEW_LCD_MSG             (u32)0x00000001      /* LCD should update with new message */
#define _LCD_FLAGS_MESSAGE_IN_QUEUE       (u32)0x00000001      /* There is an LCD message in the MessageSenderQueue */
#define _LCD_FLAGS_SM_MANUAL              (u32)0x00000004      /* Run the LCD SM in manual mode */
#define _LCD_FLAGS_UPDATE_IN_PROGRESS     (u32)0x00000008      /* LCD update is currently in progress */

#define LCD_CONTROL_COMMAND               (u8)0x00             /* Control byte to LCD command is coming */
#define LCD_CONTROL_DATA                  (u8)0x40             /* Control byte to LCD command is coming */

#define LCD_STARTUP_DELAY                 (u8)40               /* Time in ms to wait for LCD startup */
#define LCD_CONTROL_COMMAND_DELAY         (u8)200              /* Time in ms to wait for LCD Command Instructions */
#define LCD_INIT_MSG_DISP_TIME            (u32)1000

#define LCD_MESSAGE_OVERHEAD_SIZE         (u8)1                /* Number of header bytes for an LCD message */
#define LCD_MAX_LINE_DISPLAY_SIZE         (u8)20               /* Maximum message length displayable on a single line */ 
#define LCD_MAX_MESSAGE_SIZE              (u8)40               /* Maximum message length on a single line of the 
                                                                  display assuming message starts at far left of screen
                                                                  Only 20 characters can be displayed and remaining characters 
                                                                  will be off the screen but still in LCD RAM */
/*------------------------------------------------------------------------------
Operational Notes:
RS and R/W lines are controlled to enable various states:

RS			R/W				Operation
0				 0				Instruction Write
0				 1				Read busy flag (DB7) and address counter (DB0 - DB6)
1				 0				Data Write
1				 1				Data Read

However, the I²C implementation of the LCD does not support reading data
back from the module.  Therefore, the R/W bit is always 0 which leaves
only RS to selection Instruction (Command) or Data mode.

For a two-line display, the character addresses are 0x00 - 0x27 (line 1)
and 0x40 - 0x67 (line 2), but only 20 chars can be displayed.  The extra
space can be used for scrolling displays.
*/


/* LCD Commands */
/* "CMD" requires RS = 0, R/W = 0 */
/* Data sheet initialization values */
#define   NUM_CONTROL_CMD     (u8)6
#define		LCD_FUNCTION_CMD		(u8)0x38		/* Literal to set 8-bit bus, 2-line, 5x8 chars, function table 00 */
#define		LCD_FUNCTION2_CMD		(u8)0x39		/* Literal to set 8-bit bus, 2-line, 5x8 chars, function table 01 */
#define   LCD_BIAS_CMD        (u8)0x14    /* Set 1/5 bias and fixed on low */
#define   LCD_CONTRAST_CMD    (u8)0x72    /* Set contrast (default 0x78) */
#define   LCD_DISPLAY_SET_CMD (u8)0x5E    /* Set power, icon control, contrast */
#define   LCD_FOLLOWER_CMD    (u8)0x6D    /* Set follower (not sure what this means!) */

#define		LCD_CLEAR_CMD				(u8)0x01		/* Writes spaces to all chars */
#define		LCD_HOME_CMD				(u8)0x02		/* Puts cursor at 0x00 */

#define		LCD_CURSOR_RT_CMD		(u8)0x06		/* Cursor moves right after char */
#define		LCD_CURSOR_LT_CMD		(u8)0x04		/* Cursor moves left after char */
#define   LCD_DISPLAY_RT_CMD  (u8)0x05    /* Entire display shifts right after each write */
#define   LCD_DISPLAY_LT_CMD  (u8)0x07    /* Entire display shifts left after each write */

#define		LCD_DISPLAY_CMD			(u8)0x08		/* Root literal for managing display */
#define		LCD_DISPLAY_ON			(u8)0x04		/* OR with LCD_DISPLAY_CMD to turn display on */
#define		LCD_DISPLAY_CURSOR	(u8)0x02		/* OR with LCD_DISPLAY_CMD to turn cursor on */
#define		LCD_DISPLAY_BLINK		(u8)0x01		/* OR with LCD_DISPLAY_CMD to turn cursor blink on */

#define		LCD_SHIFT_CMD				(u8)0x10		/* Root literal for display / cursor shift commands */
#define		LCD_SHIFT_DISPLAY		(u8)0x08		/* Set to operate on dislay, clear for cursor */
#define		LCD_SHIFT_RIGHT			(u8)0x04		/* Set to shift right, clear to shift left */

#define		LCD_ADDRESS_CMD			(u8)0x80		/* Root literal to set the cursor position */
																			    /* Bottom 6 bits are address (0x00-0x27 and 0x40-0x67) */
#define		LINE1_START_ADDR		(u8)0x00 		/* Constant for defining cursor location for LINE1 */
#define		LINE2_START_ADDR  	(u8)0x40 		/* Constant for defining cursor location for LINE2 */
#define   LINE1_END_ADDR      (u8)0x13    /* Constant for last displayable character address in LINE1 */
#define   LINE2_END_ADDR      (u8)0x53    /* Constant for last displayable character address in LINE2 */
#define   LINE1_END_ABSOLUTE  (u8)0x27    /* Constant for last character RAM address in LINE1 */
#define   LINE2_END_ABSOLUTE  (u8)0x67    /* Constant for last character RAM address in LINE2 */

/**********************************************************************************************************************
* Function Declarations
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void LCDCommand(u8 u8Command_);
void LCDClearChars(u8 u8Address_, u8 u8CharactersToClear_);
void LCDMessage(u8 u8Address_, u8 *u8Message_);


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/
void LcdInitialize(void);
void LcdRunActiveState(void);

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
void LcdSM_Idle(void);

  
#endif /* __LCD_C0220BIZ_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/



