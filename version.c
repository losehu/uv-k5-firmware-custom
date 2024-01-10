#include "font.h"
#define ONE_OF_ELEVEN_VER

#ifdef GIT_HASH
	#define VER     GIT_HASH
#else
	#define VER     "11"


#endif

#ifndef ONE_OF_ELEVEN_VER
	const char Version[]      = "OEFW-"VER;
	const char UART_Version[] = "UV-K5 Firmware, Open Edition, OEFW-"VER"\r\n";
#else
//#if ENABLE_CHINESE_FULL !=4
//const char Version[]      = "LOSEHU"VER;
//#else
//const char Version[]      = "LOSEHU"VER"K";
//#endif

const char Version[]      = PACKED_FILE_SUFFIX;


#endif
