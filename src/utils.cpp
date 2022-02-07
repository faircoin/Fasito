/*
 * Copyright (c) 2017-2022 by Thomas König <tom@faircoin.world>
 *
 * utils.cpp is part of Fasito, the FairCoin signature token.
 *
 * Fasito is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Fasito is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Fasito, see file COPYING.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "Arduino.h"
#include "fasito.h"
#include "fasito_error.h"

FasitoNVRam nvram;
char *commandTokens[MAX_TOKEN];

bool fasitoErrorStr(const char *errorStr)
{
    strcpy(inputBuffer, errorStr);
    return false;
}

bool fasitoError(uint8_t errorNo)
{
    strcpy(inputBuffer, errorStrings[errorNo]);
    return false;
}

bool fasitoError(uint8_t errorNo, int arg)
{
    sprintf(inputBuffer, errorStrings[errorNo], arg);
    return false;
}

void printHex(const uint8_t *buf, const size_t len, const bool addLF = false)
{
    size_t i;
    char c[3], text[len * 2];

    *text=0;
    for (i = 0; i < len; i++) {
        sprintf(c, "%02x", buf[i]);
        strcat(text, c);
    }

    //Serial.print((uint32_t)buf, HEX); Serial.print(": ");

    if (addLF)
        Serial.println(text);
    else
        Serial.print(text);
}

static const int8_t char2nibble(char c)
{
    if (c >= 48 && c <= 57)
        return c - 48;
    else if (c >= 65 && c <= 70)
        return c - 65 + 10;
    else if (c >= 97 && c <= 102)
        return c - 97 + 10;

    return -1;
}

bool parseHex(uint8_t *out, const char *in, size_t outLen)
{
    size_t i;

    for (i = 0 ; i < outLen ; i++) {
        int8_t hi = char2nibble(in[i * 2]);
        int8_t lo = char2nibble(in[(i * 2) + 1]);
        if (hi < 0 || lo < 0)
            return false;

        out[i] = (hi << 4) + lo;
    }

    return true;
}

/* buf must be NULL terminated */
const char **tokenise(char *buf, uint8_t *nTokens)
{
    if (!buf || !*buf || *buf != ' ' || !buf[1]) {
        nTokens = 0;
        return NULL;
    }

    memset(commandTokens, 0, sizeof(commandTokens));
    uint8_t i = 0;
    char *pBuf = buf;

    /* skip space after command */
    pBuf++;

    /* the first token */
    commandTokens[i++] = pBuf++;

    do {
        if (!*pBuf)
            break;

        if (*pBuf == ' ' && pBuf[1]) {
            *pBuf++ = 0; // NULL terminate token in line
            commandTokens[i++] = pBuf++;
        } else
            pBuf++;
    } while(*pBuf && i < MAX_TOKEN);

    *nTokens = i;

    return (const char **)commandTokens;
}

static void crc16_update(uint16_t &crc, uint8_t data)
{
    unsigned int i;

    crc ^= data;
    for (i = 0; i < 8; ++i) {
        if (crc & 1)
            crc = (crc >> 1) ^ 0xA001;
        else
            crc = crc >> 1;
    }
}

static uint16_t checkCRC16(uint8_t *data, uint16_t len)
{
    uint16_t i, crc = 0;
    for (i = 0 ; i < len ; i++)
        crc16_update(crc, data[i]);

    return crc;
}

void reverseBytes(uint8_t *buf, size_t len)
{
    size_t i;
    uint8_t m;

    for (i = 0; i < len / 2 ; i++) {
        m = buf[i];
        buf[i] = buf[len - i - 1];
        buf[len - i - 1] = m;
    }
}

bool comparePins(const UserPIN *userPin, const char *pin)
{
    size_t i, pinLen;

    if (!pin || !*pin)
        return false;

    if (userPin->status != userPin->SET)
        return false;

    pinLen = strlen(userPin->pin);
    if (pinLen != strlen(pin))
        return false;

    for (i = 0 ; i < pinLen ; i++)
        if (pin[i] != userPin->pin[i])
            return false;

    return true;
}

bool readEEPROM(FasitoNVRam *dst)
{
    eeprom_read_block(dst, 0, sizeof(FasitoNVRam));
    return dst->checksum == checkCRC16((uint8_t *)dst, sizeof(FasitoNVRam) - 2);
}

void writeEEPROM(FasitoNVRam *dst)
{
    //Serial.print("FasitoNVRam length: "); Serial.print(sizeof(FasitoNVRam)); Serial.println();
    dst->checksum = checkCRC16((uint8_t *)dst, sizeof(FasitoNVRam) - 2);
    //Serial.print(dst->checksum, HEX); Serial.println();
    eeprom_write_block(dst, 0, sizeof(FasitoNVRam));
}

#ifndef FASITO_EMU
static void readWord(uint8_t word, uint32_t *out)
{

    while(!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) {
        ;
    }
    /* See "Kinetis Peripheral Module Quick Reference" page 85 and
     * "K20 Sub-Family Reference Manual" page 548.
     */

    FTFL_FCCOB0 = 0x41;             // Selects the READONCE command
    FTFL_FCCOB1 = word;             // read the given word of read once area
                                    // -- this is one half of the mac addr.
    FTFL_FSTAT = FTFL_FSTAT_CCIF;   // Launch command sequence
    while(!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) {
        ;
    }
                                    // Wait for command completion

    uint8_t *out8 = (uint8_t *)out;
    *out8++ = FTFL_FCCOB5;       // collect only the top three bytes,
    *out8++ = FTFL_FCCOB6;       // in the right orientation (big endian).
    *out8++ = FTFL_FCCOB7;       // Skip FTFL_FCCOB4 as it's always 0.
    *out8++ = FTFL_FCCOB8;
}

void readMAC(uint8_t *mac)
{
    uint32_t mac32;
    uint8_t *m = (uint8_t *) &mac32;

    __disable_irq();
    readWord(0x0e, &mac32);
    mac[0] = m[0]; mac[1] = m[1]; mac[2] = m[2];
    readWord(0x0f, &mac32);
    mac[3] = m[0]; mac[4] = m[1]; mac[5] = m[2];
    __enable_irq();
}
#endif
