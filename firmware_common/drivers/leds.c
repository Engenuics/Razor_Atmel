/***********************************************************************************************************************
File: leds.c                                                                

Description:
LED driver that provides on, off, toggle, blink and PWM functionality.
The basic on/off/toggle functionality is applied directly to the LEDs.
Blinking and PWMing of LEDs rely on the EIE operating system to provide timing at
regular 1ms calls to LedUpdate().

------------------------------------------------------------------------------------------------------------------------
API:
LedNumberType: 
  EIE1     - WHITE, PURPLE, BLUE, CYAN, GREEN, YELLOW, ORANGE, RED, LCD_RED, LCD_GREEN, LCD_BLUE
  MPGL2_R01 - BLUE, GREEN, YELLOW, RED, LCD_BL
  MPGL2     - BLUE0, BLUE1, BLUE2, BLUE3, GREEN0, GREEN1, GREEN2, GREEN3, RED0, RED1, RED2, RED3, LCD_BL

LedRateType: LED_0_5HZ, LED_1HZ, LED_2HZ, LED_4HZ, LED_8HZ, 
             LED_PWM_0, LED_PWM_5, ..., LED_PWM_100
*Note that PWM values in LedRateType are continuous, thus stepping a variable of LedRateType by one will select the next 
PWM level. However, decrementing past LED_PWM_0 or incrementing past LED_PWM_100 is undefined.


Public:
void LedOn(LedNumberType eLED_)
Turn the specified LED on. LED response is immediate.
e.g. LedOn(BLUE);

void LedOff(LedNumberType eLED_)
Turn the specified LED off. LED response is immediate.
e.g. LedOff(BLUE);

void LedToggle(LedNumberType eLED_)
Toggle the specified LED.  LED response is immediate. LED must be in NORMAL mode.
e.g. LedToggle(BLUE);

void LedPWM(LedNumberType eLED_, LedRateType ePwmRate_)
Sets up an LED for PWM mode.  PWM mode requires the main loop to be running at 1ms period.
e.g. LedPWM(BLUE, LED_PWM_5);

void LedBlink(LedNumberType eLED_, LedRateType eBlinkRate_)
Sets an LED to BLINK mode.  BLINK mode requires the main loop to be running at 1ms period.
e.g. LedBlink(BLUE, LED_1HZ);

Protected:
void LedInitialize(void)
Test all LEDs and initialize to OFF state.

DISCLAIMER: THIS CODE IS PROVIDED WITHOUT ANY WARRANTY OR GUARANTEES.  USERS MAY
USE THIS CODE FOR DEVELOPMENT AND EXAMPLE PURPOSES ONLY.  ENGENUICS TECHNOLOGIES
INCORPORATED IS NOT RESPONSIBLE FOR ANY ERRORS, OMISSIONS, OR DAMAGES THAT COULD
RESULT FROM USING THIS FIRMWARE IN WHOLE OR IN PART.
***********************************************************************************************************************/

#include "configuration.h"


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_xxLed"
***********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* New variables (all shall start with G_xxLed*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */
extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

extern volatile u32 G_u32ApplicationFlags;             /* From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Led_" and be declared as static.
***********************************************************************************************************************/

/************ %LED% EDIT BOARD-SPECIFIC GPIO DEFINITIONS BELOW ***************/

#ifdef EIE1
/* LED locations: order must correspond to the order set in LedNumberType in the header file. */
static u32 Led_au32BitPositions[] = {PB_13_LED_WHT, PB_14_LED_PRP, PB_18_LED_BLU, PB_16_LED_CYN,
                                     PB_19_LED_GRN, PB_17_LED_YLW, PB_15_LED_ORG, PB_20_LED_RED,
                                     PB_10_LCD_BL_RED, PB_11_LCD_BL_GRN, PB_12_LCD_BL_BLU};

/* Control array for all LEDs in system initialized for LedInitialize().  Array values correspond to LedConfigType fields: 
     eMode         eRate      u16Count       eCurrentDuty     eActiveState     ePort      LedNumberType */
static LedConfigType Leds_asLedArray[TOTAL_LEDS] = 
{{LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* WHITE      */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* PURPLE     */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* BLUE       */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* CYAN       */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* GREEN      */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* YELLOW     */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* ORANGE     */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* RED        */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* LCD_RED    */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* LCD_GREEN  */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}  /* LCD_BLUE   */
};   
#endif /* EIE1 */

#ifdef MPGL2

#ifdef MPGL2_R01
/* LED locations: order must correspond to the order set in LedNumberType in the header file. */
static u32 Led_au32BitPositions[] = {PB_18_LED_BLU, PB_19_LED_GRN, PB_17_LED_YLW, PB_20_LED_RED, PB_11_LCD_BL};

/* Control array for all LEDs in system initialized for LedInitialize().  Array values correspond to LedConfigType fields: 
     eMode         eRate      u16Count       eCurrentDuty     eActiveState     ePort      LedNumberType */
static LedConfigType Leds_asLedArray[TOTAL_LEDS] = 
{{LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* BLUE       */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* GREEN      */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* YELLOW     */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* RED        */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* LCD_BL     */
};   
#else
/* LED locations: order must correspond to the order set in LedNumberType in the header file. */
static u32 Led_au32BitPositions[] = {PB_20_LED0_RED, PB_17_LED1_RED, PB_19_LED2_RED, PB_18_LED3_RED,
                                     PA_29_LED0_GRN, PB_02_LED1_GRN, PA_26_LED2_GRN, PA_07_LED3_GRN,
                                     PB_01_LED0_BLU, PB_13_LED1_BLU, PA_06_LED2_BLU, PA_08_LED3_BLU,
                                     PB_05_LCD_BL};

/* Control array for all LEDs in system initialized for LedInitialize().  Array values correspond to LedConfigType fields: 
     eMode         eRate      u16Count       eCurrentDuty     eActiveState     ePort      LedNumberType */
static LedConfigType Leds_asLedArray[TOTAL_LEDS] = 
{{LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* RED0       */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* RED1       */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* RED2       */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* RED3       */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTA}, /* GREEN0     */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* GREEN1     */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTA}, /* GREEN2     */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTA}, /* GREEN3     */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* BLUE0      */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* BLUE1      */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTA}, /* BLUE2      */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTA}, /* BLUE3      */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* LCD_BL     */
};  
#endif /* MPGL2_R01 */

#endif /* MPGL2 */

/************ EDIT BOARD-SPECIFIC GPIO DEFINITIONS ABOVE ***************/
 

/***********************************************************************************************************************
* Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: LedOn

Description:
Turn the specified LED on.  Automatically takes care of the active low vs. active
high LEDs.  

Requires:
  - eLED_ is a valid LED index
  - Definitions in Leds_asLedArray[eLED_] are correct

Promises:
  - Requested LED is configured to be turned on next LedUpdate()
  - Requested LED is always set to LED_NORMAL_MODE mode
*/
void LedOn(LedNumberType eLED_)
{
  u32 *pu32SetAddress;

  /* Configure set and clear addresses */
  if(Leds_asLedArray[eLED_].eActiveState == LED_ACTIVE_HIGH)
  {
    /* Active high LEDs use SODR to turn on */
    pu32SetAddress = (u32*)(&(AT91C_BASE_PIOA->PIO_SODR) + Leds_asLedArray[eLED_].ePort);
  }
  else
  {
    /* Active low LEDs use CODR to turn on */
    pu32SetAddress = (u32*)(&(AT91C_BASE_PIOA->PIO_CODR) + Leds_asLedArray[eLED_].ePort);
  }
  
  /* Turn on the LED */
  *pu32SetAddress = Led_au32BitPositions[(u8)eLED_];
  
  /* Always set the LED back to LED_NORMAL_MODE mode */
	Leds_asLedArray[(u8)eLED_].eMode = LED_NORMAL_MODE;

} /* end LedOn() */


/*----------------------------------------------------------------------------------------------------------------------
Function: LedOff

Description:
Turn the specified LED off.  Automatically takes care of the active low vs. active
high LEDs.  

Requires:
  - eLED_ is a valid LED index
  - Definitions in Leds_asLedArray[eLED_] are correct

Promises:
  - Requested LED is turned off
  - Requested LED is always set to LED_NORMAL_MODE mode
*/
void LedOff(LedNumberType eLED_)
{
  u32 *pu32ClearAddress;

  /* Configure set and clear addresses */
  if(Leds_asLedArray[eLED_].eActiveState == LED_ACTIVE_HIGH)
  {
    /* Active high LEDs use CODR to turn off */
    pu32ClearAddress = (u32*)(&(AT91C_BASE_PIOA->PIO_CODR) + Leds_asLedArray[eLED_].ePort);
  }
  else
  {
    /* Active low LEDs use SODR to turn off */
    pu32ClearAddress = (u32*)(&(AT91C_BASE_PIOA->PIO_SODR) + Leds_asLedArray[eLED_].ePort);
  }
  
  /* Clear the bit corresponding to eLED_ */
	*pu32ClearAddress = Led_au32BitPositions[(u8)eLED_];

  /* Always set the LED back to LED_NORMAL_MODE mode */
	Leds_asLedArray[(u8)eLED_].eMode = LED_NORMAL_MODE;
  
} /* end LedOff() */


/*----------------------------------------------------------------------------------------------------------------------
Function: LedToggle

Description:
Toggle the specified LED.

Requires:
  - eLED_ is a valid LED index
  - eLED_ *should* be in LED_NORMAL_MODE
  - Write access to PIOx_ODSR is enabled

Promises:
  - Requested LED is toggled if the LED is in LED_NORMAL_MODE mode
*/
void LedToggle(LedNumberType eLED_)
{
  u32 *pu32Address = (u32*)(&(AT91C_BASE_PIOA->PIO_ODSR) + Leds_asLedArray[eLED_].ePort);

  *pu32Address ^= Led_au32BitPositions[ (u8)eLED_ ];
  
} /* end LedToggle() */


/*----------------------------------------------------------------------------------------------------------------------
Function: LedPWM

Description:
Sets an LED to PWM mode

Requires:
  - eLED_ is a valid LED index
  - ePwmRate_ is an allowed duty cycle:
    LED_PWM_0, LED_PWM_5, LED_PWM_10, ..., LED_PWM_95, LED_PWM_100

Promises:
  - Requested LED is set to PWM mode at the duty cycle specified
*/
void LedPWM(LedNumberType eLED_, LedRateType ePwmRate_)
{
	Leds_asLedArray[(u8)eLED_].eMode = LED_PWM_MODE;
	Leds_asLedArray[(u8)eLED_].eRate = ePwmRate_;
	Leds_asLedArray[(u8)eLED_].u16Count = (u16)ePwmRate_;
  Leds_asLedArray[(u8)eLED_].eCurrentDuty = LED_PWM_DUTY_HIGH;

} /* end LedPWM() */


/*----------------------------------------------------------------------------------------------------------------------
Function: LedBlink

Description:
Sets an LED to BLINK mode.

Requires:
  - eLED_ is a valid LED index
  - eBlinkRate_ is an allowed frequency:
    LED_0_5HZ, LED_1HZ, LED_2HZ, LED_4HZ, LED_8HZ

Promises:
  - Requested LED is set to BLINK mode at the rate specified
*/
void LedBlink(LedNumberType eLED_, LedRateType eBlinkRate_)
{
	Leds_asLedArray[(u8)eLED_].eMode = LED_BLINK_MODE;
	Leds_asLedArray[(u8)eLED_].eRate = eBlinkRate_;
	Leds_asLedArray[(u8)eLED_].u16Count = eBlinkRate_;

} /* end LedBlink() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: LedInitialize

Description:
Initialization of LED system paramters and visual LED check.

Requires:
  - G_u32SystemTime1ms ticking
  - All LEDs already initialized to LED_NORMAL_MODE mode ON

Promises:
  - All LEDs in LED_NORMAL_MODE mode with OFF
*/
void LedInitialize(void)
{
  u32 u32Timer;
  u8  u8Index;

  u32 u32Buzzer1Frequency = 4000;
  u32 u32Buzzer2Frequency = 500;
  u32 u32StepSize = (u32Buzzer1Frequency - u32Buzzer2Frequency) / 20;

  static u8 au8LedStartupMsg[] = "LED functions ready\n\r";

#if MPGL2
  /* Test code for checking LEDs */
#if 0
  LedOn(RED0);
  LedOn(RED1);
  LedOn(RED2);
  LedOn(RED3);
  LedOn(BLUE0);
  LedOn(BLUE1);
  LedOn(BLUE2);
  LedOn(BLUE3);
  LedOn(GREEN0);
  LedOn(GREEN1);
  LedOn(GREEN2);
  LedOn(GREEN3);
#endif

#endif /* MPGL2 */
  
  /* Turn all LEDs on full, then fade them out over a few seconds */
  for(u8Index = 20; u8Index > 0; u8Index--)
  {
#ifdef STARTUP_SOUND
    /* Configure Buzzers to provide some audio during start up */
    PWMAudioSetFrequency(BUZZER1, u32Buzzer1Frequency);
    PWMAudioOn(BUZZER1);
#ifdef  EIE1
    PWMAudioSetFrequency(BUZZER2, u32Buzzer2Frequency);
    PWMAudioOn(BUZZER2);
#endif /* EIE1 */
#endif /* STARTUP_SOUND */
    
    /* Spend a little bit of time in each level of intensity */
    for(u16 j = 20; j > 0; j--)
    {
      u32Timer = G_u32SystemTime1ms;
      while( !IsTimeUp(&u32Timer, 1) );
      LedUpdate();
    }
    /* Pause for a bit on the first iteration to show the LEDs on for little while */
    if(u8Index == 20)
    {
      while( !IsTimeUp(&u32Timer, 200) );
    }
    
    /* Set the LED intensity for the next iteration */
    for(u8 j = 0; j < TOTAL_LEDS; j++)
    {
      Leds_asLedArray[j].eRate = (LedRateType)(u8Index - 1);
    }
    
    /* Set the buzzer frequency for the next iteration */
    u32Buzzer1Frequency -= u32StepSize;
    u32Buzzer2Frequency += u32StepSize;
  }

  /* Final update to set last state, hold for a short period */
  LedUpdate();
  while( !IsTimeUp(&u32Timer, 200) );
  
#ifdef STARTUP_SOUND
  /* Turn off the buzzers */
  PWMAudioOff(BUZZER1);
#ifdef  EIE1
  PWMAudioOff(BUZZER2);
#endif /* EIE1 */
  
#endif /* STARTUP_SOUND */

 
  /* The discrete LEDs are off and the backlight is on (white) -- this
  is how we will exit the LED init.  But should we set all the LEDs to
  NORMAL mode?  This would solve the LedToggle() problem described in 
  LedBasic module.  So if the code below is added, then the module
  information must be updated. */
#if 0 
  for(u8 i = 0; i < TOTAL_LEDS; i++)
  {
    Leds_asLedArray[i].eMode = LED_NORMAL_MODE;
  }
#endif

#ifdef EIE1
  LedOn(LCD_RED);
  LedOn(LCD_GREEN);
  LedOn(LCD_BLUE);
#endif
  
#ifdef MPGL2
  LedOn(LCD_BL);
#endif

  /* Final setup and report that LED system is ready */
  G_u32ApplicationFlags |= _APPLICATION_FLAGS_LED;
  DebugPrintf(au8LedStartupMsg);
  
} /* end LedInitialize() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Private functions */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: LedUpdate

Description:
Update all LEDs for the current cycle.

Requires:
 - G_u32SystemTime1ms is counting

Promises:
   - All LEDs updated based on their counters
*/
void LedUpdate(void)
{
	/* Loop through each LED */
  for(u8 i = 0; i < TOTAL_LEDS; i++)
  {
    /* Check if LED is PWMing */
    if(Leds_asLedArray[(LedNumberType)i].eMode == LED_PWM_MODE)
    {
      /* Handle special case of 0% duty cycle */
      if( Leds_asLedArray[i].eRate == LED_PWM_0 )
      {
        LedOff( (LedNumberType)i );
      }
      
      /* Handle special case of 100% duty cycle */
      else if( Leds_asLedArray[i].eRate == LED_PWM_100 )
      {
        LedOn( (LedNumberType)i );
      }
  
      /* Otherwise, regular PWM: decrement counter; toggle and reload if counter reaches 0 */
      else
      {
        if(--Leds_asLedArray[(LedNumberType)i].u16Count == 0)
        {
          if(Leds_asLedArray[(LedNumberType)i].eCurrentDuty == LED_PWM_DUTY_HIGH)
          {
            /* Turn the LED off and update the counters for the next cycle */
            LedOff( (LedNumberType)i );
            Leds_asLedArray[(LedNumberType)i].u16Count = LED_PWM_PERIOD - Leds_asLedArray[(LedNumberType)i].eRate;
            Leds_asLedArray[(LedNumberType)i].eCurrentDuty = LED_PWM_DUTY_LOW;
          }
          else
          {
            /* Turn the LED on and update the counters for the next cycle */
            LedOn( (LedNumberType)i );
            Leds_asLedArray[i].u16Count = Leds_asLedArray[i].eRate;
            Leds_asLedArray[i].eCurrentDuty = LED_PWM_DUTY_HIGH;
          }
        }
      }

      /* Set the LED back to PWM mode since LedOff and LedOn set it to normal mode */
     	Leds_asLedArray[(LedNumberType)i].eMode = LED_PWM_MODE;
      
    } /* end if PWM mode */
    
    /* LED is in LED_BLINK_MODE mode */
    else if(Leds_asLedArray[(LedNumberType)i].eMode == LED_BLINK_MODE)
    {
      /* Decrement counter; toggle and reload if counter reaches 0 */
      if( --Leds_asLedArray[(LedNumberType)i].u16Count == 0)
      {
        LedToggle( (LedNumberType)i );
        Leds_asLedArray[(LedNumberType)i].u16Count = Leds_asLedArray[(LedNumberType)i].eRate;
      }
    }
  } /* end for */
} /* end LedUpdate() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/


