/***********************************************************************************************************************
File: mpgl2-ehdw-01.c                                                                

Description:
This file provides core and GPIO functions for the mpgl2-ehdw-01 board.
***********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32SystemTime1ms;                       /* Global system time incremented every ms, max 2^32 (~49 days) */
volatile u32 G_u32SystemTime1s;                        /* Global system time incremented every second, max 2^32 (~136 years) */

/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                 /* From main.c */
extern volatile u32 G_u32ApplicationFlags;            /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Bsp_" and be declared as static.
***********************************************************************************************************************/


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public Functions */
/*--------------------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected Functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: ClockSetup

Description:
Loads all registers required to set up the processor clocks.

Requires:
  - Built-in 32kHz slow clock is active to time out the fast clock startup time

Promises:
  - EFC is set up with proper flash access wait states based on 48MHz system clock
  - PMC is set up with proper oscillators and clock sources
*/
void ClockSetup(void)
{
  /* Activate the peripheral clocks needed for the system */
  AT91C_BASE_PMC->PMC_PCER = PMC_PCER_INIT;

  /* Enable the master clock on the PKC0 clock out pin (PA_27_CLOCK_OUT) */
  AT91C_BASE_PMC->PMC_PCKR[0] = AT91C_PMC_CSS_SYS_CLK | AT91C_PMC_PRES_CLK;
  AT91C_BASE_PMC->PMC_SCER = AT91C_PMC_PCK0;

  /* Turn on the main oscillator and wait for it to start up */
  AT91C_BASE_PMC->PMC_MOR = PMC_MOR_INIT;
  while ( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MOSCXTS) );

  /* Assign main clock as crystal */
  AT91C_BASE_PMC->PMC_MOR |= (AT91C_CKGR_MOSCSEL | MOR_KEY);
  
  /* Initialize PLLA and wait for lock */
  AT91C_BASE_PMC->PMC_PLLAR = PMC_PLAAR_INIT;
  while ( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCKA) );
  
  /* Set flash wait states to allow 48 MHz system clock (2 wait states required) */
  AT91C_BASE_EFC0->EFC_FMR = AT91C_EFC_FWS_2WS;
//  AT91C_BASE_EFC1->EFC_FMR = AT91C_EFC_FWS_2WS;

  /* Assign the PLLA as the main system clock with prescaler active using the sequence suggested on pg. 472 */
  AT91C_BASE_PMC->PMC_MCKR = PMC_MCKR_INIT;
  while ( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );
  AT91C_BASE_PMC->PMC_MCKR = PMC_MCKR_PLLA;
  while ( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_MCKRDY) );

  /* Initialize UTMI for USB usage */
  AT91C_BASE_CKGR->CKGR_UCKR |= (AT91C_CKGR_UPLLCOUNT & (3 << 20)) | AT91C_CKGR_UPLLEN;
  while ( !(AT91C_BASE_PMC->PMC_SR & AT91C_PMC_LOCKU) );
   
} /* end ClockSetup */


/*----------------------------------------------------------------------------------------------------------------------
Function: SysTickSetup

Description:
Initializes the 1ms and 1s System Ticks off the core timer.

Requires:
  - NVIC is setup and SysTick handler is installed

Promises:
  - Both system timers are set and the SysTick core peripheral is configured for 1ms intervals
*/
void SysTickSetup(void)
{
  G_u32SystemTime1ms = 0;      
  G_u32SystemTime1s  = 0;   
  
  /* Load the SysTick Counter Value */
  AT91C_BASE_NVIC->NVIC_STICKRVR   = (u32)SYSTICK_COUNT - 1; /* Check value */
  AT91C_BASE_NVIC->NVIC_STICKCVR   = (0x00);                                                              
  AT91C_BASE_NVIC->NVIC_STICKCSR   = SYSTICK_CTRL_INIT;

} /* end SysTickSetup() */


/*----------------------------------------------------------------------------------------------------------------------
Function: SystemSleep

Description:
Puts the system into sleep mode.  Deep sleep mode is currently disabled, so
maximum processor power savings are not yet realized.  To enable deep sleep,
there are certain considerations for waking up that must be taken care of.

Requires:
  - SysTick is running with interrupt enabled for wake from Sleep LPM
  - RTC 1 second alarm running with interrupt for wake from Stop LPM

Promises:
  - Configures processor for maximum sleep while still allowing any required
    interrupt to wake it up.
*/
void SystemSleep(void)
{    
  /* Set the system control register for Sleep (but not Deep Sleep) */
   AT91C_BASE_PMC->PMC_FSMR &= ~AT91C_PMC_LPM;
   AT91C_BASE_NVIC->NVIC_SCR &= ~AT91C_NVIC_SLEEPDEEP;
   
   /* Set the sleep flag (cleared only in SysTick ISR */
   G_u32SystemFlags |= _SYSTEM_SLEEPING;

  /* Now enter the selected LPM */
   while(G_u32SystemFlags & _SYSTEM_SLEEPING)
   {
     __WFI();
   }

  /* Clear the sleep mode status flags */
  //AT91C_SC->PCON &= SLEEP_MODE_STATUS_CLEAR;
  
} /* end SystemSleep(void) */


/*----------------------------------------------------------------------------------------------------------------------
Function: WatchDogSetup

Description:
Configures the watchdog timer.  The dog runs at 32.768kHz from the slow built-in RC clock source which varies over operating conditions from 30kHz to 60kHz.
Since the main loop time / sleep time should be 1 ms most of the time, choosing a value
of 5 seconds should be plenty to avoid watchdog resets.  

Note: the processor allows the MR register to be written just once.

Requires:
  - SCLK is active at about 32.768kHz

Promises:
  - Watchdog is set for 5 second timeout but not yet enabled
*/
void WatchDogSetup(void)
{
  AT91C_BASE_WDTC->WDTC_WDMR = WDT_MR_INIT;
  WATCHDOG_BONE();
 
} /* end WatchDogSetup() */


/*----------------------------------------------------------------------------------------------------------------------
Function: GpioSetup

Description
Loads all registers required to set up GPIO on the processor.

Requires:
  - All configurations must match connected hardware.

Promises:
  - All I/O lines are set for their required function and start-state
*/
void GpioSetup(void)
{
  /* Set all of the pin function registers in port A */
  AT91C_BASE_PIOA->PIO_PER    = PIOA_PER_INIT;
  AT91C_BASE_PIOA->PIO_PDR    = PIOA_PDR_INIT;
  AT91C_BASE_PIOA->PIO_OER    = PIOA_OER_INIT;
  AT91C_BASE_PIOA->PIO_ODR    = PIOA_ODR_INIT;
  AT91C_BASE_PIOA->PIO_IFER   = PIOA_IFER_INIT;
  AT91C_BASE_PIOA->PIO_IFDR   = PIOA_IFDR_INIT;
  AT91C_BASE_PIOA->PIO_SODR   = PIOA_SODR_INIT;
  AT91C_BASE_PIOA->PIO_CODR   = PIOA_CODR_INIT;
  AT91C_BASE_PIOA->PIO_MDER   = PIOA_MDER_INIT;
  AT91C_BASE_PIOA->PIO_MDDR   = PIOA_MDDR_INIT;
  AT91C_BASE_PIOA->PIO_PPUDR  = PIOA_PPUDR_INIT;
  AT91C_BASE_PIOA->PIO_PPUER  = PIOA_PPUER_INIT;
  AT91C_BASE_PIOA->PIO_ABSR   = PIOA_ABSR_INIT;
  AT91C_BASE_PIOA->PIO_SCIFSR = PIOA_SCIFSR_INIT;
  AT91C_BASE_PIOA->PIO_DIFSR  = PIOA_DIFSR_INIT;
  AT91C_BASE_PIOA->PIO_SCDR   = PIOA_SCDR_INIT;
  AT91C_BASE_PIOA->PIO_OWER   = PIOA_OWER_INIT;
  AT91C_BASE_PIOA->PIO_OWDR   = PIOA_OWDR_INIT;
  
  /* Configure port B GPIO */
  AT91C_BASE_PIOB->PIO_PER    = PIOB_PER_INIT;
  AT91C_BASE_PIOB->PIO_PDR    = PIOB_PDR_INIT;
  AT91C_BASE_PIOB->PIO_OER    = PIOB_OER_INIT;
  AT91C_BASE_PIOB->PIO_ODR    = PIOB_ODR_INIT;
  AT91C_BASE_PIOB->PIO_IFER   = PIOB_IFER_INIT;
  AT91C_BASE_PIOB->PIO_IFDR   = PIOB_IFDR_INIT;
  AT91C_BASE_PIOB->PIO_SODR   = PIOB_SODR_INIT;
  AT91C_BASE_PIOB->PIO_CODR   = PIOB_CODR_INIT;
  AT91C_BASE_PIOB->PIO_MDER   = PIOB_MDER_INIT;
  AT91C_BASE_PIOB->PIO_MDDR   = PIOB_MDDR_INIT;
  AT91C_BASE_PIOB->PIO_PPUDR  = PIOB_PPUDR_INIT;
  AT91C_BASE_PIOB->PIO_PPUER  = PIOB_PPUER_INIT;
  AT91C_BASE_PIOB->PIO_ABSR   = PIOB_ABSR_INIT;
  AT91C_BASE_PIOB->PIO_SCIFSR = PIOB_SCIFSR_INIT;
  AT91C_BASE_PIOB->PIO_DIFSR  = PIOB_DIFSR_INIT;
  AT91C_BASE_PIOB->PIO_SCDR   = PIOB_SCDR_INIT;
  AT91C_BASE_PIOB->PIO_OWER   = PIOB_OWER_INIT;
  AT91C_BASE_PIOB->PIO_OWDR   = PIOB_OWDR_INIT;
  
} /* end GpioSetup() */


/*----------------------------------------------------------------------------
Function: PWMSetupAudio

Description:
Configures the PWM peripheral for audio operation on H0 and H1 channels.

Requires:
  - Peripheral resources not used for any other function.

Promises:
  - PWM is configured for PWM mode and currently off.
*/
void PWMSetupAudio(void)
{
  /* Set all intialization values */
  AT91C_BASE_PWMC->PWMC_CLK = PWM_CLK_INIT;
  AT91C_BASE_PWMC->PWMC_SCM = PWM_SCM_INIT;
  
  AT91C_BASE_PWMC_CH0->PWMC_CMR = PWM_CMR0_INIT;
  AT91C_BASE_PWMC_CH0->PWMC_CPRDR    = PWM_CPRD0_INIT; /* Set current frequency */
  AT91C_BASE_PWMC_CH0->PWMC_CPRDUPDR = PWM_CPRD0_INIT; /* Latch CPRD values */
  AT91C_BASE_PWMC_CH0->PWMC_CDTYR    = PWM_CDTY0_INIT; /* Set 50% duty */
  AT91C_BASE_PWMC_CH0->PWMC_CDTYUPDR = PWM_CDTY0_INIT; /* Latch CDTY values */

#ifdef MPGL1  
  AT91C_BASE_PWMC_CH1->PWMC_CMR = PWM_CMR1_INIT;
  AT91C_BASE_PWMC_CH1->PWMC_CPRDR    = PWM_CPRD1_INIT; /* Set current frequency  */
  AT91C_BASE_PWMC_CH1->PWMC_CPRDUPDR = PWM_CPRD1_INIT; /* Latch CPRD values */
  AT91C_BASE_PWMC_CH1->PWMC_CDTYR    = PWM_CDTY1_INIT; /* Set 50% duty */
  AT91C_BASE_PWMC_CH1->PWMC_CDTYUPDR = PWM_CDTY1_INIT; /* Latch CDTY values */
#endif /* MPGL1 */
  
} /* end PWMSetupAudio() */


/*----------------------------------------------------------------------------
Function: PWMAudioSetFrequency

Description:
Configures the PWM peripheral with the desired frequency.
Note, we don't care if we interrupt the current cycle, so the direct registers
are used rather than the double-buffered values.

Requires:
  - u32Channel_ is the channel of interest - either BUZZER1 or BUZZER2 (MPGL1 only)
  - u16Frequency_ is in Hertz and should be in the range 100 - 20,000 since
    that is the audible range.  Higher and lower frequencies are allowed, though.
  - The PWM peripheral is correctly configured for the current processor clock speed.
  - CPRE_CLCK is the clock frequency for the PWM peripheral

Promises:
  - The frequency and duty cycle values for the requested channel are calculated
    and set to their respective update registers (either set directly or latched for update)
  - If the channel is not valid, nothing happens
*/
void PWMAudioSetFrequency(u32 u32Channel_, u16 u16Frequency_)
{
  u32 u32ChannelPeriod;
  
  u32ChannelPeriod = CPRE_CLCK / u16Frequency_;
  
  if(u32Channel_ == BUZZER1)
  {
    /* Set different registers depending on if PWM is already running */
    if (AT91C_BASE_PWMC->PWMC_SR & AT91C_PWMC_CHID0)
    {
      /* Beeper is already running, so use update registers */
      AT91C_BASE_PWMC_CH0->PWMC_CPRDUPDR = u32ChannelPeriod;   
      AT91C_BASE_PWMC_CH0->PWMC_CDTYUPDR = u32ChannelPeriod >> 1; 
    }
    else
    {
      /* Beeper is off, so use direct registers */
      AT91C_BASE_PWMC_CH0->PWMC_CPRDR = u32ChannelPeriod;
      AT91C_BASE_PWMC_CH0->PWMC_CDTYR = u32ChannelPeriod >> 1;
    }
  }
  
#ifdef MPGL1 
  else if(u32Channel_ == BUZZER2)
  {
    /* Set different registers depending on if PWM is already running */
    if (AT91C_BASE_PWMC->PWMC_SR & AT91C_PWMC_CHID1)
    {
      /* Beeper is already running, so use update registers */
      AT91C_BASE_PWMC_CH1->PWMC_CPRDUPDR = u32ChannelPeriod;   
      AT91C_BASE_PWMC_CH1->PWMC_CDTYUPDR = u32ChannelPeriod >> 1; 
    }
    else
    {
      /* Beeper is off, so use direct registers */
      AT91C_BASE_PWMC_CH1->PWMC_CPRDR = u32ChannelPeriod;
      AT91C_BASE_PWMC_CH1->PWMC_CDTYR = u32ChannelPeriod >> 1;
    }
  }
#endif  
  
} /* end PWMAudioSetFrequency() */


/*----------------------------------------------------------------------------
Function: PWMAudioOn

Description:
Enables a PWM channel.

Requires:
  - All peripheral values should be configured
  - u32Channel_ is AT91C_PWMC_CHID0 or AT91C_PWMC_CHID1

Promises:
  - PWM for the selected channel is enabled
*/
void PWMAudioOn(u32 u32Channel_)
{
  /* Enable the channel */
  AT91C_BASE_PWMC->PWMC_ENA = u32Channel_;  

} /* end PWMAudioOn() */


/*----------------------------------------------------------------------------
Function: PWMAudioOff

Description:
Disables a PWM channel.

Requires:
  - u32Channel_ is AT91C_PWMC_CHID0 or AT91C_PWMC_CHID1

Promises:
  - PWM for the selected channel is disabled
*/
void PWMAudioOff(u32 u32Channel_)
{
  /* Disable the channel */
  AT91C_BASE_PWMC->PWMC_DIS = u32Channel_;  

} /* end PWMAudioOff() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/



