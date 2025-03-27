# BeagleCar
Term project for CMPT433


### Dependencies
- webServer
    - g++ for compiling C++ and cross compiling
    ```bash
    (host)$ sudo apt install g++ 
    (host)$ sudo apt install g++-aarch64-linux-gnu
    ```


- vidStreamer
    - Install ffmpeg on BeagleY-AI
    ```bash
    (target)$ sudo apt update
    (target)$ sudo apt-get install ffmpeg
    ```
    Troubleshooting:
    - If the update command fails, it can prevent installation of necessary dependencies. 
    To fix this, ensure the system time is correct
    ```bash
    # Check system time
    (target)$ date
    # Correct system time
    (target)$ sudo date -s 'YYYY-MM-DD HH:MM:SS'
    # Check the corrected time
    (target)$ date
    ```
    - Install ffmpeg development libraries on host and target
    ```bash
    (host)$ sudo apt-get install libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libavdevice-dev libavfilter-dev
    (target)$ sudo apt-get install libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libavdevice-dev libavfilter-dev
    ```
    - Install arm64 versions ffmpeg development libraries on host and target
    ```bash
    (host)$ sudo dpkg --add-architecture arm64
    (host)$ sudo apt-get install libavformat-dev libavcodec-dev libavutil-dev libswscale-dev libavdevice-dev libavfilter-dev
    ```
    Troubleshooting:
    - Program will not cross-compile if the arm64 libraries are not in the specified location
    To fix this, check install locations and move if necessary
    ```bash
    # Check if libraries have been installed
    (host)$ sudo apt list libavcodec-dev
    # Result should be something like "libavcodec-dev/stable,stable-security,now 7:5.1.6-0+deb12u1 arm64 [installed]"
    # Find locations of packages
    (target)$ dpkg -L libavcodec-dev:arm64
    # Result should contain "/usr/include/aarch64-linux-gnu/libavcodec" and "/usr/lib/aarch64-linux-gnu"
    # Otherwise, update the vidStreamer-level CMakeLists.txt with the correct directories for /include and /lib directories of cross compiler
    ```
    <!-- not sure if needed -->
    <!-- - Install pkg-config on host and target (required for CMake to configure libraries during build)
    ```bash
    (host)$ sudo apt-get install pkg-config
    (target)$ sudo apt-get install pkg-config
    ``` -->

### Web Server
- The templates and static directories need to be in the location where the executable is being ran. 
TODO: Make it be overwritten with cmake commands

