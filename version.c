#include "font.h"
#define ONE_OF_ELEVEN_VER

#ifdef GIT_HASH
	#define VER     GIT_HASH
#else
#if ENABLE_CHINESE_FULL ==0
	#define VER     "111"
#else
#define VER     "111K"
#endif

#endif

#ifndef ONE_OF_ELEVEN_VER
	const char Version[]      = "OEFW-"VER;
	const char UART_Version[] = "UV-K5 Firmware, Open Edition, OEFW-"VER"\r\n";
#else
    const char Version[]      = "LOSEHU"VER;
#endif
