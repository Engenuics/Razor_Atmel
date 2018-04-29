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
#define NUMBER_ASCII_TO_DEC     (u8)48        /* Difference between ASCII character 0-9 and the value 0-9 */
#define UPPERCASE_ASCII_TO_DEC  (u8)55        /* Difference between ASCII character A-F and the value 10-15 */
#define LOWERCASE_ASCII_TO_DEC  (u8)87        /* Difference between ASCII character a-f and the value 10-15 */

#define ASCII_CARRIAGE_RETURN   (u8)0x0D      /* ASCII CR char \r */
#define ASCII_LINEFEED          (u8)0x0A      /* ASCII LF char \n */
#define ASCII_BACKSPACE         (u8)0x08      /* ASCII Backspace char */


/* Terminal escape sequences 
("\033" converts to the single control character Esc (0x1B) 
*/
#define TERM_CLEAR_SCREEN       "\033[2J"	
#define TERM_CUR_UP_LEFT        "\033[H"	
#define TERM_CUR_R_C            "\033[r;cH"	  /* Rows and columns are indexed starting at 1 */
#define TERM_CUR_HOME           "\033[1;1H"
#define TERM_CUR_HIDE           "\033[?25l"	
#define TERM_DELETE_RIGHT       "\033[K"	

#define TERM_FORMAT_RESET       "\033[0m"	
#define TERM_BOLD               "\033[1m"
#define TERM_UNDERSCORE         "\033[4m"
#define TERM_BLINK              "\033[5m"
#define TERM_REVERSE            "\033[7m"
#define TERM_CONCEAL            "\033[8m"

#define TERM_TEXT_BLK           "\033[30m"
#define TERM_TEXT_RED           "\033[31m"
#define TERM_TEXT_GRN           "\033[32m"
#define TERM_TEXT_YLW           "\033[33m"
#define TERM_TEXT_BLU           "\033[34m"
#define TERM_TEXT_PRP           "\033[35m"
#define TERM_TEXT_CYN           "\033[36m"
#define TERM_TEXT_WHT           "\033[37m"

#define TERM_BKG_BLK            "\033[40m"
#define TERM_BKG_RED            "\033[41m"
#define TERM_BKG_GRN            "\033[42m"
#define TERM_BKG_YLW            "\033[43m"
#define TERM_BKG_BLU            "\033[44m"
#define TERM_BKG_PRP            "\033[45m"
#define TERM_BKG_CYN            "\033[46m"
#define TERM_BKG_WHT            "\033[47m"


#define MESSAGE_OK              "OK\r\n"
#define MESSAGE_OK_SIZE         (u8)(sizeof(MESSAGE_OK) - 1)

#define MESSAGE_FAIL            "FAIL\r\n"
#define MESSAGE_FAIL_SIZE       (u8)(sizeof(MESSAGE_FAIL) - 1)

#define MESSAGE_ON              "ON\r\n"
#define MESSAGE_ON_SIZE         (u8)(sizeof(MESSAGE_ON) - 1)

#define MESSAGE_OFF             "OFF\r\n"
#define MESSAGE_OFF_SIZE        (u8)(sizeof(MESSAGE_OFF) - 1)

#define MESSAGE_TASK_INIT       " task initializing... "
#define MESSAGE_TASK_INIT_SIZE  (u8)(sizeof(MESSAGE_TASK_INIT) - 1)


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
