#https://github.com/wms2537/python-udp-mjpeg-server/blob/main/udp_stream.py


import socket
import cv2
import numpy as np

UDP_IP = "192.168.7.1"
UDP_PORT = 12345

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

try:
    while True:
        data, addr = sock.recvfrom(65535)
        frameData = np.frombuffer(data, dtype=np.uint8)

        img = cv2.imdecode(frameData, cv2.IMREAD_COLOR)

        if img is not None:
            cv2.imshow("MJPEG Stream", img)
            #https://stackoverflow.com/questions/35372700/whats-0xff-for-in-cv2-waitkey1
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break
        else:
            print("Error decoding frame")
except KeyboardInterrupt:
    print("Receiver stopped.")
finally:
    sock.close()
    cv2.destroyAllWindows()
