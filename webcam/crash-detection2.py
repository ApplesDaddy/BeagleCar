#!/usr/bin/env python3
#https://docs.beagleboard.org/boards/beagley/ai/demos/beagley-ai-object-detection-tutorial.html
#https://gist.github.com/fernandoremor/8d9efb81e25360ab38245c8e96d870c8
from v4l2 import *
import fcntl
import mmap
import time
import socket
import cv2
import numpy as np
import argparse
import sys
import os

video_driver_id = 3

def color_change_detected(prev_avg, curr_avg, threshold=20.0):
    dist = np.linalg.norm(np.array(curr_avg[:3]) - np.array(prev_avg[:3]))
    return dist > threshold

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument(
        '--resolution',
        default='640x480',
        help='Desired webcam resolution') 
    args = parser.parse_args()

    resW, resH = map(int, args.resolution.split('x'))

    vd = open(f'/dev/video{video_driver_id}', 'rb+', buffering=0)

    cp = v4l2_capability()
    fcntl.ioctl(vd, VIDIOC_QUERYCAP, cp)

    fmt = v4l2_format()
    fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE
    fcntl.ioctl(vd, VIDIOC_G_FMT, fmt)

    fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG
    fmt.fmt.pix.width = resW  
    fmt.fmt.pix.height = resH  
    fcntl.ioctl(vd, VIDIOC_S_FMT, fmt)

    parm = v4l2_streamparm()
    parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE
    parm.parm.capture.capability = V4L2_CAP_TIMEPERFRAME
    fcntl.ioctl(vd, VIDIOC_G_PARM, parm)
    fcntl.ioctl(vd, VIDIOC_S_PARM, parm)

    req = v4l2_requestbuffers()
    req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE
    req.memory = V4L2_MEMORY_MMAP
    req.count = 4
    fcntl.ioctl(vd, VIDIOC_REQBUFS, req)

    buffers = []
    for ind in range(req.count):
        buf = v4l2_buffer()
        buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE
        buf.memory = V4L2_MEMORY_MMAP
        buf.index = ind
        fcntl.ioctl(vd, VIDIOC_QUERYBUF, buf)

        mm = mmap.mmap(
            vd.fileno(),
            buf.length,
            mmap.MAP_SHARED,
            mmap.PROT_READ | mmap.PROT_WRITE,
            offset=buf.m.offset)
        buffers.append(mm)
        fcntl.ioctl(vd, VIDIOC_QBUF, buf)

    buf_type = v4l2_buf_type(V4L2_BUF_TYPE_VIDEO_CAPTURE)
    fcntl.ioctl(vd, VIDIOC_STREAMON, buf_type)

    UDP_IP = "192.168.7.1"
    UDP_PORT = 12345
    MESSAGE_PORT = 12346
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    message_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

    output_dir = "frame_data"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # variables for crash detection 
    stream_start_time = time.time()    # when the stream starts
    cooldown_end_time = 0              # no cooldown period at first.
    prev_avg_color = None              # init previous average color

    try:
        frame_number = 0
        start_time = time.time()
        frame_count = 0

        while True:
            buf = v4l2_buffer()
            buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE
            buf.memory = V4L2_MEMORY_MMAP
            fcntl.ioctl(vd, VIDIOC_DQBUF, buf)

            mm = buffers[buf.index]
            mjpeg_data = mm.read()
            mm.seek(0)
            fcntl.ioctl(vd, VIDIOC_QBUF, buf)

            nparr = np.frombuffer(mjpeg_data, np.uint8)
            frame = cv2.imdecode(nparr, cv2.IMREAD_COLOR)
            if frame is None:
                continue  # skip if frame can't be decoded.

            height = frame.shape[0]
            bottom_half = frame[height // 2:, :, :]  # get bottom half
            curr_avg_color = cv2.mean(bottom_half)     # calculate current average color

            now = time.time()
            # only start crash detection after 5 seconds.
            if now - stream_start_time > 5:
                if now >= cooldown_end_time:
                    if prev_avg_color is not None:
                        if color_change_detected(prev_avg_color, curr_avg_color, threshold=20.0):
                            print("CRASH DETECTED!")
                            cv2.imwrite(os.path.join(output_dir, f"crash_frame_{frame_number}.jpg"), frame)
                            message_sock.sendto(b'CRASH DETECTED!\n', (UDP_IP, MESSAGE_PORT))
                            # cooldown period 5 seconds
                            cooldown_end_time = now + 5
            # update previous avg colour
            prev_avg_color = curr_avg_color

            encoded_frame = cv2.imencode('.jpg', frame)[1].tobytes()
            # send frame
            sock.sendto(encoded_frame, (UDP_IP, UDP_PORT))

            frame_number += 1
            frame_count += 1

            current_time = time.time()
            elapsed_time = current_time - start_time
            if elapsed_time > 1.0:
                fps = frame_count / elapsed_time
                print(f"FPS: {fps:.2f}")
                start_time = current_time
                frame_count = 0

    except KeyboardInterrupt:
        print("\n>> Stop streaming")
        fcntl.ioctl(vd, VIDIOC_STREAMOFF, buf_type)
        sock.close()
        vd.close()
        print("Streaming stopped.")

if __name__ == "__main__":
    main()
