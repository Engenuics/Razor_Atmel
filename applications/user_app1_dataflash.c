/*!*********************************************************************************************************************
@file user_app1.c                                                                
@brief Reads manufacturer's information from a data flash chip on the Blade SPI

------------------------------------------------------------------------------------------------------------------------
GLOBALS
- NONE

CONSTANTS
- NONE

TYPES
- NONE

PUBLIC FUNCTIONS
- NONE

PROTECTED FUNCTIONS
- void UserApp1Initialize(void)
- void UserApp1RunActiveState(void)


**********************************************************************************************************************/

#include "configuration.h"

/***********************************************************************************************************************
Global variable definitions with scope across entire project.
All Global variable names shall start with "G_<type>UserApp1"
***********************************************************************************************************************/
/* New variables */
volatile u32 G_u32UserApp1Flags;                          /*!< @brief Global state flags */


/*--------------------------------------------------------------------------------------------------------------------*/
/* Existing variables (defined in other files -- should all contain the "extern" keyword) */
extern volatile u32 G_u32SystemTime1ms;                   /*!< @brief From main.c */
extern volatile u32 G_u32SystemTime1s;                    /*!< @brief From main.c */
extern volatile u32 G_u32SystemFlags;                     /*!< @brief From main.c */
extern volatile u32 G_u32ApplicationFlags;                /*!< @brief From main.c */

extern u32 BLADE_SPI_FLAGS;                               /*!< @brief From configuration.h */


/***********************************************************************************************************************
Global variable definitions with scope limited to this local application.
Variable names shall start with "UserApp1_<type>" and be declared as static.
***********************************************************************************************************************/
static fnCode_type UserApp1_pfStateMachine;               /*!< @brief The state machine function pointer */

static SpiPeripheralType* UserApp1_Spi;                   /*!< @brief Pointer to peripheral object */
static SpiConfigurationType UserApp1_eBladeDataflashSPI;

static u8 UserApp1_au8RxBuffer[U16_UA1_RX_BUFFER_SIZE];   /*!< @brief Space for received bytes */
static u8 *UserApp1_pu8RxBufferNextChar;                  /*!< @brief Pointer to next char to be written in the RxBuffer  */
static u8 *UserApp1_pu8RxBufferUnreadChar;                /*!< @brief Pointer to next char in RxBuffer to be read by application */

static u8 UserApp1_au8MessageManufacturerID[] = {0x9F, SPI_DUMMY, SPI_DUMMY, SPI_DUMMY, SPI_DUMMY};
static u32 UserApp1_CurrentMsgToken;


/**********************************************************************************************************************
Function Definitions
**********************************************************************************************************************/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @publicsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------------------*/
/*! @protectedsection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/

/*!--------------------------------------------------------------------------------------------------------------------
@fn void UserApp1Initialize(void)

@brief
Initializes the State Machine and its variables.

Should only be called once in main init section.

Requires:
- NONE

Promises:
- NONE

*/
void UserApp1Initialize(void)
{
  /* Initialize pointers */
  UserApp1_pu8RxBufferUnreadChar = UserApp1_au8RxBuffer;
  UserApp1_pu8RxBufferNextChar = UserApp1_au8RxBuffer;
  
  /* Setup the SPI configuration variable and request the SPI peripheral */
  UserApp1_eBladeDataflashSPI.u32CsPin  = BLADE_CS_PIN;
  UserApp1_eBladeDataflashSPI.eBitOrder = SPI_MSB_FIRST;
  UserApp1_eBladeDataflashSPI.SpiPeripheral = BLADE_SPI;
  UserApp1_eBladeDataflashSPI.pCsGpioAddress = BLADE_BASE_PORT;
  UserApp1_eBladeDataflashSPI.ppu8RxNextByte = &UserApp1_pu8RxBufferNextChar;
  UserApp1_eBladeDataflashSPI.u16RxBufferSize = U16_UA1_RX_BUFFER_SIZE;
  UserApp1_eBladeDataflashSPI.pu8RxBufferAddress = UserApp1_au8RxBuffer;
  
  UserApp1_Spi = SpiRequest(&UserApp1_eBladeDataflashSPI);
  BLADE_SPI_FLAGS = 0;
  
  /* If good initialization, set state to Idle */
  if( UserApp1_Spi != NULL )
  {    
    DebugPrintf("Blade dataflash task ready\n\r");
    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }
  else
  {
    /* The task isn't properly initialized, so shut it down and don't run */
    UserApp1_pfStateMachine = UserApp1SM_Error;
  }

} /* end UserApp1Initialize() */

  
/*!----------------------------------------------------------------------------------------------------------------------
@fn void UserApp1RunActiveState(void)

@brief Selects and runs one iteration of the current state in the state machine.

All state machines have a TOTAL of 1ms to execute, so on average n state machines
may take 1ms / n to execute.

Requires:
- State machine function pointer points at current state

Promises:
- Calls the function to pointed by the state machine function pointer

*/
void UserApp1RunActiveState(void)
{
  UserApp1_pfStateMachine();

} /* end UserApp1RunActiveState */


/*------------------------------------------------------------------------------------------------------------------*/
/*! @privatesection */                                                                                            
/*--------------------------------------------------------------------------------------------------------------------*/


/**********************************************************************************************************************
State Machine Function Definitions
**********************************************************************************************************************/
/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for the message */
static void UserApp1SM_Idle(void)
{
  if( WasButtonPressed(BUTTON0) )
  {
    ButtonAcknowledge(BUTTON0);
    UserApp1_CurrentMsgToken = SpiWriteData(UserApp1_Spi, 
                                        sizeof(UserApp1_au8MessageManufacturerID),
                                        UserApp1_au8MessageManufacturerID);
    UserApp1_pfStateMachine = UserApp1SM_WaitResponse;
  }

} /* end UserApp1SM_Idle() */
     

/*-------------------------------------------------------------------------------------------------------------------*/
/* Wait for the data flash message to finish */
static void UserApp1SM_WaitResponse(void)          
{
  u8 u8ManufacturerId = 0xff;
  u8 u8DeviceId1 = 0xff;
  u8 u8DeviceId2 = 0xff;
  u8 u8Extended  = 0xff;
  u8 au8AsciiMsg[] = "FF\n\r";
  
  if(QueryMessageStatus(UserApp1_CurrentMsgToken) == COMPLETE)
  {
    /* The rx buffer should contain 1 dummy, the Manufacturer ID (0x1F), 
    Device ID1 (0x24), Device ID2 (0x00), Extended Info (0x00) */
    UserApp1_pu8RxBufferUnreadChar++;
    u8ManufacturerId = *UserApp1_pu8RxBufferUnreadChar++;
    u8DeviceId1 = *UserApp1_pu8RxBufferUnreadChar++;
    u8DeviceId2 = *UserApp1_pu8RxBufferUnreadChar++;
    u8Extended  = *UserApp1_pu8RxBufferUnreadChar++;
    
    DebugPrintf("Dataflash Manufacturer ID: 0x");
    au8AsciiMsg[0] = HexToASCIICharUpper( (u8ManufacturerId & 0xF0) >> 4);
    au8AsciiMsg[1] = HexToASCIICharUpper( (u8ManufacturerId & 0x0F) >> 0);
    DebugPrintf(au8AsciiMsg);
    
    DebugPrintf("Dataflash Device ID1: 0x");
    au8AsciiMsg[0] = HexToASCIICharUpper( (u8DeviceId1 & 0xF0) >> 4);
    au8AsciiMsg[1] = HexToASCIICharUpper( (u8DeviceId1 & 0x0F) >> 0);
    DebugPrintf(au8AsciiMsg);

    DebugPrintf("Dataflash Device ID2: 0x");
    au8AsciiMsg[0] = HexToASCIICharUpper( (u8DeviceId2 & 0xF0) >> 4);
    au8AsciiMsg[1] = HexToASCIICharUpper( (u8DeviceId2 & 0x0F) >> 0);
    DebugPrintf(au8AsciiMsg);

    DebugPrintf("Dataflash Device Extended Data: 0x");
    au8AsciiMsg[0] = HexToASCIICharUpper( (u8Extended & 0xF0) >> 4);
    au8AsciiMsg[1] = HexToASCIICharUpper( (u8Extended & 0x0F) >> 0);
    DebugPrintf(au8AsciiMsg);

    UserApp1_pfStateMachine = UserApp1SM_Idle;
  }
  
} /* end UserApp1SM_WaitResponse() */



/*-------------------------------------------------------------------------------------------------------------------*/
/* Handle an error */
static void UserApp1SM_Error(void)          
{
  
} /* end UserApp1SM_Error() */




/*--------------------------------------------------------------------------------------------------------------------*/
/* End of File                                                                                                        */
/*--------------------------------------------------------------------------------------------------------------------*/
