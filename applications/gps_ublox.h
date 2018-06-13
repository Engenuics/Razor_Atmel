/*******************************************************************************
* File: gps_ublox.h                                                                
* Author: Jason Long                                                                                                     
*******************************************************************************/

#ifndef __GPS_UBLOX_H
#define __GPS_UBLOX_H


/*******************************************************************************
* Constants / Definitions
*******************************************************************************/
/* GGu32GPSFlags bits */
#define _GPS_FLAGS_SM_MANUAL        0x00000001    /* The GPS SM is being run in manual mode */
#define _GPS_FLAGS_DATA_OUTPUT      0x00000002    /* Enable GPS data output messages */
#define _GPS_FLAGS_PASSTHROUGH      0x00000004    /* Enable GPS pass-through */
#define _GPS_FLAGS_DATA_TRUNCATED   0x00010000    /* GPSExtractFieldFromNMEA truncated a data field */
#define _GPS_FLAGS_2D_FIX           0x00020000    /* The GPS has a 2D fix */
#define _GPS_FLAGS_3D_FIX           0x00040000    /* The GPS has a 3D fix */

#define _GPS_NEW_DATA_LOCATION      0x00100000    /* New Lat / Long position data has been loaded */
#define _GPS_NEW_DATA_FIX           0x00200000    /* New fix data has been loaded */
#define _GPS_NEW_DATA_SPEED         0x00400000    /* New speed / direction data has been loaded */
#define _GPS_NEW_DATA_SV            0x00800000    /* New satellite vehicle data has been loaded */

#define GPS_STARTUP_TIME            (u32)100      /* ms to wait before GPS power on and reset release */
#define GPS_STARTUP_MSG_TIMEOUT     (u32)3000     /* ms to wait until the GPS spits out startup info */
#define GPS_STARTUP_MESSAGES        (u32)5        /* Number of startup messages expected from GPS module */
#define GPS_MESSAGE_TIMEOUT         (u32)100      /* Timeout value for GPS messaging functions */
#define GPS_DEBUG_DATA_RATE         (u32)5000     /* Time value in ms for GPS data output to debug */

#define GPS_RX_BUFFER_SIZE          (u16)2048     /* Number of chars in circular LGau8GPSRxBuffer */

#define GPS_UART_MSG_SRC            BLADE_UART_MSG_SRC

#define GPS_RESPONSE_TIMEOUT        (u32)5000     /* Max time in ms to wait for an expected response to a UBX message */

/* State machine error codes (max 99) */
#define GPS_ERROR_NONE              (u8)0         /* No error flagged */ 
#define GPS_ERROR_INCOMING_TIMEOUT  (u8)1         /* Expected characters took too long to arrive */ 
#define GPS_ERROR_NMEA_FAIL_LF      (u8)2         /* An expected <LF> character was not <LF> */ 
#define GPS_ERROR_UBX_FAIL_SYNC2    (u8)3         /* An expected UBX_SYNC_CHAR2 character was not UBX_SYNC_CHAR2 */ 
#define GPS_ERROR_UBX_FAIL_HEADER   (u8)4         /* An expected case was reached on incoming header */ 
#define GPS_ERROR_UBX_FAIL_CHECKSUM (u8)5         /* A UBX message was received but the checksum failed */ 



/* Data that we care about for now.  All arrays are null-terminated strings */
typedef struct
{
  u8 strLatitude[14];         /* ASCII latitude in signed decimal degrees */
  s32 s32Latitude;            /* Signed numeric latitude in degrees x 11930465 */
  u8 strLongitude[14];        /* ASCII longitude in signed decimal degrees */
  s32 s32Longitude;           /* Signed numeric longitude in degrees x 11930465 */
  u8 strMSLAltitude[8];       /* Number with example of 499.6 - not sure on full range and precision or negative */
  s32 s32Altitude;            /* Signed number of altitude in m */
  u8 strFixStatus[3];         /* NA, 2D, 3D               */
  u8 strNumSVsTracking[3];    /* Number in range 0-32     */
  u8 strNumSVsUsed[3];        /* Number in range 0-12     */
  u8 strSpeed[14];            /* Speed e.g. 100km/h       */
  u32 u32Speed;               /* Unsigned speed in km/h x 1000  */
  u8 strHeading[18];          /* Heading e.g. 348 degrees */
  u32 u32Heading;             /* Unsigned numeric heading */
  u8 u8Dummy[3];              /* Pad for 4-byte alignment */
} GPSDataStructType;

typedef struct
{
  u8 u8SVID[3];               /* ASCII string for SV ID # */
  u8 u8CNo;                   /* Numerical value for CNo */
} GPSSVDataType;

typedef enum {GPGGA, GPGSA, GPVTG, GPGSV} GPSMessageType;

typedef struct
{
  u8 u8Class;                 /* ublox binary message CLASS */
  u8 u8MsgID;                 /* ublox binary message ID */
  u16 u16Length;              /* ublox binary message length */
  u8 *pu8Payload;             /* Pointer to ublox binary message content */
} UbxMessageType;


typedef struct
{
  u8 u8Class;                 /* ublox binary message CLASS */
  u8 u8MsgID;                 /* ublox binary message ID */
  u32 u32TimeStamp;           /* ms count when message was posted */
  u8 u8Dummy1;                /* Dummy pad for 4-byte alignment */
  u8 u8Dummy2;                /* Dummy pad for 4-byte alignment */
} UbxResponseRecordType;

typedef struct
{
  UbxResponseRecordType sRecord;  /* Message record struct */
  void* pNextRecord;              /* Pointer to the next record */
} UbxResponseListNodeType;

typedef enum {FAIL_NAK = 1, FAIL_TIMEOUT} UbxMsgFailType;

typedef struct
{
  u8 u8Class;                 /* ublox binary message CLASS */
  u8 u8MsgID;                 /* ublox binary message ID */
  UbxMsgFailType eFailReason; /* Message failure code */
} UbxFailRecordType;

typedef struct
{
  UbxFailRecordType sRecord;  /* Message record struct */
  void* pNextRecord;          /* Pointer to the next record */
} UbxFailMsgListNodeType;



/*******************************************************************************
* Function Declarations
*******************************************************************************/
/* Private functions */
void GpsClearAllData(void);
void GPSClearSVData(GPSSVDataType*);
bool GPSVerifyMessageNMEA(u8*);
void GPSProcessMessageNMEA(void);
void GPSProcessMessageUBX(UbxMessageType* UbxMessage_);
void GPSExtractFieldFromNMEA(u8, u8*);
void GPSShowData(void);

void GPSAdvanceRxBufferParserPointer(void);
void GPSAdvanceRxCurrentCharPointer(void);
void GPSAdvanceRxNextCharPointer(void);

bool QueueUbxTxMessage(UbxMessageType* psMessage_, bool bResponseExpected_);
void QueueMsgResponse(UbxResponseRecordType* psResponseRecord_);
void DequeueMsgResponse(u8 u8Class_, u8 u8MsgID_);
void CheckMessageTimeOut(void);
void QueueMsgFail(UbxFailRecordType* psFailRecord_);

/* Public functions */
void GpsPowerOff(void);
void GpsPowerOn(void);

/* Protected functions */
void GpsIOInitialize(void);
void GpsInitialize(void);

void GPSSMIdle(void);       

void GPSSMIncomingNMEA(void);
void GPSSMWaitForLF(void);

void GPSSMIncomingUBX(void);
void GPSSMWaitUbxHeader(void);
void GPSSMUbxPayload(void);        
void GPSSMUbxWaitChecksum(void);        

void GPSSMError(void);


/* Messages Information:

Default NMEA messages from ublox [data of interest]
GGA - Global positioning system fix data [Lat, Long, Sats used, Altitude]
GLL - Latitude and longitude, with time of positon fix and status [Lat, Long, Time]
GSA - GNSS DOP and Active Satellites [Fix status]
GSV - GNSS Satellites in View (up to four message) [SVs in view, SVID, SVCNo]
RMC - Recommended Minimum data [Time, Lat, Long, Heading]
VTG - Course over ground and Ground speed [Heading, Speed]

To get the data we want, need to parse GGA, GSA, VTG and GSV
*/

#define GPS_MESSAGES             (u8)4      /* Number of GPS messages in GPSMessageType*/
#define MAX_FIELD_LENGTH         (u8)15     /* Maximum chars in a GPS data field */

#define NMEA_MSG_START_CHAR      (u8)'$'    /* All NMEA messages start with this char */
#define NMEA_MSG_CHECKSUM_CHAR   (u8)'*'    /* All NMEA message checksums start with this char */
#define NMEA_MSG_ADDRESS_SIZE    (u8)5      /* Max length of the NMEA Address field */

/* GGA NMEA message fields of interest*/
#define GGA_UTC_TIME			       (u8)1
#define GGA_LATITUDE			       (u8)2
#define GGA_LAT_DIRECTION		     (u8)3
#define GGA_LONGITUDE			       (u8)4
#define GGA_LONG_DIRECTION		   (u8)5
#define GGA_FIX_INDICATOR		     (u8)6
#define GGA_SATS_USED			       (u8)7
#define GGA_HDOP				         (u8)8
#define GGA_ALTITUDE			       (u8)9

/* GSA NMEA message fields of interest */
#define GSA_MODE   				       (u8)1
#define GSA_FIX_INDICATOR		     (u8)2

/* VTG NMEA message fields of interest */
#define VTG_COURSE  			       (u8)1
#define VTG_SPEED_KPH			       (u8)7

/* GSV NMEA message fields of interest */
#define GSV_NUMBER_OF_MESSAGES	 (u8)1 	
#define GSV_MESSAGE_NUMBER		   (u8)2
#define GSV_SATS_IN_VIEW		     (u8)3
/* Offset by 4 fields for each sat */
#define GSV_SAT_PRN				       (u8)4
#define GSV_SAT_CNO				       (u8)7
#define GPS_MAX_SV_DATA          (u8)16      /* Max number of tracked SVs that GSV will provide data for */


/* ublox binary commands
To set certain power modes and other receiver parameters, ublox binary messages are used.
The packet structure for ublox binary is as follows:

SYNC1  SYNC2  CLASS   ID     LENGTH  PAYLOAD  CHECKSUM
0xb5   0x62   1byte   1byte  2bytes  Length   2bytes
*/

#define   UBX_SYNC_CHAR1        (u8)0xb5       /* First SYNC character in a ublox binary message frame */
#define   UBX_SYNC_CHAR2        (u8)0x62       /* Second SYNC character in a ublox binary message frame */
#define   UBX_SYNC_BYTES_SIZE   (u8)2          /* Number of SYNC bytes */
#define   UBX_MSG_HEADER_SIZE   (u8)6          /* Size of class, ID and length information */
#define   UBX_MSG_OVERHEAD_SIZE (u8)8          /* Total number of overhead bytes in UBX message */

#define   UBX_SYNC_CHAR1_INDEX  (u8)0          /* Packet location of SYNC1 byte */
#define   UBX_SYNC_CHAR2_INDEX  (u8)1          /* Packet location of SYNC2 byte */
#define   UBX_CLASS_INDEX       (u8)2          /* Packet location of CLASS byte */
#define   UBX_ID_INDEX          (u8)3          /* Packet location of ID byte */
#define   UBX_LENGTH_LO_INDEX   (u8)4          /* Packet location of LENGTH low byte */
#define   UBX_LENGTH_HI_INDEX   (u8)5          /* Packet location of LENGTH high byte */
#define   UBX_PAYLOAD_INDEX     (u8)6          /* Location of the start of the payload bytes */


/* Class definitions */
#define   UBXMSG_CLASS_NAV     (u8)0x01        /* Navigation Results: Position, Speed, Time, Acc, Heading, DOP, SVs used */
#define   UBXMSG_CLASS_RXM     (u8)0x02        /* Receiver Manager Messages: Satellite Status, RTC Status */
#define   UBXMSG_CLASS_INF     (u8)0x04        /* Information Messages: Printf-Style Messages, with IDs such as Error, Warning, Notice */
#define   UBXMSG_CLASS_ACK     (u8)0x05        /* Ack/Nack Messages: as replies to CFG Input Messages */
#define   UBXMSG_CLASS_CFG     (u8)0x06        /* Configuration Input Messages: Set Dynamic Model, Set DOP Mask, Set Baud Rate, etc. */
#define   UBXMSG_CLASS_MON     (u8)0x0A        /* Monitoring Messages: Comunication Status, CPU Load, Stack Usage, Task Status */
#define   UBXMSG_CLASS_AID     (u8)0x0B        /* AssistNow Aiding Messages: Ephemeris, Almanac, other A-GPS data input */
#define   UBXMSG_CLASS_TIM     (u8)0x0D        /* Timing Messages: Timepulse Output, Timemark Results */
#define   UBXMSG_CLASS_ESF     (u8)0x10        /* External Sensor Fusion Messages: External sensor measurements and status information */
#define   UBXMSG_CLASS_NMEA    (u8)0xF0        /* Standard NMEA messages class */

/* ACK class messages */
#define   UBXMSG_ID_ACK               (u8)0x01        /* Message Acknowledge */
#define   UBXMSG_LENGTH_ACK           (u8)2           /* Length of ACK message */ 
#define   UBXMSG_ACK_CLSID_INDEX      (u16)0          /* Index in payload of Class ID of Acknowledged Message */
#define   UBXMSG_ACK_MSGID_INDEX      (u16)1          /* Index in payload of Message ID of Acknowledged Message */

#define   UBXMSG_ID_NAK               (u8)0x00        /* Message Not-Acknowledge */
#define   UBXMSG_LENGTH_NAK           (u8)2           /* Length of NAK message */ 
#define   UBXMSG_NAK_CLSID_INDEX      (u16)0          /* Index in payload of Class ID of Not-Acknowledged Message */
#define   UBXMSG_NAK_MSGID_INDEX      (u16)1          /* Index in payload of Message ID of Not-Acknowledged Message */

/* CFG class message IDs and Lengths (list includes only messages of interest) */
#define   UBXMSG_ID_CFG        (u8)0x09        /* Clear, Save and Load configurations */
#define   UBXMSG_ID_FXN        (u8)0x0E        /* FixNOW configuration */ 
#define   UBXMSG_ID_MSG        (u8)0x01        /* Set Message Rate */ 
#define   UBXMSG_ID_NAV5       (u8)0x24        /* Get/Set Navigation Engine Settings */ 
#define   UBXMSG_ID_NMEA       (u8)0x17        /* Set/Get the NMEA protocol configuration */ 
#define   UBXMSG_ID_PM2        (u8)0x3B        /* Extended Power Management configuration */ 
#define   UBXMSG_ID_PM         (u8)0x32        /* Power Management configuration */ 
#define   UBXMSG_ID_PRT        (u8)0x00        /* Get/Set Port Configuration */ 
#define   UBXMSG_ID_RATE       (u8)0x08        /* Navigation/Measurement Rate Settings */ 
#define   UBXMSG_ID_RXM        (u8)0x11        /* RXM configuration */ 
#define   UBXMSG_ID_SBAS       (u8)0x16        /* SBAS configuration */ 
#define   UBXMSG_ID_TMODE2     (u8)0x3D        /* Time Mode Settings 2 */ 
#define   UBXMSG_ID_TMODE      (u8)0x31        /* Time Mode Settings */ 
#define   UBXMSG_ID_TP         (u8)0x07        /* TimePulse Parameters */ 

#define   UBXMSG_LENGTH_CFG_1  (u16)12         /* Length of Clear, Save and Load configurations */
#define   UBXMSG_LENGTH_CFG_2  (u16)13         /* Length of Clear, Save and Load configurations */
#define   UBXMSG_LENGTH_FXN    (u16)36         /* Length of FixNOW configuration */ 
#define   UBXMSG_LENGTH_MSG    (u16)3          /* Length of Set Message Rate (single) */ 
#define   UBXMSG_LENGTH_NAV5   (u16)36         /* Length of Get/Set Navigation Engine Settings */ 
#define   UBXMSG_LENGTH_NMEA   (u16)4          /* Length of Set/Get the NMEA protocol configuration */ 
#define   UBXMSG_LENGTH_PM2    (u16)44         /* Length of Extended Power Management configuration */ 
#define   UBXMSG_LENGTH_PM     (u16)24         /* Length of Power Management configuration */ 
#define   UBXMSG_LENGTH_PRT    (u16)20         /* Length of Get/Set Port Configuration */ 
#define   UBXMSG_LENGTH_RATE   (u16)6          /* Length of Navigation/Measurement Rate Settings */ 
#define   UBXMSG_LENGTH_RXM    (u16)2          /* Length of RXM configuration */ 
#define   UBXMSG_LENGTH_SBAS   (u16)8          /* Length of SBAS configuration */ 
#define   UBXMSG_LENGTH_TMODE2 (u16)28         /* Length of Time Mode Settings 2 */ 
#define   UBXMSG_LENGTH_TMODE  (u16)28         /* Length of Time Mode Settings */ 
#define   UBXMSG_LENGTH_TP     (u16)20         /* Length of TimePulse Parameters */ 

/* FXN Message Fields */
#define   FXN_FLAGS0            (u8)0x1E        /* Default 0x0C Configuration flag byte */
#define   FXN_FLAGS1            (u8)0x00        /* Configuration flag byte */
#define   FXN_FLAGS2            (u8)0x00        /* Configuration flag byte */
#define   FXN_FLAGS3            (u8)0x00        /* Configuration flag byte */
#define   FXN_T_REACQ0          (u8)0x00        /* Default 0 SV re-acquire time (ms) */
#define   FXN_T_REACQ1          (u8)0x00        /* SV re-acquire time (ms) */
#define   FXN_T_REACQ2          (u8)0x00        /* SV re-acquire time (ms) */
#define   FXN_T_REACQ3          (u8)0x00        /* SV re-acquire time (ms) */
#define   FXN_T_ACQ0            (u8)0x00        /* Default 0 SV acquire time (ms) */
#define   FXN_T_ACQ1            (u8)0x00        /* SV acquire time (ms) */
#define   FXN_T_ACQ2            (u8)0x00        /* SV acquire time (ms) */
#define   FXN_T_ACQ3            (u8)0x00        /* SV acquire time (ms) */
#define   FXN_T_REACQ_OFF0      (u8)0x10        /* Default 0x00002710 Time receiver is off if re-acquire fails (ms) */
#define   FXN_T_REACQ_OFF1      (u8)0x27        /* Time receiver is off if re-acquire fails (ms) */
#define   FXN_T_REACQ_OFF2      (u8)0x00        /* Time receiver is off if re-acquire fails (ms) */
#define   FXN_T_REACQ_OFF3      (u8)0x00        /* Time receiver is off if re-acquire fails (ms) */
#define   FXN_T_ACQ_OFF0        (u8)0x10        /* Default 0x00002710 Time receiver is off if acquire fails (ms) */
#define   FXN_T_ACQ_OFF1        (u8)0x27        /* Time receiver is off if acquire fails (ms) */
#define   FXN_T_ACQ_OFF2        (u8)0x00        /* Time receiver is off if acquire fails (ms) */
#define   FXN_T_ACQ_OFF3        (u8)0x00        /* Time receiver is off if acquire fails (ms) */
#define   FXN_T_ON0             (u8)0xE8        /* Default 0x000007d0 Time receiver is on after fix (ms) */
#define   FXN_T_ON1             (u8)0x03        /* Time receiver is on after fix (ms) */
#define   FXN_T_ON2             (u8)0x00        /* Time receiver is on after fix (ms) */
#define   FXN_T_ON3             (u8)0x00        /* Time receiver is on after fix (ms) */
#define   FXN_T_OFF0            (u8)0x28        /* Default 0xFFFFFC18 Sleep time after on time (ms) */
#define   FXN_T_OFF1            (u8)0x23        /* Sleep time after on time (ms) */
#define   FXN_T_OFF2            (u8)0x00        /* Sleep time after on time (ms) */
#define   FXN_T_OFF3            (u8)0x00        /* Sleep time after on time (ms) */

/* PM Message Fields */
#define   PM_MSG_VERSION       (u8)0           /* Message version */
#define   PM_FLAGS0            (u8)0x04        /* Default 0x04 Configuration flag byte */
//#define   PM_FLAGS0_BIT_SELECT (u8)0x10        /* Bit in FLAGS0 for EXTINT pin select */
//#define   PM_FLAGS0_BIT_WAKE   (u8)0x20        /* Bit in FLAGS0 for EXTINT pin high keeps receiver awake */
//#define   PM_FLAGS0_BIT_BACKUP (u8)0x40        /* Bit in FLAGS0 for EXTINT pin low for receiver backup mode */
#define   PM_FLAGS1            (u8)0x90        /* Default 0x90 Configuration flag byte */
//#define   PM_FLAGS1_BIT_PEAK   (u8)0x01        /* Bit in FLAGS1 to enable peak current */
//#define   PM_FLAGS1_BIT_TFIX   (u8)0x04        /* Bit in FLAGS1 controls when on-time count starts */
//#define   PM_FLAGS1_BIT_RTC    (u8)0x08        /* Bit in FLAGS1 adds wake-up cycles to update the RTC */
//#define   PM_FLAGS1_BIT_EPH    (u8)0x10        /* Bit in FLAGS1 sets automatic Ephemeris update */
#define   PM_FLAGS2            (u8)0x00        /* Configuration flag byte */
//#define   PM_FLAGS2_BIT_OFF    (u8)0x01        /* Bit in FLAGS2 to prevent entering inactive */
//#define   PM_FLAGS2_BIT_MODE   (u8)0x02        /* Bit in FLAGS2 to select ON/OFF (0) or Cyclic (1) Power Save mode */
#define   PM_FLAGS3            (u8)0x00        /* Configuration flag byte */
#define   PM_UPDATE_PERIOD0    (u8)0x10        /* Default 0x000003E8 Position update period (4 bytes, ms) */
#define   PM_UPDATE_PERIOD1    (u8)0x27        /* Position update period (4 bytes, ms) */
#define   PM_UPDATE_PERIOD2    (u8)0x00        /* Position update period (4 bytes, ms) */
#define   PM_UPDATE_PERIOD3    (u8)0x00        /* Position update period (4 bytes, ms) */
#define   PM_SEARCH_PERIOD0    (u8)0x10        /* Default 0x00002710 Acquisition retry period (4 bytes, ms) */
#define   PM_SEARCH_PERIOD1    (u8)0x27        /* Acquisition retry period (4 bytes, ms) */
#define   PM_SEARCH_PERIOD2    (u8)0x00        /* Acquisition retry period (4 bytes, ms) */
#define   PM_SEARCH_PERIOD3    (u8)0x00        /* Acquisition retry period (4 bytes, ms) */
#define   PM_GRID_OFFSET0      (u8)0x00        /* Grid offset relative to GPS start of week (4 bytes, ms) */
#define   PM_GRID_OFFSET1      (u8)0x00        /* Grid offset relative to GPS start of week (4 bytes, ms) */
#define   PM_GRID_OFFSET2      (u8)0x00        /* Grid offset relative to GPS start of week (4 bytes, ms) */
#define   PM_GRID_OFFSET3      (u8)0x00        /* Grid offset relative to GPS start of week (4 bytes, ms) */
#define   PM_ON_TIME0          (u8)0x01        /* Default 0x02 On time after fix (2 bytes, s) */
#define   PM_ON_TIME1          (u8)0x00        /* On time after fix (2 bytes, s) */
#define   PM_MIN_SEARCH_TIME0  (u8)0x00        /* Default 0 Minimum search time (2 bytes, s) */
#define   PM_MIN_SEARCH_TIME1  (u8)0x00        /* Minimum search time (2 bytes, s) */


/* RXM Message Fields */
#define   RXM_RESERVED         (u8)8           /* Always set to 8 */
#define   RXM_LPMODE_MAX       (u8)0           /* Select max power mode */
#define   RXM_LPMODE_POWERSAVE (u8)1           /* Select Power Save mode */
#define   RXM_LPMODE_ECO       (u8)4           /* Select ECO mode */

/* MON class message */
#define   UBXMSG_ID_HW         (u8)0x09        /* HW status message */
#define   UBXMSG_ID_HW2        (u8)0x0B        /* HW2 status message */
#define   UBXMSG_ID_IO         (u8)0x02        /* IO status status message */
#define   UBXMSG_ID_VER        (u8)0x04        /* Receiver/Software/ROM version message */

#define   UBXMSG_VER_SW_INDEX    (u16)0        /* Index in MON-VER for software version string */
#define   UBXMSG_VER_SW_LENGTH   (u16)30       /* Length in MON-VER for software version string */
#define   UBXMSG_VER_HW_INDEX    (u16)30       /* Index in MON-VER for hardware version string */
#define   UBXMSG_VER_HW_LENGTH   (u16)10       /* Length in MON-VER for hardware version string */
#define   UBXMSG_VER_ROM_INDEX   (u16)40       /* Index in MON-VER for software version string */
#define   UBXMSG_VER_ROM_LENGTH  (u16)30       /* Length in MON-VER for software version string */

/* Message IDs for NMEA standard messages for use with some UBX commands */
#define   UBXMSG_NMEA_ID_GGA     (u8)0x00      /* */
#define   UBXMSG_NMEA_ID_GLL     (u8)0x01      /* */
#define   UBXMSG_NMEA_ID_GSV     (u8)0x03      /* */
#define   UBXMSG_NMEA_ID_RMC     (u8)0x04      /* */
#define   UBXMSG_NMEA_ID_VTG     (u8)0x05      /* */
 



/* Commands of interest:
UBX-CFG-PRT   Configures the various communication ports (want only UART1)
UBX-CFG-MSG   Enables / disables messages and sets messaging rate (1Hz, only those required for Nav)
UBX-CFG-TP    Power mode settings
UBX-CFG-TP2
UBX-CFG-PM
UBX-CFG-PM2   Eco Mode configured by default - acquisition engine powered off, only tracking engine active once enough satellites are available
              Power Save Mode - Cyclic tracking or on/off operation
UBX-CFG-RXM
*/


/*******************************************************************************
* Hardware definitions
*******************************************************************************/
#define GPS_UART_PIMO               P4_29_BLADE_U3_PIMO 
#define GPS_UART_POMI               P4_28_BLADE_U3_POMI
#define GPS_I2C_SCL                 P0_11_BLADE_I2C2_SCL
#define GPS_I2C_SDA                 P0_10_BLADE_I2C2_SDA
#define GPS_PWR_EN                  P1_31_BLADE_IO1
#define GPS_1PPS                    P1_30_BLADE_IO0
#define GPS_BACKUP_RST              P0_09_BLADE_IO4
#define GPS_SAFEBOOT                P0_08_BLADE_IO5

#define GPS_GPIO0_PINSEL0_CLEAR     0xFFF00FFF     
#define GPS_GPIO1_PINSEL3_CLEAR     0x0FFFFFFF     

/* GPS UART Peripheral Allocation */
#define GPS_UART                    BLADE_UART
#define GPS_UART_IRQHandler         BLADE_UART_IRQHandler
#define GPS_UART_PCONP_BIT          BLADE_UART_PCONP_BIT

/* UART clock PCLKU0 = PCLK / 4 = 3000000.  
For 9600 baud:
9600 = PCLKU0 / (16 * DL x (1 + DIVADDVAL / MULVAL) 
DL = 13, DIVADDVAL = 1, MULVAL = 2
Baud clock = 9615 (-0.16% error)
*/

#define GPS_UART_DLM_INIT          (u32)0x00000000
#define GPS_UART_DLL_INIT          (u32)0x0000000D
#define GPS_UART_FDR_INIT          (u32)0x00000021
/* Bit Set Description
    31:08 Reserved 

    07 [0] MULVAL = 2
    06 [0] "
    05 [1] "
    04 [0] "

    03 [0] DIVADDVAL = 1
    02 [0] "
    01 [0] "
    00 [1] "
*/

#define GPS_UART_FCR_INIT (u32)0x00000007
/* Bit Set Description
    31-08 Reserved 

    07 [0] Rx Trigger Level 1 character
    06 [0] "
    05 [0] Reserved
    04 [0] "

    03 [0] No DMA
    02 [1] Reset the Tx FIFO
    01 [1] Reset the Rx FIFO 
    00 [1] FIFOs enabled
*/

#define GPS_UART_LCR_INIT (u32)0x00000083
/* Bit Set Description
    31-08 Reserved 

    07 [1] Divisor latch access enabled
    06 [0] Break Control disabled
    05 [0] Parity Select N/A
    04 [0] "

    03 [0] Parity disabled
    02 [0] 1 stop bit
    01 [1] 8 data bits
    00 [1] "
*/

#define GPS_UART_IER_INIT (u32)0x00000000
/* Bit Set Description
    31 - 12 Reserved

    11 [0] Reserved
    10 [0] "
    09 [0] Auto baud timeout interrupt disabled
    08 [0] End of auto baud interrupt disabled

    07 [0] CTS interrupt disabled
    06 [0] Reserved
    05 [0] "
    04 [0] "

    03 [0] Modem status interrupt disabled
    02 [0] Rx line interrupt disabled
    01 [0] THRE interrupt interrupt disabled
    00 [0] Receive data interrupt disabled
*/

#define UART_INT_ID_MASK    (u32)0x0000000E
#define UART_INT_ID_RLS     (u32)0x00000006
#define UART_INT_ID_RDA     (u32)0x00000004
#define UART_INT_ID_CTI     (u32)0x0000000C
#define UART_INT_ID_THRE    (u32)0x00000002



#endif /* __GPS_UBLOX_H */