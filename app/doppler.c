//
// Created by RUPC on 2024/1/30.
//
#include "app/mdc1200.h"
#include <string.h>
#include "driver/keyboard.h"
#include "driver/st7565.h"
#include "driver/bk4819.h"
#include "external/printf/printf.h"
#include "misc.h"
#include "settings.h"
#include "radio.h"
#include "app.h"
#include "audio.h"
#include "functions.h"
#include "frequencies.h"
#include "driver/system.h"
#include "ui/ui.h"
#include "driver/uart.h"
#include "stdbool.h"

#include "app/doppler.h"
