# BeagleCar
Term project for CMPT433


### Dependencies
- webServer
    - g++ for compiling C++ and cross compiling
    ```bash
    (host)$ sudo apt install g++
    (host)$ sudo apt install g++-aarch64-linux-gnu
    ```
- lcd/video streaming
    ```sh
        (host)$ sudo apt-get install ffmpeg:arm64
        (host) sudo apt-get install libavcodec:arm64
        (host) sudo apt-get install libavformat:arm64
        (target)$ sudo apt-get install ffmpeg
        (target)$ sudo apt install liblgpio-dev
    ```

### Web Server
- The templates and static directories need to be in the location where the executable is being ran.
TODO: Make it be overwritten with cmake commands



## Manually Running CMake

To manually run CMake from the command line use:

```shell
  # Regenerate build/ folder and makefiles:
  rm -rf build/         # Wipes temporary build folder
  cmake -S . -B build   # Generate makefiles in build\

  # Build (compile & link) the project
  cmake --build build
```

## Running on Target

### LCD config:

* Load the SPI Overlay on the board (just do once)
  a. Edit config file:
     `(byai)$ sudo nano /boot/firmware/extlinux/extlinux.conf`
  b. Edit the last section to make it say:
     ```
      label microSD (default)
          kernel /Image
          append console=ttyS2,115200n8 root=/dev/mmcblk1p3 ro rootfstype=ext4 resume=/dev/mmcblk1p2 rootwait net.ifnames=0 quiet
          fdtdir /
          fdt /ti/k3-am67a-beagley-ai.dtb
          fdtoverlays /overlays/k3-am67a-beagley-ai-spidev0.dtbo
          initrd /initrd.img
     ```
     (If you are also enabling PWM, make the `fdtoverlays` line be a space-separated list of .dtbo files.)
  c. Reboot.
* At each boot you'll need to either:
  a. Change the SPI to be usable by anyone:
     `sudo chmod a+rw /dev/spidev0.*`
  b. Run the program with root access.
