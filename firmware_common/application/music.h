/**********************************************************************************************************************
File: music.h      

Description:
Definitions for musical notes

***********************************************************************************************************************/


/* Note lengths */
#define MEASURE_TIME              (u16)2000  /* Time in ms for 1 measure (1 full note) - should be divisible by 16 */
#define FULL_NOTE                 (u16)(MEASURE_TIME)
#define HALF_NOTE                 (u16)(MEASURE_TIME / 2)
#define QUARTER_NOTE              (u16)(MEASURE_TIME / 4)
#define EIGHTH_NOTE               (u16)(MEASURE_TIME / 8)
#define SIXTEENTH_NOTE            (u16)(MEASURE_TIME / 16)

#define FN                        FULL_NOTE                 
#define HN                        HALF_NOTE                 
#define QN                        QUARTER_NOTE              
#define EN                        EIGHTH_NOTE               
#define SN                        SIXTEENTH_NOTE            

/* Note length adjustments */
#define REGULAR_NOTE_ADJUSTMENT   (u16)50
#define STACCATO_NOTE_TIME        (u16)75
#define HOLD_NOTE_ADJUSTMENT      (u16)0

#define RT                        REGULAR_NOTE_ADJUSTMENT
#define ST                        STACCATO_NOTE_TIME        
#define HT                        HOLD_NOTE_ADJUSTMENT            

/* Musical note definitions */
#define NOTE_C3                   (u16)131
#define NOTE_C3_SHARP             (u16)139
#define NOTE_D3_FLAT              (u16)139
#define NOTE_D3                   (u16)147
#define NOTE_D3_SHARP             (u16)156
#define NOTE_E3_FLAT              (u16)156
#define NOTE_E3                   (u16)165
#define NOTE_F3                   (u16)175
#define NOTE_F3_SHARP             (u16)185
#define NOTE_G3_FLAT              (u16)185
#define NOTE_G3                   (u16)196
#define NOTE_G3_SHARP             (u16)208
#define NOTE_A3                   (u16)220
#define NOTE_A3_SHARP             (u16)233
#define NOTE_B3                   (u16)245
#define NOTE_C4                   (u16)262
#define NOTE_C4_SHARP             (u16)277
#define NOTE_D4                   (u16)294
#define NOTE_D4_SHARP             (u16)311
#define NOTE_E4                   (u16)330
#define NOTE_F4                   (u16)349
#define NOTE_F4_SHARP             (u16)370
#define NOTE_G4                   (u16)392
#define NOTE_G4_SHARP             (u16)415
#define NOTE_A4                   (u16)440
#define NOTE_A4_SHARP             (u16)466
#define NOTE_B4                   (u16)494
#define NOTE_C5                   (u16)523
#define NOTE_C5_SHARP             (u16)554
#define NOTE_D5                   (u16)587
#define NOTE_D5_SHARP             (u16)622
#define NOTE_E5                   (u16)659
#define NOTE_F5                   (u16)698
#define NOTE_F5_SHARP             (u16)740
#define NOTE_G5                   (u16)784
#define NOTE_G5_SHARP             (u16)831
#define NOTE_A5                   (u16)880
#define NOTE_A5_SHARP             (u16)932
#define NOTE_B5                   (u16)988
#define NOTE_C6                   (u16)1047
#define NOTE_C6_SHARP             (u16)1109
#define NOTE_D6                   (u16)1175
#define NOTE_D6_SHARP             (u16)1245
#define NOTE_E6                   (u16)1319
#define NOTE_F6                   (u16)1397
#define NOTE_F6_SHARP             (u16)1480
#define NOTE_G6                   (u16)1568
#define NOTE_G6_SHARP             (u16)1661
#define NOTE_A6                   (u16)1760
#define NOTE_A6_SHARP             (u16)1865
#define NOTE_B6                   (u16)1976
#define NONE                      (u16)0

/* Musical note definitions - short hand */
#define C3                   (u32)NOTE_C3
#define C3S                  (u32)NOTE_C3_SHARP
#define D3                   (u32)NOTE_D3
#define D3S                  (u32)NOTE_D3_SHARP
#define E3                   (u32)NOTE_E3
#define F3                   (u32)NOTE_F3
#define F3S                  (u32)NOTE_F3_SHARP
#define G3                   (u32)NOTE_G3
#define G3S                  (u32)NOTE_G3_SHARP
#define A3                   (u32)NOTE_A3
#define A3S                  (u32)NOTE_A3_SHARP
#define B3                   (u32)NOTE_B3
#define C4                   (u32)NOTE_C4  /* Middle C */
#define C4S                  (u32)NOTE_C4_SHARP
#define D4                   (u32)NOTE_D4
#define D4S                  (u32)NOTE_D4_SHARP
#define E4                   (u32)NOTE_E4
#define F4                   (u32)NOTE_F4
#define F4S                  (u32)NOTE_F4_SHARP
#define G4                   (u32)NOTE_G4
#define G4S                  (u32)NOTE_G4_SHARP
#define A4                   (u32)NOTE_A4
#define A4S                  (u32)NOTE_A4_SHARP
#define B4                   (u32)NOTE_B4
#define C5                   (u32)NOTE_C5
#define C5S                  (u32)NOTE_C5_SHARP
#define D5                   (u32)NOTE_D5
#define D5S                  (u32)NOTE_D5_SHARP
#define E5                   (u32)NOTE_E5
#define F5                   (u32)NOTE_F5
#define F5S                  (u32)NOTE_F5_SHARP
#define G5                   (u32)NOTE_G5
#define G5S                  (u32)NOTE_G5_SHARP
#define A5                   (u32)NOTE_A5
#define A5S                  (u32)NOTE_A5_SHARP
#define B5                   (u32)NOTE_B5
#define C6                   (u32)NOTE_C6
#define C6S                  (u32)NOTE_C6_SHARP
#define D6                   (u32)NOTE_D6
#define D6S                  (u32)NOTE_D6_SHARP
#define E6                   (u32)NOTE_E6
#define F6                   (u32)NOTE_F6
#define F6S                  (u32)NOTE_F6_SHARP
#define G6                   (u32)NOTE_G6
#define G6S                  (u32)NOTE_G6_SHARP
#define A6                   (u32)NOTE_A6
#define A6S                  (u32)NOTE_A6_SHARP
#define B6                   (u32)NOTE_B6
#define NO                   (u32)NONE