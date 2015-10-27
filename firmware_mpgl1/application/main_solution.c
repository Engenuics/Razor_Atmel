/***********************************************************************************************************************
File: main.c                                                                

Description:
Container for the MPG firmware.  
***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */

/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Main_" and be declared as static.
***********************************************************************************************************************/
static u8 Main_u8Servers = 0;                  /* Number of active servers */


/***********************************************************************************************************************
Main Program
***********************************************************************************************************************/

void main(void)
{
  /* Pointer example code */
  u8 u8Test = 0xA5;
  u8* pu8Example;
  u32 u32Test = 0x0000ffff;
  u32* pu32Example;

  /* Load the addresses into our pointer variables */
  pu8Example = &u8Test;
  pu32Example = &u32Test;

  /* Access the variables via the pointers (two different ways) */
  *pu8Example += 1;
  (*pu32Example)++;

  /* Move the pointers (watch out for the second one!) */
  pu8Example++;
  *pu32Example++;
  
  /* Struct example code */
  u8 u8CurrentServer;
  ServerType sServer1;
  ServerType* psServerParser;

  psServerParser = &sServer1;
  sServer1.u8ServerNumber = 18;
  u8CurrentServer = psServerParser->u8ServerNumber;

  while(1)
  {
    
  } /* end while(1) main super loop */
  
} /* end main() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
