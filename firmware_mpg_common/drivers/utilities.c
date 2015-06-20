/***********************************************************************************************************************
File: utilities.c                                                                

Description:
Various useful functions.

API:
bool IsTimeUp(u32 *pu32SavedTick_, u32 u32Period_)
Calculates if u32Period_ time has passed (in ms) using comparison of
pu32SavedTick_ against G_u32SystemTime1ms.  
e.g. do nothing until 1 second has elapsed
u32ApplicationTimer = G_u32SystemTime1ms;
if( !IsTimeUp(&u32ApplicationTimer, 1000)
{
  // Time has elapsed so update timer for next period and do whatever
  u32ApplicationTimer = G_u32SystemTime1ms;
}


***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
u8 G_au8MessageOK[]   = MESSAGE_OK;            /* Common "OK" message */
u8 G_au8MessageFAIL[] = MESSAGE_FAIL;          /* Common "FAIL" message */
u8 G_au8MessageON[]   = MESSAGE_ON;            /* Common "ON" message */
u8 G_au8MessageOFF[]  = MESSAGE_OFF;           /* Common "OFF" message */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;        /* From scroll-ehw-ae.c */
extern volatile u32 G_u32SystemTime1s;         /* From scroll-ehw-ae.c */

extern volatile u32 G_u32SystemFlags;          /* From main.c */
extern volatile u32 G_u32ApplicationFlags;     /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Util_" and be declared as static.
***********************************************************************************************************************/


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------
Function: IsTimeUp
  
Description:
Checks if the difference between the current time and the saved time is greater
than the period specified. The referenced current time is always G_u32SystemTime1ms.

Requires:
  - *pu32SavedTick_ points to the saved tick value (in ms)
  - u32Period_ is the desired period in ms

Promises:
  - Returns FALSE if u32Period_ has not elapsed
  - Returns TRUE if u32Period_ has elapsed
*/
bool IsTimeUp(u32 *pu32SavedTick_, u32 u32Period_)
{
  u32 u32TimeElapsed;
  
  /* Check to see if the timer in question has rolled */
  if(G_u32SystemTime1ms >= *pu32SavedTick_)
  {
    u32TimeElapsed = G_u32SystemTime1ms - *pu32SavedTick_;
  }
  else
  {
    u32TimeElapsed = (0xFFFFFFFF - *pu32SavedTick_) + G_u32SystemTime1ms;
  }

  /* Now determine if time is up */
  if(u32TimeElapsed < u32Period_)
  {
    return(FALSE);
  }
  else
  {
    return(TRUE);
  }

} /* end IsTimeUp() */


/*-----------------------------------------------------------------------------/
Function: ASCIIHexCharToChar

Description:
Determines the numerical value of a hexidecimal ASCII char of that number
('0' - 'F' or '0' - 'f' -> 0 - 15).

Requires:
  - Standard ASCII table is in use
 
Promises:
  - If valid 0-9, A-F or a-f, returns the numerical value of the ASCII char
  - Otherwise returns 0xff
*/
u8 ASCIIHexCharToChar(u8 u8Char_)
{
  if( (u8Char_ >= '0') && (u8Char_ <= '9') )
    return(u8Char_ - NUMBER_ASCII_TO_DEC);
       
  if( (u8Char_ >= 'A') && (u8Char_ <= 'F') )
    return(u8Char_ - UPPERCASE_ASCII_TO_DEC);
  
  if( (u8Char_ >= 'a') && (u8Char_ <= 'f') )
    return(u8Char_ - LOWERCASE_ASCII_TO_DEC);
  
  return(0xff);

} /* end ASCIIHexChartoChar */


/*-----------------------------------------------------------------------------/
Function: HexToASCIICharUpper

Description:
Determines the ASCII char of a single digit number
0 - 15 -> '0' - 'F'

Requires:
  - Standard ASCII table is in use
 
Promises:
  - If valid 0-15, returns the corresponding ASCII hex char 0-9, A-F
  - Otherwise returns 0xff
*/
u8 HexToASCIICharUpper(u8 u8Char_)
{
  u8 u8ReturnValue = 0xFF;
  
  if( u8Char_ <= 9 )
  {
    u8ReturnValue = u8Char_ + NUMBER_ASCII_TO_DEC;
  }
  
  else if( (u8Char_ >= 10) && (u8Char_ <= 15) )
  {
    u8ReturnValue = u8Char_ + UPPERCASE_ASCII_TO_DEC;
  }
  
  return(u8ReturnValue);

} /* end HexToASCIICharUpper */


/*-----------------------------------------------------------------------------/
Function: HexToASCIICharLower

Description:
Determines the ASCII char of a single digit number
0 - 15 -> '0' - 'f'

Requires:
  - Standard ASCII table is in use
 
Promises:
  - If valid 0-15, returns the corresponding ASCII hex char 0-9, a-f
  - Otherwise returns 0xFF
*/
u8 HexToASCIICharLower(u8 u8Char_)
{
  u8 u8ReturnValue = 0xFF;
  
  if( u8Char_ <= 9 )
  {
    u8ReturnValue = u8Char_ + NUMBER_ASCII_TO_DEC;
  }
  
  else if( (u8Char_ >= 10) && (u8Char_ <= 15) )
  {
    u8ReturnValue = u8Char_ + LOWERCASE_ASCII_TO_DEC;
  }
  
  return(u8ReturnValue);

} /* end HexToASCIICharLower */


/*-----------------------------------------------------------------------------/
Function: NumberToAscii

Description:
Converts a long into an ASCII string.  Maximum of 10 digits + NULL.

Requires:
  - u32Number_ is the number to convert
  - *pu8AsciiString_ points to the destination string location
 
Promises:
  - Null-terminated string of the number is loaded to pu8AsciiString_
  - Returns the number of digits
*/
u8 NumberToAscii(u32 u32Number_, u8* pu8AsciiString_)
{
  bool bFoundDigit = FALSE;
  u8 au8AsciiNumber[11];
  u8 u8CharCount = 0;
  u32 u32Divider = 1000000000;
  u8 u8Temp;
  
  /* Parse out all the digits, start counting after leading zeros */
  for(u8 i = 0; i < 10; i++)
  {
    u8Temp = (u32Number_ / u32Divider);
    
    /* Check for first non-leading zero */
    if(u8Temp != 0)
    {
      bFoundDigit = TRUE;
    }

    /* As long as a non-leading zero has been found, add the ASCII char */
    if(bFoundDigit)
    {
      au8AsciiNumber[u8CharCount] = u8Temp + 0x30;
      u8CharCount++;
    }
    
    /* Update for next iteration */
    u32Number_ %= u32Divider;
    u32Divider /= 10;
  }
  
  /* Handle special case where u32Number == 0 */
  if(!bFoundDigit)
  {
    u8CharCount = 1;
    au8AsciiNumber[0] = '0';
  }
  
  /* Add the null and copy to destination */
  au8AsciiNumber[u8CharCount] = NULL;
  strcpy((char *)pu8AsciiString_, (const char*)au8AsciiNumber);
  
  return(u8CharCount);

} /* end NumberToAscii() */


/*-----------------------------------------------------------------------------/
Function: SearchString

Description:
Searches a string for another string.  Finds only an exact match of the string (case sensitive).
Character following matched string must be space, <CR> or <LF>.

Requires:
  - Standard ASCII table is in use
  - pu8TargetString_ points to the start of a NULL, <CR> or <LF> terminated string to search
  - pu8MatchString_ points to the start of a NULL, <CR> or <LF> terminated string to be found in pu8TargetString_
 
Promises:
  - Returns TRUE if the string is found
*/
bool SearchString(u8* pu8TargetString_, u8* pu8MatchString_)
{
  u8* pu8MatchChar = pu8MatchString_;
  u8* pu8TargetChar = pu8TargetString_;
  
  do
  {
    /* Scan for the current character of pu8MatchString_ in pu8TargetString_ */
    while( (*pu8MatchChar != *pu8TargetChar) && (*pu8TargetChar != NULL) && 
           (*pu8TargetChar != ASCII_LINEFEED) && (*pu8TargetChar != ASCII_CARRIAGE_RETURN) )
    {
      pu8TargetChar++;
    }
    
    /* Exit if we're at the end of the target string */
    if( (*pu8TargetChar == NULL) || 
        (*pu8TargetChar == ASCII_LINEFEED) || (*pu8TargetChar == ASCII_CARRIAGE_RETURN) )
    {
      return(FALSE);
    }
    
    /* If we found a match, look for next char */
    while ( *pu8MatchChar == *pu8TargetChar )
    {
      pu8MatchChar++;
      pu8TargetChar++;
      
      /* At the end of the match string? */
      if( (*pu8MatchChar == NULL) || (*pu8MatchChar == ASCII_LINEFEED) || (*pu8MatchChar == ASCII_CARRIAGE_RETURN) )
      {
        /* Check if the next character in pu8TargetChar is space, <CR>, <LF> or ':' */
        if( (*pu8TargetChar == ' ') ||
            (*pu8TargetChar == ASCII_CARRIAGE_RETURN) ||
            (*pu8TargetChar == ASCII_LINEFEED) ||
            (*pu8TargetChar == ':')  )
        {
          return(TRUE);
        }
      }
    }

    /* At the end of the target string? */
    if( (*pu8TargetChar == NULL) || (*pu8TargetChar == ASCII_LINEFEED) || (*pu8TargetChar == ASCII_CARRIAGE_RETURN) )
    {
      return(FALSE);
    }
    
    /* If here, match was not found (yet), but as long as there are more characters in pu8TargetChar,
    then we can scan again. */
    
    /* Reset match pointer back to the start of its string */
    pu8MatchChar = pu8MatchString_;
  } while ( (*pu8TargetChar != NULL) && 
            (*pu8TargetChar != ASCII_LINEFEED) && (*pu8TargetChar != ASCII_CARRIAGE_RETURN) );
  
  /* If we get here, no match was found */
  return(FALSE);

} /* end SearchString */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
