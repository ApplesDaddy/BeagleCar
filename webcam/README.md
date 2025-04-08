on beagle:

1) pip install v4l2
2) pip install opencv-python
3) `python3 crash-detection2.py`

    (crash-detection2 logic: if the average colour of the bottom half of frame(n-1) 
                             is different from frame(n)'s, detect crash. 5 sec 
                             cooldown)

OR

4) `python3 crash-detection3.py`

    (crash-detection3 logic: populate moving average with the average colour of the bottom
                             half of frames for 5 seconds. then, if 5 consecutive frames deviate from the moving average, detect crash. 5 sec cooldown)




on host:
on first terminal:
    `python3 receiveUDP.py`

on second terminal
    `nc -u -l -p 12346`


## troubleshooting
`error: externally-managed-environment` message?

Create a virtual environment. Check the venv's pip is being used with `which pip`. If the wrong one is used, try `python3 -m pip install`
To create venv:
```sh
sudo apt install python3-venv
python3 -m venv .venv
source .venv/bin/activate
```
---
`TypeError: unsupported operand type(s) for +: 'range' and 'list'` from importing v4l2?

manually fix the lines in the file (see error message) by wrapping `range()` in `list()` (example: `list(range())`)