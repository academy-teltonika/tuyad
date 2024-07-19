**Tuya cloud communication daemon.**

**Quick-start guide:**

1. Clone the repo
2. Specify device information in the makefile field "EXEC_TUYA_ARGS"
2. run 'make library'
3. run 'make run-daemon'

**Required options**: <br>
-p <PRODUCT_ID> -d <DEVICE_ID> -s <DEVICE_SECRET>

**Optional flags:** <br>
-D             # runs the application in daemon mode.

**Make targets:** <br>
all            # creates the daemon executable. <br>
run            # runs the executable in non-daemon mode. <br>
run-daemon     # runs the executable in daemon mode. <br>
library        # buils the Tuya library. <br>
clean          # removes all build files (except library files). <br>
clean-library  # removes all library build files. <br>

**Logging** <br>
Action messages are logged to /tmp/tuya_action.log <br>
Daemon status messages are logged using syslog with the identifier "tuyad". <br>
Your logging utility may or may not need additional configuration to accept these log messages. <br>
