/*!**********************************************************************************************************************
@file main.c                                                                
@brief Main system file for the EiE firmware.  
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


/*!**********************************************************************************************************************
@fn void main(void)
@brief Main program where all tasks are initialized and executed.


***********************************************************************************************************************/

void main(void)
{
  u32 u32Counter = 0;
  
  /* Initialize the PIO registers required for our button (B0) and LED (B19) */
  //AT91C_BASE_PIOB->PIO_PDR  = ~0x00080001; // Disables PIO control on B0 and B19
  //AT91C_BASE_PIOB->PIO_ODR  = ~0x00080000; // Set the LED line as an output  
  //AT91C_BASE_PIOB->PIO_OWDR = ~0x00080000; // All the processor to right to the LED pin

  //AT91C_BASE_PIOB->PIO_WPMR = 0x50494F00; // Enables write access to PIO registers
  AT91C_BASE_PMC->PMC_PCER  = 0x27FCED73;
  AT91C_BASE_PIOB->PIO_PER  = 0x00080001; // Enables PIO control on B0 and B19
  AT91C_BASE_PIOB->PIO_OER  = 0x00080000; // Set the LED line as an output  
  AT91C_BASE_PIOB->PIO_OWER = 0x00080000; // All the processor to right to the LED pin
  //AT91C_BASE_PIOB->PIO_WPMR = 0x50494F01; // Locks write access to PIO registers
  
  while(1)
  {
    for(u32 i = 0; i < 1000; i++)
    {
      
    }
    
    u32Counter++;
  } 
  
} /* end main() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
