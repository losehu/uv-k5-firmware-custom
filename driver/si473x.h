#ifndef SI473X_H
#define SI473X_H

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SI47XX_FM,
    SI47XX_AM,
    SI47XX_LSB,
    SI47XX_USB,
    SI47XX_CW,
} SI47XX_MODE;

typedef enum {
    SI47XX_BW_6_kHz,
    SI47XX_BW_4_kHz,
    SI47XX_BW_3_kHz,
    SI47XX_BW_2_kHz,
    SI47XX_BW_1_kHz,
    SI47XX_BW_1_8_kHz,
    SI47XX_BW_2_5_kHz,
} SI47XX_FilterBW;

typedef enum {
    SI47XX_SSB_BW_1_2_kHz,
    SI47XX_SSB_BW_2_2_kHz,
    SI47XX_SSB_BW_3_kHz,
    SI47XX_SSB_BW_4_kHz,
    SI47XX_SSB_BW_0_5_kHz,
    SI47XX_SSB_BW_1_0_kHz,
} SI47XX_SsbFilterBW;

typedef enum {
    CMD_POWER_UP = 0x01,
    CMD_GET_REV = 0x10,
    CMD_POWER_DOWN = 0x11,
    CMD_SET_PROPERTY = 0x12,
    CMD_GET_PROPERTY = 0x13,
    CMD_GET_INT_STATUS = 0x14,
    CMD_PATCH_ARGS = 0x15,
    CMD_PATCH_DATA = 0x16,
    CMD_FM_TUNE_FREQ = 0x20,
    CMD_FM_SEEK_START = 0x21,
    CMD_FM_TUNE_STATUS = 0x22,
    CMD_FM_RSQ_STATUS = 0x23,
    CMD_FM_RDS_STATUS = 0x24,
    CMD_FM_AGC_STATUS = 0x27,
    CMD_FM_AGC_OVERRIDE = 0x28,
    CMD_TX_TUNE_FREQ = 0x30,
    CMD_TX_TUNE_POWER = 0x31,
    CMD_TX_TUNE_MEASURE = 0x32,
    CMD_TX_TUNE_STATUS = 0x33,
    CMD_TX_ASQ_STATUS = 0x34,
    CMD_TX_RDS_BUF = 0x35,
    CMD_TX_RDS_PS = 0x36,
    CMD_AM_TUNE_FREQ = 0x40,
    CMD_AM_SEEK_START = 0x41,
    CMD_AM_TUNE_STATUS = 0x42,
    CMD_AM_RSQ_STATUS = 0x43,
    CMD_AM_AGC_STATUS = 0x47,
    CMD_AM_AGC_OVERRIDE = 0x48,
    CMD_WB_TUNE_FREQ = 0x50,
    CMD_WB_TUNE_STATUS = 0x52,
    CMD_WB_RSQ_STATUS = 0x53,
    CMD_WB_SAME_STATUS = 0x54,
    CMD_WB_ASQ_STATUS = 0x55,
    CMD_WB_AGC_STATUS = 0x57,
    CMD_WB_AGC_OVERRIDE = 0x58,
    CMD_AUX_ASRC_START = 0x61,
    CMD_AUX_ASQ_STATUS = 0x65,
    CMD_GPIO_CTL = 0x80,
    CMD_GPIO_SET = 0x81,
} SI47XX_Commands;

typedef enum {
    FLG_CTSIEN = 0x80,
    FLG_GPO2IEN = 0x40,
    FLG_PATCH = 0x20,
    FLG_XOSCEN = 0x10,
    FLG_FREEZE = 0x02,
    FLG_FAST = 0x01,
    FLG_SEEKUP = 0x08,
    FLG_WRAP = 0x04,
    FLG_CANCEL = 0x02,
    FLG_INTACK = 0x01,
    FLG_STATUSONLY = 0x04,
    FLG_MTFIFO = 0x02,
    FLG_GPO3OEN = 0x08,
    FLG_GPO2OEN = 0x04,
    FLG_GPO1OEN = 0x02,
    FLG_GPO3LEVEL = 0x08,
    FLG_GPO2LEVEL = 0x04,
    FLG_GPO1LEVEL = 0x02,
    FLG_BLETHA_0 = 0x00,
    FLG_BLETHA_12 = 0x40,
    FLG_BLETHA_35 = 0x80,
    FLG_BLETHA_U = FLG_BLETHA_12 | FLG_BLETHA_35,
    FLG_BLETHB_0 = FLG_BLETHA_0,
    FLG_BLETHB_12 = 0x10,
    FLG_BLETHB_35 = 0x20,
    FLG_BLETHB_U = FLG_BLETHB_12 | FLG_BLETHB_35,
    FLG_BLETHC_0 = FLG_BLETHA_0,
    FLG_BLETHC_12 = 0x04,
    FLG_BLETHC_35 = 0x08,
    FLG_BLETHC_U = FLG_BLETHC_12 | FLG_BLETHC_35,
    FLG_BLETHD_0 = FLG_BLETHA_0,
    FLG_BLETHD_12 = 0x01,
    FLG_BLETHD_35 = 0x02,
    FLG_BLETHD_U = FLG_BLETHD_12 | FLG_BLETHD_35,
    FLG_RDSEN = 0x01,
    FLG_DEEMPH_NONE = 0x00,
    FLG_DEEMPH_50 = 0x01,
    FLG_DEEMPH_75 = 0x02,
    FLG_RSQREP = 0x08,
    FLG_RDSREP = 0x04,
    FLG_STCREP = 0x01,
    FLG_ERRIEN = 0x40,
    FLG_RSQIEN = 0x08,
    FLG_RDSIEN = 0x04,
    FLG_STCIEN = 0x01,
    FLG_RDSNEWBLOCKB = 0x20,
    FLG_RDSNEWBLOCKA = 0x10,
    FLG_RDSSYNCFOUND = 0x04,
    FLG_RDSSYNCLOST = 0x02,
    FLG_RDSRECV = 0x01,
    FLG_GRPLOST = 0x04,
    FLG_RDSSYNC = 0x01,
    FLG_AMPLFLT = 0x01,
    FLG_AMCHFLT_6KHZ = 0x00,
    FLG_AMCHFLT_4KHZ = 0x01,
    FLG_AMCHFLT_3KHZ = 0x02,
    FLG_AMCHFLT_2KHZ = 0x03,
    FLG_AMCHFLT_1KHZ = 0x04,
    FLG_AMCHFLT_1KHZ8 = 0x05,
    FLG_AMCHFLT_2KHZ5 = 0x06,
} SI47XX_Flags;

// Define Si4735 Function modes
typedef enum {
    FUNC_FM = 0x00,
    FUNC_AM = 0x01,
    FUNC_VER = 0x0F,
} SI47XX_FunctionModes;

// Define Si4735 Output modes
typedef enum {
    OUT_RDS = 0x00, // RDS only
    OUT_ANALOG = 0x05,
    OUT_DIGITAL1 = 0x0B, // DCLK, LOUT/DFS, ROUT/DIO
    OUT_DIGITAL2 = 0xB0, // DCLK, DFS, DIO
    OUT_BOTH = OUT_ANALOG | OUT_DIGITAL2,
} SI47XX_OutputModes;

typedef union {
    struct {
        uint8_t AUDIOBW: 4;  //!<  0 = 1.2kHz (default); 1=2.2kHz; 2=3kHz; 3=4kHz;
        //!<  4=500Hz; 5=1kHz
        uint8_t SBCUTFLT: 4; //!<  SSB side band cutoff filter for band passand low
        //!<  pass filter
        uint8_t AVC_DIVIDER: 4; //!<  set 0 for SSB mode; set 3 for SYNC mode;
        uint8_t AVCEN: 1;       //!<  SSB Automatic Volume Control (AVC) enable;
        //!<  0=disable; 1=enable (default);
        uint8_t SMUTESEL: 1;    //!<  SSB Soft-mute Based on RSSI or SNR
        uint8_t DUMMY1: 1;      //!<  Always write 0;
        uint8_t
                DSP_AFCDIS: 1; //!<  0=SYNC MODE, AFC enable; 1=SSB MODE, AFC disable.
    } param;
    uint8_t raw[2];
} SsbMode;

// Define Si47xx Status flag masks (bits the chip fed us)
typedef enum {
    STATUS_CTS = 0x80,
    STATUS_ERR = 0x40,
    STATUS_RSQINT = 0x08,
    STATUS_RDSINT = 0x04,
    STATUS_ASQINT = 0x02,
    STATUS_STCINT = 0x01,
    STATUS_BLTF = 0x80,
    STATUS_AFCRL = 0x02,
    STATUS_VALID = 0x01,
    STATUS_BLENDINT = 0x80,
    STATUS_MULTHINT = 0x20,
    STATUS_MULTLINT = 0x10,
    STATUS_SNRHINT = 0x08,
    STATUS_SNRLINT = 0x04,
    STATUS_RSSIHINT = 0x02,
    STATUS_RSSILINT = 0x01,
    STATUS_SMUTE = 0x08,
    STATUS_PILOT = 0x80,
    STATUS_OVERMOD = 0x04,
    STATUS_IALH = 0x02,
    STATUS_IALL = 0x01,
} SI47XX_StatusFlagMasks;

// Define Si47xx Property codes
typedef enum {
    PROP_GPO_IEN = (uint16_t) 0x0001,
    PROP_DIGITAL_INPUT_FORMAT = 0x0101,
    PROP_DIGITAL_OUTPUT_FORMAT = 0x0102,
    PROP_DIGITAL_INPUT_SAMPLE_RATE = 0x0103,
    PROP_DIGITAL_OUTPUT_SAMPLE_RATE = 0x0104,
    PROP_REFCLK_FREQ = 0x0201,
    PROP_REFCLK_PRESCALE = 0x0202,
    PROP_FM_DEEMPHASIS = 0x1100,
    PROP_FM_CHANNEL_FILTER = 0x1102,
    PROP_FM_BLEND_STEREO_THRESHOLD = 0x1105,
    PROP_FM_BLEND_MONO_THRESHOLD = 0x1106,
    PROP_FM_ANTENNA_INPUT = 0x1107,
    PROP_FM_MAX_TUNE_ERROR = 0x1108,
    PROP_FM_RSQ_INT_SOURCE = 0x1200,
    PROP_FM_RSQ_SNR_HI_THRESHOLD = 0x1201,
    PROP_FM_RSQ_SNR_LO_THRESHOLD = 0x1202,
    PROP_FM_RSQ_RSSI_HI_THRESHOLD = 0x1203,
    PROP_FM_RSQ_RSSI_LO_THRESHOLD = 0x1204,
    PROP_FM_RSQ_MULTIPATH_HI_THRESHOLD = 0x1205,
    PROP_FM_RSQ_MULTIPATH_LO_THRESHOLD = 0x1206,
    PROP_FM_RSQ_BLEND_THRESHOLD = 0x1207,
    PROP_FM_SOFT_MUTE_RATE = 0x1300,
    PROP_FM_SOFT_MUTE_SLOPE = 0x1301,
    PROP_FM_SOFT_MUTE_MAX_ATTENUATION = 0x1302,
    PROP_FM_SOFT_MUTE_SNR_THRESHOLD = 0x1303,
    PROP_FM_SOFT_MUTE_RELEASE_RATE = 0x1304,
    PROP_FM_SOFT_MUTE_ATTACK_RATE = 0x1305,
    PROP_FM_SEEK_BAND_BOTTOM = 0x1400,
    PROP_FM_SEEK_BAND_TOP = 0x1401,
    PROP_FM_SEEK_FREQ_SPACING = 0x1402,
    PROP_FM_SEEK_TUNE_SNR_THRESHOLD = 0x1403,
    PROP_FM_SEEK_TUNE_RSSI_THRESHOLD = 0x1404,
    PROP_FM_RDS_INT_SOURCE = 0x1500,
    PROP_FM_RDS_INT_FIFO_COUNT = 0x1501,
    PROP_FM_RDS_CONFIG = 0x1502,
    PROP_FM_RDS_CONFIDENCE = 0x1503,
    PROP_FM_AGC_ATTACK_RATE = 0x1700,
    PROP_FM_AGC_RELEASE_RATE = 0x1701,
    PROP_FM_BLEND_RSSI_STEREO_THRESHOLD = 0x1800,
    PROP_FM_BLEND_RSSI_MONO_THRESHOLD = 0x1801,
    PROP_FM_BLEND_RSSI_ATTACK_RATE = 0x1802,
    PROP_FM_BLEND_RSSI_RELEASE_RATE = 0x1803,
    PROP_FM_BLEND_SNR_STEREO_THRESHOLD = 0x1804,
    PROP_FM_BLEND_SNR_MONO_THRESHOLD = 0x1805,
    PROP_FM_BLEND_SNR_ATTACK_RATE = 0x1806,
    PROP_FM_BLEND_SNR_RELEASE_RATE = 0x1807,
    PROP_FM_BLEND_MULTIPATH_STEREO_THRESHOLD = 0x1808,
    PROP_FM_BLEND_MULTIPATH_MONO_THRESHOLD = 0x1809,
    PROP_FM_BLEND_MULTIPATH_ATTACK_RATE = 0x180A,
    PROP_FM_BLEND_MULTIPATH_RELEASE_RATE = 0x180B,
    PROP_FM_BLEND_MAX_STEREO_SEPARATION = 0x180C,
    PROP_FM_NB_DETECT_THRESHOLD = 0x1900,
    PROP_FM_NB_INTERVAL = 0x1901,
    PROP_FM_NB_RATE = 0x1902,
    PROP_FM_NB_IIR_FILTER = 0x1903,
    PROP_FM_NB_DELAY = 0x1904,
    PROP_FM_HICUT_SNR_HIGH_THRESHOLD = 0x1A00,
    PROP_FM_HICUT_SNR_LOW_THRESHOLD = 0x1A01,
    PROP_FM_HICUT_ATTACK_RATE = 0x1A02,
    PROP_FM_HICUT_RELEASE_RATE = 0x1A03,
    PROP_FM_HICUT_MULTIPATH_TRIGGER_THRESHOLD = 0x1A04,
    PROP_FM_HICUT_MULTIPATH_END_THRESHOLD = 0x1A05,
    PROP_FM_HICUT_CUTOFF_FREQUENCY = 0x1A06,
    PROP_TX_COMPONENT_ENABLE = 0x2100,
    PROP_TX_AUDIO_DEVIATION = 0x2101,
    PROP_TX_PILOT_DEVIATION = 0x2102,
    PROP_TX_RDS_DEVIATION = 0x2103,
    PROP_TX_LINE_INPUT_LEVEL = 0x2104,
    PROP_TX_LINE_INPUT_MUTE = 0x2105,
    PROP_TX_PREEMPHASIS = 0x2106,
    PROP_TX_PILOT_FREQUENCY = 0x2107,
    PROP_TX_ACOMP_ENABLE = 0x2200,
    PROP_TX_ACOMP_THRESHOLD = 0x2201,
    PROP_TX_ACOMP_ATTACK_TIME = 0x2202,
    PROP_TX_ACOMP_RELEASE_TIME = 0x2203,
    PROP_TX_ACOMP_GAIN = 0x2204,
    PROP_TX_LIMITER_RELEASE_TIME = 0x2205,
    PROP_TX_ASQ_INTERRUPT_SOURCE = 0x2300,
    PROP_TX_ASQ_LEVEL_LOW = 0x2301,
    PROP_TX_ASQ_DURATION_LOW = 0x2302,
    PROP_TX_ASQ_LEVEL_HIGH = 0x2303,
    PROP_TX_ASQ_DURATION_HIGH = 0x2304,
    PROP_TX_RDS_INTERRUPT_SOURCE = 0x2C00,
    PROP_TX_RDS_PI = 0x2C01,
    PROP_TX_RDS_PS_MIX = 0x2C02,
    PROP_TX_RDS_PS_MISC = 0x2C03,
    PROP_TX_RDS_PS_REPEAT_COUNT = 0x2C04,
    PROP_TX_RDS_PS_MESSAGE_COUNT = 0x2C05,
    PROP_TX_RDS_PS_AF = 0x2C06,
    PROP_TX_RDS_FIFO_SIZE = 0x2C07,
    PROP_AM_DEEMPHASIS = 0x3100,
    PROP_AM_CHANNEL_FILTER = 0x3102,
    PROP_AM_AUTOMATIC_VOLUME_CONTROL_MAX_GAIN = 0x3103,
    PROP_AM_MODE_AFC_SW_PULL_IN_RANGE = 0x3104,
    PROP_AM_MODE_AFC_SW_LOCK_IN_RANGE = 0x3105,
    PROP_AM_RSQ_INTERRUPTS = 0x3200,
    PROP_AM_RSQ_SNR_HIGH_THRESHOLD = 0x3201,
    PROP_AM_RSQ_SNR_LOW_THRESHOLD = 0x3202,
    PROP_AM_RSQ_RSSI_HIGH_THRESHOLD = 0x3203,
    PROP_AM_RSQ_RSSI_LOW_THRESHOLD = 0x3204,
    PROP_AM_SOFT_MUTE_RATE = 0x3300,
    PROP_AM_SOFT_MUTE_SLOPE = 0x3301,
    PROP_AM_SOFT_MUTE_MAX_ATTENUATION = 0x3302,
    PROP_AM_SOFT_MUTE_SNR_THRESHOLD = 0x3303,
    PROP_AM_SOFT_MUTE_RELEASE_RATE = 0x3304,
    PROP_AM_SOFT_MUTE_ATTACK_RATE = 0x3305,
    PROP_AM_SEEK_BAND_BOTTOM = 0x3400,
    PROP_AM_SEEK_BAND_TOP = 0x3401,
    PROP_AM_SEEK_FREQ_SPACING = 0x3402,
    PROP_AM_SEEK_TUNE_SNR_THRESHOLD = 0x3403,
    PROP_AM_SEEK_TUNE_RSSI_THRESHOLD = 0x3404,
    PROP_AM_AGC_ATTACK_RATE = 0x3702,
    PROP_AM_AGC_RELEASE_RATE = 0x3703,
    PROP_AM_FRONTEND_AGC_CONTROL = 0x3705,
    PROP_AM_NB_DETECT_THRESHOLD = 0x3900,
    PROP_AM_NB_INTERVAL = 0x3901,
    PROP_AM_NB_RATE = 0x3902,
    PROP_AM_NB_IIR_FILTER = 0x3903,
    PROP_AM_NB_DELAY = 0x3904,
    PROP_RX_VOLUME = 0x4000,
    PROP_RX_HARD_MUTE = 0x4001,
    PROP_WB_MAX_TUNE_ERROR = 0x5108,
    PROP_WB_RSQ_INT_SOURCE = 0x5200,
    PROP_WB_RSQ_SNR_HI_THRESHOLD = 0x5201,
    PROP_WB_RSQ_SNR_LO_THRESHOLD = 0x5202,
    PROP_WB_RSQ_RSSI_HI_THRESHOLD = 0x5203,
    PROP_WB_RSQ_RSSI_LO_THRESHOLD = 0x5204,
    PROP_WB_VALID_SNR_THRESHOLD = 0x5403,
    PROP_WB_VALID_RSSI_THRESHOLD = 0x5404,
    PROP_WB_SAME_INTERRUPT_SOURCE = 0x5500,
    PROP_WB_ASQ_INTERRUPT_SOURCE = 0x5600,
    PROP_AUX_ASQ_INTERRUPT_SOURCE = 0x6600,
    PROP_DEBUG_CONTROL = 0xFF00,
    PROP_AM_RSQ_INT_SOURCE = 0x3200,
    PROP_WB_SAME_INT_SOURCE = 0x5500, // Si4707 only
    PROP_WB_ASQ_INT_SOURCE = 0x5600,
    PROP_AUX_ASQ_INT_SOURCE = 0x6600, // AUX mode - Si4735-D60 or later

    PROP_SSB_BFO = 0x0100, // Sets the Beat Frequency Offset (BFO) under SSB mode.
    PROP_SSB_MODE = 0x0101, // Sets number of properties of the SSB mode.
    PROP_SSB_RSQ_INTERRUPTS = 0x3200, // Configure Interrupts related to RSQ
    PROP_SSB_RSQ_SNR_HI_THRESHOLD =
    0x3201, // Sets high threshold for SNR interrupt
    PROP_SSB_RSQ_SNR_LO_THRESHOLD =
    0x3202, // Sets low threshold for SNR interrupt
    PROP_SSB_RSQ_RSSI_HI_THRESHOLD =
    0x3203, // Sets high threshold for RSSI interrupt
    PROP_SSB_RSQ_RSSI_LO_THRESHOLD =
    0x3204,                       // Sets low threshold for RSSI interrupt
    PROP_SSB_SOFT_MUTE_RATE = 0x3300, // Sets the attack and decay rates when
    // entering or leaving soft mute
    PROP_SSB_SOFT_MUTE_MAX_ATTENUATION =
    0x3302, // Sets the maximum attenuation during soft mute (db); 0dB to
    // disable soft mute; defaul 8dB;
    PROP_SSB_SOFT_MUTE_SNR_THRESHOLD =
    0x3303, // Sets SNR threshould to engage soft mute. Defaul 8dB
    PROP_SSB_RF_AGC_ATTACK_RATE =
    0x3700, // Sets the number of milliseconds the high RF peak detector must
    // be exceeded before decreasing the gain. Defaul 4.
    PROP_SSB_RF_AGC_RELEASE_RATE =
    0x3701, // Sets the number of milliseconds the low RF peak detector must
    // be exceeded before increasing the gain. Defaul 24.
    PROP_SSB_IF_AGC_RELEASE_RATE =
    0x3703, // Sets the number of milliseconds the low IF peak detector must
    // not be exceeded before increasing the gain. Default value is
    // 140 (approximately 40 dB / s).
    PROP_SSB_IF_AGC_ATTACK_RATE =
    0x3702, // Sets the number of milliseconds the high IF peak detector must
    // be exceeded before decreasing gain. Default value is 4
    // (approximately 1400 dB / s).

} SI47XX_PropertyCodes;

enum {
    // POWER_UP
    /* See POWER_UP_AUDIO_OUT constants above for ARG2. */
    POWER_UP_ARG1_CTSIEN = 0b10000000,  // CTS interrupt enable
    POWER_UP_ARG1_GPO2OEN = 0b01000000, // GPO2/INT output enable
    POWER_UP_ARG1_PATCH = 0b00100000,   // Patch enable
    POWER_UP_ARG1_XOSCEN =
    0b00010000, // Enable internal oscillator with external 32768 Hz crystal
    POWER_UP_ARG1_FUNC_FM = 0x0,  // FM receive mode
    POWER_UP_ARG1_FUNC_AM = 0x1,  // AM receive mode
    POWER_UP_ARG1_FUNC_TX = 0x2,  // FM transmit mode - not Si4735 or Si4707
    POWER_UP_ARG1_FUNC_WB = 0x3,  // WB receive mode - not Si4735
    POWER_UP_ARG1_FUNC_AUX = 0x4, // Auxiliary input mode - Si4735-D60 or later
    POWER_UP_ARG1_FUNC_REV = 0xF, // Query chip's hardware and firmware revisions
    // FM_TUNE_FREQ, AM_TUNE_FREQ
    FM_TUNE_FREQ_ARG1_FREEZE = 0b10,
    TUNE_FREQ_ARG1_FAST = 0b01, // Fast, inaccurate tune
    // FM_SEEK_START, AM_SEEK_START
    SEEK_START_ARG1_SEEK_UP = 0b1000, // 1 = Seek up, 0 = Seek down
    SEEK_START_ARG1_WRAP = 0b0100,    // Wrap when band limit reached
    // FM_TUNE_STATUS, AM_TUNE_STATUS, WB_TUNE_STATUS
    TUNE_STATUS_ARG1_CANCEL_SEEK = 0b10, // Cancel seek operation - not WB
    TUNE_STATUS_ARG1_CLEAR_INT = 0b01,   // Clear STC interrupt
    // FM_RSQ_STATUS, AM_RSQ_STATUS, WB_RSQ_STATUS
    RSQ_STATUS_ARG1_CLEAR_INT = 0b1, // Clear RSQ and related interrupts
    // FM_RDS_STATUS
    RDS_STATUS_ARG1_STATUS_ONLY = 0b100,
    RDS_STATUS_ARG1_CLEAR_FIFO = 0b010, // Clear RDS receive FIFO
    RDS_STATUS_ARG1_CLEAR_INT = 0b001,  // Clear RDS interrupt
    // WB_SAME_STATUS
    SAME_STATUS_ARG1_CLEAR_BUFFER = 0b10, // Clear SAME receive buffer
    SAME_STATUS_ARG1_CLEAR_INT = 0b01,    // Clear SAME interrupt
    // AUX_ASQ_STATUS, WB_ASQ_STATUS
    ASQ_STATUS_ARG1_CLEAR_INT = 0b1, // Clear ASQ interrupt
    // FM_AGC_OVERRIDE, AM_AGC_OVERRIDE, WB_AGC_OVERRIDE
    AGC_OVERRIDE_ARG1_DISABLE_AGC = 0b1, // Disable AGC
    // GPIO_CTL, GPIO_SET
    GPIO_ARG1_GPO3 = 0b1000, // GPO3
    GPIO_ARG1_GPO2 = 0b0100, // GPO2
    GPIO_ARG1_GPO1 = 0b0010, // GPO1
};

// Command responses
// Names that begin with FIELD are argument masks.  Others are argument
// constants.
enum {
    // FM_TUNE_STATUS, AM_TUNE_STATUS, WB_TUNE_STATUS
    FIELD_TUNE_STATUS_RESP1_SEEK_LIMIT =
    0b10000000,                            // Seek hit search limit - not WB
    FIELD_TUNE_STATUS_RESP1_AFC_RAILED = 0b10, // AFC railed
    FIELD_TUNE_STATUS_RESP1_SEEKABLE =
    0b01, // Station could currently be found by seek,
    FIELD_TUNE_STATUS_RESP1_VALID = 0b01, // that is, the station is valid
    // FM_RSQ_STATUS, AM_RSQ_STATUS, WB_RSQ_STATUS
    /* See RSQ interrupts above for RESP1. */
    FIELD_RSQ_STATUS_RESP2_SOFT_MUTE = 0b1000,  // Soft mute active - not WB
    FIELD_RSQ_STATUS_RESP2_AFC_RAILED = 0b0010, // AFC railed
    FIELD_RSQ_STATUS_RESP2_SEEKABLE =
    0b0001, // Station could currently be found by seek,
    FIELD_RSQ_STATUS_RESP2_VALID = 0b0001,      // that is, the station is valid
    FIELD_RSQ_STATUS_RESP3_STEREO = 0b10000000, // Stereo pilot found - FM only
    FIELD_RSQ_STATUS_RESP3_STEREO_BLEND =
    0b01111111, // Stereo blend in % (100 = full stereo, 0 = full mono) - FM
    // only
    // FM_RDS_STATUS
    /* See RDS interrupts above for RESP1. */
    FIELD_RDS_STATUS_RESP2_FIFO_OVERFLOW = 0b00000100, // FIFO overflowed
    FIELD_RDS_STATUS_RESP2_SYNC = 0b00000001, // RDS currently synchronized
    FIELD_RDS_STATUS_RESP12_BLOCK_A = 0b11000000,
    FIELD_RDS_STATUS_RESP12_BLOCK_B = 0b00110000,
    FIELD_RDS_STATUS_RESP12_BLOCK_C = 0b00001100,
    FIELD_RDS_STATUS_RESP12_BLOCK_D = 0b00000011,
    RDS_STATUS_RESP12_BLOCK_A_NO_ERRORS = 0U << 6,     // Block had no errors
    RDS_STATUS_RESP12_BLOCK_A_2_BIT_ERRORS = 1U << 6,  // Block had 1-2 bit errors
    RDS_STATUS_RESP12_BLOCK_A_5_BIT_ERRORS = 2U << 6,  // Block had 3-5 bit errors
    RDS_STATUS_RESP12_BLOCK_A_UNCORRECTABLE = 3U << 6, // Block was uncorrectable
    RDS_STATUS_RESP12_BLOCK_B_NO_ERRORS = 0U << 4,
    RDS_STATUS_RESP12_BLOCK_B_2_BIT_ERRORS = 1U << 4,
    RDS_STATUS_RESP12_BLOCK_B_5_BIT_ERRORS = 2U << 4,
    RDS_STATUS_RESP12_BLOCK_B_UNCORRECTABLE = 3U << 4,
    RDS_STATUS_RESP12_BLOCK_C_NO_ERRORS = 0U << 2,
    RDS_STATUS_RESP12_BLOCK_C_2_BIT_ERRORS = 1U << 2,
    RDS_STATUS_RESP12_BLOCK_C_5_BIT_ERRORS = 2U << 2,
    RDS_STATUS_RESP12_BLOCK_C_UNCORRECTABLE = 3U << 2,
    RDS_STATUS_RESP12_BLOCK_D_NO_ERRORS = 0U << 0,
    RDS_STATUS_RESP12_BLOCK_D_2_BIT_ERRORS = 1U << 0,
    RDS_STATUS_RESP12_BLOCK_D_5_BIT_ERRORS = 2U << 0,
    RDS_STATUS_RESP12_BLOCK_D_UNCORRECTABLE = 3U << 0,
    // WB_SAME_STATUS - TODO

    // AUX_ASQ_STATUS, WB_ASQ_STATUS
    /* See ASQ interrupts above for RESP1. */
    FIELD_AUX_ASQ_STATUS_RESP2_OVERLOAD =
    0b1, // Audio input is currently overloading ADC
    FIELD_WB_ASQ_STATUS_RESP2_ALERT = 0b1, // Alert tone is present
    // FM_AGC_STATUS, AM_AGC_STATUS, WB_AGC_STATUS
    FIELD_AGC_STATUS_RESP1_DISABLE_AGC = 0b1, // True if AGC disabled
};

typedef union {
    struct {
        // status ("RESP0")
        uint8_t STCINT: 1;
        uint8_t DUMMY1: 1;
        uint8_t RDSINT: 1;
        uint8_t RSQINT: 1;
        uint8_t DUMMY2: 2;
        uint8_t ERR: 1;
        uint8_t CTS: 1;
        // RESP1
        uint8_t RSSIILINT: 1; //!<  RSSI Detect Low.
        uint8_t RSSIHINT: 1;  //!<  RSSI Detect High.
        uint8_t SNRLINT: 1;   //!<  SNR Detect Low.
        uint8_t SNRHINT: 1;   //!<  SNR Detect High.
        uint8_t MULTLINT: 1;  //!<  Multipath Detect Low
        uint8_t MULTHINT: 1;  //!<  Multipath Detect High
        uint8_t DUMMY3: 1;
        uint8_t BLENDINT: 1; //!<  Blend Detect Interrupt.
        // RESP2
        uint8_t VALID: 1; //!<  Valid Channel.
        uint8_t AFCRL: 1; //!<  AFC Rail Indicator.
        uint8_t DUMMY4: 1;
        uint8_t
                SMUTE: 1; //!<  Soft Mute Indicator. Indicates soft mute is engaged.
        uint8_t DUMMY5: 4;
        // RESP3
        uint8_t STBLEND: 7; //!<  Indicates amount of stereo blend in% (100 = full
        //!<  stereo, 0 = full mono).
        uint8_t PILOT: 1;   //!<  Indicates stereo pilot presence.
        // RESP4 to RESP7
        uint8_t RSSI; //!<  RESP4 - Contains the current receive signal strength
        //!<  (0–127 dBμV).
        uint8_t SNR;  //!<  RESP5 - Contains the current SNR metric (0–127 dB).
        uint8_t MULT; //!<  RESP6 - Contains the current multipath metric. (0 = no
        //!<  multipath; 100 = full multipath)
        uint8_t FREQOFF; //!<  RESP7 - Signed frequency offset (kHz).
    } resp;
    uint8_t raw[8];
} RSQStatus;

typedef union {
    struct {
        // ARG1
        uint8_t AGCDIS: 1; // if set to 1 indicates if the AGC is disabled. 0 = AGC
        // enabled; 1 = AGC disabled.
        uint8_t DUMMY: 7;
        // ARG2
        uint8_t AGCIDX; // AGC Index; If AMAGCDIS = 1, this byte forces the AGC gain
        // index; 0 = Minimum attenuation (max gain)
    } arg;
    uint8_t raw[2];
} SI47XX_AgcOverrride;

typedef union {
    struct {
        uint8_t
                FAST: 1; //!<  ARG1 - FAST Tuning. If set, executes fast and
        //!<  invalidated tune. The tune status will not be accurate.
        uint8_t FREEZE: 1; //!<  Valid only for FM (Must be 0 to AM)
        uint8_t DUMMY1: 4; //!<  Always set 0
        uint8_t
                USBLSB: 2; //!<  SSB Upper Side Band (USB) and Lower Side Band (LSB)
        //!<  Selection. 10 = USB is selected; 01 = LSB is selected.
        uint8_t FREQH;  //!<  ARG2 - Tune Frequency High byte.
        uint8_t FREQL;  //!<  ARG3 - Tune Frequency Low byte.
        uint8_t ANTCAPH; //!<  ARG4 - Antenna Tuning Capacitor High byte.
        uint8_t ANTCAPL; //!<  ARG5 - Antenna Tuning Capacitor Low byte. Note used
        //!<  for FM.
    } arg;
    uint8_t raw[5];
} SI47XX_SetFrequency;

typedef union {
    struct {
        uint8_t AMCHFLT: 4; //!<  Selects the bandwidth of the AM channel filter.
        uint8_t DUMMY1: 4;
        uint8_t AMPLFLT: 1; //!<  Enables the AM Power Line Noise Rejection Filter.
        uint8_t DUMMY2: 7;
    } param;
    uint8_t raw[2];
} SI47XX_BW_Config; // AM_CHANNEL_FILTER

void SI47XX_PowerUp();

void SI47XX_PowerDown();

void SI47XX_SetFreq(uint16_t freq);

void SI47XX_ReadRDS(uint8_t buf[13]);

void SI47XX_SwitchMode(SI47XX_MODE mode);

bool SI47XX_IsSSB();

void RSQ_GET();

void SI47XX_SetAutomaticGainControl(uint8_t AGCDIS, uint8_t AGCIDX);

void SI47XX_Seek(bool up, bool wrap);

uint16_t SI47XX_getFrequency(bool *valid);

void SI47XX_SetBandwidth(SI47XX_FilterBW AMCHFLT, bool AMPLFLT);

void SI47XX_SetSsbBandwidth(SI47XX_SsbFilterBW bw);

void SI47XX_SetSeekFmLimits(uint16_t bottom, uint16_t top);

void SI47XX_SetSeekAmLimits(uint16_t bottom, uint16_t top);

void SI47XX_SetSeekFmSpacing(uint16_t spacing);

void SI47XX_SetSeekAmSpacing(uint16_t spacing);

void SI47XX_SetSeekFmRssiThreshold(uint16_t value);

void SI47XX_SetSeekAmRssiThreshold(uint16_t value);

void SI47XX_SetBFO(int16_t bfo);

void SI47XX_SetSsbCapacitor(uint16_t v);

uint32_t Read_FreqSaved();

bool FreqCheck(uint32_t f);

extern SI47XX_MODE si4732mode;
extern RSQStatus rsqStatus;
extern uint16_t siCurrentFreq;
extern uint16_t divider;

#define PATCH_SIZE 15832


#define SI4732_FREQ_ADD 0X01FE0




#endif /* end of include guard: SI473X_H */
