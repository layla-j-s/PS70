#include <esp_now.h>
#include <WiFi.h>
//Bellow I define the pins I will use for the RGB LED and the Buttons
#define BUTTON_PIN_1 21  // GIOP21 pin connected to button
#define BUTTON_PIN_2 22
#define BUTTON_PIN_3 19
#define PIN_RED    18 // GIOP23
#define PIN_GREEN  23 // GIOP22
#define PIN_BLUE   5 // GIOP21


// REPLACE WITH YOUR RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xC8, 0xF0, 0x9E, 0x47, 0xE8, 0x54};

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  bool d; //We only needed a boolean to be sent
} struct_message;

// Create a struct_message called myData
struct_message myData;

esp_now_peer_info_t peerInfo;

int Button1;
int Button2;
int Button3;

// I initiate the "last" and "current" states of the three buttons, with the last always being LOW at the beggining of the program
int lastState1 = LOW;
int lastState2 = LOW;
int lastState3 = LOW;
int currentState1;
int currentState2;
int currentState3;

// The arrays are initialised here
int order[] = {0, 0, 0}; //This array is initialised with zeroes but will contain the current combination of buttons
int right_order[] = {2,3,1}; //This array contains the right combination
int place = 0; //This is an index/counter that is incemented whenever a button is pressed
int LorU = 0; //This is a variable that stores whether the previous state of the solenoid was LOW or HIGH

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Transmission initiation code
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}


void setup() {
  // initialize serial communication at 115200 bits per second:
  Serial.begin(115200);
  //I initiate the Button pins as inputs with internal PULLUP resistors
  pinMode(BUTTON_PIN_1, INPUT_PULLUP);
  pinMode(BUTTON_PIN_2, INPUT_PULLUP);
  pinMode(BUTTON_PIN_3, INPUT_PULLUP);
  //I initialise the RGB pins as outputs
  pinMode(PIN_RED, OUTPUT);
  pinMode(PIN_GREEN, OUTPUT);
  pinMode(PIN_BLUE, OUTPUT);
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  // Register peer
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
}
}
void loop() {
  // read the state of all the buttons
  currentState1 = digitalRead(BUTTON_PIN_1);
  currentState2 = digitalRead(BUTTON_PIN_2);
  currentState3 = digitalRead(BUTTON_PIN_3);
  //The if statement only acts when the button is released, not pressed - specifically for button 1
  if(lastState1 == HIGH && currentState1 == LOW){
    Serial.println("The button 1 was pressed"); //For debugging - I print the button that was pressed
    //RGB LED goes blue because a button was pressed
    analogWrite(PIN_RED, 0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE,  255);
    delay(500); //The LED stays blue for half a second
    analogWrite(PIN_BLUE,  0); //Then it stops
    order[place] = 1; //The number of button pressed (1) is stored in the array
    place = place + 1; //The "place" counter is incremented, as another button was pressed
  }
  else if(lastState2 == HIGH && currentState2 == LOW){
    Serial.println("The button 2 was pressed"); //For debugging - I print the button that was pressed
    //RGB LED goes blue because a button was pressed
    analogWrite(PIN_RED,   0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE,  255);
    delay(500); //The LED stays blue for half a second
    analogWrite(PIN_BLUE,  0); //Then it stops
    order[place] = 2; //The number of button pressed (2) is stored in the array
    place = place + 1; //The "place" counter is incremented, as another button was pressed
    }
  else if(lastState3 == HIGH && currentState3 == LOW){
    Serial.println("The button 3 was pressed"); //For debugging - I print the button that was pressed
    //RGB LED goes blue because a button was pressed
    analogWrite(PIN_RED,   0);
    analogWrite(PIN_GREEN, 0);
    analogWrite(PIN_BLUE,  255);
    delay(500); //The LED stays blue for half a second
    analogWrite(PIN_BLUE,  0); //Then it stops
    order[place] = 3; //The number of button pressed (3) is stored in the array
    place = place + 1; //The "place" counter is incremented, as another button was pressed
    }
  
  // As this is a loop and the next iteration is about to start, the last state of the button assigned the value of the current one for all three buttons
  lastState1 = currentState1;
  lastState2 = currentState2;
  lastState3 = currentState3;

  // When the counter "place" is equal to three, three buttons were pressed already, which means we can check wheather the order is correct
  if (place == 3){
    //We check if all elements of the current array match those of the correct array
    if (order[0] == right_order[0] && order[1] == right_order[1] && order[2] == right_order[2]){ //If it's correct
      Serial.println("Correct combination!"); // We print that the combination was correct (for debugging purposes)
      //We make the RGB LED green for half a second
      analogWrite(PIN_RED,   0);
      analogWrite(PIN_GREEN, 255);
      analogWrite(PIN_BLUE,  0);
      delay(500); 
      analogWrite(PIN_GREEN,  0);
      LorU = LorU + 1; //We increment the variable that counts whenever a signal is sent 
      if (LorU % 2 == 1){ //If the variable is odd
      myData.d = false; // The data we will send will be "false" - which opens the solenoid
      }
      else{
        myData.d = true; // If the variable is even, it sends "true" - which locks the solenoid
        }
      // Send the boolean via ESP-NOW
      esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
      //I then re-initiate the array - "empty" out all elements of the array
      order[0] = 0;
      order[1] = 0;
      order[2] = 0;
      place = 0; // The index/counter "place" is back at the beggining
      
      }
     else{ //If the combination is not correct
      // We print that the combination was incorrect (for debugging purposes)
      Serial.println("Failed combination!");
      //We then make the RGB LED Red for half a second
      analogWrite(PIN_RED,   255);
      analogWrite(PIN_GREEN, 0);
      analogWrite(PIN_BLUE,  0);
      delay(500);
      analogWrite(PIN_RED,  0);
      //I then re-initiate the array - "empty" out all elements of the array
      order[0] = 0;
      order[1] = 0;
      order[2] = 0;
      place = 0; // The index/counter "place" is back at the beggining
      }
    }
  }
