// Adapted from: https://github.com/Ladvien/arduino_ble_sense

#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

#define POSSIBLE_LABELS_LENGTH 4
#define LEDR (22u)
#define LEDG (23u)
#define LEDB (24u)

// #define DEBUG true // Comment this out to compile code for production

// Replacement class for Serial
#if !DEBUG
class NullSerialClass
{
  public:
  void begin(int speed) {}
  void println(int x, int type=DEC) {}
  void println(const char *p=NULL) {}
  void println(const String p) {}
  void print(int x, int type=DEC) {}
  void print(const char *p) {}
  void print(const String p) {}
  int available() {return 0;}
  int read() {return -1;}
  void flush() {}
} NullSerial;
#define Serial NullSerial
#endif

// x, y, z acceleration in Gs
float xAcc = 0,
      yAcc = 0,
      zAcc = 0;

bool canSendData = false;

String possibleLabels[POSSIBLE_LABELS_LENGTH] = {"WALKING", "SITTING", "LYING_DOWN", "FALLING"}; // Remember to update POSSIBLE_LABELS_LENGTH
String label = "UNDEFINED";


// Device info
const char* nameOfPeripheral = "Arduino Nano 33 BLE Sense";
const char* uuidOfService = "00001101-0000-1000-8000-00805f9b34fb";
const char* uuidOfReadChar = "00001142-0000-1000-8000-00805f9b34fb";
const char* uuidOfWriteChar = "00001143-0000-1000-8000-00805f9b34fb";

// BLE Service
BLEService IMUService(uuidOfService);

// Setup the incoming data characteristic.
const int WRITE_BUFFER_SIZE = 256;
bool WRITE_BUFFER_FIXED_LENGTH = false;

// Read / Write Characteristics
BLECharacteristic readChar(uuidOfReadChar, BLEWriteWithoutResponse | BLEWrite, WRITE_BUFFER_SIZE, WRITE_BUFFER_FIXED_LENGTH);
BLECharacteristic writeChar(uuidOfWriteChar, BLERead | BLENotify | BLEBroadcast, "123456789123456789123456789123456789123456789123456789123456789123456789");

/*
    MAIN
*/
void setup() {
  // Start serial.
  Serial.begin(9600);

#if DEBUG
  // Ensure serial port is ready.
  while (!Serial);
#endif

  // Prepare LED pins.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(LEDR, OUTPUT);
  pinMode(LEDG, OUTPUT);
  pinMode(LEDB, OUTPUT);

  // Start IMU
  startIMU();

  // Start BLE.
  startBLE();

  // Create BLE service and characteristics.
  BLE.setLocalName(nameOfPeripheral);
  BLE.setAdvertisedService(IMUService);
  IMUService.addCharacteristic(readChar);
  IMUService.addCharacteristic(writeChar);
  BLE.addService(IMUService);

  // Bluetooth LE connection handlers.
  BLE.setEventHandler(BLEConnected, onBLEConnected);
  BLE.setEventHandler(BLEDisconnected, onBLEDisconnected);

  // Event driven reads.
  readChar.setEventHandler(BLEWritten, onGetData);

  // Let's tell devices about us.
  BLE.advertise();

#if DEBUG
  // Print out full UUID and MAC address.
  Serial.println("Peripheral advertising info: ");
  Serial.print("Name: ");
  Serial.println(nameOfPeripheral);
  Serial.print("MAC: ");
  Serial.println(BLE.address());
  Serial.print("Service UUID: ");
  Serial.println(IMUService.uuid());
  Serial.print("readCharacteristic UUID: ");
  Serial.println(uuidOfReadChar);
  Serial.print("writeCharacteristic UUID: ");
  Serial.println(uuidOfWriteChar);


  Serial.println("Bluetooth device active, waiting for connections...");
#endif
}


void loop() {
  BLEDevice central = BLE.central();

  if (central)
  {
    // Only send data if we are connected to a central device
    while (central.connected()) {
      // Read the data from the IMU
      getIMUData();

      if (canSendData && isValidLabel(label)) {
        showYellowLight();

        // Send the sensor values to the central device
        sendIMUData();
      } else {
        showGreenLight();
      }
    }
  } else {
    showRedLight();
  }
}


/*
    BLUETOOTH
*/
void startBLE() {
  if (!BLE.begin())
  {
    Serial.println("starting BLE failed!");
    while (1);
  }
}

void onGetData(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic, update LED
  Serial.println("Received Command");

  byte test[256];
  int dataLength = readChar.readValue(test, 256);

  String command = "";

  for (int i = 0; i < dataLength; i++) {
    command += (char)test[i];
  }

  if(command == "STOP" || isValidLabel(command)) {
    blinkLight("blue");
  }

  // Parse command
  if (command == "STOP") {
    Serial.println("Stopping to record");

    reset();
  } else if (isValidLabel(command)) {
    Serial.print("Starting to record ");
    Serial.println(command);

    canSendData = true;
    label = String(command);
  } else {
    Serial.print("Invalid command: ");
    Serial.println(command);

    showErrorLight();
  }
}

void sendIMUData() {
  // Send data in format: xAcc,yAcc,zAcc,label
  String dataString = String(xAcc) + "," + String(yAcc) + "," + String(zAcc) + "," + String(label);

  char data[dataString.length() + 1];
  dataString.toCharArray(data, dataString.length() + 1);

  writeChar.writeValue(data);
}

void onBLEConnected(BLEDevice central) {
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
  showGreenLight();
}

void onBLEDisconnected(BLEDevice central) {
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
  reset();
  showRedLight();
}


/*
   IMU
*/
void startIMU() {
  // initialize IMU
  if (!IMU.begin()) {
    Serial.println("Failed to start IMU!");
    while (1);
  }
}

void getIMUData() {
  if (IMU.accelerationAvailable()) {
    IMU.readAcceleration(xAcc, yAcc, zAcc);
  }
}


/*
   LEDS
*/
void showGreenLight() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, HIGH);
}

void showYellowLight() {
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, LOW);
  digitalWrite(LEDB, HIGH);
}

void showRedLight() {
  digitalWrite(LEDR, LOW);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
}

void showBlueLight() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, LOW);
}

void blinkLight(String color) {
  if (color == "green") {
    showGreenLight();
  } else if (color == "yellow") {
    showYellowLight();
  } else if (color == "blue") {
    showBlueLight();
  } else {
    showErrorLight();
  }
  
  delay(1000);
}

void showErrorLight() {
  // Blink the red light three times
  showRedLight();

  delay(250);
  hideAllLight();
  delay(250);

  showRedLight();

  delay(250);
  hideAllLight();
  delay(250);

  showRedLight();

  delay(250);
}

void hideAllLight() {
  digitalWrite(LEDR, HIGH);
  digitalWrite(LEDG, HIGH);
  digitalWrite(LEDB, HIGH);
}


/*
   HELPERS
*/
bool isValidLabel(String testLabel) {
  for (int i = 0; i < POSSIBLE_LABELS_LENGTH; i++) {
    if (possibleLabels[i] == testLabel) {
      return true;
    }
  }
  return false;
}

void reset() {
  canSendData = false;
  label = "UNDEFINED";
}
