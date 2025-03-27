# BeagleCar
Term project for CMPT433

Equipment:

BeagleBone Y-AI
Logitech C270 720p webcam




### Dependencies
- webServer
    - g++ for compiling C++ and cross compiling
    ```bash
    (host)$ sudo apt install g++
    (host)$ sudo apt install g++-aarch64-linux-gnu
    ```
    - ffmpeg for video segmenting
    ```bash
    (host)$ sudo apt install ffmpeg
    (byai)$ sudo apt install ffmpeg
    ```
- webcam
    1) openCV
    ```bash
    (target)$ sudo apt-get install libv4l-dev
    (target)$ sudo apt-get install libopencv-dev
    (target)$ sudo apt-get install ffmpeg
    ```
    2) compile code
    ```bash
    (host)$ make
    (host)$ make install
    ```

    3) VLC (to view video on host)
    ```bash
    (host) $ sudo apt install vlc
    ```
    4) run on target
     ```bash
    (target) $ ./capture_exec -F -o -c0 | ffmpeg -i pipe:0 -vcodec copy -f mjpeg udp://192.168.7.1:1234
    ```

    4.5) if 4 doesn't work ('pipe:0: Invalid data found when processing input'),
    first check where usb webcam shows up:


     ```bash
    (target) v4l2-ctl --list-devices
    ```

   then replace (/dev/video/_) with where the webcam is recognized (ex. video1/video2/video3)

   ```bash
   (target) $ ./capture_exec -F -o -c0 -d /dev/video_ | ffmpeg -i pipe:0 -vcodec copy -f mjpeg udp://192.168.7.1:1234
   ```

    5) Open VLC, click ‘Media – Open Network Stream’, set the network URL
    ‘udp://@:1234’, and click the play button. Then, the VLC will show the video
    stream.

- lcd/video streaming
    ```sh
        (host) sudo apt-get install libavcodec-dev
        (host) sudo apt-get install libavformat-dev
        (host) sudo apt-get install libswscale-dev
        (host) sudo apt-get install libavutil-dev

        (target)$ sudo apt-get install ffmpeg
        (target)$ sudo apt install liblgpio-dev
    ```

- udp terminal send testing
    ```sh
        (host) sudo apt-get install libncurses5-dev
    ```

### Web Server
- You can run the example server after building by doing the following:
```bash
$ cd build/testing/
$ ./webserverTest
```
From there you can go to the following addresses:
- 0.0.0.0:8080
- 0.0.0.0:8080/template/<some integer value>
- 0.0.0.0:8080/template_file/<some integer value>
- 0.0.0.0:8080/load_file
- 0.0.0.0:8080/static/websocket_page.html
- 0.0.0.0:8080/static/websocket_video.html

*For the websocket examples you should see the websocket messages both in the terminal log and in the console log on the webpage.

- Other requirments:
    - The templates and static directories need to be in the location where the executable is being ran.
    - Websocket streaming only verified to work on chrome
    - ffmpeg command to reincode a video and keyframes every 50 frames so it can be segmented to 2 second chunks.... kindly provided by chatgpt; Sample video from: https://www.sample-videos.com/
    ```bash
    $ ffmpeg -i input.mp4 -c:v libx264 -g 50 -keyint_min 50 -sc_threshold 0 -f segment -segment_time 2 output%03d.mp4
    ```
    - ffmpeg command to fragment each of the outputs; there is a shell script in the video directory to do both of the commands automatically.
    ```bash
    $ ffmpeg -i non_fragmented.mp4 -movflags frag_keyframe+empty_moov+default_base_moof fragmented.mp4
    ```



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

### udp program
To run receiver:
```sh
(target or host) $ ./udp
```

To run sender with hardware inputs: (joystick, rotary encoder)
```sh
(target) $ ./udp --sender
# q to quit
```
To run sender on terminal with keyboard inputs (w/a/s/d=joystick, j/l=encoder, e=encoder push, space=joystick push)
```sh
(target or host) $ ./udp --sender --terminal
# q to quit
```
If you want to run the terminal sender without ncurses:
- change `getch()` to `getchar()` in `app/src/main.c`
- comment out the dependency in `app/CMakeLists.txt`

Sender/receiver IP addresses can be changed in `app/include/udp_constants.h`
