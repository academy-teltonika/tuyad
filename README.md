**Tuya cloud communication daemon.**

**Quick-start guide:**

1. Clone
2. make library
3. make run-daemon

Required positional parameters:
<PRODUCT_ID> <DEVICE_ID> <DEVICE_SECRET>

**Optional flags:**
-D             # runs the daemon in non-daemon mode.

**Make targets:**
all            # creates the daemon executable (library must be build separately).
run            # runs the executable in non-daemon mode.
run-daemon     # runs the executable in daemon mode.
library        # buils the tuya library.
clean          # removes all build files (except library files).
clean-library  # removes all library build files.
