/*
 * Copyright (c) 2017 by Thomas König <tom@fair-coin.org>
 *
 * main.cpp is part of Fasito, the FairCoin signature token.
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
#include <secp256k1.h>
#include <secp256k1_schnorr.h>

#include "fasito.h"
#include "commands.h"
#include "fasito_error.h"
#include "utils.h"

secp256k1_context *ctx = NULL;

/* serial communication */
char inputBuffer[INPUT_BUFFER_SIZE];
uint16_t inputBufferIndex = 0;

uint8_t macAddress[6];

bool handleCommand();
extern const Command *getCommand(char *buf);

static void custom_illegal_callback_fn(const char* str, void* data) {
    (void)data;
    Serial.print("[libsecp256k1] illegal argument: ");
    Serial.println(str);
    delay(1000);
}

static void custom_error_callback_fn(const char* str, void* data) {
    (void)data;
    Serial.print("[libsecp256k1] internal consistency check failed: ");
    Serial.println(str);
    delay(1000);
}

void yield(void)
{
    return;
}

void setup()
{
    pinMode(LED, OUTPUT);
    Serial.begin(115200);
    digitalWrite(LED, HIGH);
    initNonceStorage();

    delay(2000);
    readMAC(macAddress);
    Serial.println(CLS "\r\n");
    printVersion();
    *inputBuffer = 0;
    inputBufferIndex = 0;

    ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
    secp256k1_context_set_error_callback(ctx, custom_error_callback_fn, NULL);
    secp256k1_context_set_illegal_callback(ctx, custom_illegal_callback_fn, NULL);

    if (!readEEPROM(&nvram)) {
        Serial.println("Error NVRam checksum error");
        nvram.fasitoStatus = nvram.EMPTY;
    }

    if (nvram.fasitoStatus != nvram.CONFIGURED) {
        Serial.println("Fasito not configured. Clearing NVRam.");
        memset(&nvram, 0, sizeof(FasitoNVRam));
        nvram.version = CONFIG_VERSION;
        writeEEPROM(&nvram);
    }

    Serial.println("\r\nStatus overview:");
    printStatus();

    digitalWrite(LED, LOW);
}

void loop()
{
    int c;

    if (Serial.available() > 0) {
        c = Serial.read();

        if (serialEcho) {
            if (c == '\r') {
                Serial.println();
            } else {
                char echo[2] = { 0, 0 };
                echo[0] = c;
                Serial.print(echo);
            }
        }

        if (c == '\r') {
            digitalWrite(LED, HIGH);

            inputBuffer[inputBufferIndex] = 0; // terminate string

            if (inputBufferIndex > 0) {
                if (!handleCommand()) {
                    Serial.print("ERROR ");
                    Serial.println(inputBuffer); // inputBuffer contains the error message
                } else {
                    Serial.println("OK");
                }
            }

            *inputBuffer = 0;
            inputBufferIndex = 0;
            Serial.flush();
            digitalWrite(LED, LOW);

            return;
        }

        if (inputBufferIndex >= (INPUT_BUFFER_SIZE - 1))
            inputBufferIndex = 0;

        inputBuffer[inputBufferIndex++] = c;
    } else {
        WFI;
    }
}

bool handleCommand()
{
    uint8_t nTokens = 0;
    const Command *c = getCommand(inputBuffer);

    if (c == NULL || !c->handler)
        return fasitoError(E_COMMAND_NOT_FOUND);

    if (c->requireLogin && !loggedIn)
        return fasitoError(E_NOT_LOGGED_IN);

    return c->handler(tokenise(&inputBuffer[c->len], &nTokens), nTokens);
}
