/*
 * Copyright (c) 2017-2022 by Thomas König <tom@faircoin.world>
 *
 * Arduino.cpp is part of Fasito, the FairCoin signature token.
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
#include "stdio.h"
#include <unistd.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstdint>

void eeprom_read_block(void *buf, const void *addr, uint32_t len)
{
    std::ifstream eepromFile ("eeprom.bin", std::ios::in | std::ios::binary);

    eepromFile.read((char*) buf, len);
    eepromFile.close();
}

void eeprom_write_block(const void *buf, void *addr, uint32_t len)
{
    std::ofstream eepromFile ("eeprom.bin", std::ios::out | std::ios::binary);

    eepromFile.write((char*) buf, len);
    eepromFile.close();
}

void delay(long n)
{
    // sleep(n / 1000);
}

void pinMode(uint8_t pin, uint8_t mode)
{
    // stub
}

void digitalWrite(uint8_t pin, uint8_t val)
{
    // stub
}