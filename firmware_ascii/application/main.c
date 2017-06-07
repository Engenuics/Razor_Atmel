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
  bool bLedOn = FALSE;
  
  /* Initialize GPIO registers to allow access to output B19 
  and input B0 */
  AT91C_BASE_PIOB->PIO_PER  = 0x00080001;
  AT91C_BASE_PIOB->PIO_OER  = 0x00080000;
  AT91C_BASE_PIOB->PIO_OWER = 0x00080000;
  
  /* Turn on the clock signal to PIOB */
  AT91C_BASE_PMC->PMC_PCER = (1 << AT91C_ID_PIOB);
  
  /* Disable the watchdog */
  AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;
 
#if 1
  /* If a development board is connected, use the following to properly
  initialize all of the outputs */
  AT91C_BASE_PIOB->PIO_PER  = 0x01BFFF57;
  AT91C_BASE_PIOB->PIO_OER  = 0x01BFFFE0;
  AT91C_BASE_PIOB->PIO_OWER = 0x01FFFE00;
  AT91C_BASE_PIOB->PIO_CODR = 0x01BFFE00;
#endif
  
   
  /* This loop runs the embedded system forever */
  while(1)
  {
    /* Check bit 0 in PDSR */
    if( !(AT91C_BASE_PIOB->PIO_PDSR & 0x00000001) )
    {
      /* Button is pressed, so run the blink code */
      
      /* Check if the LED is on or off */
      if(bLedOn)
      {
        /* The LED is on, so turn it off and update bLedOn */
        AT91C_BASE_PIOB->PIO_CODR = 0x00080000;
        bLedOn = FALSE;
      }
      else
      {
        /* The LED is off, so turn it on and update bLedOn */
        AT91C_BASE_PIOB->PIO_SODR = 0x00080000;
        bLedOn = TRUE;
      }
    }
    /* Button is not pressed, so do nothing */
    else
    {
      /* Make sure that the LED is off */
        AT91C_BASE_PIOB->PIO_CODR = 0x00080000;
        bLedOn = FALSE;      
    }
    
    for(u32 i = 0; i < 100000; i++)
    {
      /* Do nothing - just wait! */  
    }
    
    u32Counter++;
  } 

  
  











  
#if 0
  AT91C_BASE_PIOB->PIO_OER  = 0x00080000;  // Set the LED line as an output  
  AT91C_BASE_PIOB->PIO_ODR  = ~0x00080000; // Clear output from all other lines

  AT91C_BASE_PIOB->PIO_OWER = 0x00080000;  // Allow the processor to right to the LED pin
  AT91C_BASE_PIOB->PIO_OWDR = ~0x00080000; // Disable writes to all other pins

    if(AT91C_BASE_PIOB->PIO_ODSR & 0x00080000)
    {
      /* The bit is set, so clear it */
      AT91C_BASE_PIOB->PIO_CODR = 0x00080000;
    }
    else
    {
      /* The bit is clear, so set it */
      AT91C_BASE_PIOB->PIO_SODR = 0x00080000;
    }
#endif
  
} /* end main() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
