
#include "driver/bk4819.h"
#include "driver/crc.h"
#include "driver/uart.h"
#include "mdc1200.h"
#include "misc.h"
#include <string.h>
#include "driver/eeprom.h"


const uint8_t mdc1200_pre_amble[] = {0x00, 0x00, 0x00};
const uint8_t mdc1200_sync[5] = {0x07, 0x09, 0x2a, 0x44, 0x6f};

const uint8_t mdc1200_sync_suc_xor[5] = {0xfb, 0x72, 0x40, 0x99, 0xa7};


void error_correction(void *data) {    // can correct up to 3 or 4 corrupted bits (I think)

    int i;
    uint8_t shift_reg;
    uint8_t syn;
    uint8_t *data8 = (uint8_t *) data;

    for (i = 0, shift_reg = 0, syn = 0; i < MDC1200_FEC_K; i++) {
        const uint8_t bi = data8[i];
        int bit_num;
        for (bit_num = 0; bit_num < 8; bit_num++) {
            uint8_t b;
            unsigned int k = 0;

            shift_reg = (shift_reg << 1) | ((bi >> bit_num) & 1u);
            b = ((shift_reg >> 6) ^ (shift_reg >> 5) ^ (shift_reg >> 2) ^ (shift_reg >> 0)) & 1u;
            syn = (syn << 1) | (((b ^ (data8[i + MDC1200_FEC_K] >> bit_num)) & 1u) ? 1u : 0u);

            if (syn & 0x80) k++;
            if (syn & 0x20) k++;
            if (syn & 0x04) k++;
            if (syn & 0x02) k++;

            if (k >= 3) {    // correct a bit error
                int ii = i;
                int bn = bit_num - 7;
                if (bn < 0) {
                    bn += 8;
                    ii--;
                }
                if (ii >= 0)
                    data8[ii] ^= 1u << bn;   // fix a bit
                syn ^= 0xA6;   // 10100110
            }
        }
    }
}

bool decode_data(void *data) {
    uint16_t crc1;
    uint16_t crc2;
    uint8_t *data8 = (uint8_t *) data;

    {    // de-interleave

        unsigned int i;
        unsigned int k;
        unsigned int m;
        uint8_t deinterleaved[(MDC1200_FEC_K * 2) * 8];  // temp individual bit storage

        // interleave order
        //  0, 16, 32, 48, 64, 80,  96,
        //  1, 17, 33, 49, 65, 81,  97,
        //  2, 18, 34, 50, 66, 82,  98,
        //  3, 19, 35, 51, 67, 83,  99,
        //  4, 20, 36, 52, 68, 84, 100,
        //  5, 21, 37, 53, 69, 85, 101,
        //  6, 22, 38, 54, 70, 86, 102,
        //  7, 23, 39, 55, 71, 87, 103,
        //  8, 24, 40, 56, 72, 88, 104,
        //  9, 25, 41, 57, 73, 89, 105,
        // 10, 26, 42, 58, 74, 90, 106,
        // 11, 27, 43, 59, 75, 91, 107,
        // 12, 28, 44, 60, 76, 92, 108,
        // 13, 29, 45, 61, 77, 93, 109,
        // 14, 30, 46, 62, 78, 94, 110,
        // 15, 31, 47, 63, 79, 95, 111

        // de-interleave the received bits
        for (i = 0, k = 0; i < 16; i++) {
            for (m = 0; m < MDC1200_FEC_K; m++) {
                const unsigned int n = (m * 16) + i;
                deinterleaved[k++] = (data8[n >> 3] >> ((7 - n) & 7u)) & 1u;
            }
        }

        // copy the de-interleaved bits back into the data buffer
        for (i = 0, m = 0; i < (MDC1200_FEC_K * 2); i++) {
            unsigned int k;
            uint8_t b = 0;
            for (k = 0; k < 8; k++)
                if (deinterleaved[m++])
                    b |= 1u << k;
            data8[i] = b;
        }
    }

    // try to correct the odd corrupted bit
    error_correction(data);

    // rx'ed de-interleaved data (min 14 bytes) looks like this ..
    //
    // OP  ARG  ID    CRC   STATUS  FEC bits
    // 01  80   1234  2E3E  00      6580A862DD8808

    crc1 = compute_crc(data, 4);
    crc2 = ((uint16_t) data8[5] << 8) | (data8[4] << 0);

    return (crc1 == crc2) ? true : false;
}

// **********************************************************
// TX

void xor_modulation(void *data, const unsigned int size) {    // exclusive-or succesive bits - the entire packet
    unsigned int i;
    uint8_t *data8 = (uint8_t *) data;
    uint8_t prev_bit = 0;
    for (i = 0; i < size; i++) {
        int bit_num;
        uint8_t in = data8[i];
        uint8_t out = 0;
        for (bit_num = 7; bit_num >= 0; bit_num--) {
            const uint8_t new_bit = (in >> bit_num) & 1u;
            if (new_bit != prev_bit)
                out |= 1u << bit_num;        // previous bit and new bit are different - send a '1'
            prev_bit = new_bit;
        }
        data8[i] = out ^ 0xff;
    }
}

uint8_t *encode_data(void *data) {
    // R=1/2 K=7 convolutional coder
    //
    // OP  ARG  ID    CRC   STATUS  FEC bits
    // 01  80   1234  2E3E  00      6580A862DD8808
    //
    // 1. reverse the bit order for each byte of the first 7 bytes (to undo the reversal performed for display, above)
    // 2. feed those bits into a shift register which is preloaded with all zeros
    // 3. for each bit, calculate the modulo-2 sum: bit(n-0) + bit(n-2) + bit(n-5) + bit(n-6)
    // 4. then for each byte of resulting output, again reverse those bits to generate the values shown above

    uint8_t *data8 = (uint8_t *) data;

    {    // add the FEC bits to the end of the data
        unsigned int i;
        uint8_t shift_reg = 0;
        for (i = 0; i < MDC1200_FEC_K; i++) {
            unsigned int bit_num;
            const uint8_t bi = data8[i];
            uint8_t bo = 0;
            for (bit_num = 0; bit_num < 8; bit_num++) {
                shift_reg = (shift_reg << 1) | ((bi >> bit_num) & 1u);
                bo |= (((shift_reg >> 6) ^ (shift_reg >> 5) ^ (shift_reg >> 2) ^ (shift_reg >> 0)) & 1u) << bit_num;
            }
            data8[MDC1200_FEC_K + i] = bo;
        }
    }


    {    // interleave the bits

        unsigned int i;
        unsigned int k;
        uint8_t interleaved[(MDC1200_FEC_K * 2) * 8];   // temp individual bit storage

        // interleave order
        //  0, 16, 32, 48, 64, 80,  96,
        //  1, 17, 33, 49, 65, 81,  97,
        //  2, 18, 34, 50, 66, 82,  98,
        //  3, 19, 35, 51, 67, 83,  99,
        //  4, 20, 36, 52, 68, 84, 100,
        //  5, 21, 37, 53, 69, 85, 101,
        //  6, 22, 38, 54, 70, 86, 102,
        //  7, 23, 39, 55, 71, 87, 103,
        //  8, 24, 40, 56, 72, 88, 104,
        //  9, 25, 41, 57, 73, 89, 105,
        // 10, 26, 42, 58, 74, 90, 106,
        // 11, 27, 43, 59, 75, 91, 107,
        // 12, 28, 44, 60, 76, 92, 108,
        // 13, 29, 45, 61, 77, 93, 109,
        // 14, 30, 46, 62, 78, 94, 110,
        // 15, 31, 47, 63, 79, 95, 111

        // bit interleaver
        for (i = 0, k = 0; i < (MDC1200_FEC_K * 2); i++) {
            unsigned int bit_num;
            const uint8_t b = data8[i];
            for (bit_num = 0; bit_num < 8; bit_num++) {
                interleaved[k] = (b >> bit_num) & 1u;
                k += 16;
                if (k >= sizeof(interleaved))
                    k -= sizeof(interleaved) - 1;
            }
        }

        // copy the interleaved bits back to the data buffer
        for (i = 0, k = 0; i < (MDC1200_FEC_K * 2); i++) {
            int bit_num;
            uint8_t b = 0;
            for (bit_num = 7; bit_num >= 0; bit_num--)
                if (interleaved[k++])
                    b |= 1u << bit_num;
            data8[i] = b;
        }
    }

    return data8 + (MDC1200_FEC_K * 2);
}

unsigned int MDC1200_encode_single_packet(void *data, const uint8_t op, const uint8_t arg, const uint16_t unit_id) {
    unsigned int size;
    uint16_t crc;
    uint8_t *p = (uint8_t *) data;

    memcpy(p, mdc1200_pre_amble, sizeof(mdc1200_pre_amble));
    p += sizeof(mdc1200_pre_amble);
    memcpy(p, mdc1200_sync, sizeof(mdc1200_sync));
    p += sizeof(mdc1200_sync);

    p[0] = op;
    p[1] = arg;
    p[2] = (unit_id >> 8) & 0x00ff;
    p[3] = (unit_id >> 0) & 0x00ff;
    crc = compute_crc(p, 4);
    p[4] = (crc >> 0) & 0x00ff;
    p[5] = (crc >> 8) & 0x00ff;
    p[6] = 0;      // unknown field (00 for PTTIDs, 76 for STS and MSG)

    p = encode_data(p);

    size = (unsigned int) (p - (uint8_t *) data);

    xor_modulation(data, size);

    return size;
}


struct {
    uint8_t bit;
    uint8_t prev_bit;
    uint8_t xor_bit;
    uint64_t shift_reg;
    unsigned int bit_count;
    unsigned int stage;
    bool inverted_sync;
    unsigned int data_index;
    uint8_t data[40];
} rx;

void MDC1200_reset_rx(void) {
    memset(&rx, 0, sizeof(rx));
}

bool MDC1200_process_rx_data(
        const void *buffer,
        const unsigned int size,
        //const bool inverted,
        uint8_t *op,
        uint8_t *arg,
        uint16_t *unit_id) {
    const uint8_t *buffer8 = (const uint8_t *) buffer;
    unsigned int index;

    // 04 8D BF 66 58   sync
    // FB 72 40 99 A7   inverted sync
    //
    // 04 8D BF 66 58   40 C4 B0 32 BA F9 33 18 35 08 83 F6 0C 36 .. 80 87 20 23 2C AE 22 10 26 0F 02 A4 08 24
    // 04 8D BF 66 58   45 DB 03 07 BC FA 35 2E 33 0E 83 0E 83 69 .. 86 92 02 05 28 AC 26 34 22 0B 02 0B 02 4E

    memset(&rx, 0, sizeof(rx));

    for (index = 0; index < size; index++) {
        int bit;
        const uint8_t rx_byte = buffer8[index];

        for (bit = 7; bit >= 0; bit--) {
            unsigned int i;

            rx.prev_bit = rx.bit;

            rx.bit = (rx_byte >> bit) & 1u;

            rx.xor_bit = (rx.xor_bit ^ rx.bit) & 1u;  // toggle our bit if the rx bit is high

            rx.shift_reg = (rx.shift_reg << 1) | rx.xor_bit;
            rx.bit_count++;

            // *********

            if (rx.stage == 0) {    // looking for the 40-bit sync pattern

                const unsigned int sync_bit_ok_threshold = 32;

                if (rx.bit_count >= 40) {
                    // 40-bit sync pattern
                    uint64_t sync_nor = 0x07092a446fu;            // normal
                    uint64_t sync_inv = 0xffffffffffu ^ sync_nor; // bit inverted

                    sync_nor ^= rx.shift_reg;
                    sync_inv ^= rx.shift_reg;

                    unsigned int nor_count = 0;
                    unsigned int inv_count = 0;
                    for (i = 40; i > 0; i--, sync_nor >>= 1, sync_inv >>= 1) {
                        nor_count += sync_nor & 1u;
                        inv_count += sync_inv & 1u;
                    }
                    nor_count = 40 - nor_count;
                    inv_count = 40 - inv_count;


                    if (nor_count >= sync_bit_ok_threshold || inv_count >= sync_bit_ok_threshold) {    // good enough

                        rx.inverted_sync = (inv_count > nor_count) ? true : false;
                        rx.data_index = 0;
                        rx.bit_count = 0;
                        rx.stage = 1;


                    }
                }

                continue;
            }

            if (rx.bit_count < 8)
                continue;

            rx.bit_count = 0;

            rx.data[rx.data_index++] = rx.shift_reg & 0xff;  // save the last 8 bits

            if (rx.data_index < (MDC1200_FEC_K * 2))
                continue;


            if (!decode_data(rx.data)) {
                MDC1200_reset_rx();


                continue;
            }

            // extract the info from the packet
            *op = rx.data[0];
            *arg = rx.data[1];
            *unit_id = ((uint16_t) rx.data[2] << 8) | (rx.data[3] << 0);


            // reset the detector
            MDC1200_reset_rx();

            return true;
        }
    }

    MDC1200_reset_rx();

    return false;
}

uint8_t mdc1200_rx_buffer[5 + (MDC1200_FEC_K * 2)];
unsigned int mdc1200_rx_buffer_index = 0;

uint8_t mdc1200_op;
uint8_t mdc1200_arg;
uint16_t mdc1200_unit_id;
uint8_t mdc1200_rx_ready_tick_500ms;


void MDC1200_init(void) {
//    memcpy(mdc1200_sync_suc_xor, mdc1200_sync, sizeof(mdc1200_sync));
//    xor_modulation(mdc1200_sync_suc_xor, sizeof(mdc1200_sync_suc_xor));

    MDC1200_reset_rx();
}

uint16_t extractHex(const char *str) {
    uint16_t result = 0;
    while (*str) {
        char c = *str++;
        if (c >= '0' && c <= '9') {
            result = (result << 4) | (c - '0');

        } else if (c >= 'A' && c <= 'F') {
            result = (result << 4) | (c - 'A' + 10);
        } else {
            continue; // 遇到非十六进制字符，停止解析
        }
    }
    return result;
}

#ifdef  ENABLE_MDC1200_CONTACT
uint8_t contact_num=0;
//uint16_t MDC_ADD[6] = {0x1D00, 0x1D40, 0x1D80,0x1DC0,0X1F90,0X1FD0};//SHIT ADDRESS COMBINE :(
void mdc1200_update_contact_num()
{
    EEPROM_ReadBuffer(MDC_NUM_ADD, (uint8_t *)&contact_num, 1);
    if(contact_num>MAX_CONTACT_NUM)contact_num=0;
}
bool mdc1200_contact_find(uint16_t mdc_id, char *contact) {
    mdc1200_update_contact_num();
    uint16_t add = 0x1D00;
    for (uint8_t i = 0; i < contact_num; i++) {
        uint8_t read_once[16]={0};

        EEPROM_ReadBuffer(add , read_once, 16);
        if (mdc_id == (uint16_t) (read_once[1] | (read_once[0] << 8))) {
            for (int j = 0; j < 14; ++j) {
                if(read_once[2+j]<' '||read_once[2+j]>'~')
                    return false;
            }

            memcpy(contact,read_once+2,14);

            return true;
        }
                add+=16;
        if(add==0x1E00)add=0X1F90;
    }
    return false;
}
#endif
