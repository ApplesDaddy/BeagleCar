#!/bin/bash

#Start the hats
/home/cmpt433-user/start-hats.sh || exit 1
echo "Started the Hats."

# Connect to the wifi
iwctl station wlan0 disconnect
echo "Disconnected wlan0"

SSID="TEST"
DEVICE="wlan0" 

iwctl station wlan0 scan
mkdir -p /var/lib/iwd
iwctl station "$DEVICE" connect "$SSID"
echo "Connected to WIFI"


# Start sending the video feed
cd /home/cmpt433-user
source /home/cmpt433-user/.BEAGLE_CAR_VENV/bin/activate

nohup python crash-detection2.py &
echo "video send up"

# Start receiving controlls for the car 
nohup /home/cmpt433-user/udp &
echo "udp receive up"

echo "Starting sleep so the processes don't close in the background."
sleep 10000000



