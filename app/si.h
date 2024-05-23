#ifndef APP_SI_H
#define APP_SI_H

#include "../driver/keyboard.h"
#include <stdbool.h>
#include <stdint.h>

void SI_init();
void SI_update();
bool SI_key(KEY_Code_t key, bool bKeyPressed, bool bKeyHeld);
void SI_render();
void SI_deinit();

#endif /* end of include guard: APP_SI_H */
