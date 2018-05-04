/*!**********************************************************************************************************************
@file leds.c                                                                
@brief LED driver and API

This driver provides on, off, toggle, blink and PWM functionality.
The basic on/off/toggle functionality is applied directly to the LEDs.
Blinking and PWMing of LEDs rely on the EIE operating system to provide timing at
regular 1ms calls to LedUpdate().

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- LedNumberType:

  EIE1  {WHITE, PURPLE, BLUE, CYAN, 
         GREEN, YELLOW, ORANGE, RED, 
         LCD_RED, LCD_GREEN, LCD_BLUE}

  MPGL2 {BLUE0, BLUE1, BLUE2, BLUE3, 
         GREEN0, GREEN1, GREEN2, GREEN3, 
         RED0, RED1, RED2, RED3, LCD_BL}

  MPGL2_R01 {BLUE, GREEN, YELLOW, RED, LCD_BL}

- LedRateType:

  {LED_0_5HZ = 1000, LED_1HZ = 500, LED_2HZ = 250, LED_4HZ = 125, LED_8HZ = 63,

  LED_PWM_0 = 0, LED_PWM_5 = 1, LED_PWM_10 = 2, LED_PWM_15 = 3, LED_PWM_20 = 4, 
  LED_PWM_25 = 5, LED_PWM_30 = 6, LED_PWM_35 = 7, LED_PWM_40 = 8, LED_PWM_45 = 9, 
  LED_PWM_50 = 10, LED_PWM_55 = 11, LED_PWM_60 = 12, LED_PWM_65 = 13, LED_PWM_70 = 14, 
  LED_PWM_75 = 15, LED_PWM_80 = 16, LED_PWM_85 = 17, LED_PWM_90 = 18, LED_PWM_95 = 19, 
  LED_PWM_100 = 20} 

*Note that PWM values in LedRateType are continuous, thus stepping a variable of 
LedRateType by one will select the next PWM level. However, decrementing past 
LED_PWM_0 or incrementing past LED_PWM_100 is undefined.

PUBLIC FUNCTIONS
- void LedOn(LedNumberType eLED_)
- void LedOff(LedNumberType eLED_)
- void LedToggle(LedNumberType eLED_)
- void LedBlink(LedNumberType eLED_, LedRateType eBlinkRate_)
- void LedPWM(LedNumberType eLED_, LedRateType ePwmRate_)

PROTECTED FUNCTIONS
- void LedInitialize(void)
- void LedUpdate(void)

***********************************************************************************************************************/

#include "configuration.h"


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>Led"
***********************************************************************************************************************/
/*--------------------------------------------------------------------------------------------------------------------*/
/* New variables (all shall start with G_xxLed*/


/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */
extern volatile u32 G_u32SystemTime1ms;                /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                 /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                  /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;             /*!< @brief From main.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Led_<type>" and be declared as static.
***********************************************************************************************************************/

/************ %LED% EDIT BOARD-SPECIFIC GPIO DEFINITIONS BELOW ***************/

#ifdef EIE1
/*! LED locations: order must correspond to the order set in LedNumberType in the header file. */
static u32 Led_au32BitPositions[] = {PB_13_LED_WHT, PB_14_LED_PRP, PB_18_LED_BLU, PB_16_LED_CYN,
                                     PB_19_LED_GRN, PB_17_LED_YLW, PB_15_LED_ORG, PB_20_LED_RED,
                                     PB_10_LCD_BL_RED, PB_11_LCD_BL_GRN, PB_12_LCD_BL_BLU};

/*! Control array for all LEDs in system initialized for LedInitialize().  Array values correspond to LedConfigType fields: 
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
/*! LED locations: order must correspond to the order set in LedNumberType in the header file. */
static u32 Led_au32BitPositions[] = {PB_18_LED_BLU, PB_19_LED_GRN, PB_17_LED_YLW, PB_20_LED_RED, PB_11_LCD_BL};

/*! Control array for all LEDs in system initialized for LedInitialize().  Array values correspond to LedConfigType fields: 
     eMode         eRate      u16Count       eCurrentDuty     eActiveState     ePort      LedNumberType */
static LedConfigType Leds_asLedArray[TOTAL_LEDS] = 
{{LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* BLUE       */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* GREEN      */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* YELLOW     */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* RED        */
 {LED_PWM_MODE, LED_PWM_100, LED_PWM_100, LED_PWM_DUTY_HIGH, LED_ACTIVE_HIGH, LED_PORTB}, /* LCD_BL     */
};   
#else
/*! LED locations: order must correspond to the order set in LedNumberType in the header file. */
static u32 Led_au32BitPositions[] = {PB_20_LED0_RED, PB_17_LED1_RED, PB_19_LED2_RED, PB_18_LED3_RED,
                                     PA_29_LED0_GRN, PB_02_LED1_GRN, PA_26_LED2_GRN, PA_07_LED3_GRN,
                                     PB_01_LED0_BLU, PB_13_LED1_BLU, PA_06_LED2_BLU, PA_08_LED3_BLU,
                                     PB_05_LCD_BL};

/*! Control array for all LEDs in system initialized for LedInitialize().  Array values correspond to LedConfigType fields: 
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
 

/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn void LedOn(LedNumberType eLED_)

@brief Turn the specified LED on.  

This function automatically takes care of the active low vs. active high LEDs.
The function works immediately (it does not require the main application
loop to be running). 

Currently it only supports one LED at a time.

Example:

LedOn(BLUE);


Requires:
- Definitions in Leds_asLedArray[eLED_] are correct

@param eLED_ is a valid LED index

Promises:
- eLED_ is turned on 
- eLED_ is set to LED_NORMAL_MODE mode

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


/*!----------------------------------------------------------------------------------------------------------------------
@fn void LedOff(LedNumberType eLED_)

@brief Turn the specified LED off.

This function automatically takes care of the active low vs. active high LEDs.
It works immediately (it does not require the main application
loop to be running). 

Currently it only supports one LED at a time.

Example:

LedOff(BLUE);


Requires:
- Definitions in Leds_asLedArray[eLED_] are correct

@param eLED_ is a valid LED index

Promises:
- eLED_ is turned off 
- eLED_ is set to LED_NORMAL_MODE mode

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


/*!----------------------------------------------------------------------------------------------------------------------
@fn void LedToggle(LedNumberType eLED_)

@brief Toggles the specified LED from on to off or vise-versa.

LED must be in NORMAL mode or else the state may change due to PWM or blinking.
This function automatically takes care of the active low vs. active high LEDs.
It works immediately (it does not require the main application
loop to be running). 

Currently it only supports one LED at a time.

Example:

LedToggle(BLUE);


Requires:
- Write access to PIOx_ODSR is enabled
- eLED_ *should* be in LED_NORMAL_MODE

@param eLED_ is a valid LED index

Promises:
- eLED_ is toggled if the LED is in LED_NORMAL_MODE mode.  If 
not in NORMAL mode, the behaviour is undefined.

*/
void LedToggle(LedNumberType eLED_)
{
  u32 *pu32Address = (u32*)(&(AT91C_BASE_PIOA->PIO_ODSR) + Leds_asLedArray[eLED_].ePort);

  *pu32Address ^= Led_au32BitPositions[ (u8)eLED_ ];
  
} /* end LedToggle() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void LedBlink(LedNumberType eLED_, LedRateType eBlinkRate_)

@brief Sets an LED to BLINK mode with the rate given.

BLINK mode requires the main loop to be running at 1ms period. If the main
loop timing is regularly off, the blinking timing may be affected although
unlikely to a noticeable degree.  Use LedOff(eLED_) to stop PWM mode and 
return to NORMAL mode.

Example to blink the PURPLE LED at 1Hz:

LedBlink(PURPLE, LED_1HZ);


Requires:
@param eLED_ is a valid LED index
@param eBlinkRate_ is an allowed blinking rate - use a LED_frequency from LedRateType

Promises:
- eLED_ is set to PWM mode at the duty cycle rate specified

*/
void LedBlink(LedNumberType eLED_, LedRateType eBlinkRate_)
{
	Leds_asLedArray[(u8)eLED_].eMode = LED_BLINK_MODE;
	Leds_asLedArray[(u8)eLED_].eRate = eBlinkRate_;
	Leds_asLedArray[(u8)eLED_].u16Count = eBlinkRate_;

} /* end LedBlink() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void LedPWM(LedNumberType eLED_, LedRateType ePwmRate_)

@brief Sets an LED to PWM mode with the rate given.

The PWM output is bit-bashed based on the 1ms system timing.  Therefore,
PWM mode requires the main loop to be running properly. If the main 
loop timing is longer than 1ms, noticeable glitches will be observed
in the PWM signal to the LED.  Even if all applications are working properly,
there still may be some jitter due to applications taking processor time.

Use LedOff(eLED_) to stop PWM mode and return to NORMAL mode.

Example to turn on the BLUE LED with 5% duty cycle:

LedPWM(BLUE, LED_PWM_5);


Requires:
@param eLED_ is a valid LED index
@param ePwmRate_ is an allowed duty cycle - use a PWM value from LedRateType

Promises:
- eLED_ is set to PWM mode at the duty cycle rate specified

*/
void LedPWM(LedNumberType eLED_, LedRateType ePwmRate_)
{
	Leds_asLedArray[(u8)eLED_].eMode = LED_PWM_MODE;
	Leds_asLedArray[(u8)eLED_].eRate = ePwmRate_;
	Leds_asLedArray[(u8)eLED_].u16Count = (u16)ePwmRate_;
  Leds_asLedArray[(u8)eLED_].eCurrentDuty = LED_PWM_DUTY_HIGH;

} /* end LedPWM() */


/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!----------------------------------------------------------------------------------------------------------------------
@fn void LedInitialize(void)

@brief Initialization of LED system parameters and visual LED check.

Use STARTUP_SOUND in configuration.h to enable or disable an irritating
sound that happens when the LEDs are fading.  It is off by default.


Requires:
- All LEDs already initialized to LED_PWM_MODE mode @ LED_PWM_100
- G_u32SystemTime1ms ticking

Promises:
- All discrete LEDs in LED_NORMAL_MODE mode and OFF
- Backlight LED(s) on for white backlight

*/
void LedInitialize(void)
{
  u32 u32Timer;
  u8  u8Index;

  u32 u32Buzzer1Frequency = 4000;
  u32 u32Buzzer2Frequency = 500;
  u32 u32StepSize = (u32Buzzer1Frequency - u32Buzzer2Frequency) / 20;

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
 
  /* Set all the LEDs to NORMAL mode */
  for(u8 i = 0; i < TOTAL_LEDS; i++)
  {
    Leds_asLedArray[i].eMode = LED_NORMAL_MODE;
  }

  /* Backlight on and white */
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
  DebugPrintf("LED functions ready\n\r");
  
} /* end LedInitialize() */


/*!----------------------------------------------------------------------------------------------------------------------
@fn void LedUpdate(void)

@brief Update all LEDs for the current cycle. 

This impacts only LEDs that are set to be blinking or PWM.

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


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
/* Currently this task does not need to run as a state machine */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/


