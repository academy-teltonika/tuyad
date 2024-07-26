# OpenWRT Tuya cloud communication daemon

## **Quick-start guide**

1. Clone the repo
2. Put your device details in _**tuya-daemon/files/tuyad.config**_
3. Compile the package using OpenWRT
4. Install the .ipk file into the router using OPKG

### **Make targets** <br>

all            _# creates the daemon executable._ <br>
clean          _# removes all build files (except library files)._ <br>

### **Logging** <br>

Action messages are logged to /tmp/tuya_action.log <br>
Daemon status messages are logged using syslog with the identifier "tuyad". <br>
Your logging utility may or may not need additional configuration to accept these log messages. <br>

### **Daemon mode** <br>

The application fully relies on **procd** to run it as a background process.

If for any reason you need to run the application directly (without procd),
you must specify the following required options<br>

-p PRODUCT_ID <br>
-d DEVICE_ID <br>
-s DEVICE_SECRET <br>
