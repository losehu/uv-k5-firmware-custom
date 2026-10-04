
#ifdef ENABLE_MESSENGER

#include <string.h>
#include "app/messenger.h"
#include "driver/st7565.h"
#include "ui/messenger.h"
#include "ui/helper.h"

void UI_DisplayMSG(void) {
    UI_DisplayClear();

    if (!msgComposeMode) {
        if (msgHistoryCount == 0) {
            UI_PrintStringSmall("NO MESSAGE", 0, 127, 3);
            GUI_DisplaySmallest("MENU:NEW  EXIT:BACK", 26, 50, false, true);
        } else {
            uint8_t y = 62 - msgHistoryCount * 6;
            for (uint8_t i = 0; i < msgHistoryCount; ++i, y += 6)
                GUI_DisplaySmallest(rxMessage[i], 1, y, false, true);
        }
    } else {
        const char *mode = keyboardType == NUMERIC ? "123" :
                           keyboardType == UPPERCASE ? "ABC" : "abc";

        UI_PrintStringSmall("BROADCAST", 0, 127, 0);
        GUI_DisplaySmallest(mode, 114, 1, false, true);
        GUI_DisplaySmallest("TO: ALL ON FREQUENCY", 24, 10, false, true);

        cMessage[cIndex] = '_';
        const char split = cMessage[18];
        cMessage[18] = '\0';
        UI_PrintStringSmall(cMessage, 1, 0, 3);
        cMessage[18] = split;
        if (cIndex >= 18)
            UI_PrintStringSmall(cMessage + 18, 1, 0, 4);
        cMessage[cIndex] = '\0';
        GUI_DisplaySmallest("MENU:SEND  F:DELETE", 26, 49, false, true);
        GUI_DisplaySmallest("*:ABC/abc/123  EXIT:LIST", 14, 57, false, true);
    }

    ST7565_BlitFullScreen();
}

#endif
