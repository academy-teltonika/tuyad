# OpenWRT Tuya cloud communication and esp controller daemon [WIP]

## Quick-start guide

1. Install ubus and espcommd.
2. Install this application using "cmake . && make"
2. Start ubusd and run espcommd. 
3. Start this application with the correct tuya credentials (-p <PRODUCT_ID> -d <DEVICE_ID> -s <DEVICE_SECRET)
4. Send commands using Tuya actions and observe the results

## Tuya action codes

list_devices

sysinfo

toggle_pin (required arguments):

1. port: string
2. pin: integer
3. power: enum <"on"|"off">

read_sensor (required arguments):

1. port: string
2. pin: integer
3. sendor: string
4. model: string

## TODO

[ ] Implement graceful ubus failure handling. Possible to restart?
[ ] Create OpenWRT packages and install the application into the router.
[x] Restore "write to log" Tuya action functionality.
[x] Finish implementing the Tuya action for reading sensor data.
[ ] Fix "DHT returned no data" being OK (should be Err).
