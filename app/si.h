#ifndef APP_SI_H
#define APP_SI_H
#include "spectrum.h"

#include "../driver/keyboard.h"
#include <stdbool.h>
#include <stdint.h>

void SI_init();
void SI_update();
bool SI_key(KEY_Code_t key, bool bKeyPressed, bool bKeyHeld);
void SI_render();
void SI_deinit();
void Key_FM(KeyboardState kbds) ;
    void HandleUserInput() ;
void SI4732_Main();

#endif /* end of include guard: APP_SI_H */
