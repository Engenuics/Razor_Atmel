/***********************************************************************************************************************
File: captouch.c

Description:
Driver for Capacitive Touch sensors. This code supports the horizontal slider on the MPG2 board
and any future buttons/sliders that may be added.

------------------------------------------------------------------------------------------------------------------------
API:
void CapTouchOn(void)
Activates the Captouch sensors

void CapTouchOff(void)
De-activates the Captouch sensors

u8 CaptouchCurrentHSlidePosition(void)
Returns the current horizontal slider value where "0" is the furthest left and "255" is the furthest right on the slider.

u8 CaptouchCurrentVSlidePosition(void)
Returns the current vertical slider value where "0" is the location near the bottom of the board and "255" is the top.


------------------------------------------------------------------------------------------------------------------------
DISCLAIMER: THIS CODE IS PROVIDED WITHOUT ANY WARRANTY OR GUARANTEES.  USERS MAY
USE THIS CODE FOR DEVELOPMENT AND EXAMPLE PURPOSES ONLY.  ENGENUICS TECHNOLOGIES
INCORPORATED IS NOT RESPONSIBLE FOR ANY ERRORS, OMISSIONS, OR DAMAGES THAT COULD
RESULT FROM USING THIS FIRMWARE IN WHOLE OR IN PART.
***********************************************************************************************************************/

#include "configuration.h"
#include "libqtouch.h"


#define GET_SENSOR_STATE(SENSOR_NUMBER) (qt_measure_data.qt_touch_status.sensor_states[(SENSOR_NUMBER/8)] & (1 << (SENSOR_NUMBER % 8)))
#define GET_ROTOR_SLIDER_POSITION(ROTOR_SLIDER_NUMBER) qt_measure_data.qt_touch_status.rotor_slider_values[ROTOR_SLIDER_NUMBER]


/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_CapTouch"
***********************************************************************************************************************/


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemFlags;                  /* From main.c */
extern volatile u32 G_u32ApplicationFlags;             /* From main.c */

extern volatile u32 G_u32SystemTime1ms;                /* From board-specific source file */
extern volatile u32 G_u32SystemTime1s;                 /* From board-specific source file */

/* measurement data */
extern qt_touch_lib_measure_data_t qt_measure_data;

extern u32 G_u32DebugFlags;                            /* From debug.c */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "CapTouch_" and be declared as static.
***********************************************************************************************************************/
/* New variables */
fnCode_type CapTouch_pfnStateMachine;                  /* The state machine function pointer */

u32 CapTouch_u32Timer;                                 /* Measurement timer */        
u8 CapTouch_u8CurrentHSliderValue;                     /* Current horizontal slider value */
u8 CapTouch_u8CurrentVSliderValue;                     /* Current vertical slider value */


/***********************************************************************************************************************
Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/* Public functions                                                                                                   */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: CapTouchOn

Description:
Sets the state machine to "Measuring" so that the captouch functionality is active.  This is fairly processor intensive.

Requires:

Promises:
  - Horizontal CapTouch sensor is active

*/
void CapTouchOn(void)
{
  CapTouch_pfnStateMachine = CapTouchSM_Measure;
  
} /* end CapTouchOn() */


/*----------------------------------------------------------------------------------------------------------------------
Function: CapTouchOff

Description:
Sets the state machine to "Idle" so that the captouch functionality is disabled.

Requires:

Promises:
  - Horizontal CapTouch sensor is not active
*/
void CapTouchOff(void)
{
  CapTouch_pfnStateMachine = CapTouchSM_Idle;
  CapTouch_u8CurrentHSliderValue = 0;
  CapTouch_u8CurrentVSliderValue = 0;
  
} /* end CapTouchOff() */


/*----------------------------------------------------------------------------------------------------------------------
Function: CaptouchCurrentHSlidePosition

Description:
Returns the current slider count (0-254)

Requires:

Promises:
  - Returns the current slider count (0-254)
*/
u8 CaptouchCurrentHSlidePosition(void)
{
  return CapTouch_u8CurrentHSliderValue;
  
} /* end CaptouchCurrentHSlidePosition() */


/*----------------------------------------------------------------------------------------------------------------------
Function: CaptouchCurrentVSlidePosition

Description:
Returns the current slider count (0-254)

Requires:

Promises:
  - Returns the current slider count (0-254)
*/
u8 CaptouchCurrentVSlidePosition(void)
{
  return CapTouch_u8CurrentVSliderValue;
  
} /* end CaptouchCurrentVSlidePosition() */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Protected functions                                                                                                */
/*--------------------------------------------------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------------------------------------------------
Function: CapTouchInitialize

Description:
This function initializes the module. The library version is checked, the
buttons being used are initialized, and the configuration parameters are set.

The qt_xxx functions are part of the QTouch API.

Requires:

Promises:
  - Returns ERROR if the library doesn't match what's expected, else SUCCESS

*/
ErrorStatus CapTouchInitialize(void)
{
	/* Ensure library is correct */
	if (CapTouchVerify() != SUCCESS)
	{
		return (ERROR);
	}

	/* Reset touch sensing */
	qt_reset_sensing();

  /* Configure the GPIO used for the Horizontal Slider */
  qt_enable_slider( CHANNEL_0, CHANNEL_2, NO_AKS_GROUP, 50u, HYST_50, RES_8_BIT, 0u  );

  /* Configure the GPIO used for the Vertical Slider */
  qt_enable_slider( CHANNEL_19, CHANNEL_21, NO_AKS_GROUP, 50u, HYST_50, RES_8_BIT, 0u  );

  /* Initialise and calibrate touch sensing (all sensors must be enabled first)*/
  qt_init_sensing();
  qt_calibrate_sensing();

  /* Set the parameters like recalibration threshold, Max_On_Duration etc .
  These are user configurable */
  CapTouchSetParameters( );

  /* Address to pass address of user functions */
  /* This function is called after the library has made capacitive measurements,
  but before it has processed them. The user can use this hook to apply filter
  functions to the measured signal values.(Possibly to fix sensor layout faults) */
  qt_filter_callback = 0;

  CapTouch_u32Timer = 0;
  CapTouch_u8CurrentHSliderValue = 0;
  CapTouch_u8CurrentVSliderValue = 0;
  
  CapTouch_pfnStateMachine = CapTouchSM_Idle;
  G_u32ApplicationFlags |= _APPLICATION_FLAGS_CAPTOUCH;
  return (SUCCESS);
}


/*----------------------------------------------------------------------------------------------------------------------
Function CapTouchRunActiveState()

Description:
Selects and runs one iteration of the current state in the state machine.
All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
  - State machine function pointer points at current state

Promises:
  - Calls the function to pointed by the state machine function pointer
*/
void CapTouchRunActiveState(void)
{
  CapTouch_pfnStateMachine();

} /* end CapTouchRunActiveState */


/*----------------------------------------------------------------------------------------------------------------------
Function: CapTouchVerify

Description:
This function verifies the captouch library that's being used and makes sure
the capabilities (and version) match what's expected and required.

Requires:

Promises:
  - Returns ERROR if the library doesn't match what's expected, else SUCCESS

*/
ErrorStatus CapTouchVerify(void)
{
    u32 u32Error = 0;

    qt_lib_siginfo_t plib_sig;

    /* Read library signature info */
    qt_measure_sensors( 0 );
    qt_get_library_sig(&plib_sig);

    /* QTouch or Matrix */
    if((plib_sig.lib_sig_lword & 0x01) == 0)
    {
        DebugPrintf("Library QTouch ");
    }
    else
    {
        DebugPrintf("Library QMatrix ");
        u32Error++;
    }

    /* Max channels should be 32 */
    if ( ((plib_sig.lib_sig_lword >> 3) & 0x7F) != 32)
    {
       u32Error++;
    }

    /* Should supoprt sliders/roters */
    if( ((plib_sig.lib_sig_lword >> 10) & 0x01) == 0)
    {
        DebugPrintf("supports only keys \r\n");
        u32Error++;
    }
    else
    {
        DebugPrintf("supports keys and rotors/sliders. ");
    }

    /* Should supoprt 8 sliders */
    if ( ((plib_sig.lib_sig_lword >> 11) & 0x1F) != 8 )
    {
        u32Error++;
    }

    /* Current version: $0440 */
    DebugPrintf("Version ");
    DebugPrintNumber(plib_sig.library_version);
    DebugLineFeed();
    if ( (plib_sig.library_version) != 0x440)
    {
       u32Error++;
    }

    /* if error count = 0, all is good */
    if (u32Error == 0)
    {
       return (SUCCESS);
    }

    return (ERROR);
}


/*----------------------------------------------------------------------------------------------------------------------
Function: CapTouchSetParameters

Description:

This will fill the default threshold values in the configuration
data structure. These values can be tweaked to optimize performance.

Requires:
  - DEF_QT_xxx values are defined in touch_api.h

Promises:
  -

*/
static void CapTouchSetParameters( void )
{
    /*  This can be modified by the user to different values   */
    qt_config_data.qt_di              = DEF_QT_DI;
    qt_config_data.qt_neg_drift_rate  = DEF_QT_NEG_DRIFT_RATE;
    qt_config_data.qt_pos_drift_rate  = DEF_QT_POS_DRIFT_RATE;
    qt_config_data.qt_max_on_duration = DEF_QT_MAX_ON_DURATION;
    qt_config_data.qt_drift_hold_time = DEF_QT_DRIFT_HOLD_TIME;
    qt_config_data.qt_recal_threshold = DEF_QT_RECAL_THRESHOLD;
    qt_config_data.qt_pos_recal_delay = DEF_QT_POS_RECAL_DELAY;
}


/*----------------------------------------------------------------------------------------------------------------------
Function: CapTouchUpdateSensorReadings

Description:

This is the main read routine to get the cap touch sensor
readings. It should be called periodically every 25ms.

Requires:
  - u16CurrentTimeMs is the current value of the tick timer
    (ideally in increments of 25ms)

Promises:
  - returns TRUE for successful read
  - returns FALSE if read times out !!!!! TODO 

*/
BOOL CapTouchUpdateSensorReadings(u16 u16CurrentTimeMs_)
{
  /*status flags to indicate the re-burst for library*/
  uint16_t status_flag = 0u;
  uint16_t burst_flag = 0u;

	do
	{
		/*  one time measure touch sensors    */
		status_flag = qt_measure_sensors( u16CurrentTimeMs_ );

		burst_flag = status_flag & QTLIB_BURST_AGAIN;

	} while (burst_flag);

    return (TRUE);  // TODO: add timeout to return false
}

#if 0
/*----------------------------------------------------------------------------------------------------------------------
Function: u8CapTouchGetButtonValue

Description:

Call this function after a successful read from CapTouchUpdateSensorReadings
to obtain the value of a button reading.

Requires:
  - eButtonNumber_ is the button to read

Promises:
  - returns 8-bit value for sensor reading

*/
u8 u8CapTouchGetButtonValue(ButtonNumberType eButtonNumber_)
{
    return (GET_SENSOR_STATE(eButtonNumber_));
}
#endif

/*----------------------------------------------------------------------------------------------------------------------
Function: u8CapTouchGetSliderValue

Description:

Call this function after a successful read from CapTouchUpdateSensorReadings
to obtain the value of a slider reading.

Requires:
  - eSliderNumber_ is the slider to read

Promises:
  - returns 8-bit value for sensor reading

*/
u8 u8CapTouchGetSliderValue(SliderNumberType eSliderNumber_)
{
  return (GET_ROTOR_SLIDER_POSITION(eSliderNumber_));
}


/*----------------------------------------------------------------------------------------------------------------------
Function: CapTouchGetDebugValues

Description:

Call this function after a successful read to obtains the raw channel measurement
and reference value. This will indicate what the 'normal' values should be when
the sensor is idle/not active (no finger present) along with the current reading.

Requires:

Promises:
  - *pu16Measure_ returns the value of the current measurement
  - *pu16Reference_ returns the normal, untouched value

*/
void CapTouchGetDebugValues(u8 u8Channel, u16* pu16Measure_, u16* pu16Reference_)
{
  *pu16Measure_ = qt_measure_data.channel_signals[u8Channel];
  *pu16Reference_ = qt_measure_data.channel_references[u8Channel];
}


/***********************************************************************************************************************
State Machine Function Definitions
***********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
void CapTouchSM_Idle(void)
{
  /* Do nothing here as the sensor is off */
  
} /* end CapTouchSM_Idle */


/*--------------------------------------------------------------------------------------------------------------------*/
void CapTouchSM_Measure(void)
{
  static u32 u32DebugPrintTimer = 0;
  static u8 au8CaptouchValuesMessage[] = "Captouch (H:V) "; 
  
  if( IsTimeUp(&CapTouch_u32Timer, QTOUCH_MEASUREMENT_TIME_MS) )
  {
    CapTouch_u32Timer = G_u32SystemTime1ms;
    CapTouchUpdateSensorReadings((u16)G_u32SystemTime1ms);

    /* Read the horizontal slider */
    CapTouch_u8CurrentHSliderValue = u8CapTouchGetSliderValue(SLIDER0);
    
    /* Read the vertical slider */
    CapTouch_u8CurrentVSliderValue = u8CapTouchGetSliderValue(SLIDER1);

  }
  
  /* Print the current values if Debug function is enabled */
  if(G_u32DebugFlags & _DEBUG_CAPTOUCH_VALUES_ENABLE)
  {
    if( IsTimeUp(&u32DebugPrintTimer, QTOUCH_DEBUG_PRINT_PERIOD) )
    {
      u32DebugPrintTimer = G_u32SystemTime1ms;

      DebugPrintf(au8CaptouchValuesMessage);
      DebugPrintNumber(CapTouch_u8CurrentHSliderValue);
      DebugPrintf(":");
      DebugPrintNumber(CapTouch_u8CurrentVSliderValue);
      DebugLineFeed();
    }
   
  }
               
} /* end CapTouchSM_Idle */