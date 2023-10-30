# Simulator project for LVGL embedded GUI Library

The [LVGL](https://github.com/lvgl/lvgl) is written mainly for microcontrollers and embedded systems however you can run the library **on your PC** as well without any embedded hardware. The code written on PC can be simply copied when your are using an embedded system.

Using a PC simulator instead of an embedded hardware has several advantages:
* **Costs $0** because you don't have to buy or design PCB
* **Fast** because you don't have to design and manufacture PCB
* **Collaborative** because any number of developers can work in the same environment
* **Developer friendly** because much easier and faster to debug on PC

This version was created specifically for using with the UoA FSAE Dashboard.  Please use it to develop GUI's without having to have it in the car.

## Requirements
This project is configured for VSCode and only tested on Linux, although this may work on OSx or WSL. It requires a working version of GCC, GDB and make in your path.

To allow debugging inside VSCode you will also require a GDB [extension](https://marketplace.visualstudio.com/items?itemName=webfreak.debug) or other suitable debugger.

* **SDL** a low level driver library to use graphics, handle mouse, keyboard etc.

## Usage

### Get the PC project

Clone the PC project and the related sub modules:

```bash
git clone --recursive https://github.com/UOA-FSAE/dashboard-simulator.git
```

### Install SDL
You can download SDL from https://www.libsdl.org/

On on Linux you can install it via terminal:
```bash
sudo apt-get update && sudo apt-get install -y build-essential libsdl2-dev
```

### Using the simulator
From the top level directory, just call 
```
make all -j
```
Then, run the binary created in `/build/bin` and it will launch a window with a preview of what the dashboard looks like.  The rest is up to you :)

Make sure that the `screens.h`, `screens.c`, `vehicle.h` and `vehicle.c` are synced with the dashboard repository to make sure your changes tested on here are actually reflected on the real dashboard.  Also ensure that all fonts are copied into the `fonts` folder, and declared with `LV_LV_FONT_DECLARE(lower_case_font_variable_name)` to make them globally available.  This is also done in screens.  Only `screens.c` should be edited in this repository.

### Optional library
There are also FreeType and FFmpeg support. You can install FreeType support with:
```bash
# FreeType support
wget https://kumisystems.dl.sourceforge.net/project/freetype/freetype2/2.13.2/freetype-2.13.2.tar.xz
tar -xf freetype-2.13.2.tar.xz
cd freetype-2.13.2
make
make install
```

The FFmpeg support can be installed with:
```bash
# FFmpeg support
git clone https://git.ffmpeg.org/ffmpeg.git ffmpeg
cd ffmpeg
git checkout release/6.0
./configure --disable-all --disable-autodetect --disable-podpages --disable-asm --enable-avcodec --enable-avformat --enable-decoders --enable-encoders --enable-demuxers --enable-parsers --enable-protocol='file' --enable-swscale --enable-zlib
make
sudo make install
```

And then remove all the comments in the `Makefile` on `INC` and `LDLIBS` lines. They should be:
```Makefile
INC 				:= -I./ui/simulator/inc/ -I./ -I./lvgl/ -I/usr/include/freetype2 -L/usr/local/lib
LDLIBS	 			:= -lSDL2 -lm -lfreetype -lavformat -lavcodec -lavutil -lswscale -lm -lz -lpthread
```

### Setup
To allow custom UI code an `lv_conf.h` file placed at `ui/simulator/inc` will automatically override this projects lv_conf.h file. By default code under `ui` is ignored so you can reuse this repository for multiple projects. You will need to place a call from `main.c` to your UI's entry function.

To build and debug, press F5. You should now have your UI displayed in a new window and can access all the debug features of VSCode through GDB.

To allow temporary modification between simulator and device code, a SIMULATOR=1 define is added globally.
