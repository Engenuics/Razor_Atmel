/*!**********************************************************************************************************************
@file main.h
@brief Header file for main.c.
***********************************************************************************************************************/

#ifndef __MAIN_H
#define __MAIN_H

/*!**********************************************************************************************************************
Firmware Version - Printed FIRMWARE_MAIN_REV.FIRMWARE_SUB_REV1 FIRMWARE_SUB_REV2

FIRMWARE_MAIN_REV: should be updated only with RELEASES to Github.

FIRMWARE_SUB_REV1: should be updated when a pull request is made to the MASTER branch.
***********************************************************************************************************************/

#define FIRMWARE_VERSION     {'F','i','r','m','w','a','r','e',' ','v','e','r','s','i','o','n',':',' ', \
                             FIRMWARE_MAIN_REV, '.', FIRMWARE_SUB_REV1, FIRMWARE_SUB_REV2,'\n','\n','\r','\0'}

#define FIRMWARE_MAIN_REV    '1'               
#define FIRMWARE_SUB_REV1    '0'
#define FIRMWARE_SUB_REV2    '0'


/***********************************************************************************************************************
* Constant Definitions
***********************************************************************************************************************/

/* G_u32SystemFlags */
#define _SYSTEM_SLEEPING                (u32)0x00000001   /*!< G_u32SystemFlags set into sleep mode to go back to sleep if woken before 1ms period */
#define _SYSTEM_INITIALIZING            (u32)0x00000002   /*!< G_u32SystemFlags set when system is in initialization phase */


/**********************************************************************************************************************
Function Declarations
**********************************************************************************************************************/




#endif /* __MAIN_H */
/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
