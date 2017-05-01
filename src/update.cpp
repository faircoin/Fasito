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
#include "fasito_error.h"

#define WAIT_FOR_COMMAND_COMPLETION(a) while (!(FTFL_FSTAT & FTFL_FSTAT_CCIF)) ;

#define SECTOR_SIZE  0x0800

#define CMD_PGM4     0x06
#define CMD_ERSSCR   0x09

#define SEAL_BITS    0b01100100
#define UNSEAL_BITS  0b11011110

#define B(b) ((b) & 0xff)

/* Execute the flash command */
FASTRUN static void executeCMD(volatile uint8_t *pFstat)
{
    *pFstat = FTFL_FSTAT_CCIF;
    while (!(*pFstat & FTFL_FSTAT_CCIF)) ; // wait for the command to complete
}

FASTRUN static bool eraseSector0()
{
    // make sure no other operation is taking place
    WAIT_FOR_COMMAND_COMPLETION();

    // clear previous error flags
    FTFL_FSTAT = FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;

    // command: erase flash sector
    FTFL_FCCOB0 = CMD_ERSSCR;

    FTFL_FCCOB1 = 0;
    FTFL_FCCOB2 = 0;
    FTFL_FCCOB3 = 0;

    executeCMD(&FTFL_FSTAT);

    WAIT_FOR_COMMAND_COMPLETION();

    return !(FTFL_FSTAT & (FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL | FTFL_FSTAT_MGSTAT0));
}

FASTRUN static bool programLongword(const uint32_t address, const uint32_t value)
{
    // make sure no other operation is taking place
    WAIT_FOR_COMMAND_COMPLETION();

    // clear previous error flags
    FTFL_FSTAT = FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL;

    // command: program long word
    FTFL_FCCOB0 = CMD_PGM4;

    // 24bit address to program
    FTFL_FCCOB1 = B(address >> 16);
    FTFL_FCCOB2 = B(address >> 8);
    FTFL_FCCOB3 = B(address);

    // bytes to program
    FTFL_FCCOB4 = B(value >> 24);
    FTFL_FCCOB5 = B(value >> 16);
    FTFL_FCCOB6 = B(value >> 8);
    FTFL_FCCOB7 = B(value);

    executeCMD(&FTFL_FSTAT);

    WAIT_FOR_COMMAND_COMPLETION();

    return !(FTFL_FSTAT & (FTFL_FSTAT_ACCERR | FTFL_FSTAT_FPVIOL | FTFL_FSTAT_MGSTAT0));
}

FASTRUN static bool programFSEC(const uint8_t state)
{
    uint8_t sector[SECTOR_SIZE];
    const uint8_t *pFlash = 0x00;

    __disable_irq();

    for (int i = 0 ; i < SECTOR_SIZE ; i++) {
        sector[i] = pFlash[i];
    }

    sector[0x040c] = state;

    if (!eraseSector0()) {
        __enable_irq();
        return false;
    }

    const uint32_t *sectorB64 = (uint32_t *) sector;
    bool fOK = true;
    for (int i = 0 ; i < SECTOR_SIZE / 4; i++) {
        if (!programLongword(i * 4, sectorB64[i])) {
            fOK = false;
        }
    }

    __enable_irq();

    return fOK;
}

static bool doSeal(bool fSeal)
{
    Serial.print(fSeal ? "SEAL: " : "UNSEAL: ");
    Serial.print("starting to flash... "); Serial.flush();

    if (!programFSEC(fSeal ? SEAL_BITS : UNSEAL_BITS))
        return fasitoErrorStr("could not program status bits");

    Serial.println("done.");

    return true;
}

bool sealDevice()
{
    if (!doSeal(true))
        return fasitoError(E_COULD_NOT_PROGRAM_PROT_BITS);

    return true;
}

bool unsealDevice()
{
    if (!doSeal(false))
        return fasitoError(E_COULD_NOT_PROGRAM_PROT_BITS);

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
