/*
 * Copyright (c) 2017-2022 by Thomas König <tom@faircoin.world>
 *
 * fasito_error.h is part of Fasito, the FairCoin signature token.
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

#ifndef SRC_FASITO_ERROR_H_
#define SRC_FASITO_ERROR_H_

enum {
    E_COMMAND_NOT_FOUND,
    E_NOT_LOGGED_IN,
    E_PIN_TOO_SHORT,
    E_PIN_TOO_LONG,
    E_NO_PIN,
    E_INVALID_ADMIN_PUB_KEY,
    E_INVALID_DEVICE_VERFICATION_KEY,
    E_INVALID_PIN,
    E_TOKEN_LOCKED,
    E_INVALID_ARGUMENTS,
    E_TOKEN_ALREADY_INITIALIZED,
    E_INVALID_ADMIN_SIGNATURE,
    E_INDEX_OUT_OF_RANGE,
    E_INVALID_NODE_ID,
    E_SLOT_BUSY,
    E_COULD_NOT_CREATE_HASH,
    E_COULD_NOT_CREATE_PRIV_KEY,
    E_COULD_NOT_CREATE_ECDSA_SIG,
    E_INVALID_HEX_PARAM,
    E_SLOT_NOT_CONFIGURED,
    E_DUPLICATE_NODE_ID,
    E_DUPLICATE_PRIV_KEY,
    E_COULD_NOT_CREATE_SCHNORR_SIG,
    E_COULD_NOT_PROGRAM_PROT_BITS,
};

extern const char *errorStrings[];


#endif /* SRC_FASITO_ERROR_H_ */
