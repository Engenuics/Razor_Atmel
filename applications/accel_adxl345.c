/******************************************************************************
File: accel.c                                                                

Description:
LCD demonstration application.

DISCLAIMER: THIS CODE IS PROVIDED WITHOUT ANY WARRANTY OR GUARANTEES.  USERS MAY
USE THIS CODE FOR DEVELOPMENT AND EXAMPLE PURPOSES ONLY.  ENGENUICS TECHNOLOGIES
INCORPORATED IS NOT RESPONSIBLE FOR ANY ERRORS, OMISSIONS, OR DAMAGES THAT COULD
RESULT FROM USING THIS FIRMWARE IN WHOLE OR IN PART.

******************************************************************************/

#include "configuration.h"


/*******************************************************************************
* "Global Global" Definitions (variables for scope across multiple files)
*******************************************************************************/
/*----------------------------------------------------------------------------*/
/* New variables  (all shall start with GGxxAccel) */
fnCode_type GGAccelStateMachine;           /* State machine function pointer */

u8 *GGpu8AccelRxBufferPointer;             /* Pointer to current byte in accelerometer Rx buffer */   

/*----------------------------------------------------------------------------*/
/* Existing Global Global variables (defined in other files -- should all contain 
the "extern" keyword and indicate which file they are defined in). */
extern u32 GGu32SysTime1ms;                /* From deva7-ehdw-01.c */
extern u32 GGu32SysTime1s;                 /* From deva7-ehdw-01.c */

extern u8  GGau8LCDAccelMessage1[];        /* From lcd_nhd_c0220biz.c */

extern u32 GGu32I2CFlags;                  /* From i2c_lpc214x.c */
extern u16 GGu16I2C0BytesToReceive;        /* From i2c_lpc214x.c */


/******************************************************************************
* "Local Global" Definitions (variables for data transfer between functions in 
* this file.  All Local Global variables names shall start with "LGxx".
******************************************************************************/
u32 LGu32AccelFlags;                          /* Application flag bits */
u8  LGau8AccelRxBuffer[ACCEL_RX_BUFFER_SIZE]; /* Space for incoming data from accelerometer */

u32 LGu32AccelFreeFallTimeStart;              /* System time that freefall started */  

/******************************************************************************
* Function Definitions
******************************************************************************/

/* Protected functions */

/*-----------------------------------------------------------------------------
Function: AccelPowerOn

Description:
Enables power to the Accelerometer daughter board.

Requires:
  - GPIO for pin P0.9 properly configured as an output to ACCEL_PWR_EN
    (which is an active-low signal)

Promises:
  - Drives ACCEL_PWR_EN low
*/

void AccelPowerOn(void)
{
  u32 *pu32ClearAddress = (u32*)(&FIO0CLR);
  
  /* Clear the bit corresponding to P0_09_U1_PIMO */
  *pu32ClearAddress = P0_09_U1_PIMO;

} /* end AccelPowerOn */


/*-----------------------------------------------------------------------------
Function: AccelPowerOff

Description:
Disables power to the Accelerometer daughter board.

Requires:
  - GPIO for pin P0.9 properly configured as an output to ACCEL_PWR_EN
    (which is an active-low signal)

Promises:
  - Drives ACCEL_PWR_EN high
*/

void AccelPowerOff(void)
{
  u32 *pu32SetAddress = (u32*)(&FIO0SET);
  
  /* Set the bit corresponding to P0_09_U1_PIMO */
  *pu32SetAddress = P0_09_U1_PIMO;

} /* end AccelPowerOff */


/*-----------------------------------------------------------------------------
Function: IsAccelInterrupt1Active

Description:
Returns the value of the ACCEL_INT1 line from the accelerometer. 

Requires:
  - GPIO for pin P0.29 properly configured as an input from ACCEL_INT1
    (which is an active-high signal)

Promises:
  - Returns value of ACCEL_INT1
*/

bool IsAccelInterrupt1Active(void)
{
  u32 *pu32SetAddress = (u32*)(&FIO0PIN);
  
  /* Read the bit value corresponding to P0_29_B_IO1 from port 0 GPIO */
  if( *pu32SetAddress & P0_29_B_IO1 )
  {
    return( TRUE );
  }
  else
  {
    return( FALSE );
  }

} /* end IsAccelInterrupt1Active */


/*-----------------------------------------------------------------------------
Function: IsAccelInterrupt2Active

Description:
Returns the value of the ACCEL_INT2 line from the accelerometer. 

Requires:
  - GPIO for pin P0.28 properly configured as an input from ACCEL_INT2
    (which is an active-high signal)

Promises:
  - Returns value of ACCEL_INT2
*/

bool IsAccelInterrupt2Active(void)
{
  u32 *pu32SetAddress = (u32*)(&FIO0PIN);
  
  /* Read the bit value corresponding to P0_28_B_IO0 from port 0 GPIO */
  if( *pu32SetAddress & P0_28_B_IO0 )
  {
    return( TRUE );
  }
  else
  {
    return( FALSE );
  }

} /* end IsAccelInterrupt2Active */


/*-----------------------------------------------------------------------------
Function: AccelInitialize

Description:
Sets up the Accelerometer application and initializes accelerometer registers for
tap and freefall thresholds, interrupt support, power control and data format.

Requires:
  - I²C application initialized 
  - LED application initialized
  - LCD application initialized

Promises:
  - Outputs system status and hands-off to the Idle state
*/

void AccelInitialize(void)
{
  u8 au8AccelSetupData0[] = {I2C_ADDRESS_ACCEL_WRITE, ADDRESS_THRESH_TAP,
                             ADXL345_THRESH_TAP_INIT, 0, 0, 0,
                             ADXL345_DUR_INIT, ADXL345_LATENT_INIT, 
                             ADXL345_WINDOW_INIT};  
  
  u8 au8AccelSetupData1[] = {I2C_ADDRESS_ACCEL_WRITE, ADDRESS_THRESH_ACT,
                             ADXL345_THRESH_ACT_INIT, ADXL345_THRESH_INACT_INIT,
                             ADXL345_TIME_INACT_INIT, ADXL345_ACT_INACT_CTL_INIT,
                             ADXL345_THRESH_FF_INIT, ADXL345_TIME_FF_INIT, 
                             ADXL345_TAP_AXES_INIT};
  
  u8 au8AccelSetupData2[] = {I2C_ADDRESS_ACCEL_WRITE, ADDRESS_BW_RATE,
                             ADXL345_BW_RATE_INIT, ADXL345_POWER_CTL_INIT,
                             ADXL345_INT_ENABLE_INIT, ADXL345_INT_MAP_INIT, 0,
                             ADXL345_DATA_FORMAT_INIT};
  
  u8 au8AccelInit[]         = "Initializing Accelerometer";
  u8 au8AccelFoundMsg[]     = {"  ADXL345 Found - ID: 0xE5"};
  u8 au8AccelNotFoundMsg[]  = {"  ADXL345 Not Found\n\n\r"};
  u8 au8AccelInitPassMsg[]  = {"  Initialization Passed\n\n\r"};
  u8 au8AccelInitFailMsg[]  = {"  Initialization Failed\n\n\r"};
  u32 u32Timer;

  /* Turn on the Accelerometer daughter board */
  AccelPowerOn();
  /* Clear flags */
  LGu32AccelFlags = 0;

  /* Announce on the debug port that the Accelerometer application is starting */
  UartQueueTxString(&au8AccelInit[0]);
  UartForceSend();

  /* Clear the LCD screen and set the backlight to green */
  LCDCommand(LCD_CLEAR_CMD);
  I2CForceTransfer();
  LedSet(RGB_GREEN, NORMAL, LED_ON);

  /* Add a small delay between LCD commands */
  u32Timer = GGu32SysTime1ms;
  while( !IsTimeUp(&GGu32SysTime1ms, &u32Timer, 10, NO_RESET_TARGET_TIMER) ) ;

  /* Display the LCD message */
  LCDMessage(LINE1_START_ADDR, &GGau8LCDAccelMessage1[0]);
  I2CForceTransfer();
  I2CForceTransfer();
  
  /* Set the buzzer frequency so it is ready to be enabled but keep it off for now */
  PWMAudioSetFrequency(500);
  PWMAudioOff();

  /* ### Accelerometer-specific initialization ### */
  // Read accelerometer device ID
  GGpu8AccelRxBufferPointer = &LGau8AccelRxBuffer[0];
  AccelSetupDataRead( ADDRESS_WHO_AM_I, 1 );
  I2CForceTransfer(); /* Hand-crank the I2C SM to WRITE the slave's "who am I" register address */
  I2CForceTransfer(); /* Hand-crank the I2C SM to READ the data from the slave's register */
  
  // Output accelerometer device ID status to the debug port
  if( LGau8AccelRxBuffer[0] == ACCELEROMETER_ID )
  { /* Device found */
    UartQueueTxString( &au8AccelFoundMsg[0] );
    UartForceSend();

    /* Send configuration data to the accelerometer */
    QueueI2C0Message( sizeof( au8AccelSetupData0 ), &au8AccelSetupData0[0] );
    I2CForceTransfer(); /* Hand-crank the I2C SM to WRITE the slave's setup data (part 0) */

    QueueI2C0Message( sizeof( au8AccelSetupData1 ), &au8AccelSetupData1[0] );
    I2CForceTransfer(); /* Hand-crank the I2C SM to WRITE the slave's setup data (part 1) */

    QueueI2C0Message( sizeof( au8AccelSetupData2 ), &au8AccelSetupData2[0] );
    I2CForceTransfer(); /* Hand-crank the I2C SM to WRITE the slave's setup data (part 2) */
    
    /* Check to see if the configuration was successful */
    if( GGu32I2CFlags & _I2C_FLAGS_ERROR )
    { /* Error occured during configuration */
      UartQueueTxString( &au8AccelInitFailMsg[0] );
      UartForceSend();
    }
    else
    { /* No errors occured */
      UartQueueTxString( &au8AccelInitPassMsg[0] );
      UartForceSend();
    }
  }
  else
  { /* Device not found */
    UartQueueTxString( &au8AccelNotFoundMsg[0] );
    UartForceSend();
  }
  /* ### Accelerometer-specific initialization ### */
  
  /* Set the function pointer to the starting state and initialize flags */
  GGAccelStateMachine = AccelSMIdle;  
  
} /* end AccelInitialize */


/*-----------------------------------------------------------------------------
Function: AccelRegAddress

Description:
Writes a register address to the accelerometer for a future READ or WRITE operation

Requires:
  - u8AccelRegAddress_ is a valid register address in the accelerometer

Promises:
  - The address byte is queued and will be sent to the accelerometer at the next
    available time.

*/

void AccelRegAddress( u8 u8AccelRegAddress_ )
{
  static u8 au8AccelWriteRegMessage[] = {I2C_ADDRESS_ACCEL_WRITE, 0x00};
  
  au8AccelWriteRegMessage[1] = u8AccelRegAddress_;
  /* Queue the message for writing */
  QueueI2C0Message( sizeof( au8AccelWriteRegMessage ), au8AccelWriteRegMessage );
  
} /* end AccelRegAddress */


/*-----------------------------------------------------------------------------
Function: AccelSetupDataWrite

Description:
Sets up the Accelerometer application for a data WRITE operation to a specified
register address

Requires:
  - u8AccelWriteAddress_ is a valid register address in the accelerometer
  - u8AccelWriteData_ is a single data byte to be written to the accelerometer
    register

Promises:
  - The address and data bytes are queued and will be sent to the accelerometer 
    at the next available time.

*/

void AccelSetupDataWrite( u8 u8AccelWriteAddress_, u8 u8AccelWriteData_ )
{
  static u8 au8AccelWriteDataMessage[] = {I2C_ADDRESS_ACCEL_WRITE, 0x00, 0x00};
  
  au8AccelWriteDataMessage[1] = u8AccelWriteAddress_;
  au8AccelWriteDataMessage[2] = u8AccelWriteData_;
  
  /* Queue the message for writing */
  QueueI2C0Message( sizeof( au8AccelWriteDataMessage ), au8AccelWriteDataMessage );
  
} /* end AccelSetupDataWrite */


/*-----------------------------------------------------------------------------
Function: AccelSetupDataRead

Description:
Sets up the Accelerometer application for a data READ operation of a specified
number of bytes

Requires:
  - u8AccelReadAddress_ is a valid register address to read in the accelerometer
  - u8AccelBytesToRead_ is a non-zero number of successive bytes to read

Promises:
  - Queues a message to write the register address to the accelerometer so it may
    later be read.
  - Sets the global "bytes to receive" variable indicating a read is set to start.

*/

void AccelSetupDataRead( u8 u8AccelReadAddress_, u8 u8AccelBytesToRead_ )
{
  /* Queue up READ register address for write to I2C */
  AccelRegAddress( u8AccelReadAddress_ );
  
  /* Set number of bytes to receive */
  GGu16I2C0BytesToReceive = u8AccelBytesToRead_;
  
} /* end AccelSetupDataRead */


/******************************************************************************
* State Machine Function Definitions
******************************************************************************/

/*----------------------------------------------------------------------------*/
/* Wait here for the time interval ACCEL_INTERVAL to elapse.  Then initiate the 
   proper read sequence of bytes from the accelerometer depending upon whether we 
   are in Debug Mode, Interrupt Mode or Normal Mode. */
void AccelSMIdle(void)
{
  static u32 u32AccelInterval = ACCEL_INTERVAL;
  
  /* Check to see if the interval ACCEL_INTERVAL has passed before initiating a 
     read from the accelerometer. */
  if( --u32AccelInterval == 0 )
  { 
    u32AccelInterval = ACCEL_INTERVAL;
   
    /* Queue the system to read data from the accelerometer.  Read different data 
       values depending upon whether we are in Debug Mode, Interrupt Mode 
       (clearing active interrupts), or Normal Mode (reading motion data, default) */
    GGpu8AccelRxBufferPointer = &LGau8AccelRxBuffer[0];

    /* Check for Button 0 press to initiate Debug Mode */
    if( WasButtonPressed( BUTTON0 ) )
    { /* Debug Mode, read all registers */
      ButtonAcknowledge( BUTTON0 );
      
      LGu32AccelFlags |= _ACCEL_FLAGS_DEBUG_MODE;
      AccelSetupDataRead( ADDRESS_THRESH_TAP, 29 );
    }
    
    /* Check for Line 1/2 interrupts to initiate Interrupt Mode */
    else if( IsAccelInterrupt1Active() || IsAccelInterrupt2Active() )
    { /* Interrupt Mode, read interrupt source register to clear interrupts */
      
      if( IsAccelInterrupt1Active() )
      { /* Tap detected */
        LGu32AccelFlags |= _ACCEL_FLAGS_INT1_ACTIVE;
      }
      if( IsAccelInterrupt2Active() )
      { /* Freefall detected */
        LGu32AccelFlags |= _ACCEL_FLAGS_INT2_ACTIVE;
      }
      
      LGu32AccelFlags |= _ACCEL_FLAGS_INTERRUPT_MODE;
      AccelSetupDataRead( ADDRESS_INT_SOURCE, 1 );
    }
    
    /* Otherwise we are in Normal Mode */
    else
    {/* Normal Mode, read motion data registers */
      
      LGu32AccelFlags |= _ACCEL_FLAGS_NORMAL_MODE;
      AccelSetupDataRead( ADDRESS_DATAX0, 6 );
    }
    
    GGAccelStateMachine = AccelSMReceiveWaitData;  
  }
} /* end AccelSMIdle() */


/*----------------------------------------------------------------------------*/
/* Wait here while we are still waiting for data to be received, i.e. 
   GGu16I2C0BytesToReceive is non-zero.  Once data is received, perform correct
   action depending upon whether we are in Debug Mode, Interrupt Mode or Normal
   Mode. */
void AccelSMReceiveWaitData(void)
{ 
  u8 au8AccelMotionDataMsg[] = {"+xxx, +yyy, +zzz"};
  u8 au8AccelInterrupt1Msg[] = {"\nTap detected      "};
  u8 au8AccelInterrupt2Msg[] = {"\nFreefall detected "};
  
  u8   u8Counter = 0;
  u16  u16Temp = 0;
  u8   au8TestMessage[] = {"Test Message\n"};
  
  u8 au8AccelDebugString[]  = {"\nRegister:Value"};
  u8 au8AccelRegAddrString[] = "00";
  u8 au8AccelRegValString[]  = "000";
  u8 au8AccelRegMsgString[]  = {"00:000"};
  
  /* Check to see if the data read has been completed */
  if( GGu16I2C0BytesToReceive == 0 )
  {
    
    /* Normal Mode - display X, Y and Z-axis readings */
    if( LGu32AccelFlags & _ACCEL_FLAGS_NORMAL_MODE )
    {
      for( u8 index = 0; index <= 4; index +=2 )
        {
        /* Grab the current reading and check the sign bit */
        u16Temp = (u16)LGau8AccelRxBuffer[index] + 
                  (u16)(LGau8AccelRxBuffer[index + 1] << 8);
        if( u16Temp & 0x8000 )
        {
          au8AccelMotionDataMsg[u8Counter] = '-';
          u16Temp = ~u16Temp + 1;
        }
        u8Counter++;
        au8AccelMotionDataMsg[u8Counter++] = (u16Temp / 100) + 0x30;
        u16Temp %= 100;
        au8AccelMotionDataMsg[u8Counter++] = (u16Temp / 10)  + 0x30;
        au8AccelMotionDataMsg[u8Counter]   = (u16Temp % 10)  + 0x30;
        u8Counter += 3;
      }   
      LCDMessage( LINE2_START_ADDR, &au8AccelMotionDataMsg[0] );
      
      LGu32AccelFlags &= ~_ACCEL_FLAGS_NORMAL_MODE;
    }

    /* Interrupt Mode - process interrupt lines 1 and 2 */
    if( LGu32AccelFlags & _ACCEL_FLAGS_INTERRUPT_MODE )
    {
      if( LGu32AccelFlags & _ACCEL_FLAGS_INT1_ACTIVE )
      { /* Interrupt 1 - Tap detected */

        LCDMessage( LINE1_START_ADDR, &au8AccelInterrupt1Msg[0] );
      
        /* Set LCD backlight to blue */
        LedSet(RGB_RED  , NORMAL, LED_OFF);
        LedSet(RGB_GREEN, NORMAL, LED_OFF);
        LedSet(RGB_BLUE , NORMAL, LED_ON );
      
        LGu32AccelFlags &= ~_ACCEL_FLAGS_INT1_ACTIVE;
      }
      
      if( LGu32AccelFlags & _ACCEL_FLAGS_INT2_ACTIVE )
      { /* Interrupt 2 - Freefall detected */

        LCDMessage( LINE1_START_ADDR, &au8AccelInterrupt2Msg[0] );

        /* Set LCD backlight to red */
        LedSet(RGB_RED  , NORMAL, LED_ON );
        LedSet(RGB_GREEN, NORMAL, LED_OFF);
        LedSet(RGB_BLUE , NORMAL, LED_OFF);
      
        /* Save system time when freefall started */
        LGu32AccelFreeFallTimeStart = GGu32SysTime1ms;
        /* Disable tap interrupt to prevent it from being triggered after freefall */
        LGu32AccelFlags |= _ACCEL_FLAGS_INT1_DISABLED;
        AccelSetupDataWrite( ADDRESS_INT_ENABLE, _ADXL345_INT_ENABLE_FREEFALL );
        
        LGu32AccelFlags &= ~_ACCEL_FLAGS_INT2_ACTIVE;
      }
      
      LGu32AccelFlags &= ~_ACCEL_FLAGS_INTERRUPT_MODE;
    }
    
    /* Debug Mode - write contents of registers to UART */
    if( LGu32AccelFlags & _ACCEL_FLAGS_DEBUG_MODE )
    {     
      UartQueueTxString( &au8AccelDebugString[0] );
      
      /* Loop through and display each register address and value */
      for( u8 index = 0; index < 29; index++ )
      {      
        /* Format 2-digit register ADDRESS to an ASCII string */
        u16Temp = index + ADDRESS_THRESH_TAP;
        for(u8 i = 0; i < 2; i++)
        {       
          au8AccelRegAddrString[1 - i] = u16Temp % 10 + '0';   
          u16Temp /= 10;
        }
        
        /* Format 3-digit register VALUE to an ASCII string */
        u16Temp = LGau8AccelRxBuffer[index];
        for(u8 i = 0; i < 3; i++)
        {       
          au8AccelRegValString[2 - i] = u16Temp % 10 + '0';   
          u16Temp /= 10;
        }

        /* Build output message string */
        au8AccelRegMsgString[0] = au8AccelRegAddrString[0];
        au8AccelRegMsgString[1] = au8AccelRegAddrString[1];
        
        au8AccelRegMsgString[3] = au8AccelRegValString[0];
        au8AccelRegMsgString[4] = au8AccelRegValString[1];
        au8AccelRegMsgString[5] = au8AccelRegValString[2];
        
        /* Write string to debug */
        UartQueueTxString( &au8AccelRegMsgString[0] );
      }

      LCDMessage(LINE1_START_ADDR, &GGau8LCDAccelMessage1[0]);
      
      /* Set LCD backklight to green */
      LedSet(RGB_RED  , NORMAL, LED_OFF);
      LedSet(RGB_GREEN, NORMAL, LED_ON );
      LedSet(RGB_BLUE , NORMAL, LED_OFF);

      LGu32AccelFlags &= ~_ACCEL_FLAGS_DEBUG_MODE;
    }    
    
    /* Re-enable the tap interrupt once 3 seconds has elapsed since freefall */
    if( IsTimeUp( &GGu32SysTime1ms, &LGu32AccelFreeFallTimeStart, ACCEL_FREEFALL_INTERVAL*1000, NO_RESET_TARGET_TIMER ) && (LGu32AccelFlags & _ACCEL_FLAGS_INT1_DISABLED) )
    { /* Re-enable tap interrupt */
      LGu32AccelFlags &= _ACCEL_FLAGS_INT1_DISABLED;
      AccelSetupDataWrite( ADDRESS_INT_ENABLE, _ADXL345_INT_ENABLE_SINGLE_TAP | _ADXL345_INT_ENABLE_FREEFALL );
      UartQueueTxString( &au8TestMessage[0] );
    }
    
    GGAccelStateMachine = AccelSMIdle;
  }
} /* end AccelSMReceiveI2CWaitData() */
    