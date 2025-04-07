# start access point
sudo /etc/init.d/hostapd start

# start dns server
sudo service isc-dhcp-server start

# set wlan0 static IP
sudo ip addr add 192.168.10.1/24 dev wlan0
