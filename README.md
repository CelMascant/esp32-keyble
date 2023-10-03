# esp32-keyble with wiegand keypad and access control

ESP32 port of the keyble library
working, with wiegand keypad and access control.

Thanks go to <a href="https://github.com/RoP09">RoP09</a>, <a href="https://github.com/tc-maxx">tc-maxx</a>, <a href="https://github.com/henfri">henfri</a>, <a href="https://github.com/MariusSchiffer">MariusSchiffer</a> and of course <a href="https://github.com/oyooyo">oyooyo</a> for their brillant work!

Changes/additions made:

- platformio.ini changed "platform = espressif32" to "platform = espressif32@^2.0.0" to make it work in 2023
- quick fix for wrong mqtt credentials issue: ESP now prints error codes only once every 10 seconds.
- added wiegand interface, tag and pin stored in SPIFFS.

Step by step setup:

- get the original keyble js application (https://github.com/oyooyo/keyble) running and create the credentials (MAC, user_id, user_key).
- Erase the flash first because of SPIFFS usage and maybe stored old WiFi credentials.
- Upload the project (I use Platformio).
- Connect to the ESP32's WiFi network.
- By default the SSID is "KeyBLEBridge" with default password "eqivalock".
- After you have connected to the network, go to 192.168.4.1 with you browser.
- At rootlevel you will just see a simple webpage.
- Click on the gear to connect to your WiFi Network.
- Now you will see the AutoConnect portal page.
- Go to "Configure new AP" to enter your credentials.
- AutoConnect scans for networks in reach, just choose the one you want to connect to.
- I recommend you to disable DHCP and give the bridge a static IP.
- The ESP32 reboots, so you have to access it with the new given IP.
- Follow the link and enter MQTT and KeyBLE credentials.
- After saving the credentials click on "Reset" to reboot.
- You well be redirected to the main page of AutoConnect.
- Click on "Home" to see the entered credentials.
- Now everthing is set up and the ESP publishes its states to the given MQTT Broker.
- It publishes the state once on startup and has then to be triggert from outside.

- It is highly recommended to change the Master pin after upload!!!

The bridge publishes to the given topic you entered at setup.

Endpoints are:

- /battery true for good, false for low
- /command 1 for status, 2 to unlock, 3 to lock, 4 to open and 5 to toggle
- /rssi signalstrength
- /state as strings; locked, unlocked, open, moving, timeout
- /task working; if the lock has already received a command to be executed, waiting; ready to receive a command
- /access String, seperated with ";", access controller state, access controller mode, access controller last_ID, access controller last_Rejected

The /task endpoint is usefull, because the ESP toggles between WiFi and BLE connections. If the bridge is connected to WiFi and recieves a command via MQTT, it disables WiFi, connects to the lock via BLE, reconnects after the BLE task has finished and publishes the new state to the mqtt broker.

Access controler functions:
- During normal operation, entered pins and tags are checked and access control is carried out.
- Keyboard entrys must be completed with a #.
- When the MasterPin (11223344) is entered, it switches to programming mode.
- To change a tag, 1 must be selected. To change a pin, 2 must be selected.
- Next, the tag or pin number must be selected.
- Finally, a tag must be read or a pin entered.
- If a 0 is entered instead of a pin or tag, the selected pin or tag will be deleted.

- pin values are 1000 to 4294967296 as this is the largest number a long int can store.
- There is storage space for 40 pins and 40 tags.
- pins and tags are stored in /data/pins.txt and /data/tags.txt and can be edited befor upload.
- Make sure there's a ";" between each pin. Only numbers are allowed. 
- The first pin in the array is the Master pin.

Example inputs:
normal operation:
- 123456# -> no stored pin matches -> access denied.
- 1234# -> stored pin 1 matches -> access garanted.

programming a new tag:
1. 11223344# -> stored pin 0 matches -> Master pin -> switch to programming mode
2. 1# -> switch to pin programming
3. 15# -> tag number 15 should be changed
4. tag was readed -> tag number 15 is changed to the readed tag ID
  The programm automaticly stores the new values in SPIFFS.

programming a new pin:
1. 11223344# -> stored pin 0 matches -> Master pin -> switch to programming mode
2. 2# -> switch to pin programming
3. 5# -> pin number 5 should be changed
4. 5555# -> pin number 5 is changed to 5555
  The programm automaticly stores the new values in SPIFFS.

If at step 4 "0#" is entered, the selected entry is deleted.

TODO
- change webpage (own design) and WiFi-handling
- add tag and pin administrative functions to webpage
- add door sensor option (no locking when door is opened)
- register user to keyble feature
- error handling
- more endpoints like IP, uptime, etc.
- a command queue topic to get commands while the bridge was busy
- 3D printed adapter for KEYBLE to my special door

Have fun!
