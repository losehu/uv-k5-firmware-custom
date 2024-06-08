
#ifndef MDC1200H
#define MDC1200H

#include <stdint.h>
#include <stdbool.h>

#define MDC1200_FEC_K   7        // R=1/2 K=7 convolutional coder

// 0x00 (0x81) emergency alarm
// 0x20 (0x00) emergency alarm ack
//
// 0x01 (0x80) is PTT ID
// 0x01 (0x00) is POST ID
// 0x11 (0x8A) is REMOTE MONITOR
// 0x22 (0x06) is STATUS REQ
// 0x2B (0x0C) is RADIO ENABLE
// 0x2B (0x00) is RADIO DISABLE
// 0x35 (0x89) is CALL ALERT
// 0x46 (0xXX) is STS XX
// 0x47 (0xXX) is MSG XX
//
// 0x63 (0x85) is RADIO CHECK
// 0x30 (0x00) is RADIO CHECK ack
//
// * CALL ALERT [Double packet - 2 codewords, 1234 places call to 5678]
// 3589 5678 830D 1234 [Spectra, Astro Saber "PAGE", Maxtrac "CA" w/Ack Expected=Y]
// 3589 5678 810D 1234 [Maxtrac "CA" w/Ack Expected=N]
//
// * VOICE SELECTIVE CALL [Double packet - 2 codewords, 1234 places call to 5678]
// 3589 5678 8205 1234 [Spectra "CALL"]
// 3589 5678 8015 1234 [Maxtrac "SC", Astro Saber "CALL"]
// 
// * CALL ALERT ACKNOWLEDGE [Double packet - 2 codewords, 5678 acks the call from 1234]
// 3589 1234 A000 5678
// 
// * SIMPLE STATUS [unit 1234 transmits status number X]
// 460X 1234
// 
// * STATUS ACKNOWLEDGE
// 2300 1234
// 
// * STATUS REQUEST [i.e. unit 5678 report your last status]
// 2206 5678
// 
// * STATUS RESPONSE [from target 5678 when interrogated]
// 060X 5678
// 
// * INBOUND MESSAGE
// 470X 1234 [ack expected]
// 070X 1234 [ack not expected CDM1550]
// 
// * INBOUND MESSAGE ACKNOWLEDGE
// 2300 1234
// 
// * REMOTE MONITOR [No MDC response from target unless it has PTT ID]
// 118A 5678 [118A per KA6SQG]
// 
// * SELECTIVE RADIO INHIBIT [Fixed end inhibits target 5678]
// 2B00 5678
// 
// * SELECTIVE RADIO INHIBIT ACKNOWLEDGE [5678 acks the inhibit]
// 0B00 5678
// 
// * SELECTIVE RADIO INHIBIT CANCEL [Fixed end enables target 5678]
// 2B0C 5678
// 
// * SELECTIVE RADIO INHIBIT CANCEL [5678 acks the enable]
// 0B0C 5678
// 
// * REQUEST TO TALK [Unit 1234 asks fixed end for permission to PTT]
// 4001 1234 [CDM1550 dedicated button]
// 4101 1234 [CDM1550 slaved to mic PTT]
// 
// * REQUEST TO TALK ACKNOWLEDGE
// 2300 1234 [general ack - not same as permission to PTT]

enum mdc1200_op_code_e {
    MDC1200_OP_CODE_PTT_ID = 0x01,
    MDC1200_OP_CODE_POST_ID = 0x01,
    MDC1200_OP_CODE_REMOTE_MONITOR = 0x11,
    MDC1200_OP_CODE_STATUS_REQ = 0x22,
    MDC1200_OP_CODE_RADIO_ENABLE = 0x2B,
    MDC1200_OP_CODE_RADIO_DISABLE = 0x2B,
    MDC1200_OP_CODE_CALL_ALERT = 0x35,
    MDC1200_OP_CODE_STS_XX = 0x46,
    MDC1200_OP_CODE_MSG_XX = 0x47,
    MDC1200_OP_CODE_RADIO_CHECK = 0x63
};
typedef enum mdc1200_op_code_e mdc1200_op_code_t;

extern const uint8_t mdc1200_sync[5];
extern const uint8_t mdc1200_sync_suc_xor[5];

extern uint8_t mdc1200_op;
extern uint8_t mdc1200_arg;
extern uint16_t mdc1200_unit_id;

bool mdc1200_contact_find(uint16_t mdc_id, char *contact);

extern uint8_t mdc1200_rx_buffer[5 + (MDC1200_FEC_K * 2)];
extern unsigned int mdc1200_rx_buffer_index;

bool MDC1200_process_rx_data(const void *buffer, const unsigned int size, uint8_t *op, uint8_t *arg, uint16_t *unit_id);

extern uint8_t mdc1200_rx_ready_tick_500ms;

unsigned int MDC1200_encode_single_packet(void *data, const uint8_t op, const uint8_t arg, const uint16_t unit_id);

void MDC1200_reset_rx(void);

void MDC1200_init(void);


uint16_t extractHex(const char *str);

//extern uint16_t MDC_ADD[6];
#define MDC_NUM_ADD 0X1FFF
#define MAX_CONTACT_NUM 21
extern uint8_t contact_num;

#endif
