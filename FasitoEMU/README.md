# Fasito emulator
(c) 2017-2022 by Thomas König <tom@faircoin.world>

This program was compiled form the Fasito sources available here:  
`https://github.com/faircoin/Fasito.git`

If started without any argument it's in 'command line' mode and will
accept your commands on stdin. Type `HELP` (uppercase) to see
a list of all commands.

To create a virtual serial device such as a real Fasito would
issue the following command:  
```
socat -d pty,raw,echo=0,link=dev/ttyACM1 EXEC:"./FasitoEMU -c"
```

To test your virtual serial device use minicom like this:  
`minicom -D dev/ttyACM1`

To run a CVN with a correctly configured virtual Fasito start the
FairCoin wallet like this:  
`faircoind -cvn=fasito -fasitodevice=dev/ttyACM1`

Further information about Fasito can be found in it's github repository.
