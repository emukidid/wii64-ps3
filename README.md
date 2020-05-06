# wii64-ps3
A very old proof of concept in porting mupen64 (wii64 specifically from 2011) to the PS3 using PSL1GHT v1.

## Features
* No audio
* No hardware accelerated graphics
* Basic input
* Pure interpreter
* Menu?
* USB loading?
* USB saving?

## Future
If someone would like to try and get it compiling with PSL1GHT v2 (https://github.com/ps3dev/PSL1GHT) then I'd be happy to try and merge upstream changes from newer Wii64 releases back into this or even try at porting the Wii64 dynarec over.

Issues we'd run into at the time with the state of homebrew on the PS3 were mostly due to the newness of the library and toolchain but also the fact that we couldn't get decent performance at all from the pure interpreter and we were unable to execute from heap (no dynarec). The pure interpreter ran faster on the Wii than it did with the PS3's main CPU, probably due to the fact that there's no branch prediction.
