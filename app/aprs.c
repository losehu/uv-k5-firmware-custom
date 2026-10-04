#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "app/aprs.h"
#include "app/messenger.h"

#ifdef ENABLE_MESSENGER

#define APRS_HDLC_BUF_SIZE 96u
#define APRS_LEAD_FLAGS 32u

typedef struct {
    uint16_t bits;
    uint8_t ones;
    uint8_t level;
} APRS_HdlcWriter;

static uint8_t aprsHdlcBuffer[APRS_HDLC_BUF_SIZE];

static void APRS_PutBit(APRS_HdlcWriter *writer, bool bit) {
    if (!bit)
        writer->level ^= 1;
    if (writer->level)
        aprsHdlcBuffer[writer->bits >> 3] |= 0x80u >> (writer->bits & 7u);
    writer->bits++;
}

static void APRS_PutByte(APRS_HdlcWriter *writer, uint8_t byte, bool stuff) {
    for (uint8_t i = 0; i < 8; i++) {
        const bool bit = (byte >> i) & 1u;
        APRS_PutBit(writer, bit);
        if (stuff && bit) {
            if (++writer->ones == 5) {
                APRS_PutBit(writer, false);
                writer->ones = 0;
            }
        } else {
            writer->ones = 0;
        }
    }
}

static uint16_t APRS_PutDataByte(APRS_HdlcWriter *writer,
                                 uint16_t crc, uint8_t byte) {
    crc ^= byte;
    for (uint8_t i = 0; i < 8; i++) {
        const bool bit = (byte >> i) & 1u;
        APRS_PutBit(writer, bit);
        if (bit) {
            if (++writer->ones == 5) {
                APRS_PutBit(writer, false);
                writer->ones = 0;
            }
        } else {
            writer->ones = 0;
        }
        crc = (crc & 1) ? (crc >> 1) ^ 0x8408 : crc >> 1;
    }
    return crc;
}

// Minimal APRS position TX, derived from bcanata/uv-k5-firmware-ta1js
// (Apache-2.0). RX, menus, timers, paths and position parsing are omitted.
__attribute__((cold, noinline)) void APRS_Transmit(const char *position) {
    // Pre-encoded NRZI/HDLC bits for the fixed APOVK5-0 destination.
    static const uint8_t encodedDestination[] = {
        0x2b, 0x53, 0x04, 0x8c, 0xe4, 0xce, 0xae
    };
    static const uint8_t suffix[] = {0x61, 0x03, 0xf0, '!'};
    memset(aprsHdlcBuffer, 0, sizeof(aprsHdlcBuffer));
    memset(aprsHdlcBuffer, 1, APRS_LEAD_FLAGS);
    memcpy(aprsHdlcBuffer + APRS_LEAD_FLAGS,
           encodedDestination, sizeof(encodedDestination));

    // Continue after the fixed destination with the editable source callsign.
    APRS_HdlcWriter writer = {312, 0, 0};
    uint16_t crc = 0x8ccc;
    for (uint8_t i = 0; i < APRS_CALL_LENGTH; i++) {
        uint8_t character = position[i];
        if (character >= 'a' && character <= 'z')
            character -= 'a' - 'A';
        crc = APRS_PutDataByte(&writer, crc, character << 1);
    }
    for (uint8_t i = 0; i < sizeof(suffix); i++)
        crc = APRS_PutDataByte(&writer, crc, suffix[i]);
    position += APRS_POSITION_OFFSET;
    for (uint8_t i = 0; i < APRS_POSITION_LENGTH; i++)
        crc = APRS_PutDataByte(&writer, crc, position[i]);
    position += APRS_POSITION_LENGTH;
    for (uint8_t i = 0; i < APRS_SUFFIX_LENGTH; i++)
        crc = APRS_PutDataByte(&writer, crc, position[i]);
    crc = ~crc;
    for (uint8_t i = 0; i < 2; i++)
        APRS_PutByte(&writer, (uint8_t)(crc >> (i * 8)), true);
    APRS_PutByte(&writer, 0x7e, false);

    const uint8_t bytes = ((writer.bits + 15u) >> 4) << 1;
    MSG_FSKSendData(aprsHdlcBuffer, bytes, true);
}

#endif
