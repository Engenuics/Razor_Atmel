/*******************************************************************************
* File: lcd_bitmaps.h                                                                
* Description:
* Bitmap data for 1-bit black & white dot matrix LCD
*******************************************************************************/

#ifndef __LCDBITMAPS_H
#define __LCDBITMAPS_H

#include "lcd_NHD-C12864LZ.h"
#include "configuration.h"


/*******************************************************************************
* Constants / Definitions
*******************************************************************************/
#define LCD_IMAGE_PADDLE_ROW_SIZE         (u8)2
#define LCD_IMAGE_PADDLE_COL_SIZE         (u8)11
#define LCD_IMAGE_PADDLE_COL_BYTES        (u8)((LCD_IMAGE_PADDLE_COL_SIZE * LCD_PIXEL_BITS / 8) + 1)

#define LCD_IMAGE_BALL_ROW_SIZE           (u8)3
#define LCD_IMAGE_BALL_COL_SIZE           (u8)3
#define LCD_IMAGE_BALL_COL_BYTES          (u8)((LCD_IMAGE_BALL_COL_SIZE * LCD_PIXEL_BITS / 8) + 1)

#define LCD_IMAGE_ARROW_ROW_SIZE          (u8)12
#define LCD_IMAGE_ARROW_COL_SIZE          (u8)6
#define LCD_IMAGE_ARROW_COL_BYTES         (u8)((LCD_IMAGE_ARROW_COL_SIZE * LCD_PIXEL_BITS / 8) + 1)


/*******************************************************************************
* Bookmarks
********************************************************************************
$$$$$ Small Font Character Bitmaps
%%%%% Big Font Character Bitmaps
^^^^^ Image Bitmaps
*******************************************************************************/


/*******************************************************************************
* Single Color Small Font Character Set                                    $$$$$
*******************************************************************************/


#endif /* __LCDBITMAPS_H */


