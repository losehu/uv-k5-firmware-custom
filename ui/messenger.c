
#ifdef ENABLE_MESSENGER

#include <string.h>
#include "app/messenger.h"
#include "driver/st7565.h"
#include "ui/messenger.h"
#include "ui/helper.h"

void UI_DisplayMSG(void) {
    UI_DisplayClear();

    if (msgComposeMode == MSG_VIEW_HOME) {
        UI_PrintStringSmall("MENU:NEW", 0, 127, 1);
        UI_PrintStringSmall("UP:LIST", 0, 127, 3);
        UI_PrintStringSmall("DOWN:APRS", 0, 127, 5);
    } else if (msgComposeMode == MSG_VIEW_HISTORY) {
        if (msgHistoryCount == 0) {
            UI_PrintStringSmall("NO LIST", 0, 127, 3);
        } else {
 #ifdef ENABLE_ENGLISH
            uint8_t end = msgHistoryOffset + MSG_HISTORY_VISIBLE;
            if (end > msgHistoryCount)
                end = msgHistoryCount;
            uint8_t y = 55 - (end - msgHistoryOffset) * 5;
            for (uint8_t i = msgHistoryOffset; i < end; ++i, y += 5)
 #else
            uint8_t y = 55 - msgHistoryCount * 5;
            for (uint8_t i = 0; i < msgHistoryCount; ++i, y += 5)
 #endif
                GUI_DisplaySmallest(rxMessage[i], 1, y, false, true);
        }
    } else {
        const char *mode = keyboardType == NUMERIC ? "123" :
                           keyboardType == UPPERCASE ? "ABC" : "abc";

        GUI_DisplaySmallest(mode, 114, 1, false, true);
        GUI_DisplaySmallest("U/D:MOVE", 18, 10, false, true);

        const char cursor = cMessage[cIndex];
        cMessage[cIndex] = '_';
        const char split = cMessage[18];
        cMessage[18] = '\0';
        UI_PrintStringSmall(cMessage, 1, 0, 3);
        cMessage[18] = split;
        if (msgComposeMode == MSG_VIEW_APRS || cIndex >= 18)
            UI_PrintStringSmall(cMessage + 18, 1, 0, 4);
        cMessage[cIndex] = cursor;
        GUI_DisplaySmallest("PTT:SEND", 42, 49, false, true);
    }

    ST7565_BlitFullScreen();
}

#endif
