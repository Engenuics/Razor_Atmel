/*******************************************************************************
File: lcd_NHD-C12864LZ.h                                                                

Description:
LCD implementation for Newhaven NHD-C12864LZ
*******************************************************************************/

#ifndef __LCD_H
#define __LCD_H


/*******************************************************************************
* Typedefs
*******************************************************************************/
typedef enum {LCD_FONT_SMALL, LCD_FONT_BIG} LcdFontType;

typedef enum {LCD_SHIFT_UP, LCD_SHIFT_DOWN, LCD_SHIFT_RIGHT, LCD_SHIFT_LEFT} LcdShiftType;

typedef enum {LCD_CONTROL_MESSAGE, LCD_DATA_MESSAGE} LcdMessageType;

/* Absolute address of one LCD pixel */
typedef struct
{
  u16 u16PixelRowAddress;
  u16 u16PixelColumnAddress;
} PixelAddressType;


/* Address structure of a rectangular group of pixels */
typedef struct
{
  u16 u16RowStart;             /* Address of top left pixel row */
  u16 u16ColumnStart;          /* Address of top left pixel column */
  u16 u16RowSize;              /* Number of rows in block */
  u16 u16ColumnSize;           /* Number of columns in block */
} PixelBlockType;


/* LCD message struct used to queue LCD data */
typedef struct
{
  LcdMessageType eMessageType; /* Command or data */ 
  u16 u16Length;               /* Number of bytes in the message */
  u8* pu8Data;                 /* Address of first data byte of the message */
} LcdQueueMessageType;


/*******************************************************************************
* Macros (board-specific: need to be adjusted to target PCB)
*******************************************************************************/
/* Public */
#define     LCD_BACKLIGHT_ON()          (LedOn(LCD_BL))
#define     LCD_BACKLIGHT_OFF()         (LedOff(LCD_BL))

/* Private */
#define     LCD_RESET_ASSERT()          (AT91C_BASE_PIOB->PIO_CODR = PB_14_LCD_RST)
#define     LCD_RESET_DEASSERT()        (AT91C_BASE_PIOB->PIO_SODR = PB_14_LCD_RST)
#define     LCD_COMMAND_MODE()          (AT91C_BASE_PIOB->PIO_CODR = PB_15_LCD_A0)
#define     LCD_DATA_MODE()             (AT91C_BASE_PIOB->PIO_SODR = PB_15_LCD_A0)
#define     LCD_CS_ASSERT()             (AT91C_BASE_PIOB->PIO_CODR = PB_12_LCD_CS)
#define     LCD_CS_DEASSERT()           (AT91C_BASE_PIOB->PIO_SODR = PB_12_LCD_CS)



/*******************************************************************************
* Application Values
*******************************************************************************/
/* Lcd_u32Flags */
#define _LCD_FLAGS_COMMAND_IN_QUEUE   0x00000001      /* Command or data in LCD */

#define _LCD_MANUAL_MODE              0x10000000      /* The task is in manual mode */

/* end Lcd_u32Flags */

/* LCD hardware definitions */
#define LCD_PIXEL_BITS                (u8)1
#define LCD_PAGE_SIZE                 (u8)8
#define LCD_PAGES                     (u8)8
#define LCD_ROWS                      (u16)(LCD_PAGES * LCD_PAGE_SIZE)
#define LCD_COLUMNS                   (u16)128

#define LCD_LEFT_MOST_COLUMN          (u16)0
#define LCD_RIGHT_MOST_COLUMN         (u16)(LCD_COLUMNS - 1)
#define LCD_TOP_MOST_ROW              (u16)0
#define LCD_BOTTOM_MOST_ROW           (u16)(LCD_ROWS - 1)
#define LCD_CENTER_COLUMN             (u16)(LCD_COLUMNS / 2)
#define LCD_IMAGE_ROWS                LCD_ROWS
#define LCD_IMAGE_COLUMNS             (u16)(LCD_COLUMNS * LCD_PIXEL_BITS / 8)

#define LCD_TX_BUFFER_SIZE            (u16)128   /* Enough for a complete page refresh */
#define LCD_RX_BUFFER_SIZE            (u16)1   /* Enough for a complete page refresh */

#define LCD_STARTUP_DELAY_200         (u32)205
#define LCD_STARTUP_DELAY_10          (u32)11
#define LCD_REFRESH_TIME              (u32)25                /* Time in ms between LCD refreshes */

/* Bitmap sizes (x = # of column pixels, y = # of row pixels) */
#define LCD_SMALL_FONT_COLUMNS        (u8)5
#define LCD_SMALL_FONT_COLUMN_BYTES   (u8)1
#define LCD_SMALL_FONT_ROWS           (u8)7
#define LCD_SMALL_FONT_SPACE          (u8)1
#define LCD_MAX_SMALL_CHARS           (u8)(LCD_COLUMNS / (LCD_SMALL_FONT_COLUMNS + LCD_SMALL_FONT_SPACE))
#define LCD_SMALL_FONT_COLSPACE       (u8)(LCD_SMALL_FONT_COLUMNS + LCD_SMALL_FONT_COLUMN_BYTES)

#define LCD_BIG_FONT_COLUMNS          (u8)10
#define LCD_BIG_FONT_COLUMN_BYTES     (u8)2
#define LCD_BIG_FONT_ROWS             (u8)14
#define LCD_BIG_FONT_SPACE            (u8)1

#define LCD_DOT_XSIZE                 (u8)1
#define LCD_DOT_YSIZE                 (u8)1
#define LCD_IMAGE_COL_SIZE_50PX       (u8)50
#define LCD_IMAGE_COL_BYTES_50PX      (u8)((LCD_IMAGE_COL_SIZE_50PX * LCD_PIXEL_BITS / 8) + 1)
#define LCD_IMAGE_ROW_SIZE_50PX       (u8)50
#define LCD_IMAGE_COL_SIZE_25PX       (u8)25
#define LCD_IMAGE_COL_BYTES_25PX      (u8)((LCD_IMAGE_COL_SIZE_25PX * LCD_PIXEL_BITS / 8) + 1)
#define LCD_IMAGE_ROW_SIZE_25PX       (u8)25

#define LCD_SMALL_FONT_ROW_SPACING    (u8)1
#define LCD_SMALL_FONT_LINE0          (u8)(0)
#define LCD_SMALL_FONT_LINE1          (u8)(1 * (LCD_SMALL_FONT_ROWS + LCD_SMALL_FONT_ROW_SPACING))
#define LCD_SMALL_FONT_LINE2          (u8)(2 * (LCD_SMALL_FONT_ROWS + LCD_SMALL_FONT_ROW_SPACING))
#define LCD_SMALL_FONT_LINE3          (u8)(3 * (LCD_SMALL_FONT_ROWS + LCD_SMALL_FONT_ROW_SPACING))
#define LCD_SMALL_FONT_LINE4          (u8)(4 * (LCD_SMALL_FONT_ROWS + LCD_SMALL_FONT_ROW_SPACING))
#define LCD_SMALL_FONT_LINE5          (u8)(5 * (LCD_SMALL_FONT_ROWS + LCD_SMALL_FONT_ROW_SPACING))
#define LCD_SMALL_FONT_LINE6          (u8)(6 * (LCD_SMALL_FONT_ROWS + LCD_SMALL_FONT_ROW_SPACING))
#define LCD_SMALL_FONT_LINE7          (u8)(7 * (LCD_SMALL_FONT_ROWS + LCD_SMALL_FONT_ROW_SPACING))

#define LCD_BIG_FONT_ROW_SPACING      (u8)1
#define LCD_BIG_FONT_LINE0            (u8)(0)
#define LCD_BIG_FONT_LINE1            (u8)(1 * (LCD_BIG__FONT_ROWS + LCD_BIG__FONT_ROW_SPACING))
#define LCD_BIG_FONT_LINE2            (u8)(2 * (LCD_BIG__FONT_ROWS + LCD_BIG__FONT_ROW_SPACING))
#define LCD_BIG_FONT_LINE3            (u8)(3 * (LCD_BIG__FONT_ROWS + LCD_BIG__FONT_ROW_SPACING))

#define LCD_FRAMES_MSG_COLUMN         (u8)(10 * LCD_SMALL_FONT_COLSPACE)
#define LCD_IP_START_COLUMN           (u8)(3 * LCD_SMALL_FONT_COLSPACE)


/*******************************************************************************
* ST7565P Controller Values
*******************************************************************************/
/* The LCD is a write-only device.  Communication with LCD begins by setting the Register Select 
bit (A0 in the datasheet) to specify instruction (0)or data(1).
Next the chip select line is asserted to indicate SPI communication is starting.
A stream of bytes is sent which will be commands or data depending on the state of A0.

An 'x' at the end of a command indicates bits need to be added into the byte to complete it.
A '_' at the end of a command means it is a two-part command and must be immediately followed by the 2nd byte.
*/

/* Commands with A0 = 0 */
#define LCD_DISPLAY_ON              (u8)0xAF
#define LCD_DISPLAY_OFF             (u8)0xAE
#define LCD_DISPLAY_LINE_SETx       (u8)0x40
#define LCD_SET_PAGE_ADDRESSx       (u8)0xB0
#define LCD_SET_COL_ADDRESS_MSNx    (u8)0x10
#define LCD_SET_COL_ADDRESS_LSNx    (u8)0x00

#define LCD_ADC_SELECT_NORMAL       (u8)0xA0
#define LCD_ADC_SELECT_REVERSE      (u8)0xA1
#define LCD_REVERSE_ON              (u8)0xA7
#define LCD_REVERSE_OFF             (u8)0xA6
#define LCD_PIXEL_TEST_ON           (u8)0xA5
#define LCD_PIXEL_TEST_OFF          (u8)0xA4
#define LCD_BIAS_HIGH               (u8)0xA3
#define LCD_BIAS_LOW                (u8)0xA2
#define LCD_RESET                   (u8)0xE2

#define LCD_COMMON_MODE0            (u8)0xC0
#define LCD_COMMON_MODE1            (u8)0xC8

/* OR the follow options in with this command */
#define LCD_POWER_CONTROLLER_SETx   (u8)0x28
#define BOOST_ON                    (u8)0x04
#define VREG_ON                     (u8)0x02
#define VFOLLOW_ON                  (u8)0x01

/* OR the follow options in with this command */
#define LCD_VOLTAGTE_REG_SETx       (u8)0x20
#define SET_BIT0                    (u8)0x01
#define SET_BIT1                    (u8)0x02
#define SET_BIT2                    (u8)0x04

#define LCD_EVOLUME_UNLOCK_         (u8)0x81
#define LCD_EVOLUME_LEVEL           (u8)0x19
#define LCD_EVOLUDM_STATUSx         (u8)0x1F

#define LCD_SLEEP_MODE_             (u8)0xAC
#define LCD_AWAKE                   (u8)0x01
#define LCD_SLEEPING                (u8)0x00

#define LCD_NOP                     (u8)0xE3


/*******************************************************************************
* Function prototypes
*******************************************************************************/
/* LCD Public API Functions */
void LcdSetPixel(PixelAddressType* sPixelAddress_);
void LcdClearPixel(PixelAddressType* sPixelAddress_);
void LcdLoadString(const unsigned char* pu8String_, LcdFontType eFont_, PixelAddressType* sStartPixel_);
void LcdLoadBitmap(u8 const* pu8Image_, PixelBlockType* sImageSize_);
void LcdClearPixels(PixelBlockType* sPixelsToClear_);
void LcdClearScreen(void);
void LcdShift(PixelBlockType eShiftArea_, u16 u16PixelsToShift_, LcdShiftType eDirection_);
bool LcdCommand(u8 u8Command_);         

/* LCD Protected Functions */
void LcdInitialize(void);
void LcdRunActiveState(void);
void LcdManualMode(void);

/* LCD Private Driver Functions */
static bool LcdSetStartAddressForDataTransfer(u8 u8Page_);         
static void LcdLoadPageToBuffer(u8 u8LocalRamPage_); 
static void LcdUpdateScreenRefreshArea(PixelBlockType* sPixelsToClear_);

/* State machine declarations */
static void LcdSM_Idle(void);
static void LcdSM_WaitTransfer(void);
static void BoardTestSM_WaitPixelTestOn(void);          
static void BoardTestSM_WaitPixelTestOff(void);          



#endif /* __LCD_H */