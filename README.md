# wii64-ps3
A very old proof of concept in porting mupen64 (wii64 specifically from 2011) to the PS3 using PSL1GHT v1.

## Features
* Bad audio
* Preliminary hardware accelerated graphics
* Basic input
* Pure interpreter
* Menu
* USB loading
* USB saving

## Future
I've had a go at getting this to work with PSL1GHT v2 (https://github.com/ps3dev/PSL1GHT) and have run into some issues I'm looking into (basially the menu loops once every 3-4 seconds and everything becomes unresponsive). Once this is resolved I'd be happy to try and merge upstream changes from newer Wii64 releases back into this and even try at porting the Wii64 dynarec over to ppc64.

Issues we'd run into at the time with the state of homebrew on the PS3 were mostly due to the newness of the library and toolchain but also the fact that we couldn't get decent performance at all from the pure interpreter and we were unable to execute from heap (no dynarec). The pure interpreter ran faster on the Wii than it did with the PS3's main CPU, probably due to the fact that there's no branch prediction.
