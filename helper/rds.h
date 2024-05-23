#ifndef RDS_H
#define RDS_H

#include <stdbool.h>
#include <stdint.h>

typedef union {
  struct {
    // status ("RESP0")
    uint8_t STCINT : 1;
    uint8_t DUMMY1 : 1;
    uint8_t RDSINT : 1;
    uint8_t RSQINT : 1;
    uint8_t DUMMY2 : 2;
    uint8_t ERR : 1;
    uint8_t CTS : 1;
    // RESP1
    uint8_t RDSRECV : 1; //!<  RDS Received; 1 = FIFO filled to minimum number
                         //!<  of groups set by RDSFIFOCNT.
    uint8_t RDSSYNCLOST : 1; //!<  RDS Sync Lost; 1 = Lost RDS synchronization.
    uint8_t
        RDSSYNCFOUND : 1; //!<  RDS Sync Found; 1 = Found RDS synchronization.
    uint8_t DUMMY3 : 1;
    uint8_t RDSNEWBLOCKA : 1; //!<  RDS New Block A; 1 = Valid Block A data has
                              //!<  been received.
    uint8_t RDSNEWBLOCKB : 1; //!<  RDS New Block B; 1 = Valid Block B data has
                              //!<  been received.
    uint8_t DUMMY4 : 2;
    // RESP2
    uint8_t RDSSYNC : 1; //!<  RDS Sync; 1 = RDS currently synchronized.
    uint8_t DUMMY5 : 1;
    uint8_t GRPLOST : 1; //!<  Group Lost; 1 = One or more RDS groups discarded
                         //!<  due to FIFO overrun.
    uint8_t DUMMY6 : 5;
    // RESP3 to RESP11
    uint8_t RDSFIFOUSED; //!<  RESP3 - RDS FIFO Used; Number of groups remaining
                         //!<  in the RDS FIFO (0 if empty).
    uint8_t BLOCKAH;     //!<  RESP4 - RDS Block A; HIGH byte
    uint8_t BLOCKAL;     //!<  RESP5 - RDS Block A; LOW byte
    uint8_t BLOCKBH;     //!<  RESP6 - RDS Block B; HIGH byte
    uint8_t BLOCKBL;     //!<  RESP7 - RDS Block B; LOW byte
    uint8_t BLOCKCH;     //!<  RESP8 - RDS Block C; HIGH byte
    uint8_t BLOCKCL;     //!<  RESP9 - RDS Block C; LOW byte
    uint8_t BLOCKDH;     //!<  RESP10 - RDS Block D; HIGH byte
    uint8_t BLOCKDL;     //!<  RESP11 - RDS Block D; LOW byte
    // RESP12 - Blocks A to D Corrected Errors.
    // 0 = No errors;
    // 1 = 1–2 bit errors detected and corrected;
    // 2 = 3–5 bit errors detected and corrected.
    // 3 = Uncorrectable.
    uint8_t BLED : 2;
    uint8_t BLEC : 2;
    uint8_t BLEB : 2;
    uint8_t BLEA : 2;
  } resp;
  uint8_t raw[13];
} si47x_rds_status;

typedef signed char ternary;
/* RDS and RBDS data */
typedef struct {
  uint16_t
      programId; // Program Identification (PI) code - unique code assigned to
                 // program. In the US, except for simulcast stations, each
                 // station has a unique PI. PI = 0 if no RDS info received.
  /* groupA and groupB indicate if the station has broadcast one or more of each
   * RDS group type and version. There is one bit for each group type.  Bit
   * number 0 is for group type 0, and so on. groupA gives version A groups
   * (packets), groupB gives version B groups. If a bit is true then one or more
   * of that group type and version has been received. Example:  If (groupA &
   * 1<<4) is true then at least one Group type 4, version A group (packet) has
   * been received. Note: If the RDS signal is weak, many bad packets will be
   * received.  Sometimes, the packets are so corrupted that the radio thinks
   * the bad data is OK.  This can cause false information to be recorded in the
   * groupA and groupB variables.
   */
  uint16_t groupA;     // One bit for each group type, version A
  uint16_t groupB;     // One bit for each group type, version B
  bool RDSSignal;      // True if RDS (or RBDS) signal currently detected
  bool RBDS;           // True if station using RBDS, else using RDS
  uint8_t programType; // Program Type (PTY) code - identifies program format -
                       // call getProgramTypeStr()
  uint8_t extendedCountryCode; // Extended Country Code (ECC) - constants
                               // defined above
  uint8_t language;            // Language Code - constants defined above
  ternary trafficProgram;      // Traffic Program flag - True if station gives
                               // Traffic Alerts
  ternary trafficAlert;        // Traffic Alert flag - True if station currently
                               // broadcasting Traffic Alert
  ternary
      music; // Music/speech flag - True if broadcasting music, false if speech
  ternary dynamicPTY;      // Dynamic PTY flag - True if dynamic (changing) PTY,
                           // false if static PTY
  ternary compressedAudio; // Compressed audio flag - True if compressed audio,
                           // false if not compressed
  ternary binauralAudio; // Binaural audio flag - True if binaural audio, false
                         // if not binaural audio
  ternary RDSStereo; // RDS stereo/mono flag - True if RDS info says station is
                     // stereo, false if mono
  char programService[9];  // Station's name or slogan - usually used like Radio
                           // Text
  uint8_t radioTextLen;    // Length of Radio Text message
  char radioText[65];      // Descriptive message from station
  char programTypeName[9]; // Program Type Name (PTYN)
  unsigned long MJD; // UTC Modified Julian Date - origin is November 17, 1858
  uint8_t hour;      // UTC Hour
  uint8_t minute;    // UTC Minute
  signed char
      offset; // Offset measured in half hours to convert UTC to local time.
              // If offset==NO_DATE_TIME then MJD, hour, minute are invalid.
} RDS;

typedef struct DateTime {
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t wday; // Day of the week, Sunday = 0
  uint8_t hour;
  uint8_t minute;
} DateTime;

typedef struct Time {
  uint8_t hour;
  uint8_t minute;
} Time;

bool SI47XX_GetLocalDateTime(DateTime *time);
bool SI47XX_GetLocalTime(Time *time);
void SI47XX_GetProgramType(char buffer[17]);
void SI47XX_ClearRDS();
bool SI47XX_GetRDS();

extern RDS rds;
extern si47x_rds_status rdsResponse;

#endif /* end of include guard: RDS_H */
