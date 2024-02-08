
#ifdef ENABLE_MESSENGER

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


    UI_DisplayClear();
    UI_PrintStringSmall("MES", 1, 127, 0);



    uint8_t mPos = 8;
    const uint8_t mLine = 7;
    for (int i = 0; i < 4; ++i) {
        GUI_DisplaySmallest(rxMessage[i], 2, mPos, false, true);
        mPos += mLine;
    }


const uint8_t *p;
    if (keyboardType == NUMERIC) {
        p= BITMAP_1;
    } else if (keyboardType == UPPERCASE) {
        p= BITMAP_TX;
    } else {
        p= BITMAP_t;
    }
    memcpy(gFrameBuffer[0], p, 6);


    cMessage[cIndex]='_';
    cMessage[cIndex+1]='\0';

    GUI_DisplaySmallest(cMessage, 5, 48, false, true);
    cMessage[cIndex]='\0';





    ST7565_BlitFullScreen();
}

#endif
