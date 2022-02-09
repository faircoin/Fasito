/*
 * Copyright (c) 2017-2022 by Thomas König <tom@faircoin.world>
 *
 * Serial.cpp is part of Fasito, the FairCoin signature token.
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

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "Serial.h"

size_t SerialEMU::write(const uint8_t *buffer, size_t size)
{
    size_t count = 0;
    while (size--) {
        ++count;
        putchar(*buffer++);
    }
    return count;
}

size_t SerialEMU::write(uint8_t n)
{
    putchar(n);
    return 1;
}

size_t SerialEMU::print(long n)
{
    uint8_t sign=0;

    if (n < 0) {
        sign = '-';
        n = -n;
    }
    return printNumber(n, 10, sign);
}


size_t SerialEMU::println(void)
{
    uint8_t buf[2]={'\r', '\n'};
    return write(buf, 2);
}

size_t SerialEMU::printNumber(unsigned long n, uint8_t base, uint8_t sign)
{
    uint8_t buf[34];
    uint8_t digit, i;

    // TODO: make these checks as inline, since base is
    // almost always a constant.  base = 0 (BYTE) should
    // inline as a call directly to write()
    if (base == 0) {
        return write((uint8_t)n);
    } else if (base == 1) {
        base = 10;
    }


    if (n == 0) {
        buf[sizeof(buf) - 1] = '0';
        i = sizeof(buf) - 1;
    } else {
        i = sizeof(buf) - 1;
        while (1) {
            digit = n % base;
            buf[i] = ((digit < 10) ? '0' + digit : 'A' + digit - 10);
            n /= base;
            if (n == 0) break;
            i--;
        }
    }
    if (sign) {
        i--;
        buf[i] = '-';
    }
    return write(buf + i, sizeof(buf) - i);
}

SerialEMU Serial;

