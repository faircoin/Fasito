/*
 * Copyright (c) 2017-2022 by Thomas König <tom@faircoin.world>
 *
 * fasito_error.cpp is part of Fasito, the FairCoin signature token.
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

static const char __err01[] = "command not found";
static const char __err02[] = "not logged in";
static const char __err03[] = "pin too short";
static const char __err04[] = "pin too long";
static const char __err05[] = "no pin";
static const char __err06[] = "admin pub key #%d is invalid";
static const char __err07[] = "device verification private key is invalid";
static const char __err08[] = "PIN is invalid. %d tries left";
static const char __err09[] = "Token is LOCKED";
static const char __err10[] = "Invalid argument";
static const char __err11[] = "Token already initialised";
static const char __err12[] = "invalid admin signature";
static const char __err13[] = "index out of range";
static const char __err14[] = "invalid node ID";
static const char __err15[] = "Slot busy";
static const char __err16[] = "Could not create hash";
static const char __err17[] = "Could not create private key. Try an other hash.";
static const char __err18[] = "Could not create ECDSA signature.";
static const char __err19[] = "invalid hex parameter.";
static const char __err20[] = "slot not configured.";
static const char __err21[] = "duplicate node ID.";
static const char __err22[] = "duplicate private key.";
static const char __err23[] = "Could not create Schnorr signature.";
static const char __err24[] = "Could not program protection bits.";

const char *errorStrings[] = {
        __err01, __err02, __err03, __err04, __err05, __err06, __err07, __err08,
        __err09, __err10, __err11, __err12, __err13, __err14, __err15, __err16,
        __err17, __err18, __err19, __err20, __err21, __err22, __err23, __err24,
};
