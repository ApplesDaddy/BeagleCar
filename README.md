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
    5) Open VLC, click ‘Media – Open Network Stream’, set the network URL
    ‘udp://@:1234’, and click the play button. Then, the VLC will show the video
    stream.


### Web Server
- The templates and static directories need to be in the location where the executable is being ran. 
TODO: Make it be overwritten with cmake commands

