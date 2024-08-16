# OpenWRT Tuya cloud communication and esp controller daemon [WIP]

## Quick-start guide

1. Install ubus and esp-controller
2. Install this application using "cmake . && make"
2. Start ubusd and run the esp-controller
3. Start this application with the correct tuya credentials (-p <PRODUCT_ID> -d <DEVICE_ID> -s <DEVICE_SECRET)
4. Send commands using Tuya actions and observe the results

## Tuya action codes

list_devices (does not require any arguments) <br>
toggle_pin (required arguments): <br>

1. port: string
2. pin: integer
3. power: enum <"on"|"off">

# TODO

1. Implement graceful ubus failure handling. Right now the application only support graceful failure when the _commesp_
   module is down.

2. Create OpenWRT packages and install the application into the router.
   :w
3.
3. Restore "write to log" Tuya action functionality.

4. Finish implementing the Tuya action for reading sensor data (80% done, but ran out of time).
