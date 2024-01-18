
//#ifdef ENABLE_MESSENGER

#include <string.h>
#include "app/spectrum.h"
#include <string.h>
#include "app/messenger.h"
#include "driver/st7565.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "settings.h"
#include "ui/messenger.h"
#include "ui/helper.h"
#include "ui/inputbox.h"
#include "ui/ui.h"

#ifdef ENABLE_DOCK
#include "app/uart.h"
#endif

void UI_DisplayMSG(void) {

    static char String[37];
    UI_DisplayClear();
    memset(String, 0, sizeof(String));
    UI_PrintStringSmall("Messenger", 1, 127, 0);
     const uint8_t *p;

    uint8_t mPos = 8;
    const uint8_t mLine = 7;
    for (int i = 0; i < 2; ++i) {
        UI_PrintStringSmall(rxMessage[i], 2, 0, i * 2 + 1);
        mPos += mLine;
    }

    if (keyboardType == NUMERIC) {
        p= BITMAP_1;
    } else if (keyboardType == UPPERCASE) {
        p= BITMAP_TX;
    } else {
        p= BITMAP_t;
    }
    memcpy(gFrameBuffer[0], p, 6);

    cMessage[cIndex] = '_';
    cMessage[cIndex + 1] = 0;

    char a;
    if (cIndex > 18) {
        a = cMessage[18];
        cMessage[18] = 0;
    }
        UI_PrintStringSmall(cMessage, 0, 0, 5);
    if (cIndex > 18) {
        cMessage[18] = a;
        UI_PrintStringSmall(cMessage+18, 0, 0, 6);
    }
    ST7565_BlitFullScreen();
    cMessage[cIndex] = 0;

}

//#endif
