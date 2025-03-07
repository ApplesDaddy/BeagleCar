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
    5) Open VLC, click ‘Media – Open Network Stream’, set the network URL
    ‘udp://@:1234’, and click the play button. Then, the VLC will show the video
    stream.

### Web Server
- You can run the example server after building by doing the following: 
```bash
$ cd build/testing/
$ ./webserverTest
```
From there you can go to the following addresses: 
0.0.0.0:8080
0.0.0.0:8080/template/<some integer value>
0.0.0.0:8080/template_file/<some integer value>
0.0.0.0:8080/load_file
0.0.0.0:8080/static/websocket_page.html 
0.0.0.0:8080/static/websocket_video.html
For the websocket examples you should see the websocket messages both in the terminal log and in the console log on the webpage.

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

