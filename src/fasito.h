/*
 * Copyright (c) 2017-2019 by Thomas König <tom@faircoin.world>
 *
 * fasito.h is part of Fasito, the FairCoin signature token.
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

#ifndef SRC_FASITO_H_
#define SRC_FASITO_H_

#include "version.h"
#include <secp256k1.h>

/* serial receive buffer size */
#define INPUT_BUFFER_SIZE 2048
extern char inputBuffer[];
extern bool loggedIn;

/* maximum number of arguments to commands */
#define MAX_TOKEN            16

#define MAX_PIN_TRIES        3
#define MAX_PIN_LENGTH       9
#define MIN_PIN_LENGTH       6

#define NUM_PRIVATE_KEYS     8
#define NUM_PRIVATE_KEYS_STR "6"
#define NUM_ADMIN_KEYS       3
#define NUM_ADMIN_KEYS_STR   "3"

#define NUM_NONCE_POOL       25
#define NUM_NONCE_POOL_STR   "25"

typedef struct UserPIN {
    /* UserPIN status */
    enum {
        NOT_SET,
        SET,
        LOCKED,
    };

    uint8_t status;
    uint8_t triesLeft;
    char pin[MAX_PIN_LENGTH + 1];
} UserPIN;


typedef struct PrivateKey {
    /* private key status */
    enum {
        EMPTY,
        SEEDED,
        INITIALISED
    };

    uint32_t nodeId;
    uint8_t  key[32];
    uint8_t  status;
} PrivateKey;


typedef struct FasitoNVRam {
    /* Fasito status */
    enum {
        EMPTY,
        CONFIGURED
    };

    uint8_t    version;
    uint8_t    fasitoStatus;

    UserPIN    userPin;
    PrivateKey privateKey[NUM_PRIVATE_KEYS];
    secp256k1_pubkey adminPublicKey[NUM_ADMIN_KEYS];

    /* checksum needs to remain the last field */
    uint16_t   resetCount;
    uint16_t   checksum;
} FasitoNVRam;


#endif /* SRC_FASITO_H_ */
