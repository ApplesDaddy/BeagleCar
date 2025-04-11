#!/bin/bash


#https://saturncloud.io/blog/how-to-get-the-directory-where-a-bash-script-is-located/#:~:text=Method%201%3A%20Using%20%240,directory%20name%20from%20the%20path.
cd "$(dirname "$0")"

cp TEST.psk /var/lib/iwd/TEST.psk
cp Beagle_car.service /etc/systemd/system/Beagle_car.service

#https://sentry.io/answers/determine-whether-a-directory-exists-in-bash/#:~:text=The%20%2Dd%20flag%20tests%20whether,man%20test%20into%20the%20terminal.
if test -d "/home/cmpt433-user"; then
    echo "Directory exists"
else
    echo "/home/cmpt433-user does not exist. Attempting to create it."
    mkdir /home/cmpt433-user || echo "Failed to create the directory" && exit 1
    echo "Directory created"
fi

cp systemd.sh /home/cmpt433-user/systemd.sh

