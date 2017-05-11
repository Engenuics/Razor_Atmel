/*!**********************************************************************************************************************
@file leds.h                                                               
@brief Header file for leds.c
**********************************************************************************************************************/

#ifndef __LEDS_H
#define __LEDS_H

#include "configuration.h"

/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/

/*! 
@enum LedNumberType
@brief Logical names for LEDs in the system.

The order of the LEDs in LedNumberType must match the order of the definition 
in Led_au32BitPositions from leds.c 
*/
#ifdef EIE1
typedef enum {WHITE = 0, PURPLE, BLUE, CYAN, GREEN, YELLOW, ORANGE, RED, LCD_RED, LCD_GREEN, LCD_BLUE} LedNumberType;
#endif

#ifdef MPGL2

#ifdef MPGL2_R01
typedef enum {BLUE = 0, GREEN, YELLOW, RED, LCD_BL} LedNumberType;
#else
typedef enum {RED0 = 0, RED1, RED2, RED3, GREEN0, GREEN1, GREEN2, GREEN3, BLUE0, BLUE1, BLUE2, BLUE3, LCD_BL} LedNumberType;
#endif /* MPGL2_R01 */

#endif /* MPGL2 */

/*! 
@enum LedRateType
@brief Standard blinky values for blinking AND PWM.  

Other blinking rate values may be added as required.  The values are the toggling period in ms.

*** The PWM rates are set up to allow incrementing and decrementing rates within the allowed values.  Be careful.
*/
typedef enum {LED_0_5HZ = 1000, LED_1HZ = 500, LED_2HZ = 250, LED_4HZ = 125, LED_8HZ = 63,
              LED_PWM_0 = 0, LED_PWM_5 = 1, LED_PWM_10 = 2, LED_PWM_15 = 3, LED_PWM_20 = 4, 
              LED_PWM_25 = 5, LED_PWM_30 = 6, LED_PWM_35 = 7, LED_PWM_40 = 8, LED_PWM_45 = 9, 
              LED_PWM_50 = 10, LED_PWM_55 = 11, LED_PWM_60 = 12, LED_PWM_65 = 13, LED_PWM_70 = 14, 
              LED_PWM_75 = 15, LED_PWM_80 = 16, LED_PWM_85 = 17, LED_PWM_90 = 18, LED_PWM_95 = 19, 
              LED_PWM_100 = 20
             } LedRateType;

/*! @cond DOXYGEN_EXCLUDE */
/* The remaining enums are only used privately to improve self-documentation of the driver */
typedef enum {LED_NORMAL_MODE, LED_PWM_MODE, LED_BLINK_MODE} LedModeType;  /*!< @enum @brief The mode determines how the task manages the LED */
typedef enum {LED_PORTA = 0, LED_PORTB = 0x80} LedPortType;                /*!< @enum @brief Offset between port registers (in 32 bit words) */
typedef enum {LED_ACTIVE_LOW = 0, LED_ACTIVE_HIGH = 1} LedActiveType;      /*!< @enum @brief ACTIVE_HIGH means the LED is on when the pin is at Vcc - hardware dependent */
typedef enum {LED_PWM_DUTY_LOW = 0, LED_PWM_DUTY_HIGH = 1} LedPWMDutyType; /*!< @enum @brief Duty cycle state when tracking */
/*! @endcond */


/*! 
@struct LedConfigType
@brief Required parameters for the task to track for each LED. 
*/
typedef struct 
{
  LedModeType eMode;              /*!< @brief Current mode */
  LedRateType eRate;              /*!< @brief Current rate */
  u16 u16Count;                   /*!< @brief Value of current duty cycle counter */
  LedPWMDutyType eCurrentDuty;    /*!< @brief Phase of the current duty cycle */
  LedActiveType eActiveState;     /*!< @brief LED hardware active type */
  LedPortType ePort;              /*!< @brief LED port position */
}LedConfigType;


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/

/*------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
void LedOn(LedNumberType eLED_);
void LedOff(LedNumberType eLED_);
void LedToggle(LedNumberType eLED_);
void LedBlink(LedNumberType eLED_, LedRateType ePwmRate_);
void LedPWM(LedNumberType eLED_, LedRateType ePwmRate_);


/*------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/
void LedInitialize(void);
void LedUpdate(void);


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/


/**********************************************************************************************************************
Constants / Definitions
**********************************************************************************************************************/
/*! @cond DOXYGEN_EXCLUDE */

#define LED_PWM_PERIOD      (u8)20    /*!< @brief Period of the PWM cycle */

/*! @endcond */




#endif /* __LEDS_H */
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
