#include <BLEDevice.h>
#include <BLE2902.h>
#include <BLEServer.h>
#include <BLEUtils.h>

//Change for motor and sensor 

//constants for pump motor
const int A1A = 15; // define pin 3 for A-1A (PWM Speed)
const int A1B = 2;
const int Sensor = 4;

//constants for sensor
int SensorValue ;
int tx_pin ;

// constants for bluetooth 
//bool deviceConnected = false;
bool IsConnected = false ;
const char* txValue = "";


BLECharacteristic *pCharacteristic;
BLECharacteristic *pCharacteristictx;




// change function names after based on app changes
//update UUID in app 
#define SERVICE_UUID           "664dea13-4d0c-47cb-a31f-2f8cae0d5e4f" // UART service UUID
#define CHARACTERISTIC_UUID "b661a339-fce2-4672-87a3-122ecef4f531" //randomly generated
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "55d2025d-cf99-4f43-bfe2-ed086454b22d"

// Values received from APP
class MyCallbacks: public BLECharacteristicCallbacks{   
  void onWrite(BLECharacteristic *pCharacteristic) {
    Serial.println("yay");
    std::string rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      Serial.println("*********");
      Serial.print("Received Value: ");  
            
      for (int i = 0; i < rxValue.length(); i++) {
        Serial.print(rxValue[i]);
      }
      Serial.println();     // Do stuff based on the command received from the app
    
      //TURN PUMP on and off based off button
      if (rxValue.find("A") != -1) { 
                Serial.print("Turning ON!");
                digitalWrite(A1A, HIGH);
                digitalWrite(A1B, LOW);
              }
      else if (rxValue.find("B") != -1) {
                Serial.print("Turning OFF!");
                digitalWrite(A1A, LOW);
                digitalWrite(A1B, LOW);
              }
              Serial.println();
              Serial.println("*********");
    }
  }
};

void setup() {
  pinMode(tx_pin, OUTPUT);      // Pin 4 provides the voltage step
  pinMode (A1A, OUTPUT);
  pinMode (A1B, OUTPUT);
  
  
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("LaylaESP32");
  BLEServer *pServer = BLEDevice::createServer();
  BLEService *pService = pServer->createService(SERVICE_UUID);

  //Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                                       CHARACTERISTIC_UUID_RX,
                                       BLECharacteristic::PROPERTY_WRITE |  BLECharacteristic::PROPERTY_WRITE_NR
                                     );
  pCharacteristictx = pService->createCharacteristic(
                    CHARACTERISTIC_UUID_TX,
                   BLECharacteristic::PROPERTY_NOTIFY
                  );
  pCharacteristic->addDescriptor(new BLE2902());
  
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristictx->addDescriptor(new BLE2902());
  
  pCharacteristictx->setCallbacks(new MyCallbacks());
  pService->start();
  BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  //Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");


//IF phone sense information
// set my server call backs 
class MyServerCallbacks: public BLEServerCallbacks {  
  void onConnect(BLEServer* pServer) {
      IsConnected = true;
      Serial.println("connected");
    };
  void onDisconnect(BLEServer* pServer) {
      IsConnected = false;
      Serial.println("Not connected");
    }
};
   pServer->setCallbacks(new MyServerCallbacks()); 
}

void loop() {
  //std::string rxValue = pCharacteristic->getValue();
  //Serial.println(rxValue.c_str());
  //Serial.println("ah");

  int SensorVal = analogRead(Sensor);
  Serial.println(SensorVal);
 
//
 if (IsConnected) {
    // Reading water level results into app
    if (SensorVal < 1090) {
    txValue = "0"; }
    else if (SensorVal > 1090 && SensorVal < 1300) {
    txValue = "1";}
    else if (SensorVal > 1300) {
    txValue = "2";}
    
    // Let's convert the value to a char array:
    //char txString; // make sure this is big enuffz
    //dtostrf(txString); // float_val, min_width, digits_after_decimal, char_buffer
////    
 //pCharacteristic->setValue(&txValue, 1); // To send the integer value
////  pCharacteristic->setValue("Hello!"); // Sending a test message
    pCharacteristictx->setValue(txValue);
//    
    pCharacteristictx->notify(); // Send the value to the app!
    Serial.print("*** Sent Message: ");
    Serial.println(txValue);
    Serial.println(" ***");
//  }


  delay(1000);
} else {Serial.println("no connected");}
}

//Things I want to add if time (later)
// Be able to customize what low, medium, high values are in app
// show last time container was emptied in app
// First assemble system 
// Input: diameter and height of containter - does math so it takes bottom 30% as turn pump off - 40-70% as medium, 80% above as top
// calculation necessary - 
// write code for it tonight 
//
