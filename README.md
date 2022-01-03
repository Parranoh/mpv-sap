# mpv-sap
A plugin for the mpv media player that enables playback of .SAP (Slight Atari Player) files.

It uses the emulator [ASAP](http://asap.sourceforge.net/) written by Piotr
Fusik.

## Building

First install the package's dependencies:

* Arch Linux: `pacman -S mpv`

* Debian/Ubuntu: `apt-get install libmpv-dev`

* Fedora: `dnf install mpv-libs-devel`

Alternatively, put `mpv/client.h` in the root directory of the
repository. Then run `make`.

## Installing

Running `make install`/`make uninstall` as either root or your user will
install files to the system or home directory, repectively.
