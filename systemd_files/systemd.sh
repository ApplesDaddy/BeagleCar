#!/bin/bash


#Start the hats
/home/cmpt433-user/start-hats.sh || exit 1
echo "Started the Hats."

# Connect to the wifi
iwctl station wlan0 disconnect
echo "Disconnect wlan0"

iwctl station wlan0 scan

SSID="TEST"
PASS="testing123"
DEVICE="wlan0" 

# Make sure the correct directory exists
mkdir -p /var/lib/iwd

# Bring up the Wi-Fi
iwctl station "$DEVICE" connect "$SSID"
echo "Connect to WIFI"



# Start the video
cd /home/cmpt433-user
source /home/cmpt433-user/.BEAGLE_CAR_VENV/bin/activate

# Run the Python script in the background
nohup python crash-detection2.py &
echo "nohup the video send"


# Start the car
/home/cmpt433-user/udp &
echo "udp receive up"

sleep 10000000
echo "Slept for 60 seconds"


