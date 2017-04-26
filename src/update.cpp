/*
 * Copyright (c) 2017 by Thomas König <tom@fair-coin.org>
 *
 * update.cpp is part of Fasito, the FairCoin signature token.
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
#include "utils.h"

#define WAIT_FOR_COMMAND_COMPLETION(a) while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ;
#define EXEC_CMD(a) \
    __disable_irq(); \
    executeInRAM(&FTFL_FSTAT); \
    __enable_irq()

#define CMD_PGM4     0x06
#define CMD_ERSSCR   0x09

#define SEAL_BITS    0b01100100
#define UNSEAL_BITS  0b11011110

/* Execute the flash command in RAM */
FASTRUN static void executeInRAM(volatile uint8_t *pFstat)
{
    *pFstat = FTFL_FSTAT_CCIF;
    while (!(*pFstat & FTFL_FSTAT_CCIF)) ; // wait for the command to complete
}

static bool eraseProtectionBits()
{
    Serial.println("Erasing first sector...");
    Serial.flush(); delay(200);

    // make sure no other operation is taking place
    WAIT_FOR_COMMAND_COMPLETION();

    // clear previous error flags
    FTFL_FSTAT = FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;

    // command: erase flash sector
    FTFL_FCCOB0 = CMD_ERSSCR;
    FTFL_FCCOB1 = 0;
    FTFL_FCCOB2 = 0;
    FTFL_FCCOB3 = 0;

    EXEC_CMD();

    WAIT_FOR_COMMAND_COMPLETION();

    return !(FTFL_FSTAT & (FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL | FTFL_FSTAT_MGSTAT0));
}

static bool programFlash(const uint8_t value)
{
    Serial.print("Programming byte: "); Serial.println(value, HEX);
    Serial.flush(); delay(200);

    // make sure no other operation is taking place
    WAIT_FOR_COMMAND_COMPLETION();

    // clear previous error flags
    FTFL_FSTAT = FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;

    // command: program long word
    FTFL_FCCOB0 = CMD_PGM4;

    // 24bit address to program (0x00040c)
    FTFL_FCCOB1 = 0x00;
    FTFL_FCCOB2 = 0x04;
    FTFL_FCCOB3 = 0x0c;

    // bytes to program
    FTFL_FCCOB4 = 0xFF;
    FTFL_FCCOB5 = 0xFF;
    FTFL_FCCOB6 = 0xFF;
    FTFL_FCCOB7 = value;

    EXEC_CMD();

    WAIT_FOR_COMMAND_COMPLETION();

    Serial.print("FTFL_FSTAT_ACCERR  : "); Serial.println((FTFL_FSTAT & FTFL_FSTAT_ACCERR  ? "ERROR" : "OK"));
    Serial.print("FTFL_FSTAT_FPVIOL  : "); Serial.println((FTFL_FSTAT & FTFL_FSTAT_FPVIOL  ? "ERROR" : "OK"));
    Serial.print("FTFL_FSTAT_MGSTAT0 : "); Serial.println((FTFL_FSTAT & FTFL_FSTAT_MGSTAT0 ? "ERROR" : "OK"));

    return !(FTFL_FSTAT & (FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL | FTFL_FSTAT_MGSTAT0));
}

bool sealDevice()
{
    if (!programFlash(SEAL_BITS))
        return false;

    return true;
}

bool unsealDevice()
{
    if (!eraseProtectionBits())
        return false;

    if (!programFlash(UNSEAL_BITS))
        return false;

    return true;
}

#if 0
extern uint16_t inputBufferIndex;

static bool updateInProgress = false;
static uint32_t flashOffset = 0;

#define MAX_LINE_LEN 0x10

typedef struct hexLine {
    uint8_t len;
    uint16_t addr;
    uint8_t type;
    uint8_t lineData[MAX_LINE_LEN];
} hexLine;

int decodeLine(hexLine *l, const char *buf)
{
    uint8_t cnt, checksum = 0, sum = 0;

    if (*buf != ':')
        return 0;

    memset(l, 0, sizeof(hexLine));

    parseHex(&l->len, &buf[1], 2);

    if (l->len > MAX_LINE_LEN)
        return 0;

    checksum += l->len;

    parseHex((uint8_t *)&l->addr, &buf[3], 4);
    checksum += (uint8_t) l->addr;
    checksum += (uint8_t) (l->addr >> 8);

    parseHex(&l->type, &buf[7], 2);
    checksum += l->type;

    for (cnt = 0 ; cnt < l->len ; cnt++) {
        parseHex(&l->lineData[cnt], &buf[9 + (cnt * 2)], 2);
        checksum += l->lineData[cnt];
    }

    parseHex(&sum, &buf[9 + (l->len * 2)], 2);

    if ((checksum += sum) & 0xff)
        return -1;

    return 1;
}

static bool handleLine()
{
    int res;
    hexLine l;

    res = decodeLine(&l, inputBuffer);
    Serial.print("Line type: "); Serial.println(l.type);

    if (!res)
        return fasitoErrorStr("invalid line");

    if (res < 0)
        return fasitoErrorStr("checksum error");

    if (l.type > 2) {
        Serial.println("unsupported line type: "); Serial.print(l.type, DEC); Serial.println();
        return false;
    }

    if (l.type == 1) {
        updateInProgress = false;
        return true;
    }

    if (l.type == 2) {
        uint16_t newOffset = 0;
        newOffset  = l.lineData[0] << 8;
        newOffset += l.lineData[1];

        flashOffset = newOffset << 4;
        Serial.print(flashOffset, HEX); Serial.println();
        return true;
    }

#if 0
    if (first) {
        page = l.addr - (l.addr % SPM_PAGESIZE);
        first = 0;
    }

    for (cnt = 0 ; cnt < l.len ; cnt++) {
        if ((l.addr + cnt) - ((l.addr + cnt) % SPM_PAGESIZE) != page) {
            program_page(page, flashBuffer);
            serialSendStr(unit, ".programmed page\r\n");
            memset(flashBuffer, 0xFF, SPM_PAGESIZE);
            page = (l.addr + cnt) - ((l.addr + cnt) % SPM_PAGESIZE);
        }

        flashBuffer[(l.addr + cnt) % SPM_PAGESIZE] = l.lineData[cnt];
    }

    if (l.type == 1) {
        program_page(page, flashBuffer);
        serialSendStr(unit, ".programmed last page\r\n");
        break;
    }
#endif
    return true;
}

bool updateFirmware()
{
    int c, lineCount = 0;
    bool ledStatus = false;
    updateInProgress = true;
    *inputBuffer = 0;
    inputBufferIndex = 0;
    flashOffset = 0;

    while(updateInProgress) {
        if (Serial.available() > 0) {
            c = Serial.read();
            if (c == '\n')
                continue;

            if (c == '\r') {
                inputBuffer[inputBufferIndex] = 0; // terminate string
                //Serial.print("["); Serial.print(inputBuffer); Serial.println("]");

                if (!((lineCount++) % 200)) {
                    ledStatus = !ledStatus;
                    digitalWrite(LED, ledStatus);
                }

                if (inputBufferIndex > 0) {
                    if (!handleLine()) {
                        return false;
                    } else {
                        Serial.println("OK");
                    }
                }

                *inputBuffer = 0;
                inputBufferIndex = 0;
                Serial.flush();

                continue;
            }

            if (inputBufferIndex >= INPUT_BUFFER_SIZE)
                inputBufferIndex = 0;

            inputBuffer[inputBufferIndex++] = c;
        } else {
            WFI;
        }
    }
    Serial.println("DONE");
    return true;
}
#else
bool updateFirmware()
{
    return false;
}
#endif
