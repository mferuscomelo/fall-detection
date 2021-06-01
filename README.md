# Fall Detection
Create your own fall detection sensor using an Arduino!

## How it works
This code runs on an [Arduino Nano 33 BLE Sense](https://store.arduino.cc/arduino-nano-33-ble-sense) and uses the inbuilt 9 axis inertial sensor to detect falls using a custom trained machine learning model.

### Setup
Clone this repository onto your local computer. 

### Collecting Data
Follow these steps to collect your own data to train your model.
1. Upload `CollectData.ino` (from the `data-collection` directory) to your Arduino.
2. Create a `data` directory in `data-collection`.
3. Add a subfolder in the `data` directory with the name of the person whose data you are collection. Feel free to name it something like `Person1` if you want the data to remain anonymous.
4. Run `data-collection/main.py` from the terminal using ```py ./data-collection/main.py```
5. Connect to your Arduino over Bluetooth Low Energy (BLE) by selecting the Arduino from the list.
6. Once connected, the Arduino's light will turn green and you will be prompted for a command to send to the Arduino. Here is a list of available commands: ```stop, WALKING, STAIRS_DOWN, STAIRS_UP, SITTING, LYING_DOWN```. The Arduino's LED will change to blue if a valid command has been received. If something went wrong, the LED will light up red. To start recording an action, send the command e.g: `WALKING`. To stop recording an action, send `stop`. If you want to change the types of actions that can be recorded, edit `possibleLabels` in `CollectData.ino` to include your action. **Important:** remember to update `POSSIBLE_LABELS_LENGTH` with the length of `possibleLabels`.

    The Arduino will then start recording the action whose data will then be stored in the person's directory in `data-collection/data`. The data, stored in the form of a CSV, will contain the following information: `x_acc`, `y_acc`, `z_acc`, `x_gyro`, `y_gyro`, `z_gyro`, `label_name`, `label_num` 
    
7. 
