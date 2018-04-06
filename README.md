# MEGA65-IDE

## Build Instructions

For now, I've just copy-pasted this info from this forum thread:

* https://groups.google.com/forum/#!topic/c65gs-development/dF3w4VHJSmc

For the mega65-ide you need the pre-requisite "cc65"
```
cd $GIT_ROOT

git clone https://github.com/cc65/cc65.git
cd cc65
make
sudo make install PREFIX=/usr/local
```

now, cc65 should be installed. check it by typing "**cc65**"

now clone and build the IDE

```
cd $GIT_ROOT
git clone https://github.com/MEGA65/mega65-ide
cd mega65-ide
make
```
note that a "**M65IDE.D81**" file is created.

you need to make an sdcardimage-file using the script in the mega65-core project
it is assumed that you already have the mega65-core project

```
cd $GIT_ROOT
cd mega65-core
cd src
cd utilities
./makesdcardimg.sh mysdcardimage
```
where "mysdcardimage" is whatever you want.

you need to have the xemu installed/made:

* https://github.com/MEGA65/xemu
* https://github.com/lgblgblgb/xemu/wiki/Source#compilation

You need to either:
* copy two files, mysdcardimage.img from the mega65-core project (./src/utilities/mysdcardimage.img), and M65IDE.D81 from the IDE project
* OR, provide the full path of the two files listed above.

The second option is preferred, so lets run the xemu(emulator) and have the emulator mount the D81-file containing the special IDE programs:
```
cd $GIT_ROOT
cd xemu
./build/bin/xmega65.native -skipunhandledmem -sdimg ../../../mega65-core/src/utilities/mysdcardimage.img -8 ../../../mega65-ide/M65IDE.D81
```

and the xemu should start, dropped into c64 mode, then:

```
load"*",8,1
run
```

NOTE: that I dont use the c65.native because I dont have the roms for it. the xmega65.native seems to work.
NOTE that I need to "-skipunhandledmem" else it crashes.

## Paul's suggested steps

Once you have the .d81, you should be able to just use that from in Xemu.  Note that I am just using the C65 Xemu, not M65 one, for the moment, with something like:

xc65.native -8 M65IDE.D81

then LOAD"*",8,1 and RUN

Paul.

## Controls

Retrieved from "**input.c**" comments for keys:

```
  // Management of windows
  C= 1 (129) - Window 1 (or close other windows if pressed twice)
  C= 2 (149) - Window 2 (create if not currently enabled)
  C= 3 (150) - Window 3 (create if not currently enabled)
  C= 4 (151) - Window 4 (create if not currently enabled)
  C= 5 (152) - Window 5 (create if not currently enabled)
  C= B (191) - Switch to previous possible buffer in this window
  C= N (191) - Switch to next possible buffer in this window
  C= - (220) - Widen current window
  SHIFT - (221) - Reduce width of current window by 1

  C= Q (171) - Press 3 times to quit without saving

  // Usual cursor navigation stuff
  DOWN (17) - move cursor down
  UP (145) - move cursor up
```
