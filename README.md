# BeagleCar
Term project for CMPT433


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

### Web Server
- The templates and static directories need to be in the location where the executable is being ran. 

- Websocket streaming only verified to work on chrome
- ffmpeg command to reincode a video and keyframes every 50 frames so it can be segmented to 2 second chunks.... kindly provided by chatgpt - untested since i ran the windows executable version. Sample video from: https://www.sample-videos.com/
```bash 
$ ffmpeg -i input.mp4 -c:v libx264 -g 50 -keyint_min 50 -sc_threshold 0 -f segment -segment_time 2 output%03d.mp4
```
- ffmpeg command to fragment each of them
```bash
$ ffmpeg -i non_fragmented.mp4 -movflags frag_keyframe+empty_moov+default_base_moof fragmented.mp4
```

