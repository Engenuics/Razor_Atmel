/***********************************************************************************************************************
File: main.h

Description:
Header file for main.c.
*******************************************************************************/

#ifndef __MAIN_H
#define __MAIN_H

/***********************************************************************************************************************
* Firmware Version - Printed FIRMWARE_MAIN_REV.FIRMWARE_SUB_REV1 FIRMWARE_SUB_REV2
* See releasenotes.txt for firmware details.
***********************************************************************************************************************/
#define FIRMWARE_MAIN_REV               '0'
#define FIRMWARE_SUB_REV1               '0'
#define FIRMWARE_SUB_REV2               '1'


/***********************************************************************************************************************
* Constant Definitions
***********************************************************************************************************************/
#define MAX_DRINKS          (u8)3       /* Maximum number of drinks a server can hold */
#define MAX_SERVERS         (u8)3       /* Maximum number of servers */
#define DRINK_SERVE_TIME    (u32)30000  /* Loop iterations before drink is removed */


/***********************************************************************************************************************
* Type Definitions
***********************************************************************************************************************/
typedef enum {EMPTY, BEER, SHOOTER, WINE, HIBALL} DrinkType;

typedef struct
{
  u8 u8ServerNumber;                    /* Unique token for this item */
  DrinkType asServingTray[MAX_DRINKS];  /* Data payload array */
  void* psNextServer;                   /* Pointer to next ServerType*/
} ServerType;


/***********************************************************************************************************************
* Function declarations
***********************************************************************************************************************/
bool InitializeServer(ServerType** psServer_);
bool CreateServer(ServerType** psServerList_);


#endif /* __MAIN_H */