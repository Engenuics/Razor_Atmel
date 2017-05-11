/*!*********************************************************************************************************************
@file configuration.h      
@brief Main configuration header file for project.  

This file bridges many of the generic features of the firmware to the 
specific features of the design. The definitions should be updated
to match the target hardware.  
 
Bookmarks:
@@@@@ GPIO board-specific parameters  #GPIOLED


***********************************************************************************************************************/

#ifndef __CONFIG_H
#define __CONFIG_H

/**********************************************************************************************************************
Runtime switches
***********************************************************************************************************************/
//#define MPGL2_R01                   /*!< Use with MPGL2-EHDW-01 revision board */


/**********************************************************************************************************************
Type Definitions
**********************************************************************************************************************/


/**********************************************************************************************************************
Includes
***********************************************************************************************************************/
/* Common header files */
#include <stdlib.h>
#include <string.h>
#include "AT91SAM3U4.h"
#include "exceptions.h"
#include "interrupts.h"
#include "core_cm3.h"
#include "main.h"
#include "typedefs.h"

/* Common driver header files */
#include "leds.h"         /* #GPIOLED */

/* EIEF1-PCB-01 specific header files */
#ifdef EIE1
#include "eief1-pcb-01.h"
#endif /* EIE1 */

#ifdef MPGL2
/* MPGL2-specific header files */
#ifdef MPGL2_R01
#include "mpgl2-ehdw-01.h"
#else
#include "mpgl2-ehdw-02.h"
#endif /* MPGL2_R01 */
#endif /* MPGL2 */

/* Common application header files */
#include "user_app1.h"


/***********************************************************************************************************************
@@@@@ GPIO board-specific parameters
***********************************************************************************************************************/
/*----------------------------------------------------------------------------------------------------------------------
%LED% LED Configuration  #GPIOLED                                                                                       
------------------------------------------------------------------------------------------------------------------------
Update the values below for the LEDs on the board.  Any name can be used for the LED definitions.
Open the LED source.c and edit Led_au32BitPositions and Leds_asLedArray with the correct values for the LEDs in the system.  
*/

#ifdef EIE1
#define TOTAL_LEDS            (u8)11        /*!< Total number of LEDs in the system */
#endif /* EIE1 */


#ifdef MPGL2

#ifdef MPGL2_R01
#define TOTAL_LEDS            (u8)5         /*!< Total number of LEDs in the system */
#else
#define TOTAL_LEDS            (u8)13        /*!< Total number of LEDs in the system */
#endif /* MPGL2_R01 */

#endif /* MPGL2 */



#endif /* __CONFIG_H */
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

