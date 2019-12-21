HYELEC/PEAKMETER MS8236 USB DRIVER FOR LINUX
--------------------------------------------

The Hyelec/PeakMeter MS8236 is a low cost true RMS multimeter with USB interface.
This software helps users to connect the meter to a computer running Linux for logging or display purposes.
The ms8236 USB interface appears to the computer as a serial port running at 2400 baud.
The ms8236 sends 22 bytes of information that reflect icons and LCD segments that are active.
The program ms8236usb receives the active LCD information and translates this to log text on stdout.

To save the information to a log file at the command line enter :

```
$ ms8236usb > log
```

When you have saved enough log data type Control+C to quit logging.
    
Example log entry:

```
  -36.34 mV DC Auto Sat Sep 17 16:19:16 2016
``` 

To import the log file into a spreadsheet use the open command and specify 'space' as the separator.

This software has been tested on Ubuntu 16.04. It may work on your system.
Send any bug or improvement suggestions to richard.jones.1952@gmail.com

Note. This program was quickly put together to show that the meter was working.
      It may not look pretty, but is quite useful.

## INSTALLATION INSTRUCTIONS

To install software on your Linux machine download the files into a folder, and at the command line
change directory into that folder and type :

```
$ make install
```

To run the software enter :

```
$ ms8236usb
```

## CORRELATING WITH OTHER STREAMS

Multimeter readings can be combined with another log or stream source uinsg a third party tool [lmerge](https://github.com/rianhunter/lmerge) as follows:

```
lmerge <(tail -n1 -f /path/to/file) <(ms8236usb) | tee /tmp/combined-output
```

`tail` can be replaced by anything streaming to `STDOUT`.
