# Setup Instructions

You can run `setup.sh` or 


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

