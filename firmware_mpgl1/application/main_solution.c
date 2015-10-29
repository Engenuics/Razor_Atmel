/***********************************************************************************************************************
File: main.c                                                                

Description:
Container for the MPG firmware.  
***********************************************************************************************************************/

#include "configuration.h"

#ifdef SOLUTION
/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_"
***********************************************************************************************************************/
/* New variables */

/*--------------------------------------------------------------------------------------------------------------------*/
/* External global variables defined in other files (must indicate which file they are defined in) */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "Main_" and be declared as static.
***********************************************************************************************************************/
static u8 Main_u8Servers = 0;                  /* Number of people slinging drinks */


/***********************************************************************************************************************
Main Program
***********************************************************************************************************************/

void main(void)
{
  u32 u32UselessVariableForExample;
  u32 au32BigArray[] = {5, 4, 3, 2, 1};
  DrinkType aeDrinkArray[3] = {BEER, SHOOTER};
  ServerType* psServerList = 0;

  /* Pointer example code */
  u8 u8Test = 0xA5;
  u8* pu8Example;
  u32 u32Test = 0x0000ffff;
  u32* pu32Example;

  /* Load the addresses into our pointer variables */
  pu8Example = &u8Test;
  pu32Example = &u32Test;

  /* Access the variables via the pointers (two different ways) */
  *pu8Example += 1;
  (*pu32Example)++;

  /* Move the pointers (watch out for the second one!) */
  pu8Example++;
  *pu32Example++;
  
  /* Struct example code */
  u8 u8CurrentServer;
  ServerType sServer1;
  ServerType* psServerParser;

  psServerParser = &sServer1;
  sServer1.u8ServerNumber = 18;
  u8CurrentServer = psServerParser->u8ServerNumber;

  while(1)
  {
    
    
  } /* end while(1) main super loop */
  
} /* end main() */


/***********************************************************************************************************************
* Function definitions
***********************************************************************************************************************/

/*---------------------------------------------------------------------------------------------------------------------/
Function InitializeServer

Description:
Initializes a new server.  A new server has an empty tray of drinks and is
assigned the next number available.

Requires:
  - psServer_ points to the server to be initialized
  - Main_u8Servers holds the current number of active servers

Promises:
  - Returns TRUE if the server is initialized
  - Returns FALSE if the server cannot be initialized as there are too many
*/
bool InitializeServer(ServerType* psServer_)
{
  /* Check that we have are not at the maximum server limit */
  if(Main_u8Servers == 255)
  {
    return(FALSE);
  }

  /* Room for more servers so add this new one */
  Main_u8Servers++;
  psServer_->u8ServerNumber = Main_u8Servers;

  /* Load up some drinks */
  for(u8 i = 0; i < MAX_DRINKS; i++)
  {
    psServer_->asServingTray[i] = EMPTY;
  }

  return(TRUE);
  
} /* end InitializeServer() */


/*---------------------------------------------------------------------------------------------------------------------/
Function CreateServer

Description:
Creates a new, unitialized server object.

Requires:
  - psServerList_ points to the server list where the server is added.
  - Needs enough heap space to create a ServerType object

Promises:
  - Returns TRUE if the server is created; the new server object is added to the end of psServerList_ 
  - Returns FALSE if the server cannot be created
*/
bool CreateServer(ServerType* psServerList_)
{
  ServerType* psNewServer = 0;
  ServerType* pServerParser;
  
  /* Try to create the server object */
  psNewServer = malloc( sizeof(ServerType) );
  
  /* Check that we have are not at the maximum server limit */
  if(psNewServer == NULL)
  {
    return(FALSE);
  }

  /* Server created successfully, so add to the list */
  pServerParser = psServerList_;
  while(pServerParser->psNextServer != NULL)
  {
    pServerParser = pServerParser->psNextServer;
  }
  
  /* pServerParser is now pointing at the end node update the end pointer to the new node */
  pServerParser->psNextServer = psNewServer;

  return(TRUE);
  
} /* end CreateServer() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
#endif /* SOLUTION */