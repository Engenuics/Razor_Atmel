/******************************************************************************
File: accel.h                                                               

Description:
Accelerometer application header file.

DISCLAIMER: THIS CODE IS PROVIDED WITHOUT ANY WARRANTY OR GUARANTEES.  USERS MAY
USE THIS CODE FOR DEVELOPMENT AND EXAMPLE PURPOSES ONLY.  ENGENUICS TECHNOLOGIES
INCORPORATED IS NOT RESPONSIBLE FOR ANY ERRORS, OMISSIONS, OR DAMAGES THAT COULD
RESULT FROM USING THIS FIRMWARE IN WHOLE OR IN PART.

******************************************************************************/

#ifndef __ACCEL_H
#define __ACCEL_H


/******************************************************************************
* Constants
******************************************************************************/

/* LGu32AccelFlags */
#define _ACCEL_FLAGS_NORMAL_MODE    0x00000001      /* Set when accelerometer is in Normal Mode */
#define _ACCEL_FLAGS_INTERRUPT_MODE 0x00000002      /* Set when accelerometer is in Interrupt Mode */
#define _ACCEL_FLAGS_DEBUG_MODE     0x00000004      /* Set when accelerometer is in Debug Mode */
#define _ACCEL_FLAGS_INT1_ACTIVE    0x00000010      /* Set when accelerometer Interrupt 1 is active (tap) */
#define _ACCEL_FLAGS_INT2_ACTIVE    0x00000020      /* Set when accelerometer Interrupt 2 is active (freefall) */
#define _ACCEL_FLAGS_INT1_DISABLED  0x00000100      /* Set when accelerometer Interrupt 1 is disabled (tap) */
#define _ACCEL_FLAGS_INT2_DISABLED  0x00000200      /* Set when accelerometer Interrupt 2 is disabled (freefall) */

#define ACCEL_INTERVAL              (u16)100        /* x 1 ms between motion sensor reads */
#define ACCEL_FREEFALL_INTERVAL     (u8)3           /* x 1 s to disable tap interrupt after freefall */
#define ACCEL_RX_BUFFER_SIZE        (u8)32          /* Maximum receive buffer size for accelerometer data */

/* I2C addresses for accelerometers */
#define I2C_ADDRESS_ADXL345_READ    (u8)0x3b
#define I2C_ADDRESS_ADXL345_WRITE   (u8)0x3a
#define I2C_ADDRESS_ACCEL_READ      I2C_ADDRESS_ADXL345_READ
#define I2C_ADDRESS_ACCEL_WRITE     I2C_ADDRESS_ADXL345_WRITE

/* Expected device ID values read from ID registers */
#define AD345_ID                    (u8)0xE5
#define ACCELEROMETER_ID            AD345_ID

/* Registers in ADXL345 accelerometer */
#define ADDRESS_WHO_AM_I            (u8)0x00
#define ADDRESS_THRESH_TAP          (u8)0x1d
#define ADDRESS_OFSX                (u8)0x1e
#define ADDRESS_OFSY                (u8)0x1f
#define ADDRESS_OFSZ                (u8)0x20
#define ADDRESS_DUR                 (u8)0x21
#define ADDRESS_LATENT              (u8)0x22
#define ADDRESS_WINDOW              (u8)0x23
#define ADDRESS_THRESH_ACT          (u8)0x24
#define ADDRESS_THRESH_INACT        (u8)0x25
#define ADDRESS_TIME_INACT          (u8)0x26
#define ADDRESS_ACT_INACT_CTL       (u8)0x27
#define ADDRESS_THRESH_FF           (u8)0x28
#define ADDRESS_TIME_FF             (u8)0x29
#define ADDRESS_TAP_AXES            (u8)0x2a
#define ADDRESS_ACT_TAP_STATUS      (u8)0x2b
#define ADDRESS_BW_RATE             (u8)0x2c
#define ADDRESS_POWER_CTL           (u8)0x2d
#define ADDRESS_INT_ENABLE          (u8)0x2e
#define ADDRESS_INT_MAP             (u8)0x2f
#define ADDRESS_INT_SOURCE          (u8)0x30
#define ADDRESS_DATA_FORMAT         (u8)0x31
#define ADDRESS_DATAX0              (u8)0x32
#define ADDRESS_DATAX1              (u8)0x33
#define ADDRESS_DATAY0              (u8)0x34
#define ADDRESS_DATAY1              (u8)0x35
#define ADDRESS_DATAZ0              (u8)0x36
#define ADDRESS_DATAZ1              (u8)0x37
#define ADDRESS_FIFO_CTL            (u8)0x38
#define ADDRESS_FIFO_STATUS         (u8)0x39

/* Register initialization values */

/* Tap threshold is 62.5 mg / step */
#define ADXL345_THRESH_TAP_INIT     (u8)0x35        /* 3.313 g's  */
/* Tap duration is 625 us / step */
#define ADXL345_DUR_INIT            (u8)0x12        /* 11.25 ms   */

/* Latent and window are 1.25 ms / step */
#define ADXL345_LATENT_INIT         (u8)0           /* 0 ms       */
#define ADXL345_WINDOW_INIT         (u8)0           /* 0 ms       */

/* Activity thresholds are 62.5 mg / step */
#define ADXL345_THRESH_ACT_INIT     (u8)32          /* 2 g's      */
#define ADXL345_THRESH_INACT_INIT   (u8)10          /* 0.625 g's  */

/* Time for inactivity is 1 s / step */
#define ADXL345_TIME_INACT_INIT     (u8)10          /* 10 s       */

#define ADXL345_ACT_INACT_CTL_INIT  (u8)0xff
/* Bit Set Description
    07 [1] AC mode for ACT
    06 [1] ACT_X enabled
    05 [1] ACT_Y enabled
    04 [1] ACT_Z enabled

    03 [1] AC mode for INACT 
    02 [1] INACT_X enabled
    01 [1] INACT_Y enabled
    00 [1] INACT_Z enabled
*/

/* Freefall threshold is 62.5 mg / step */
//#define ADXL345_THRESH_FF_INIT    (u8)7
#define ADXL345_THRESH_FF_INIT      (u8)0x05        /* 0.313 g's  */

/* Freefall time threshold is 5 ms / step */
//#define ADXL345_TIME_FF_INIT      (u8)50
#define ADXL345_TIME_FF_INIT        (u8)0x15        /* 105 ms     */

/* Z tap axis on */
#define ADXL345_TAP_AXES_INIT       (u8)0x01

#define ADXL345_BW_RATE_INIT        (u8)0x07
/* Bit Set Description
    07 [0] 0
    06 [0] 0
    05 [0] 0
    04 [0] Normal power operation

    03 [0] Rate code for 12.5Hz output data rate 
    02 [1] "
    01 [1] "
    00 [1] "
*/

#define ADXL345_POWER_CTL_INIT    (u8)0x08
/* Bit Set Description
    07 [0] 0
    06 [0] 0
    05 [0] Link mode off
    04 [0] Auto sleep off

    03 [1] Measurement mode 
    02 [0] Sleep mode off
    01 [0] Readings in sleep mode 8Hz
    00 [0] "
*/

#define _ADXL345_INT_ENABLE_DATA_READY  (u8)0x80
#define _ADXL345_INT_ENABLE_SINGLE_TAP  (u8)0x40
#define _ADXL345_INT_ENABLE_DOUBLE_TAP  (u8)0x20
#define _ADXL345_INT_ENABLE_ACTIVITY    (u8)0x10
#define _ADXL345_INT_ENABLE_INACTIVITY  (u8)0x08
#define _ADXL345_INT_ENABLE_FREEFALL    (u8)0x04
#define _ADXL345_INT_ENABLE_WATERMARK   (u8)0x02
#define _ADXL345_INT_ENABLE_OVERRUN     (u8)0x01

#define ADXL345_INT_ENABLE_INIT   (u8)_ADXL345_INT_ENABLE_SINGLE_TAP | _ADXL345_INT_ENABLE_FREEFALL
/* Bit Set Description
    07 [0] DATA_READY interrupt off
    06 [1] SINGLE_TAP interrupt on
    05 [0] DOUBLE_TAP interrupt off
    04 [0] Activity interrupt off

    03 [0] Inactivity interrupt off
    02 [1] FREE_FALL interrupt on
    01 [0] Watermark interrupt off
    00 [0] Overrun interrupt off
*/

#define ADXL345_INT_MAP_INIT      (u8)0x0f
/* Bit Set Description
    07 [0] DATA_READY interrupt 1
    06 [0] SINGLE_TAP interrupt 1
    05 [0] DOUBLE_TAP interrupt 1
    04 [0] Activity interrupt 1

    03 [1] Inactivity interrupt 2
    02 [1] FREE_FALL interrupt 2
    01 [1] Watermark interrupt 2
    00 [1] Overrun interrupt 2
*/

#define ADXL345_DATA_FORMAT_INIT  (u8)0x03
/* Bit Set Description
    07 [0] Self test off
    06 [0] SPI mode 0 (NA)
    05 [0] Interrupts active high
    04 [0] 0

    03 [0] 10-bit mode 
    02 [0] Right justified data
    01 [1] +/- 16g mode
    00 [1] "
*/

#define ADXL345_FIFO_CTL_INIT     (u8)0x00
/* Bit Set Description
    07 [0] FIFO bypassed
    06 [0] "
    05 [0] Trigger INT1
    04 [0] Samples bits (none)

    03 [0] "
    02 [0] "
    01 [0] "
    00 [0] "
*/

/******************************************************************************
* Function Prototypes
******************************************************************************/
/* Private Functions */

/* Protected Functions */
void AccelInitialize(void);
void AccelPowerOn(void);
void AccelPowerOff(void);
bool IsAccelInterrupt1Active(void);
bool IsAccelInterrupt2Active(void);
void AccelRegAddress( u8 u8AccelRegAddress_ );
void AccelSetupDataWrite( u8 u8AccelWriteAddress_, u8 u8AccelWriteData_ );
void AccelSetupDataRead( u8 u8AccelReadAddress_, u8 u8AccelBytesToRead_ );

/* Protected State Machine functions */
void AccelSMIdle(void);          
void AccelSMReceiveWaitData(void);

/* Public Functions */


#endif /* __ACCEL_H */