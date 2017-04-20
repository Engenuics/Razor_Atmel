/***********************************************************************************************************************
File: captouch.h                                                               

Description:
Header file for captouch.c

DISCLAIMER: THIS CODE IS PROVIDED WITHOUT ANY WARRANTY OR GUARANTEES.  USERS MAY
USE THIS CODE FOR DEVELOPMENT AND EXAMPLE PURPOSES ONLY.  ENGENUICS TECHNOLOGIES
INCORPORATED IS NOT RESPONSIBLE FOR ANY ERRORS, OMISSIONS, OR DAMAGES THAT COULD
RESULT FROM USING THIS FIRMWARE IN WHOLE OR IN PART.

***********************************************************************************************************************/

#ifndef __CAPTOUCH_H
#define __CAPTOUCH_H

#include "configuration.h"


/***********************************************************************************************************************
Type Definitions
***********************************************************************************************************************/
typedef enum {SLIDER0, SLIDER1} SliderNumberType;


/***********************************************************************************************************************
Constants / Definitions
***********************************************************************************************************************/
#define QTOUCH_MEASUREMENT_TIME_MS  (u32)25               /* Interval in ms between captouch sweeps */
#define QTOUCH_DEBUG_PRINT_PERIOD   (u32)200              /* Interval in ms between printing captouch current values */


/***********************************************************************************************************************
* Function Declarations
***********************************************************************************************************************/
/* Public Functions */
void CapTouchOn(void);
void CapTouchOff(void);
u8 CaptouchCurrentHSlidePosition(void);
u8 CaptouchCurrentVSlidePosition(void);


BOOL CapTouchUpdateSensorReadings(u16 u16CurrentTimeMs_);
//u8 u8CapTouchGetButtonValue(ButtonNumberType eButtonNumber_);
u8 u8CapTouchGetSliderValue(SliderNumberType eSliderNumber_);

// For debug use only
void CapTouchGetDebugValues(u8 u8Channel_, u16* pu16Measure_, u16* pu16Reference_);

/* Protected Functions */
ErrorStatus CapTouchInitialize(void);
void CapTouchRunActiveState(void);

/* Private Functions */
static ErrorStatus CapTouchVerify(void);
static void CapTouchSetParameters(void);


/***********************************************************************************************************************
State Machine Declarations
***********************************************************************************************************************/
void CapTouchSM_Idle(void);             
void CapTouchSM_Measure(void);

void CapTouchSM_Error(void);         



#endif /* __CAPTOUCH_H */
