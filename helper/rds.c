#include "rds.h"
#include "../driver/si473x.h"
#include <string.h>

si47x_rds_status rdsResponse = {0};

enum {
  PI_H = 4, // Also "Block A"
  PI_L,
  Block_B_H,
  Block_B_L,
  Block_C_H,
  Block_C_L,
  Block_D_H,
  Block_D_L
};

#define MAKE_WORD(hb, lb) (((uint8_t)(hb) << 8U) | (uint8_t)lb)

enum { NO_DATE_TIME = 127 };
RDS rds = {.offset = NO_DATE_TIME};

enum {
  RDS_THRESHOLD = 3,     // Threshold for larger variables
  RDS_BOOL_THRESHOLD = 7 // Threshold for boolean variables
};

static char make_printable(char ch) {
  // Replace non-ASCII char with space
  if (ch < 32 || 126 < ch)
    ch = ' ';
  return ch;
}

/* RDS and RBDS data */
static ternary _abRadioText;       // Indicates new radioText[] string
static ternary _abProgramTypeName; // Indicates new programTypeName[] string
/* RDS data counters */
static uint8_t _extendedCountryCode_count;
static uint8_t _language_count;

void SI47XX_ClearRDS() {
  memset(&rds, 0, sizeof(RDS));
  rds.offset = NO_DATE_TIME;
}

bool SI47XX_GetRDS() {

  bool new_info = false;
  uint8_t segment;

  while (1) {
    SI47XX_ReadRDS(rdsResponse.raw);

    // Check for RDS signal
    rds.RDSSignal = rdsResponse.raw[2] & FIELD_RDS_STATUS_RESP2_SYNC;
    // Get number of RDS groups (packets) available
    uint8_t num_groups = rdsResponse.raw[3];
    // Stop if nothing returned
    if (!num_groups)
      break;

    /* Because PI is resent in every packet's Block A, we told the radio its OK
     * to give us packets with a corrupted Block A.
     */
    // Check if PI received is valid
    if ((rdsResponse.raw[12] & FIELD_RDS_STATUS_RESP12_BLOCK_A) !=
        RDS_STATUS_RESP12_BLOCK_A_UNCORRECTABLE) {
      // Get PI code
      rds.programId = MAKE_WORD(rdsResponse.raw[PI_H], rdsResponse.raw[PI_L]);
    }
    // Get PTY code
    rds.programType = ((rdsResponse.raw[Block_B_H] & 0b00000011) << 3U) |
                      (rdsResponse.raw[Block_B_L] >> 5U);
    // Get Traffic Program bit
    rds.trafficProgram = (bool)(rdsResponse.raw[Block_B_H] & 0b00000100);

    // Get group type (0-15)
    uint8_t type = rdsResponse.raw[Block_B_H] >> 4U;
    // Get group version (0=A, 1=B)
    bool version = rdsResponse.raw[Block_B_H] & 0b00001000;

    // Save which group type and version was received
    if (version) {
      rds.groupB |= 1U << type;
    } else {
      rds.groupA |= 1U << type;
    }

    // Groups 0A & 0B - Basic tuning and switching information
    // Group 15B - Fast basic tuning and switching information
    /* Note: We support both Groups 0 and 15B in case the station has poor
     * reception and RDS packets are barely getting through.  This increases
     * the chances of receiving this info.
     */
    if (type == 0 || (type == 15 && version == 1)) {
      // Various flags
      rds.trafficAlert = (bool)(rdsResponse.raw[Block_B_L] & 0b00010000);
      rds.music = (bool)(rdsResponse.raw[Block_B_L] & 0b00001000);
      bool DI = rdsResponse.raw[Block_B_L] & 0b00000100;

      // Get segment number
      segment = rdsResponse.raw[Block_B_L] & 0b00000011;
      // Handle DI code
      switch (segment) {
      case 0:
        rds.dynamicPTY = DI;
        break;
      case 1:
        rds.compressedAudio = DI;
        break;
      case 2:
        rds.binauralAudio = DI;
        break;
      case 3:
        rds.RDSStereo = DI;
        break;
      }

      // Groups 0A & 0B
      if (type == 0) {
        // Program Service
        char *ps = &rds.programService[segment * 2];
        *ps++ = make_printable(rdsResponse.raw[Block_D_H]);
        *ps = make_printable(rdsResponse.raw[Block_D_L]);
      }
      new_info = true;
    }
    // Group 1A - Extended Country Code (ECC) and Language Code
    else if (type == 1 && version == 0) {
      // We are only interested in the Extended Country Code (ECC) and
      // Language Code for this Group.

      // Get Variant code
      switch (rdsResponse.raw[Block_C_H] & 0b01110000) {
      case (0 << 4): // Variant==0
        // Extended Country Code
        // Check if count has reached threshold
        if (_extendedCountryCode_count < RDS_THRESHOLD) {
          uint8_t ecc = rdsResponse.raw[Block_C_L];
          // Check if datum changed
          if (rds.extendedCountryCode != ecc) {
            _extendedCountryCode_count = 0;
            new_info = true;
          }
          // Save new data
          rds.extendedCountryCode = ecc;
          ++_extendedCountryCode_count;
        }
        break;
      case (3 << 4): // Variant==3
        // Language Code
        // Check if count has reached threshold
        if (_language_count < RDS_THRESHOLD) {
          uint8_t language = rdsResponse.raw[Block_C_L];
          // Check if datum changed
          if (rds.language != language) {
            _language_count = 0;
            new_info = true;
          }
          // Save new data
          rds.language = language;
          ++_language_count;
        }
        break;
      }
    }
    // Groups 2A & 2B - Radio Text
    else if (type == 2) {
      // Check A/B flag to see if Radio Text has changed
      uint8_t new_ab = (bool)(rdsResponse.raw[Block_B_L] & 0b00010000);
      if (new_ab != _abRadioText) {
        // New message found - clear buffer
        _abRadioText = new_ab;
        for (uint8_t i = 0; i < sizeof(rds.radioText) - 1; i++)
          rds.radioText[i] = ' ';
        rds.radioTextLen = sizeof(rds.radioText); // Default to max length
      }
      // Get segment number
      segment = rdsResponse.raw[Block_B_L] & 0x0F;

      // Get Radio Text
      char *rt;       // Next position in rds.radioText[]
      uint8_t *block; // Next char from segment
      uint8_t i;      // Loop counter
      // TODO maybe: convert RDS non ASCII chars to UTF-8 for terminal interface
      if (version == 0) { // 2A
        rt = &rds.radioText[segment * 4];
        block = &rdsResponse.raw[Block_C_H];
        i = 4;
      } else { // 2B
        rt = &rds.radioText[segment * 2];
        block = &rdsResponse.raw[Block_D_H];
        i = 2;
      }
      // Copy chars
      do {
        // Get next char from segment
        char ch = *block++;
        // Check for end of message marker
        if (ch == '\r') {
          // Save new message length
          rds.radioTextLen = rt - rds.radioText;
        }
        // Put next char in rds.radioText[]
        *rt++ = make_printable(ch);
      } while (--i);
      new_info = true;
    }
    // Group 4A - Clock-time and date
    else if (type == 4 && version == 0) {
      // Only use if received perfectly.
      /* Note: Error Correcting Codes (ECC) are not perfect.  It is possible
       * for a block to be damaged enough that the ECC thinks the data is OK
       * when it's damaged or that it can recover when it cannot.  Because
       * date and time are useless unless accurate, we require that the date
       * and time be received perfectly to increase the odds of accurate data.
       */
      if ((rdsResponse.raw[12] &
           (FIELD_RDS_STATUS_RESP12_BLOCK_B | FIELD_RDS_STATUS_RESP12_BLOCK_C |
            FIELD_RDS_STATUS_RESP12_BLOCK_D)) ==
          (RDS_STATUS_RESP12_BLOCK_B_NO_ERRORS |
           RDS_STATUS_RESP12_BLOCK_C_NO_ERRORS |
           RDS_STATUS_RESP12_BLOCK_D_NO_ERRORS)) {
        // Get Modified Julian Date (MJD)
        rds.MJD = (rdsResponse.raw[Block_B_L] & 0b00000011) << 15UL |
                  rdsResponse.raw[Block_C_H] << 7U |
                  rdsResponse.raw[Block_C_L] >> 1U;

        // Get hour and minute
        rds.hour = (rdsResponse.raw[Block_C_L] & 0b00000001) << 4U |
                   rdsResponse.raw[Block_D_H] >> 4U;
        rds.minute = (rdsResponse.raw[Block_D_H] & 0x0F) << 2U |
                     rdsResponse.raw[Block_D_L] >> 6U;

        // Check if date and time sent (not 0)
        if (rds.MJD || rds.hour || rds.minute || rdsResponse.raw[Block_D_L]) {
          // Get offset to convert UTC to local time
          rds.offset = rdsResponse.raw[Block_D_L] & 0x1F;
          // Check if offset should be negative
          if (rdsResponse.raw[Block_D_L] & 0b00100000) {
            rds.offset = -rds.offset; // Make it negative
          }
          new_info = true;
        }
      }
    }
    // Group 10A - Program Type Name
    else if (type == 10 && version == 0) {
      // Check A/B flag to see if Program Type Name has changed
      uint8_t new_ab = (bool)(rdsResponse.raw[Block_B_L] & 0b00010000);
      if (new_ab != _abProgramTypeName) {
        // New name found - clear buffer
        _abProgramTypeName = new_ab;
        for (uint8_t i = 0; i < sizeof(rds.programTypeName) - 1; i++)
          rds.programTypeName[i] = ' ';
      }
      // Get segment number
      segment = rdsResponse.raw[Block_B_L] & 0x01;

      // Get Program Type Name
      char *name = &rds.programTypeName[segment * 4];
      *name++ = make_printable(rdsResponse.raw[Block_C_H]);
      *name++ = make_printable(rdsResponse.raw[Block_C_L]);
      *name++ = make_printable(rdsResponse.raw[Block_D_H]);
      *name = make_printable(rdsResponse.raw[Block_D_L]);
      new_info = true;
    }
  }
  return new_info;
}

#define DAYS_PER_YEAR 365U
// Leap year
#define DAYS_PER_LEAP_YEAR (DAYS_PER_YEAR + 1)
// Leap year every 4 years
#define DAYS_PER_4YEARS (DAYS_PER_YEAR * 4 + 1)
// Leap year every 4 years except century year (divisable by 100)
#define DAYS_PER_100YEARS (DAYS_PER_4YEARS * (100 / 4) - 1)

// Get last RDS date and time converted to local date and time.
// Returns true if current station has sent date and time.  Otherwise, it
// returns false and writes nothing to structure. Only provides info if mode==FM
// and station is sending RDS data.
bool SI47XX_GetLocalDateTime(DateTime *time) {
  // Look for date/time info
  if (rds.offset == NO_DATE_TIME)
    return false; // No date or time info available

  // Origin for Modified Julian Date (MJD) is November 17, 1858, Wednesday.
  // Move origin to Jan. 2, 2000, Sunday.
  // Note: We don't use Jan. 1 to compensate for the fact that 2000 is a leap
  // year.
  unsigned short days = rds.MJD - (                          // 1858-Nov-17
                                      14 +                   // 1858-Dec-1
                                      31 +                   // 1859-Jan-1
                                      DAYS_PER_YEAR +        // 1860-Jan-1
                                      10 * DAYS_PER_4YEARS + // 1900-Jan-1
                                      DAYS_PER_100YEARS +    // 2000-Jan-1
                                      1);                    // 2000-Jan-2

  // Convert UTC date and time to local date and time.
  // Combine date and time
  unsigned long date_time = ((unsigned long)days) * (24 * 60) +
                            ((unsigned short)rds.hour) * 60 + rds.minute;
  // Adjust offset from units of half hours to minutes
  int16_t offset = (int16_t)(rds.offset) * 30;
  // Compute local date/time
  date_time += offset;
  // Break down date and time
  time->minute = date_time % 60;
  date_time /= 60;
  time->hour = date_time % 24;
  days = date_time / 24;

  // Compute day of the week - Sunday = 0
  time->wday = days % 7;

  // Compute year
  unsigned char leap_year = 0; /* 1 if leap year, else 0 */
  // Note: This code assumes all century years (2000, 2100...) are not leap
  // years. This will break in 2400 AD.  However, RDS' date field will overflow
  // long before 2400 AD.
  time->year = days / DAYS_PER_100YEARS * 100 + 2000;
  days %= DAYS_PER_100YEARS;
  if (!(days < DAYS_PER_YEAR)) {
    days++; // Adjust for no leap year for century year
    time->year += days / DAYS_PER_4YEARS * 4;
    days %= DAYS_PER_4YEARS;
    if (days < DAYS_PER_LEAP_YEAR) {
      leap_year = 1;
    } else {
      days--; // Adjust for leap year for first of 4 years
      time->year += days / DAYS_PER_YEAR;
      days %= DAYS_PER_YEAR;
    }
  }

  // Compute month and day of the month
  if (days < 31 + 28 + leap_year) {
    if (days < 31) {
      /* January */
      time->month = 1;
      time->day = days + 1;
    } else {
      /* February */
      time->month = 2;
      time->day = days + 1 - 31;
    }
  } else {
    /* March - December */
    enum { NUM_MONTHS = 10 };
    static const unsigned short month[NUM_MONTHS] = {
        0,
        31,
        31 + 30,
        31 + 30 + 31,
        31 + 30 + 31 + 30,
        31 + 30 + 31 + 30 + 31,
        31 + 30 + 31 + 30 + 31 + 31,
        31 + 30 + 31 + 30 + 31 + 31 + 30,
        31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
        31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30};
    unsigned short value; // Value from table
    unsigned char mon;    // Index to month[]

    days -= 31 + 28 + leap_year;
    // Look up month
    for (mon = NUM_MONTHS; days < (value = (month[--mon]));)
      ;
    time->day = days - value + 1;
    time->month = mon + 2 + 1;
  }
  return true;
}

bool SI47XX_GetLocalTime(Time *time) {
  // Look for date/time info
  if (rds.offset == NO_DATE_TIME)
    return false; // No date or time info available

  // Convert UTC to local time
  /* Note: If the offset is negative, 'hour' and 'minute' could become negative.
   * To compensate, we add 24 to hour and 60 to minute.  We then do a modulus
   * division (%24 and %60) to correct for any overflow caused by either a
   * positive offset or the above mentioned addition.
   */
  time->hour = (rds.hour + rds.offset / 2 + 24) % 24;
  time->minute = (rds.minute + rds.offset % 2 * 30 + 60) % 60;
  return true;
}

void SI47XX_GetProgramType(char buffer[17]) {
  typedef struct {
    char *pRds;
    char *pRdbs;
  } RDS_PTY;

  static const RDS_PTY PTY_NAMES[] = {
      {"No program type", "No program type"},
      {"News", "News"},
      {"Current affairs", "Information"},
      {"Information", "Sport"},
      {"Sport", "Talk"},
      {"Education", "Rock"},
      {"Drama", "Classic Rock"},
      {"Culture", "Adult Hits"},
      {"Science", "Soft Rock"},
      {"Variable", "Top 40"},
      {"Pop", "Country Music"},
      {"Rock", "Music Oldies"},
      {"Easy listening", "Soft Music"},
      {"Light classical", "Nostalgia"},
      {"Serious classical", "Jazz"},
      {"Other Music", "Classical"},
      {"Weather", "Rhythm and Blues"},
      {"Finance", "Soft Rhythm and Blues"},
      {"Children's programs", "Language"},
      {"Social Affairs", "Religious Music"},
      {"Religion", "Religious Talk"},
      {"Phone-in talk", "Personality"},
      {"Travel", "Public"},
      {"Leisure", "College"},
      {"Jazz Music", "Unassigned"},
      {"Country Music", "Unassigned"},
      {"National Music", "Unassigned"},
      {"Oldies Music", "Unassigned"},
      {"Folk Music", "Unassigned"},
      {"Documentary", "Weather"},
      {"Alarm Test", "Emergency Test"},
      {"Alarm", "Emergency"},
  };

  const RDS_PTY *pty = &PTY_NAMES[rds.programType];
  if (rds.RBDS) {
    strncpy(buffer, pty->pRdbs, 16);
  } else {
    strncpy(buffer, pty->pRds, 16);
  }
  buffer[16] = '\0';
}
