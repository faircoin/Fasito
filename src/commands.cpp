/*
 * Copyright (c) 2017 by Thomas König <tom@fair-coin.org>
 *
 * commands.cpp is part of Fasito, the FairCoin signature token.
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

#include <Arduino.h>
#include <string.h>
#include <secp256k1.h>
#include <secp256k1_schnorr.h>

#include "utils.h"
#include "commands.h"
#include "fasito_error.h"
#include "update.h"

#define ENABLE_INSCURE_FUNC 1

extern secp256k1_context *ctx;
extern uint8_t macAddress[];

bool serialEcho = false;
bool loggedIn   = false;

static const char* userPINStatus[]     = { "NOT_SET", "SET",    "LOCKED" };
static const char* privateKeyStatus[]  = { "EMPTY",   "SEEDED", "CONFIGURED" };
static const char* fasitoNVRamStatus[] = { "EMPTY",   "CONFIGURED" };

static uint8_t savedNonces[NUM_NONCE_POOL][32];
static uint8_t singleNonce[32];
static uint8_t nonceCursor;

enum DeviceSigantureType {
    INITKEY = 1,
    RESETKEY
};

void printHelp()
{
    printVersion();
    Serial.println("======================================");
    Serial.println("             Help screen:\r\n");
    Serial.println("HELP\r\n\t- prints this help screen");
    Serial.println("VERSION\r\n\t- clears the screen and prints the version number");
    Serial.println("ECHO\r\n\t- toggles local echo (default: off)");
    Serial.println("LOGIN <PIN>\r\n\t- logs into Fasito");
    Serial.println("LOGOUT\r\n\t- logs out from Fasito");
    Serial.println("CHGPIN <old PIN> <new PIN>\r\n\t- changes the user PIN");
    Serial.println("RSTPIN <new PIN> <optional: admin signature>\r\n\t- resets a locked user PIN by a device admin. Leaving out the admin sig print out the hasToSign");
    Serial.println("NONCE <key index: 0-" NUM_PRIVATE_KEYS_STR "> <sha256 hashToSign> <sha256 randomData>\r\n\t- creates a new nonce pair in the device pool and print out the public part");
    Serial.println("PARTSIG <key index: 0-" NUM_PRIVATE_KEYS_STR "> <nonce slot: 0-" NUM_NONCE_POOL_STR "> <sha256 hashToSign> <sum of all other public nonces>\r\n\t- creates partial signature");
    Serial.println("ECDSA <index: 0-" NUM_PRIVATE_KEYS_STR "> <sha256 hashToSign>\r\n\t- creates an ECDSA signature of the hashToSign");
    Serial.println("SCHNORR <index: 0-" NUM_PRIVATE_KEYS_STR "> <sha256 hashToSign>\r\n\t- creates an EC-Schnorr signature of the hashToSign");
    Serial.println("SEAL\r\n\t- seals the device by making the flash memory read-only");
    Serial.println("UNSEAL\r\n\t- un-seals the device by making the flash memory read-write again");
    Serial.println("INFO\r\n\t- prints out device information");
    Serial.println("INITKEY <index: 0-" NUM_PRIVATE_KEYS_STR "> <CVN ID:0x12345678> <sha256 hash>\r\n\t- initialises a pre-seeded key");
    Serial.println("KYPROOF <index: 0-" NUM_PRIVATE_KEYS_STR ">\r\n\t- creates a key proof signature for the key");
    Serial.println("INIT <PIN> <admin pub key #1> <admin pub key #2> <admin pub key #3> <device manager private key>\r\n\t- initialises the token");
    Serial.println("ERASE <optional: admin signature>\r\n\t- erases ALL configuration data from the token. It can than safely be initialised again for the next user");
    Serial.println("RSTKEY <index: 0-" NUM_PRIVATE_KEYS_STR "> <optional: admin signature>\r\n\t- cleans and pre-seeds a key");
    Serial.println("GETPBKY <key index: 0-" NUM_PRIVATE_KEYS_STR ">\r\n\t- prints the requested public key DER encoded. First line is uncompressed, second is the compressed key");
    Serial.println("SNONCE <key index: 0-" NUM_PRIVATE_KEYS_STR "> <sha256 hashToSign> <sha256 randomData>\r\n\t- creates a new nonce pair, stores the private part on the device and prints out the public part");
    Serial.println("CLRPOOL\r\n\t- clears the nonce pool");
#if ENABLE_INSCURE_FUNC
    Serial.println("DUMP\r\n\t- dumps the contents of the eeporm and internal data structurs");
    Serial.println("SETKEY <index: 0-" NUM_PRIVATE_KEYS_STR "> <CVN ID:0x12345678> <sha256 hash>\r\n\t- initialises a pre-seeded key");
#endif
}

void printVersion()
{
    Serial.println(CLS "\r\nFasito - FairCoin signature token v" __FASITO_VERSION__ );
}

void printStatus()
{
    uint8_t i;

    Serial.print("Fasito version    : v" __FASITO_VERSION__  "\r\n");
    Serial.print("Serial number     : "); printHex(macAddress, 6, true);
    Serial.print("Token status      : "); Serial.println(fasitoNVRamStatus[nvram.fasitoStatus]);
    const uint8_t nProtectionState = FTFL_FSEC;
    Serial.print("Protection status : 0x"); Serial.print(nProtectionState, BIN); Serial.print(" (0x"); Serial.print(nProtectionState, HEX); Serial.println(")");
    Serial.print("Config version    : "); Serial.println(nvram.version);
    Serial.print("Config checksum   : "); Serial.print(nvram.checksum, HEX); Serial.println();
    Serial.print("Nonce pool size   : "); Serial.print(NUM_NONCE_POOL); Serial.println("\r\n");
    Serial.print("User PIN          : "); Serial.print(userPINStatus[nvram.userPin.status]);
    Serial.print(" (tries left: ");     Serial.print(nvram.userPin.triesLeft); Serial.println(")\r\n");

    for (i = 0 ; i < NUM_PRIVATE_KEYS ; i++) {
        char line[80];
        sprintf(line, "Key #%d            : 0x%08x (%s%s)", i, (int)nvram.privateKey[i].nodeId, privateKeyStatus[nvram.privateKey[i].status],
                (i + 1 == NUM_PRIVATE_KEYS) ? ", protected" : "");
        Serial.println(line);
    }
    Serial.println("\r\n");
}

void initNonceStorage()
{
    memset(savedNonces, 0, sizeof(savedNonces));
    nonceCursor = 0;

    memset(singleNonce, 0, sizeof(singleNonce));
}

static int verifyAdminSignature(const char *sig, uint8_t *hash)
{
    /* EC-Schnorr signatures size is 64 bytes */
    if (!sig || !*sig || strlen(sig) != 64 * 2 ) {
        reverseBytes(hash, 32); // the 'signchaindata' RPC command expects a reversed hash
        Serial.print("UNLOCK HASH: "); printHex(hash, 32, true);
        return -1;
    }

    Serial.println("checking signature.");

    uint8_t schnorrSig[64];
    size_t sigLen = strlen(sig) / 2;
    if (!parseHex(schnorrSig, sig, sigLen))
        return 0;

    uint8_t i;
    bool verified = false;
    for (i = 0 ; i < NUM_ADMIN_KEYS ; i++) {
        if (secp256k1_schnorr_verify(ctx, schnorrSig, hash, &nvram.adminPublicKey[i])) {
            Serial.print("signed by admin #"); Serial.print(i + 1); Serial.println();
            verified = true;
            break;
        }
    }

    return verified ? 1 : 0;
}

static bool createHash(const char *dataSeed, const uint16_t seedLen, uint8_t *hash)
{
    uint16_t dataLen = MAX_PIN_LENGTH + 6 + seedLen;
    uint8_t data[dataLen];

    memcpy(data, nvram.userPin.pin, MAX_PIN_LENGTH);
    memcpy(&data[MAX_PIN_LENGTH], macAddress, 6);
    memcpy(&data[MAX_PIN_LENGTH + 6], dataSeed, seedLen);

    return secp256k1_hash_sha256(ctx, hash, data, dataLen) != 0;
}

static bool createProofSignature(uint8_t *sig, const uint8_t *data, const size_t nDataLen)
{
    uint8_t hashToSign[32];
    if (!secp256k1_hash_sha256(ctx, hashToSign, data, nDataLen))
        return fasitoError(E_COULD_NOT_CREATE_HASH);

    if (!secp256k1_schnorr_sign(ctx, sig, hashToSign, nvram.privateKey[NUM_PRIVATE_KEYS - 1].key, secp256k1_nonce_function_rfc6979, NULL))
        return fasitoError(E_COULD_NOT_CREATE_SCHNORR_SIG);

    return true;
}

static bool getIndexParameter(const char *indexChar, uint8_t &index, uint8_t maxEntries)
{
    /* check the index parameter */
    size_t len = strlen(indexChar);
    if (!*indexChar || *indexChar < '0' || *indexChar > '9' || len > 2)
        return fasitoError(E_INVALID_ARGUMENTS);

    index = *indexChar - '0';

    if (len == 2) {
        if (!indexChar[1] || indexChar[1] < '0' || indexChar[1] > '9')
            return fasitoError(E_INVALID_ARGUMENTS);
        index *= 10;
        index += indexChar[1] - '0';
    }

    if (index > maxEntries)
        return fasitoError(E_INDEX_OUT_OF_RANGE);

    return true;
}

static bool getHexParameter(const char *t, uint8_t *hash, size_t outLen)
{
    if (!t || strlen(t) != outLen * 2)
        return fasitoError(E_INVALID_HEX_PARAM);

    if (!parseHex(hash, t, outLen))
        return fasitoError(E_INVALID_HEX_PARAM);

    return true;
}

static bool equalHash(const uint8_t *h1, const uint8_t *h2)
{
    if (!h1 || !h2 || !*h1 || !*h2)
        return false;

    for (int i = 0 ; i < 32 ; i++) {
        if (h1[i] != h2[i])
            return false;
    }

    return true;
}

static bool checkDuplicateKeyInfo(uint32_t nNodeId, uint8_t *key)
{
    int i;

    PrivateKey *p = nvram.privateKey;

    for (i = 0 ; i < NUM_PRIVATE_KEYS ; i++) {
        if (p[i].status != PrivateKey::INITIALISED)
            continue;

        if (p[i].nodeId == nNodeId)
            return fasitoError(E_DUPLICATE_NODE_ID);

        if (equalHash(key, p[i].key))
            return fasitoError(E_DUPLICATE_PRIV_KEY);
    }

    return true;
}

/**
 * Initialise Fasito with the initial PIN, the 3 admin public keys, seeds the private keys and
 * sets the device manager private key
 * INIT <PIN> <admin pub key #1> <admin pub key #2> <admin pub key #3> <device manager private key>
 */
static bool cmdInitFasito(const char **tokens, const uint8_t nTokens)
{
    int i;

    if (nTokens != 5)
        return fasitoError(E_INVALID_ARGUMENTS);

    if (nvram.fasitoStatus != nvram.EMPTY)
        return fasitoError(E_TOKEN_ALREADY_INITIALIZED);

    memset(&nvram, 0, sizeof(FasitoNVRam));
    nvram.version = 1;

    UserPIN *up = &nvram.userPin;
    const char *pin = tokens[0];

    if (!pin) return fasitoError(E_NO_PIN);
    if (strlen(pin) < MIN_PIN_LENGTH) return fasitoError(E_PIN_TOO_SHORT);
    if (strlen(pin) > MAX_PIN_LENGTH) return fasitoError(E_PIN_TOO_LONG);

    strcpy(up->pin, pin);
    up->status = up->SET;
    up->triesLeft = MAX_PIN_TRIES;

    /* check the length of the public keys
     * 32 * 2 + 1 = 65
     * hex encoded 65 * 2 = 130
     */
    uint8_t derKey[65];
    for (i = 0; i < NUM_ADMIN_KEYS ; i++) {
        if (strlen(tokens[i + 1]) != 130 || !parseHex(derKey, tokens[i + 1], 65))
            return fasitoError(E_INVALID_ADMIN_PUB_KEY, i + 1);

        if (!secp256k1_ec_pubkey_parse(ctx, &nvram.adminPublicKey[i], derKey, 65)) {
            Serial.println("secp256k1_ec_pubkey_parse failed");
            return fasitoError(E_INVALID_ADMIN_SIGNATURE);
        }
    }

    /* serves as seed for the user private keys
     * and as device verification private key
     */
    uint8_t devicePrivateKey[32];
    if (strlen(tokens[4]) != 64 || !parseHex(devicePrivateKey, tokens[4], 32) || !secp256k1_ec_seckey_verify(ctx, devicePrivateKey))
        return fasitoError(E_INVALID_DEVICE_VERFICATION_KEY);

    PrivateKey *pk = nvram.privateKey;
    for (i = 0 ; i < NUM_PRIVATE_KEYS ; i++) {
        pk[i].status = PrivateKey::SEEDED;
        memcpy(pk[i].key, devicePrivateKey, 32);
    }

    pk[NUM_PRIVATE_KEYS - 1].status = PrivateKey::INITIALISED;
    nvram.fasitoStatus = nvram.CONFIGURED;

    writeEEPROM(&nvram);
    return true;
}

/**
 * LOGIN <PIN>
 */
static bool cmdCheckPin(const char **tokens, const uint8_t nTokens)
{
    UserPIN *userPin = &nvram.userPin;

    if (userPin->status == userPin->LOCKED)
        return fasitoError(E_TOKEN_LOCKED);

    if (userPin->status == userPin->NOT_SET)
        return fasitoError(E_NO_PIN);

    if (nTokens != 1 || !comparePins(userPin, tokens[0])) {
        fasitoError(E_INVALID_PIN, --userPin->triesLeft);
        loggedIn = false;
        if (!userPin->triesLeft) {
            userPin->status = userPin->LOCKED;
            Serial.println("The token is now locked.");
        }

        writeEEPROM(&nvram);
        return false;
    }

    if (userPin->triesLeft != MAX_PIN_TRIES) {
        userPin->triesLeft = MAX_PIN_TRIES;
        writeEEPROM(&nvram);
    }

    loggedIn = true;
    return true;
}

/**
 * LOGOUT
 */
static bool cmdLogout(const char **tokens, const uint8_t nTokens)
{
    Serial.println("You have been logged out.");
    loggedIn = false;
    return true;
}

/**
 * VERSION
 */
static bool cmdVersion(const char **tokens, const uint8_t nTokens)
{
    printVersion();
    return true;
}

/**
 * CHGPIN <old PIN> <new PIN>
 */
static bool cmdChangePin(const char **tokens, const uint8_t nTokens)
{
    if (nTokens != 2)
        return fasitoError(E_INVALID_ARGUMENTS);

    if (!cmdCheckPin(tokens, 1))
        return false;

    const char *pin = tokens[1];
    if (!pin) return fasitoError(E_NO_PIN);
    if (strlen(pin) < MIN_PIN_LENGTH) return fasitoError(E_PIN_TOO_SHORT);
    if (strlen(pin) > MAX_PIN_LENGTH) return fasitoError(E_PIN_TOO_LONG);

    UserPIN *up = &nvram.userPin;
    memset(up->pin, 0, MAX_PIN_LENGTH + 1);
    strcpy(up->pin, pin);

    writeEEPROM(&nvram);

    Serial.println("PIN successfully changed.");
    return true;
}

/**
 * RSTPIN <new pin> <admin signature>
 */
static bool cmdResetPin(const char **tokens, const uint8_t nTokens)
{
    char dataSeed[22 + MAX_PIN_LENGTH] = {"Fasito - PIN Recovery."};
    uint8_t hash[32];

    if (nTokens < 1 || nTokens > 2)
        return fasitoError(E_INVALID_ARGUMENTS);

    const char *pin = tokens[0];
    if (!pin)
        return fasitoError(E_NO_PIN);

    size_t pinLen = strlen(pin);
    if (!pinLen)
        return fasitoError(E_NO_PIN);
    if (pinLen < MIN_PIN_LENGTH)
        return fasitoError(E_PIN_TOO_SHORT);
    if (pinLen > MAX_PIN_LENGTH)
        return fasitoError(E_PIN_TOO_LONG);

    memset(&dataSeed[22], 0, MAX_PIN_LENGTH);
    memcpy(&dataSeed[22], pin, pinLen);

    if (!createHash(dataSeed, sizeof(dataSeed), hash))
        return fasitoError(E_COULD_NOT_CREATE_HASH);

    int8_t res = verifyAdminSignature(tokens[1], hash);
    if (!res)
        return fasitoError(E_INVALID_ADMIN_SIGNATURE);
    else if (res == -1)
        return true;

    if (nTokens != 2)
        return fasitoError(E_INVALID_ARGUMENTS);

    const char *t[] = { nvram.userPin.pin, pin };

    return cmdChangePin(t, 2);
}

/**
 * RSTKEY <index: 0-7> <sha256 hash>
 */
static bool cmdResetKey(const char **tokens, const uint8_t nTokens)
{
    char dataSeed[22 + 32] = {"Fasito - KEY Recovery."};
    uint8_t hash[32];

    if (nTokens < 1 || nTokens > 2)
        return fasitoError(E_INVALID_ARGUMENTS);

    uint8_t index = 0;
    if (!getIndexParameter(tokens[0], index, NUM_PRIVATE_KEYS - 2))
        return false;

    PrivateKey &p = nvram.privateKey[index];

    if (p.status != PrivateKey::INITIALISED)
        return fasitoError(E_SLOT_NOT_CONFIGURED);

    memcpy(&dataSeed[22], p.key, 32);

    if (!createHash(dataSeed, sizeof(dataSeed), hash))
        return fasitoError(E_COULD_NOT_CREATE_HASH);

    int8_t res = verifyAdminSignature(tokens[1], hash);
    if (!res)
        return fasitoError(E_INVALID_ADMIN_SIGNATURE);
    else if (res == -1) {
        uint8_t data[69] = { DeviceSigantureType::RESETKEY }; /* type:1 + CVNID:4 + pubkey:64 = 69 */
        if (!secp256k1_ec_pubkey_create(ctx, (secp256k1_pubkey *)&data[5], p.key))
            return fasitoErrorStr("could not create public key.");

        uint8_t *pNodeId = (uint8_t *)&p.nodeId;
        data[1] = pNodeId[3];
        data[2] = pNodeId[2];
        data[3] = pNodeId[1];
        data[4] = pNodeId[0];

        uint8_t sig[64];
        if (!createProofSignature(sig, data, sizeof(data))) {
            return false;
        }

        Serial.print("DEVICE SIGNATURE: "); printHex(sig, 64, true);
    }

    p.nodeId = 0;
    p.status = PrivateKey::SEEDED;
    memcpy(p.key, nvram.privateKey[NUM_PRIVATE_KEYS - 1].key, 32);
    writeEEPROM(&nvram);

    Serial.println("private key has been reset successfully.");
    return true;
}

/**
 * ERASE <optional: admin signature>
 */
static bool cmdEraseToken(const char **tokens, const uint8_t nTokens)
{
    const char dataSeed[] = {"Fasito - ERASE Token"};
    uint8_t hash[32];

    if (nTokens > 1)
        return fasitoError(E_INVALID_ARGUMENTS);

    if (!createHash(dataSeed, sizeof(dataSeed), hash))
        return fasitoError(E_COULD_NOT_CREATE_HASH);

    int8_t res = verifyAdminSignature(nTokens ? tokens[0] : (char *)0, hash);
    if (!res)
        return fasitoError(E_INVALID_ADMIN_SIGNATURE);
    else if (res == -1)
        return true;

    memset(&nvram, 0, sizeof(FasitoNVRam));
    nvram.version = 1;
    writeEEPROM(&nvram);
    loggedIn = false;

    Serial.println("All token data erased.");
    return true;
}

/**
 * ECHO
 */
static bool cmdEcho(const char **tokens, const uint8_t nTokens)
{
    serialEcho = !serialEcho;
    Serial.print("echo is ");
    Serial.println(serialEcho ? "ON" : "OFF");
    return true;
}

/**
 * HELP
 */
static bool cmdHelp(const char **tokens, const uint8_t nTokens)
{
    printHelp();
    return true;
}

/**
 * INFO
 */
static bool cmdInfo(const char **tokens, const uint8_t nTokens)
{
    printStatus();
    return true;
}

/**
 * INITKEY <index: 0-7> <CVN ID:0x12345678> <sha256 hash>
 */
static bool cmdInitKey(const char **tokens, const uint8_t nTokens)
{
    if (nTokens != 3)
        return fasitoError(E_INVALID_ARGUMENTS);

    uint8_t index = 0;
    if (!getIndexParameter(tokens[0], index, NUM_PRIVATE_KEYS - 2))
        return false;

    /* CVN node ID */
    if (!tokens[1] || strlen(tokens[1]) != 10)
        return fasitoError(E_INVALID_NODE_ID);

    uint8_t nodeIdBytes[4];
    if (!parseHex(nodeIdBytes, &tokens[1][2], 4))
        return fasitoError(E_INVALID_ARGUMENTS);

    /* check the private key hash */
    uint8_t newSeedKey[32];
    if (!getHexParameter(tokens[2], newSeedKey, 32))
        return false;

    PrivateKey *p = &nvram.privateKey[index];

    if (p->status != PrivateKey::SEEDED)
        return fasitoError(E_SLOT_BUSY);

    /* first check if the provided hash is a valid private key by itself */
    if (!secp256k1_ec_seckey_verify(ctx, newSeedKey))
        return fasitoError(E_COULD_NOT_CREATE_PRIV_KEY);

    /* concatenate the pre-seed and the new seed and hash them */
    uint8_t data[64], newKey[32];
    memcpy(data, p->key, 32);
    memcpy(&data[32], newSeedKey, 32);

    if (!secp256k1_hash_sha256(ctx, newKey, data, 64))
        return fasitoError(E_COULD_NOT_CREATE_HASH);

    /* verify that the result is a valid private key */
    if (!secp256k1_ec_seckey_verify(ctx, newKey))
        return fasitoError(E_COULD_NOT_CREATE_PRIV_KEY);

    reverseBytes(nodeIdBytes, 4);
    uint32_t *pNodeId = (uint32_t *)&nodeIdBytes;

    if (*pNodeId == 0)
        return fasitoError(E_INVALID_ARGUMENTS);

    if (!checkDuplicateKeyInfo(*pNodeId, newKey))
        return false;

    memcpy(p->key, newKey, 32);

    p->nodeId = *pNodeId;
    p->status = PrivateKey::INITIALISED;

    writeEEPROM(&nvram);

    Serial.println("key successfully initialised.");
    return true;
}

static bool doSign(const char **tokens, const uint8_t nTokens, bool schnorr)
{
    if (nTokens != 2)
        return fasitoError(E_INVALID_ARGUMENTS);

    uint8_t index = 0;
    if (!getIndexParameter(tokens[0], index, NUM_PRIVATE_KEYS - 2))
        return false;

    uint8_t hashToSign[32];
    if (!getHexParameter(tokens[1], hashToSign, 32))
        return false;

    PrivateKey *p = &nvram.privateKey[index];

    if (p->status != PrivateKey::INITIALISED)
        return fasitoError(E_SLOT_NOT_CONFIGURED);

    if (schnorr) {
        /* create a SCHNORR signature */
        uint8_t sig[64];

        if (!secp256k1_schnorr_sign(ctx, sig, hashToSign, p->key, secp256k1_nonce_function_rfc6979, NULL))
            return fasitoError(E_COULD_NOT_CREATE_SCHNORR_SIG);

        printHex(sig, 64, true);

        return true;
    } else {
        /* create an ECDSA signature */
        secp256k1_ecdsa_signature sig;
        if (!secp256k1_ecdsa_sign(ctx, &sig, hashToSign, p->key,NULL, NULL))
            return fasitoError(E_COULD_NOT_CREATE_ECDSA_SIG);

        size_t sigLen = 73;
        uint8_t der[sigLen];
        if (!secp256k1_ecdsa_signature_serialize_der(ctx, der, &sigLen, &sig)) {
            return fasitoErrorStr("secp256k1_ecdsa_signature_serialize_der failed");
        }

        printHex(der, sigLen, true);

        return true;
    }
}

/**
 * ECDSA <index: 0-7> <sha256 hash>
 */
static bool cmsEcdsaSign(const char **tokens, const uint8_t nTokens)
{
    return doSign(tokens, nTokens, false);
}

/**
 * SCHNORR <key index: 0-7> <sha256 hash>
 */
static bool cmdCreateSchnorrSignature(const char **tokens, const uint8_t nTokens)
{
    return doSign(tokens, nTokens, true);
}

static bool doCreateNoncePair(const char **tokens, const uint8_t nTokens, uint8_t *privateNonce, secp256k1_pubkey *publicNonce)
{
    if (nTokens != 3)
        return fasitoError(E_INVALID_ARGUMENTS);

    uint8_t index = 0;
    if (!getIndexParameter(tokens[0], index, NUM_PRIVATE_KEYS - 2))
        return false;

    uint8_t hashToSign[32];
    if (!getHexParameter(tokens[1], hashToSign, 32))
        return false;

    uint8_t randomData[32];
    if (!getHexParameter(tokens[2], randomData, 32))
        return false;

    PrivateKey *p = &nvram.privateKey[index];

    if (p->status != PrivateKey::INITIALISED)
        return fasitoError(E_SLOT_NOT_CONFIGURED);

    if (!secp256k1_schnorr_generate_nonce_pair(ctx, publicNonce, privateNonce, hashToSign, p->key, NULL, randomData))
        return fasitoErrorStr("could not generate nonce pair");

    return true;
}

/**
 * NONCE <key index: 0-7> <sha256 hashToSign> <sha256 randomData>
 */
static bool cmdCreateNonces(const char **tokens, const uint8_t nTokens)
{
    secp256k1_pubkey publicNonce;
    uint8_t privateNonce[32];

    if (!doCreateNoncePair(tokens, nTokens, privateNonce, &publicNonce))
        return false;

    memcpy(savedNonces[nonceCursor], privateNonce, 32);
    if (nonceCursor < 10)
        Serial.print("0");;
    Serial.print(nonceCursor); Serial.print(" "); printHex(publicNonce.data, 64, true);

    if (nonceCursor++ >= NUM_NONCE_POOL - 1)
        nonceCursor = 0;

    return true;
}

/**
 * create a single nonce whose private part is stored in singleNonce
 * SNONCE <key index: 0-7> <sha256 hashToSign> <sha256 randomData>
 */
static bool cmdCreateSingleNonce(const char **tokens, const uint8_t nTokens)
{
    secp256k1_pubkey publicNonce;

    if (!doCreateNoncePair(tokens, nTokens, singleNonce, &publicNonce))
        return false;

    printHex(publicNonce.data, 64, true);
    return true;
}

/**
 * PARTSIG <key index: 0-7> <nonce slot: 0-9> <sha256 hash> <sum of all other public nonces>
 */
static bool cmdCreatePartialSchnorrSignature(const char **tokens, const uint8_t nTokens)
{
    if (nTokens != 4)
        return fasitoError(E_INVALID_ARGUMENTS);

    uint8_t index = 0;
    if (!getIndexParameter(tokens[0], index, NUM_PRIVATE_KEYS - 2))
        return false;

    uint8_t nonceSlot = 0;
    if (!getIndexParameter(tokens[1], nonceSlot, NUM_NONCE_POOL - 1))
        return false;

    uint8_t hashToSign[32];
    if (!getHexParameter(tokens[2], hashToSign, 32))
        return false;

    PrivateKey *p = &nvram.privateKey[index];

    if (p->status != PrivateKey::INITIALISED)
        return fasitoError(E_SLOT_NOT_CONFIGURED);

    secp256k1_pubkey othersPublicNonces;
    if (!getHexParameter(tokens[3], othersPublicNonces.data, 64))
        return false;

    uint8_t partialSig[64];
    if (secp256k1_schnorr_partial_sign(ctx, partialSig, hashToSign, p->key, &othersPublicNonces, savedNonces[nonceSlot]) < 1)
        return fasitoErrorStr("secp256k1_schnorr_partial_sign failed");

    printHex(partialSig, 64, true);

    return true;
}

/**
 * GETPBKY <key index: 0-7>
 */
static bool cmdGetPublicKey(const char **tokens, const uint8_t nTokens)
{
    if (nTokens != 1)
        return fasitoError(E_INVALID_ARGUMENTS);

    uint8_t index = 0;
    if (!getIndexParameter(tokens[0], index, NUM_PRIVATE_KEYS - 2))
        return false;

    if (nvram.privateKey[index].status != PrivateKey::INITIALISED)
        return fasitoErrorStr("the requested key is not initialised.");

    secp256k1_pubkey pub;
    if (!secp256k1_ec_pubkey_create(ctx, &pub, nvram.privateKey[index].key))
        return fasitoErrorStr("could not create public key.");

    size_t keyLen = 74;
    uint8_t pubKey[keyLen];
    if (!secp256k1_ec_pubkey_serialize(ctx, pubKey, &keyLen, &pub, SECP256K1_EC_UNCOMPRESSED))
        return fasitoErrorStr("could not serialise public key.");

    printHex(pubKey, keyLen, true);

    keyLen = 74;
    if (!secp256k1_ec_pubkey_serialize(ctx, pubKey, &keyLen, &pub, SECP256K1_EC_COMPRESSED))
        return fasitoErrorStr("could not serialise compressed public key.");

    printHex(pubKey, keyLen, true);

    return true;
}

/**
 * KYPROOF <key index: 0-6>
 */
static bool cmdCreateKeyProof(const char **tokens, const uint8_t nTokens)
{
    if (nTokens != 1)
        return fasitoError(E_INVALID_ARGUMENTS);

    uint8_t index = 0;
    if (!getIndexParameter(tokens[0], index, NUM_PRIVATE_KEYS - 2))
        return false;

    PrivateKey &p = nvram.privateKey[index];

    if (p.status != PrivateKey::INITIALISED)
        return fasitoError(E_SLOT_NOT_CONFIGURED);

    uint8_t data[69] = { DeviceSigantureType::INITKEY }; /* type:1 + CVNID:4 + pubkey:64 = 69 */
    if (!secp256k1_ec_pubkey_create(ctx, (secp256k1_pubkey *)&data[5], p.key))
        return fasitoErrorStr("could not create public key.");

    uint8_t *pNodeId = (uint8_t *)&p.nodeId;
    data[1] = pNodeId[3];
    data[2] = pNodeId[2];
    data[3] = pNodeId[1];
    data[4] = pNodeId[0];

    uint8_t sig[64];
    if (!createProofSignature(sig, data, sizeof(data))) {
        return false;
    }

    size_t keyLen = 74;
    uint8_t derKey[keyLen];
    if (!secp256k1_ec_pubkey_serialize(ctx, derKey, &keyLen, (secp256k1_pubkey *)&data[5], SECP256K1_EC_UNCOMPRESSED))
        return fasitoErrorStr("could not serialise public key.");

    Serial.print("SIG DATA   : "); printHex(data, sizeof(data), true);
    Serial.print("RAW PUBKEY : "); printHex(&data[5], 64, true);
    Serial.print("DER PUBKEY : "); printHex(derKey, keyLen, true);
    Serial.print("SIGNATURE  : "); printHex(sig, 64, true);

    return true;
}

/**
 * UPDATE <schnorr signature>
 */
static bool cmdUpdateFirmware(const char **tokens, const uint8_t nTokens)
{
    uint8_t sig[64];
    if (!getHexParameter(tokens[0], sig, 64))
        return false;

    return updateFirmware();
}

/**
 * CLRPOOL
 */
static bool cmdClearNoncePool(const char **tokens, const uint8_t nTokens)
{
    memset(savedNonces, 0, sizeof(savedNonces));
    nonceCursor = 0;

    return true;
}

/**
 * SEAL
 */
static bool cmdSealFasito(const char **tokens, const uint8_t nTokens)
{
    if (nTokens)
        return fasitoError(E_INVALID_ARGUMENTS);

    Serial.println("About to seal this Fasito.");
    Serial.flush();

    return sealDevice();
}

/**
 * UNSEAL
 */
static bool cmdUnsealFasito(const char **tokens, const uint8_t nTokens)
{
    if (nTokens)
        return fasitoError(E_INVALID_ARGUMENTS);

    Serial.println("Unsealing this Fasito.");
    Serial.flush();

    return unsealDevice();
}

#if ENABLE_INSCURE_FUNC
static bool cmdDUMP(const char **tokens, const uint8_t nTokens)
{
    uint8_t chunck;
    int i;

    for (chunck = 0 ; chunck < sizeof(FasitoNVRam) / 64 ; chunck++) {
        printHex((uint8_t *) &nvram + chunck * 64, 64, true);
    }
    printHex((uint8_t *) &nvram + chunck * 64 , sizeof(FasitoNVRam) % 64, true);

    Serial.println("\r\nPRIVATE KEYS:");
    for (i = 0 ; i < NUM_PRIVATE_KEYS ; i++)
        pHEX("PRIV", nvram.privateKey[i].key, 32);

    Serial.println("\r\nNONCES POOL:");
    for (i = 0 ; i < NUM_NONCE_POOL ; i++) {
        Serial.print(i == nonceCursor ? "*" : " ");
        if (i < 10)
            Serial.print("0");
        Serial.print(i); Serial.print(":"); printHex(savedNonces[i], 32, true);
    }

    Serial.println("\r\nSINGLE NONCE:");
    printHex(singleNonce, 32, true);
    Serial.println();

    Serial.println("\r\nSECTOR #0:");
    for (chunck = 0 ; chunck < 2048 / 64 ; chunck++) {
        printHex((uint8_t *) (chunck * 64), 64, true);
    }
    Serial.println();

    return true;
}

/**
 * SETKEY <index: 0-7> <CVN ID:0x12345678> <sha256 hash>
 */
static bool cmdSetKey(const char **tokens, const uint8_t nTokens)
{
    if (nTokens != 3)
        return fasitoError(E_INVALID_ARGUMENTS);

    uint8_t index = 0;
    if (!getIndexParameter(tokens[0], index, NUM_PRIVATE_KEYS - 2))
        return false;

    /* CVN node ID */
    if (!tokens[1] || strlen(tokens[1]) != 10)
        return fasitoError(E_INVALID_NODE_ID);

    uint8_t nodeIdBytes[4];
    if (!parseHex(nodeIdBytes, &tokens[1][2], 4))
        return fasitoError(E_INVALID_ARGUMENTS);

    /* check the private key */
    uint8_t newKey[32];
    if (!getHexParameter(tokens[2], newKey, 32))
        return false;

    PrivateKey *p = &nvram.privateKey[index];

    if (p->status != PrivateKey::SEEDED)
        return fasitoError(E_SLOT_BUSY);

    /* first check if the provided hash is a valid private key by itself */
    if (!secp256k1_ec_seckey_verify(ctx, newKey))
        return fasitoError(E_COULD_NOT_CREATE_PRIV_KEY);

    reverseBytes(nodeIdBytes, 4);
    uint32_t *pNodeId = (uint32_t *)&nodeIdBytes;

    if (*pNodeId == 0)
        return fasitoError(E_INVALID_ARGUMENTS);

    if (!checkDuplicateKeyInfo(*pNodeId, newKey))
        return false;

    memcpy(p->key, newKey, 32);

    p->nodeId = *pNodeId;
    p->status = PrivateKey::INITIALISED;

    writeEEPROM(&nvram);

    Serial.println("key successfully initialised.");
    return true;
}
#endif

/* commands may not be longer than 7 (NULL terminator) characters */
const Command commands[] = {
        {"HELP",    cmdHelp,                             4, false},
        {"VERSION", cmdVersion,                          7, false},
        {"ECHO",    cmdEcho,                             4, false},
        {"LOGIN",   cmdCheckPin,                         5, false},
        {"LOGOUT",  cmdLogout,                           6, true },
        {"CHGPIN",  cmdChangePin,                        6, true },
        {"RSTPIN",  cmdResetPin,                         6, false},
        {"NONCE",   cmdCreateNonces,                     5, true },
        {"PARTSIG", cmdCreatePartialSchnorrSignature,    7, true },
        {"ECDSA",   cmsEcdsaSign,                        5, true },
        {"SCHNORR", cmdCreateSchnorrSignature,           7, true },
        {"SEAL",    cmdSealFasito,                       4, true },
        {"UNSEAL",  cmdUnsealFasito,                     6, true },
        {"INFO",    cmdInfo,                             4, false },
        {"INITKEY", cmdInitKey,                          7, true },
        {"INIT",    cmdInitFasito,                       4, false},
        {"ERASE",   cmdEraseToken,                       5, true },
        {"RSTKEY",  cmdResetKey,                         6, true },
        {"GETPBKY", cmdGetPublicKey,                     7, true },
        {"UPDATE",  cmdUpdateFirmware,                   6, true },
        {"SNONCE",  cmdCreateSingleNonce,                6, true },
        {"CLRPOOL", cmdClearNoncePool,                   7, true },
        {"KYPROOF", cmdCreateKeyProof,                   7, true },
#if ENABLE_INSCURE_FUNC
        {"DUMP",    cmdDUMP,                             4, false },
        {"SETKEY",  cmdSetKey,                           6, true },
#endif
};

const Command *getCommand(char *buf)
{
    if (!buf || !strlen(buf))
        return NULL;

    if (*buf == '?' || *buf == 'h')
        return &commands[0];

    size_t i, bufLen = strlen(buf);

    for (i = 0 ; i < sizeof(commands) / sizeof(Command) ; i++) {
        if (*buf != *commands[i].command)
            continue;

        if (commands[i].len <= bufLen && !strncmp(commands[i].command, buf, commands[i].len)) {
            return &commands[i];
        }
    }

    return NULL;
}

