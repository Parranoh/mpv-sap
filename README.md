# mpv-sap
A plugin for the mpv media player that enables playback of .SAP (Slight Atari Player) files.

It uses the emulator [ASAP](http://asap.sourceforge.net/) written by Piotr
Fusik.

At this point the plugin seems to work, however it is not thoroughly tested and
should not be relied on.

## Building
Install dependency `libmpv-dev` or, alternatively, put `mpv/client.h` in the
root directory of the repository. Then run `make`.

## Installing
```sh
mkdir -p ~/.config/mpv/scripts && mv mpv-sap.so ~/.config/mpv/scripts
```
