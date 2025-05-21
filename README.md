# Simulator project for LVGL embedded GUI Library

The [LVGL](https://github.com/lvgl/lvgl) is the embedded graphics library that we use on the dashboard. To aid in screen development we can simulate the screen running on a pc, then you can port (copy) over the screen files to the [dashboard](https://github.com/UOA-FSAE/dashboard).

![simulator](resources/simulation.png?raw=true)

## Requirements
This project "is configured"/only ran so far on VSCode and Linux, if you are on Windows, WSL works fine. It requires a working version of GCC, GDB and make in your path.

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

On Linux (ubuntu/deb) you can install it via terminal:
```bash
sudo apt-get update && sudo apt-get install -y build-essential libsdl2-dev
```

### Using the simulator
From the top level directory, just call 
```
make all -j && ./build/bin/demo
```
- make -j: Compiles the project (the -j flag enables parallel compilation, speeding it up).
- ./build/bin/demo: Runs the compiled simulator application.
You should see the LVGL demo application running in a new window.

Make sure that the `screens.h`, `screens.c`, `vehicle.h` and `vehicle.c` are synced with the dashboard repository (submodule) to make sure your changes tested on here are actually reflected on the real dashboard.

When using **fonts** you want to use are copied into the `fonts` folder, and declared with `LV_LV_FONT_DECLARE(lower_case_font_variable_name)` in `styles.h`, and `screens.c` to make them globally available.

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
