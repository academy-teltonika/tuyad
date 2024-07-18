**Tuya cloud communication daemon.**

**Quick-start guide:**

1. Clone the repo
2. run 'make library'
3. run 'make run-daemon'

Required positional parameters: <br>
<PRODUCT_ID> <DEVICE_ID> <DEVICE_SECRET>

**Optional flags:** <br>
-D             # runs the application in non-daemon mode.

**Make targets:** <br>
all            # creates the daemon executable (library must be build separately). <br>
run            # runs the executable in non-daemon mode. <br>
run-daemon     # runs the executable in daemon mode. <br>
library        # buils the Tuya library. <br>
clean          # removes all build files (except library files). <br>
clean-library  # removes all library build files. <br>

**Logging** <br>
Action messages are logged to /tmp/tuya_action.log <br>
Daemon status messages are logged using syslog with the identifier "tuyad". <br>
Your logging utility may or may not need additional configuration to accept these log messages. <br>
