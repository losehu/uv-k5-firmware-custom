
#include "bitmaps.h"

// all these images are on their right sides
// turn your monitor 90-deg anti-clockwise to see the images

const uint8_t BITMAP_POWERSAVE[8] =
        {
                // "PS"
                0b00000000,
                0b01111111,
                0b00010001,
                0b00001110,
                0b00000000,
                0b01000110,
                0b01001001,
                0b00110001

        };

const uint8_t BITMAP_TX[8] =
        {	// "TX"
                0b00000000,
                0b00000001,
                0b00000001,
                0b01111111,
                0b00000001,
                0b00000001,
                0b00000000,
                0b00000000
        };

const uint8_t BITMAP_RX[8] =
        {	// "RX"
                0b00000000,
                0b01111111,
                0b00001001,
                0b00011001,
                0b01100110,
                0b00000000,
                0b00000000,
                0b00000000
        };

const uint8_t BITMAP_FM[10] =
        {	// "FM"
                0b00000000,
                0b01111111,
                0b00001001,
                0b00000001,
                0b00000000,
                0b01111111,
                0b00000010,
                0b00001100,
                0b00000010,
                0b01111111
        };

const uint8_t BITMAP_BatteryLevel[2] =
        {
                0b01011101,
                0b01011101
        };
#ifdef DENABLE_SHOW_BAT_SYMBOL
#ifndef ENABLE_REVERSE_BAT_SYMBOL
// Quansheng way (+ pole to the left)
const uint8_t BITMAP_BatteryLevel1[17] =
        {
                0b00000000,
                0b00111110,
                0b00100010,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01000001,
                0b01111111
        };
#else
// reversed (+ pole to the right)
	const uint8_t BITMAP_BatteryLevel1[17] =
	{
		0b00000000,
		0b01111111,
		0b01000001,
		0b01000001,
		0b01000001,
		0b01000001,
		0b01000001,
		0b01000001,
		0b01000001,
		0b01000001,
		0b01000001,
		0b01000001,
		0b01000001,
		0b01000001,
		0b01000001,
		0b00100010,
		0b00111110
	};
#endif
#endif

const uint8_t BITMAP_USB_C[9] =
        {	// USB symbol
                0b00000000,
                0b00011100,
                0b00100111,
                0b01000100,
                0b01000100,
                0b01000100,
                0b01000100,
                0b00100111,
                0b00011100
        };

const uint8_t BITMAP_KeyLock[6] =
        {	// teeny padlock symbol
                0b00000000,
                0b01111100,
                0b01000110,
                0b01000101,
                0b01000110,
                0b01111100
        };

const uint8_t BITMAP_F_Key[6] =
        {	// F-Key symbol
                0b00000000,
                0b01011111,
                0b01000101,
                0b01000101,
                0b01000101,
                0b01000001
        };

#ifdef ENABLE_VOX
const uint8_t BITMAP_VOX[18] =
	{	// "VOX"
		0b00000000,
		0b00011111,
		0b00100000,
		0b01000000,
		0b00100000,
		0b00011111,
		0b00000000,
		0b00111110,
		0b01000001,
		0b01000001,
		0b01000001,
		0b00111110,
		0b00000000,
		0b01100011,
		0b00010100,
		0b00001000,
		0b00010100,
		0b01100011
	};
#endif


// 'XB' (cross-band/cross-VFO)
const uint8_t BITMAP_XB[12] =
        {	// "XB"
                0b00000000,
                0b01100011,
                0b00010100,
                0b00001000,
                0b00010100,
                0b01100011,
                0b00000000,
                0b01111111,
                0b01001001,
                0b01001001,
                0b01001001,
                0b00110110
        };


const uint8_t BITMAP_TDR1[16] =
        {	// "DWR"
                0b00000000,
                0b01111111,
                0b01000001,
                0b01000001,
                0b00111110,
                0b00000000,
                0b01111111,
                0b00100000,
                0b00011000,
                0b00100000,
                0b01111111,
                0b00000000,
                0b01111111,
                0b00011001,
                0b00101001,
                0b01000110
        };

const uint8_t BITMAP_TDR2[10] =
        {	// "><" .. DW on hold
                0b00000000,
                0b00100010,
                0b00110110,
                0b00011100,
                0b00001000,
                0b00000000,
                0b00001000,
                0b00011100,
                0b00110110,
                0b00100010,
        };

#ifdef ENABLE_VOICE
const uint8_t BITMAP_VoicePrompt[9] =
	{
		0b00000000,
		0b00011000,
		0b00011000,
		0b00100100,
		0b00100100,
		0b01000010,
		0b01000010,
		0b11111111,
		0b00011000
	};
#endif

#ifdef ENABLE_NOAA
const uint8_t BITMAP_NOAA[11] =
	{	// "NS"
		0b00000000,
		0b01111111,
		0b00000100,
		0b00001000,
		0b00010000,
		0b01111111,
		0b00000000,
		0b01000110,
		0b01001001,
		0b01001001,
		0b00110001
	};
#endif

const uint8_t BITMAP_Antenna[5] =
        {
                0b00000011,
                0b00000101,
                0b01111111,
                0b00000101,
                0b00000011
        };

const uint8_t BITMAP_VFO_Default[8] =
        {
                0b00000000,
                0b01111111,
                0b01111111,
                0b00111110,
                0b00111110,
                0b00011100,
                0b00011100,
                0b00001000
        };

const uint8_t BITMAP_VFO_NotDefault[8] =
        {
                0b00000000,
                0b01000001,
                0b01000001,
                0b00100010,
                0b00100010,
                0b00010100,
                0b00010100,
                0b00001000
        };

const uint8_t BITMAP_ScanList1[6] =
        {	// 'I' symbol
                0b00000000,
                0b00000000,
                0b01000010,
                0b01111110,
                0b01000010,
                0b00000000
        };

const uint8_t BITMAP_ScanList2[6] =
        {	// 'II' symbol
                0b00000000,
                0b01000010,
                0b01111110,
                0b01000010,
                0b01111110,
                0b01000010
        };

const uint8_t BITMAP_compand[6] =
        {
                0b00000000,
                0b00111100,
                0b01000010,
                0b01000010,
                0b01000010,
                0b00100100
        };
const uint8_t BITMAP_RECV[13] =
        {


                0x7F, 0x7F, 0x1B, 0x3B, 0x7F, 0x66,0X00,0x63, 0x77, 0x1C, 0x1C, 0x77, 0x63
        };
const uint8_t BITMAP_SEND[13] =
        {
                0x03, 0x03, 0x7F, 0x7F, 0x03, 0x03,0X00, 0x63, 0x77, 0x1C, 0x1C, 0x77, 0x63
        };
