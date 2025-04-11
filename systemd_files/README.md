# Setup Instructions

Assumption: Target User is cmpt433-user, and other required files such as start-hats.sh and udp are already in the home directory.
On the target, you can run `setup.sh` as sudo or manually do the following:

## File Locations

- Place `TEST.psk` in:  
  `/var/lib/iwd/TEST.psk`
- Place `Beagle_car.service` in:  
  `/etc/systemd/system`
- Place `systemd.sh` in:  
  `/home/cmpt433-user/systemd.sh`

## Managing the Service

To manually start or stop the service, run:

```bash
sudo systemctl start Beagle_car.service
```

```bash
sudo systemctl stop Beagle_car.service
```

To enable or disable the service on start up, run:

```bash
sudo systemctl enable Beagle_car.service
```

```bash
sudo systemctl disable Beagle_car.service
```
