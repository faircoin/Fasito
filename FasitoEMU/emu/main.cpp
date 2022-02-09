/*
 * Copyright (c) 2017-2022 by Thomas König <tom@faircoin.world>
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

#include <iostream>
#include <stdint.h>

#include "Serial.h"
#include "Arduino.h"

#include "commands.h"
#include "fasito.h"
#include "utils.h"
#include "fasito_error.h"

extern void setup();
extern void loop();
extern bool handleCommand();

int main(int argc, char **argv) {
    setup();

    if (argc == 2) {
        while(true)
            loop();
    } else {
        std::string line;

        while (std::getline(std::cin, line)) {
            if (line == "QUIT" || line == "quit")
                break;

            strcpy(inputBuffer, line.c_str());

            if (!handleCommand()) {
                Serial.print("ERROR ");
                Serial.println(inputBuffer); // inputBuffer contains the error message
            } else {
                Serial.println("OK");
            }
        }
    }

    return 0;
}

static const uint8_t staticMAC[] = { 0x10, 0x11, 0x12, 0x13, 0x14, 0x15 };

void readMAC(uint8_t *mac)
{
    memcpy(mac, staticMAC, 6);
}
