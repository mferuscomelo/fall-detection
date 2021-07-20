# Fall Detection
Create your own fall detection sensor using an Arduino! (README needs to be updated)

## How it works
This code runs on an [Arduino Nano 33 BLE Sense](https://store.arduino.cc/arduino-nano-33-ble-sense) and uses the inbuilt 9 axis inertial sensor to detect falls using a custom trained machine learning model.

### Setup
Clone this repository onto your local computer. 

### Collecting Data
Follow these steps to collect your own data to train your model.
1. Upload `scripts/CollectData/CollectData.ino` to your Arduino.
2. Create a folder in the project workspace e.g: `arduino`.
3. Add a subfolder in the `arduino` directory with the name of the person whose data you are collection. Feel free to name it something like `Person1` if you want the data to remain anonymous.
4. Run `scripts/collect_data.py` from the terminal using `py ./scripts/collect_data.py`
5. Connect to your Arduino over Bluetooth Low Energy (BLE) by selecting the Arduino from the list.
6. Once connected, the Arduino's light will turn green and you will be prompted for a command to send to the Arduino. Here is a list of available commands: `STOP, WALKING, SITTING, LYING_DOWN, and FALL`. The Arduino's LED will change to blue if a valid command has been received. If something went wrong, the LED will light up red. To start recording an action, send the command e.g: `WALKING`. To stop recording an action, send `STOP`. If you want to change the types of actions that can be recorded, edit `possibleLabels` in `CollectData.ino` to include your action. **Important:** remember to update `POSSIBLE_LABELS_LENGTH` with the length of `possibleLabels`.

    The Arduino will then start recording the action whose data will then be stored in the person's directory in `arduino`. The data, stored in the form of a CSV, will contain the following information: `x_acc`, `y_acc`, `z_acc`, `label_name`

### LED Signals
<!-- TODO: color-code the words -->
Use this guide to understand the Arduino's signals through its LED:
`RED (Long)`: Waiting for connection
`Red (Blink)`: Unknown command
`BLUE`: Valid command
`GREEN`: Connected over BLE
`YELLOW`: Sending data over BLE
## Detecting Falls
Upload the contents of `scripts/FallDetection` to the Arduino. The script currently needs to be plugged into the computer to work. README and script will be updated to support fall detection remotely.