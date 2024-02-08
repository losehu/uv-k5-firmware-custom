#ifndef APP_MSG_H
#define APP_MSG_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "driver/keyboard.h"

#ifdef ENABLE_MESSENGER


typedef enum KeyboardType {
    UPPERCASE,
      LOWERCASE,
      NUMERIC,
      END_TYPE_KBRD
} KeyboardType;
typedef enum MsgStatus {
    READY,
      SENDING,
      RECEIVING,
} MsgStatus;
enum {
    TX_MSG_LENGTH = 30,
    MSG_HEADER_LENGTH = 20,
    MAX_RX_MSG_LENGTH = TX_MSG_LENGTH + 2
};
//const uint8_t TX_MSG_LENGTH = 30;
//const uint8_t MAX_RX_MSG_LENGTH = TX_MSG_LENGTH + 2;
uint8_t validate_char( uint8_t rchar ) ;

extern KeyboardType keyboardType;
extern uint16_t gErrorsDuringMSG;
extern char cMessage[TX_MSG_LENGTH];
extern char rxMessage[4][MAX_RX_MSG_LENGTH + 2];
extern uint8_t hasNewMessage;
extern uint8_t keyTickCounter;

void MSG_Init();
void MSG_ProcessKeys(KEY_Code_t Key, bool bKeyPressed, bool bKeyHeld);
void MSG_Send(const char *txMessage, bool bServiceMessage);
extern unsigned char cIndex ;
//extern bool stop_mdc_rx;
extern uint8_t msgFSKBuffer[MSG_HEADER_LENGTH + MAX_RX_MSG_LENGTH];
void moveUP(char (*rxMessages)[MAX_RX_MSG_LENGTH + 2]) ;

extern MsgStatus msgStatus ;
extern bool stop_mdc_flag;

#endif

void solve_sign(const uint16_t interrupt_bits);

#endif
