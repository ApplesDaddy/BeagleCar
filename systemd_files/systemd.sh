#Start the hats
~/./start-hats.sh
sleep 1


# Connect to the wifi
~/./wifi_client.sh
sleep 1


# Start the video
source ~/.BEAGLE_CAR_VENV/bin/activate

# Run the Python script in the background
nohup python crash_detection2.py > /dev/null 2>&1 &

sleep 1


# Start the car
~/./udp
