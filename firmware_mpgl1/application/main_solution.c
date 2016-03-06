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
#if 0 /* Set this to 1 to run the example code */
  
  u32 u32UselessVariableForExample;
  u32 au32BigArray[] = {5, 4, 3, 2, 1};
  DrinkType aeDrinkArray[3] = {BEER, SHOOTER};

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
#endif
  
  /* Module exercise solution begins here */
  u32 u32LoopCounter = 0;
  ServerType* psServerList = NULL;
  ServerType* psServerListParser = NULL;
  ServerType* psServerListDoomed = NULL;
  bool bOrderDrink = FALSE;
  DrinkType eDrink = BEER;
  u8 u8EmptyCount;
  const char au8MessageDrinkOrdered[]  = "Drink ordered   ";
  const char au8MessageDrinkServed[]   = "Drink served    ";
  const char au8MessageNewServer[]     = "New server added";
  const char au8MessageNoServer[]      = "No free server  ";
  const char au8MessageServerRemoved[] = "Server removed  ";
  char au8MessageCurrent[]             = "                ";
  bool bNewMessage = FALSE;
  
  while(1)
  {
    /* Watch for a drink order */
    if(bOrderDrink)
    {
      /* Parse through the list to determine if a server has space */
      psServerListParser = psServerList;
      while( (psServerListParser != NULL) && bOrderDrink)
      {
        for(u8 i = 0; i < MAX_DRINKS; i++)
        {
          if(psServerListParser->asServingTray[i] == EMPTY)
          {
            /* Found a space: choose the type of drink based on the current system count */
            eDrink = (DrinkType)((u32LoopCounter % 4) + 1);
            psServerListParser->asServingTray[i] = eDrink;
            strcpy(au8MessageCurrent, au8MessageDrinkOrdered);
            bNewMessage = TRUE;
      
            /* Clear the drink order */
            bOrderDrink = FALSE;
            break;
          }
        } /* end for */
        
        /* Move to next server in the list */
        psServerListParser = psServerListParser->psNextServer;
        
      } /* end while */
      
      /* If bOrderDrink is still TRUE, then the drink order is still waiting so see if another server is available */
      if(bOrderDrink)
      {
        if(CreateServer(&psServerList))
        {
          /* Server was created so initialize and add to list; the drink will be ordered next iteration */
          strcpy(au8MessageCurrent, au8MessageNewServer);
          bNewMessage = TRUE;
        }
        else
        {
          strcpy(au8MessageCurrent, au8MessageNoServer);
          bNewMessage = TRUE;

          /* The order is ignored */
          bOrderDrink = FALSE;
        }
      }
    } /* end if (bOrderDrink) */

        
    /* Check if it is time to remove a drink */
    if(u32LoopCounter % DRINK_SERVE_TIME == 0)
    {
      /* Select one of the servers somewhat randomly based on the number of the last drink ordered */
      psServerListParser = psServerList;
      if(psServerListParser != NULL)
      {
        for(u8 i = 0; i < (u8)eDrink; i++)
        {
          /* Parse through to the server making sure to stay in the list */
          if(psServerListParser->psNextServer != NULL)
          {
            psServerListParser = psServerListParser->psNextServer;
          }
        }
        
        /* Look through the tray to find a drink to remove and check if the tray is empty */
        u8EmptyCount = 0;
        for(u8 i = 0; i < MAX_DRINKS; i++)
        {
          if(psServerListParser->asServingTray[i] != EMPTY)
          {
            /* If this is the first drink found on the tray, it's the one to be removed */
            if( i - u8EmptyCount == 0 )
            {
              /* Remove the drink and queue message */
              psServerListParser->asServingTray[i] = EMPTY;
              u8EmptyCount++;
              strcpy(au8MessageCurrent, au8MessageDrinkServed);
              bNewMessage = TRUE;
            }
          }
          else
          {
            u8EmptyCount++;
          }
        }
        
        /* If the server's tray is now empty, remove the server */
        if(u8EmptyCount == MAX_DRINKS)
        {
          /* Put a pointer on this node as it will be removed and put the parser back to the start of the list */
          psServerListDoomed = psServerListParser;
          psServerListParser = psServerList;
          
          /* Handle if doomed node is first */
          if(psServerListParser == psServerListDoomed)
          {
            psServerList = psServerListDoomed->psNextServer;
          }
          else
          {
            /* Find the node just before the doomed node */
            while(psServerListParser->psNextServer != psServerListDoomed)
            {
              psServerListParser = psServerListParser->psNextServer;
            }
            
            /* Connect the current node to the node after the doomed list */
            psServerListParser->psNextServer = psServerListDoomed->psNextServer;
          }
          
          /* Free the memory of the doomed node */
          free(psServerListDoomed);
          Main_u8Servers--;
          strcpy(au8MessageCurrent, au8MessageServerRemoved);
          bNewMessage = TRUE;
        } /* end if(u8EmptyCount == MAX_DRINKS) */
        
      } /* if(psServerListParser != NULL) */
    } /* if(u32LoopCounter % DRINK_SERVE_TIME == 0) */


    /* Check to see if a new message was added */
    if(bNewMessage)
    {
      /* Clear the variable: set a breakpoint here to halt whenever a new message is posted */
      bNewMessage = FALSE;
    }

    /* Pause for the next iteration - about 1ms of instructions */
    for(u32 i = 0; i < 1700; i++);
    u32LoopCounter++;

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
  - psServer_ points to the server list where a new server is to be initialized; 
    since the server list is a pointer to a linked list, this is a pointer-to-pointer
  - Main_u8Servers holds the current number of active servers

Promises:
  - Returns TRUE if the server is initialized
  - Returns FALSE if the server cannot be initialized
*/
bool InitializeServer(ServerType** psServer_)
{
  if(*psServer_ == NULL)
  {
    return(FALSE);
  }

  (*psServer_)->u8ServerNumber = Main_u8Servers;

  /* Start with an empty tray */
  for(u8 i = 0; i < MAX_DRINKS; i++)
  {
    (*psServer_)->asServingTray[i] = EMPTY;
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
bool CreateServer(ServerType** psServerList_)
{
  ServerType* psNewServer = 0;
  ServerType** pServerParser;
  
  /* Check that we have are not at the maximum server limit */
  if(Main_u8Servers >= MAX_SERVERS)
  {
    return(FALSE);
  }

  /* Try to create the server object */
  psNewServer = malloc( sizeof(ServerType) );
  
  /* Check that we have are not at the maximum server limit */
  if(psNewServer == NULL)
  {
    return(FALSE);
  }

  /* Server created successfully, so initialize and add to the list */
  Main_u8Servers++;
  InitializeServer(&psNewServer);
  pServerParser = psServerList_;
  
  /* If empty list, new node is start of list */
  if(*pServerParser == NULL)
  {
    *pServerParser = psNewServer;
  }
  
  /* If not an empty list, parse through until the last node is found */
  else
  {
    while((*pServerParser)->psNextServer != NULL)
    {
      *pServerParser = (*pServerParser)->psNextServer;
    }

    /* pServerParser is now pointing at the end node update the end pointer to the new node */
    (*pServerParser)->psNextServer = psNewServer;

  }

  return(TRUE);
  
} /* end CreateServer() */



/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File */
/*--------------------------------------------------------------------------------------------------------------------*/
#endif /* SOLUTION */