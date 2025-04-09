#Start the hats
/home/cmpt433-user/start-hats.sh
sleep 1


# Connect to the wifi
iwctl station wlan0 disconnect
sleep 0.5

iwctl station wlan0 scan
sleep 1 

SSID="TEST"
PASS="testing123"
DEVICE="wlan0" 

# Make sure the correct directory exists
mkdir -p /var/lib/iwd

# Bring up the Wi-Fi
iwctl station "$DEVICE" connect "$SSID"


sleep 1


# Start the video
cd /home/cmpt433-user
source /home/cmpt433-user/.BEAGLE_CAR_VENV/bin/activate

# Run the Python script in the background
nohup python crash-detection2.py &

sleep 1


# Start the car
nohup /home/cmpt433-user/udp &
