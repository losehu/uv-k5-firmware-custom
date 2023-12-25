#include "font.h"
#define ONE_OF_ELEVEN_VER

#ifdef GIT_HASH
	#define VER     GIT_HASH
#else
#if ENABLE_CHINESE_FULL ==0
	#define VER     "112"
#else
#define VER     "112"
#endif

#endif

#ifndef ONE_OF_ELEVEN_VER
	const char Version[]      = "OEFW-"VER;
	const char UART_Version[] = "UV-K5 Firmware, Open Edition, OEFW-"VER"\r\n";
#else
#if ENABLE_CHINESE_FULL ==0
const char Version[]      = "LOSEHU"VER;
#else
const char Version[]      = "LOSEHU"VER"K";
#endif
#endif
