#!/bin/bash

openssl req -new -x509 -nodes -newkey ec:<(openssl ecparam -name secp256k1) -keyout tmpKey.key  -days 3650 -subj "/O=key" -out tmpKey.crt 2>/dev/null
firstPrivKey=$(openssl ec -in tmpKey.key -outform DER 2>/dev/null | xxd -p -c 180 | cut -c 15-78)
firstAdmKey=$(openssl ec -in tmpKey.key -pubout -outform DER 2>/dev/null | xxd -p -c 180 | cut -c 47-)

openssl req -new -x509 -nodes -newkey ec:<(openssl ecparam -name secp256k1) -keyout tmpKey.key  -days 3650 -subj "/O=key" -out tmpKey.crt 2>/dev/null
secondPrivKey=$(openssl ec -in tmpKey.key -outform DER 2>/dev/null | xxd -p -c 180 | cut -c 15-78)
secondAdmKey=$(openssl ec -in tmpKey.key -pubout -outform DER 2>/dev/null | xxd -p -c 180 | cut -c 47-)

openssl req -new -x509 -nodes -newkey ec:<(openssl ecparam -name secp256k1) -keyout tmpKey.key  -days 3650 -subj "/O=key" -out tmpKey.crt 2>/dev/null
thirdPrivKey=$(openssl ec -in tmpKey.key -outform DER 2>/dev/null | xxd -p -c 180 | cut -c 15-78)
thirdAdmKey=$(openssl ec -in tmpKey.key -pubout -outform DER 2>/dev/null | xxd -p -c 180 | cut -c 47-)

rm tmpKey.key tmpKey.crt

samplePIN=$(shuf -i100000-999999 -n1)

echo "A possible PIN: $samplePIN"
echo "First device manager private key:"
echo " $firstPrivKey"
echo "First admin public key:"
echo " $firstAdmKey"
echo "Second device manager private key:"
echo " $secondPrivKey"
echo "Second admin public key:"
echo " $secondAdmKey"
echo "Third device manager private key:"
echo " $thirdPrivKey"
echo "Third admin public key:"
echo " $thirdAdmKey"
echo ""
echo "A possible Fasito INIT command:"
echo "INIT $samplePIN $firstAdmKey $secondAdmKey $thirdAdmKey $thirdPrivKey"
