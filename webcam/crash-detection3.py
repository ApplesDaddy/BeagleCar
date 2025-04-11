#!/usr/bin/env python3
# https://docs.beagleboard.org/boards/beagley/ai/demos/beagley-ai-object-detection-tutorial.html
# https://gist.github.com/fernandoremor/8d9efb81e25360ab38245c8e96d870c8

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


# https://stackoverflow.com/questions/47802480/create-boolean-mask-of-numpy-rgb-array-if-matches-color
def color_change_detected(moving_avg, curr_avg, threshold=20.0):
    dist = np.linalg.norm(curr_avg - moving_avg)
    return dist > threshold


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--resolution", default="640x480", help="Desired webcam resolution")
    args = parser.parse_args()

    resW, resH = map(int, args.resolution.split("x"))

    vd = open(f"/dev/video{video_driver_id}", "rb+", buffering=0)

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

        mm = mmap.mmap(vd.fileno(), buf.length, mmap.MAP_SHARED, mmap.PROT_READ | mmap.PROT_WRITE, offset=buf.m.offset)
        buffers.append(mm)
        fcntl.ioctl(vd, VIDIOC_QBUF, buf)

    buf_type = v4l2_buf_type(V4L2_BUF_TYPE_VIDEO_CAPTURE)
    fcntl.ioctl(vd, VIDIOC_STREAMON, buf_type)

    UDP_IP = "192.168.7.2"
    UDP_PORT = 12345
    MESSAGE_PORT = 12346
    message_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    CHUNK_SIZE = 65000

    output_dir = "frame_data"
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    # timing and moving average variables.
    stream_start_time = time.time()  # when the stream starts
    cooldown_end_time = 0  # No cooldown period at first

    # moving average
    moving_sum = None  # numpy array for sum of RGB (first three channels)
    moving_count = 0
    # crash mode variables.
    crash_check_mode = False
    crash_check_counter = 0
    crash_threshold = 20.0  # threshold for color deviation.

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
            # get the bottom half of the frame
            bottom_half = frame[height // 2 :, :, :]
            # calculate current average color for bottom half (only RGB).
            curr_avg_full = cv2.mean(bottom_half)  # returns (B, G, R, alpha)
            # convert np array
            curr_avg = np.array([curr_avg_full[2], curr_avg_full[1], curr_avg_full[0]])

            now = time.time()
            # update moving average before 5 seconds has passed
            if now - stream_start_time <= 5:
                if moving_sum is None:
                    moving_sum = curr_avg.copy()
                    moving_count = 1
                else:
                    moving_sum += curr_avg
                    moving_count += 1
                moving_avg = moving_sum / moving_count
            else:
                # if in a cooldown period, skip detection but update the moving average.
                if now < cooldown_end_time:
                    if moving_sum is None:
                        moving_sum = curr_avg.copy()
                        moving_count = 1
                    else:
                        moving_sum += curr_avg
                        moving_count += 1
                    moving_avg = moving_sum / moving_count
                    # and ensure we are not in crash check mode during cooldown
                    crash_check_mode = False
                    crash_check_counter = 0
                else:
                    # not in cooldown.
                    if not crash_check_mode:
                        # update the moving average normally.
                        if moving_sum is None:
                            moving_sum = curr_avg.copy()
                            moving_count = 1
                        else:
                            moving_sum += curr_avg
                            moving_count += 1
                        moving_avg = moving_sum / moving_count

                        # check if current frame deviates significantly from the moving average.
                        if color_change_detected(moving_avg, curr_avg, threshold=crash_threshold):
                            crash_check_mode = True
                            crash_check_counter = 1  # start counting deviating frames
                    else:
                        # in crash check mode: do not update the moving average.
                        if color_change_detected(moving_avg, curr_avg, threshold=crash_threshold):
                            crash_check_counter += 1
                        else:
                            # if one frame doesn't deviate, cancel crash check mode.
                            crash_check_mode = False
                            crash_check_counter = 0
                            # update the moving average with this frame.
                            moving_sum += curr_avg
                            moving_count += 1
                            moving_avg = moving_sum / moving_count

                        # 5 is arbitrary threshold
                        if crash_check_counter >= 5:
                            # process crash event.
                            print("CRASH DETECTED!")
                            cv2.imwrite(os.path.join(output_dir, f"crash_frame_{frame_number}.jpg"), frame)
                            message_sock.sendto(b"CRASH DETECTED!\n", (UDP_IP, MESSAGE_PORT))
                            # set a 5 second cooldown.
                            cooldown_end_time = now + 5
                            # reset moving average with current frame.
                            moving_sum = curr_avg.copy()
                            moving_count = 1
                            moving_avg = moving_sum / moving_count
                            # reset crash check mode.
                            crash_check_mode = False
                            crash_check_counter = 0

            encoded_frame = cv2.imencode(".jpg", frame)[1].tobytes()
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
