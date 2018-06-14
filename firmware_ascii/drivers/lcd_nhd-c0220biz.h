/*!**********************************************************************************************************************
@file lcd_nhd-c0220biz.h                                                               

@brief Header file for lcd_nhd-c0220biz.c
******************************************************************************/

#ifndef __LCD_C0220BIZ_H
#define __LCD_C0220BIZ_H

#include "configuration.h"

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/* Lcd_u32Flags */
#define _LCD_FLAGS_SM_MANUAL              (u32)0x00000001      /*!< @brief Run the LCD SM in manual mode */
/* end Lcd_u32Flags */

#define U8_LCD_ADDRESS                    (u8)0x3C             /*!< @brief Address of the On board chip of the LCD */
                                                               /* The documentation for the LCD says 0x78 however,
                                                                  that includes the Read/Write Bit */

#define LCD_CONTROL_COMMAND               (u8)0x00             /*!< @brief Control byte to LCD command is coming */
#define LCD_CONTROL_DATA                  (u8)0x40             /*!< @brief Control byte to LCD command is coming */

#define U8_LCD_MAX_LINE_DISPLAY_SIZE      (u8)20               /*!< @brief Maximum message length displayable on a single line */ 
#define U8_LCD_MAX_MESSAGE_SIZE           (u8)40               /*!< @brief Maximum message length on a single line of the 
                                                                    display assuming message starts at far left of screen
                                                                    Only 20 characters can be displayed and remaining characters 
                                                                    will be off the screen but still in LCD RAM */


/* LCD Commands
"CMD" requires RS = 0, R/W = 0 */
#define		LCD_CLEAR_CMD				(u8)0x01		/*!< @brief Writes spaces to all chars */
#define		LCD_HOME_CMD				(u8)0x02		/*!< @brief Puts cursor at 0x00 */

#define		LCD_CURSOR_RT_CMD		(u8)0x06		/*!< @brief Cursor moves right after char */
#define		LCD_CURSOR_LT_CMD		(u8)0x04		/*!< @brief Cursor moves left after char */
#define   LCD_DISPLAY_RT_CMD  (u8)0x05    /*!< @brief Entire display shifts right after each write */
#define   LCD_DISPLAY_LT_CMD  (u8)0x07    /*!< @brief Entire display shifts left after each write */

#define		LCD_DISPLAY_CMD			(u8)0x08		/*!< @brief Root literal for managing display */
#define		LCD_DISPLAY_ON			(u8)0x04		/*!< @brief OR with LCD_DISPLAY_CMD to turn display on */
#define		LCD_DISPLAY_CURSOR	(u8)0x02		/*!< @brief OR with LCD_DISPLAY_CMD to turn cursor on */
#define		LCD_DISPLAY_BLINK		(u8)0x01		/*!< @brief OR with LCD_DISPLAY_CMD to turn cursor blink on */

#define		LCD_SHIFT_CMD				(u8)0x10		/*!< @brief Root literal for display / cursor shift commands */
#define		LCD_SHIFT_DISPLAY		(u8)0x08		/*!< @brief Set to operate on dislay, clear for cursor */
#define		LCD_SHIFT_RIGHT			(u8)0x04		/*!< @brief Set to shift right, clear to shift left */

#define		LCD_ADDRESS_CMD			(u8)0x80		/*!< @brief Root literal to set the cursor position
																			         Bottom 6 bits are address (0x00-0x27 and 0x40-0x67) */

#define		LINE1_START_ADDR		(u8)0x00 		/*!< @brief Constant for defining cursor location for LINE1 */
#define		LINE2_START_ADDR  	(u8)0x40 		/*!< @brief Constant for defining cursor location for LINE2 */
#define   LINE1_END_ADDR      (u8)0x13    /*!< @brief Constant for last displayable character address in LINE1 */
#define   LINE2_END_ADDR      (u8)0x53    /*!< @brief Constant for last displayable character address in LINE2 */
#define   LINE1_END_ABSOLUTE  (u8)0x27    /*!< @brief Constant for last character RAM address in LINE1 */
#define   LINE2_END_ABSOLUTE  (u8)0x67    /*!< @brief Constant for last character RAM address in LINE2 */


/*! @cond DOXYGEN_EXCLUDE */
#define U8_LCD_STARTUP_DELAY_MS           (u8)40     /* Time in ms to wait for LCD startup */
#define U8_LCD_CONTROL_COMMAND_DELAY_MS   (u8)200    /* Time in ms to wait for LCD Command Instructions */

#define U8_LCD_MESSAGE_OVERHEAD_SIZE      (u8)1      /* Number of header bytes for an LCD message */

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

/* Data sheet initialization values */
#define   NUM_CONTROL_CMD     (u8)6
#define		LCD_FUNCTION_CMD		(u8)0x38		/* Literal to set 8-bit bus, 2-line, 5x8 chars, function table 00 */
#define		LCD_FUNCTION2_CMD		(u8)0x39		/* Literal to set 8-bit bus, 2-line, 5x8 chars, function table 01 */
#define   LCD_BIAS_CMD        (u8)0x14    /* Set 1/5 bias and fixed on low */
#define   LCD_CONTRAST_CMD    (u8)0x72    /* Set contrast (default 0x78) */
#define   LCD_DISPLAY_SET_CMD (u8)0x5E    /* Set power, icon control, contrast */
#define   LCD_FOLLOWER_CMD    (u8)0x6D    /* Set follower (not sure what this means!) */

/*! @endcond */


/**********************************************************************************************************************
* Function Declarations
**********************************************************************************************************************/

/*-------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
void LcdCommand(u8 u8Command_);
void LcdClearChars(u8 u8Address_, u8 u8CharactersToClear_);
void LcdMessage(u8 u8Address_, u8 *u8Message_);


/*-------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/
void LcdInitialize(void);
void LcdRunActiveState(void);


/*-------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*-------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
static void LcdSM_Idle(void);

  
#endif /* __LCD_C0220BIZ_H */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/



