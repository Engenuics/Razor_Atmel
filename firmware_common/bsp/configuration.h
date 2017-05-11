/*!*********************************************************************************************************************
@file configuration.h      
@brief Main configuration header file for project.  

This file bridges many of the generic features of the firmware to the 
specific features of the design. The definitions should be updated
to match the target hardware.  
 
Bookmarks:


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


#endif /* __CONFIG_H */
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/

