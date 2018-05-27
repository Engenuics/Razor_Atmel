/*!**********************************************************************************************************************
@file utilities.h    

@brief Header file for utilities.c.

***********************************************************************************************************************/

#ifndef __UTILITIES_H
#define __UTILITIES_H

/***********************************************************************************************************************
Type Definitions
***********************************************************************************************************************/


/***********************************************************************************************************************
Constants / Definitions
***********************************************************************************************************************/
#define NUMBER_ASCII_TO_DEC     (u8)48        /*!< @brief Difference between ASCII character 0-9 and the value 0-9 */
#define UPPERCASE_ASCII_TO_DEC  (u8)55        /*!< @brief Difference between ASCII character A-F and the value 10-15 */
#define LOWERCASE_ASCII_TO_DEC  (u8)87        /*!< @brief Difference between ASCII character a-f and the value 10-15 */

#define ASCII_CARRIAGE_RETURN   (u8)0x0D      /*!< @brief ASCII CR char \r */
#define ASCII_LINEFEED          (u8)0x0A      /*!< @brief ASCII LF char \n */
#define ASCII_BACKSPACE         (u8)0x08      /*!< @brief ASCII Backspace char */

/* Terminal escape sequences 
("\033" converts to the single control character Esc (0x1B) 
*/
#define TERM_CLEAR_SCREEN       "\033[2J"	    /*!< @brief Escape code to clear entire terminal screen */
#define TERM_CUR_UP_LEFT        "\033[H"	    /*!< @brief Escape code to move cursor to top left */
#define TERM_CUR_R_C            "\033[r;cH"	  /*!< @brief Rows and columns are indexed starting at 1 */
#define TERM_CUR_HOME           "\033[1;1H"   /*!< @brief Escape code to home cursor */
#define TERM_CUR_HIDE           "\033[?25l"	  /*!< @brief Escape code to hide cursor */
#define TERM_DELETE_RIGHT       "\033[K"	    /*!< @brief Escape code to delete all chars to right of cursor */

#define TERM_FORMAT_RESET       "\033[0m"	    /*!< @brief Escape code to reset terminal formatting to default */
#define TERM_BOLD               "\033[1m"     /*!< @brief Escape code to activate bold text */
#define TERM_UNDERSCORE         "\033[4m"     /*!< @brief Escape code to activate underscored text */
#define TERM_BLINK              "\033[5m"     /*!< @brief Escape code to activate blinking text (may not be supported) */
#define TERM_REVERSE            "\033[7m"     /*!< @brief Escape code to activate inverted text */
#define TERM_CONCEAL            "\033[8m"     /*!< @brief Escape code to activate hidden text */

#define TERM_TEXT_BLK           "\033[30m"    /*!< @brief Escape code for colored text */
#define TERM_TEXT_RED           "\033[31m"    /*!< @brief Escape code for colored text */
#define TERM_TEXT_GRN           "\033[32m"    /*!< @brief Escape code for colored text */
#define TERM_TEXT_YLW           "\033[33m"    /*!< @brief Escape code for colored text */
#define TERM_TEXT_BLU           "\033[34m"    /*!< @brief Escape code for colored text */
#define TERM_TEXT_PRP           "\033[35m"    /*!< @brief Escape code for colored text */
#define TERM_TEXT_CYN           "\033[36m"    /*!< @brief Escape code for colored text */
#define TERM_TEXT_WHT           "\033[37m"    /*!< @brief Escape code for colored text */

#define TERM_BKG_BLK            "\033[40m"    /*!< @brief Escape code for colored background */
#define TERM_BKG_RED            "\033[41m"    /*!< @brief Escape code for colored background */
#define TERM_BKG_GRN            "\033[42m"    /*!< @brief Escape code for colored background */
#define TERM_BKG_YLW            "\033[43m"    /*!< @brief Escape code for colored background */
#define TERM_BKG_BLU            "\033[44m"    /*!< @brief Escape code for colored background */
#define TERM_BKG_PRP            "\033[45m"    /*!< @brief Escape code for colored background */
#define TERM_BKG_CYN            "\033[46m"    /*!< @brief Escape code for colored background */
#define TERM_BKG_WHT            "\033[47m"    /*!< @brief Escape code for colored background */


#define MESSAGE_OK              "OK\r\n"                            /*!< @brief Standard message */
#define MESSAGE_OK_SIZE         (u8)(sizeof(MESSAGE_OK) - 1)        /*!< @brief Message size in bytes less NULL */

#define MESSAGE_FAIL            "FAIL\r\n"                          /*!< @brief Standard message */
#define MESSAGE_FAIL_SIZE       (u8)(sizeof(MESSAGE_FAIL) - 1)      /*!< @brief Message size in bytes less NULL */

#define MESSAGE_ON              "ON\r\n"                            /*!< @brief Standard message */
#define MESSAGE_ON_SIZE         (u8)(sizeof(MESSAGE_ON) - 1)        /*!< @brief Message size in bytes less NULL */

#define MESSAGE_OFF             "OFF\r\n"                           /*!< @brief Standard message */
#define MESSAGE_OFF_SIZE        (u8)(sizeof(MESSAGE_OFF) - 1)       /*!< @brief Message size in bytes less NULL */

#define MESSAGE_TASK_INIT       " task initializing... "            /*!< @brief Standard message */
#define MESSAGE_TASK_INIT_SIZE  (u8)(sizeof(MESSAGE_TASK_INIT) - 1) /*!< @brief Message size in bytes less NULL */


/***********************************************************************************************************************
* Function Declarations
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

bool IsTimeUp(u32 *pu32SavedTick_, u32 u32Period_);
u8 ASCIIHexCharToNum(u8);
u8 HexToASCIICharUpper(u8 u8Char_);
u8 HexToASCIICharLower(u8 u8Char_);
u8 NumberToAscii(u32 u32Number_, u8* pu8AsciiString_);
bool SearchString(u8* pu8TargetString_, u8* pu8MatchString_);


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


#endif /* __UTILITIES_H */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
