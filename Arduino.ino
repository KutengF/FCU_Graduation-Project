#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <IRremote.h>
#include <arduino-timer.h>

auto timer = timer_create_default(); // create a timer with default settings

//Auto Control settings
const float maxTemp = 32.5;
const float minTemp = 31.7;
const int ldrOn = 500; //Original 800
const int ldrOff = 750; //Original 950

//Define PIN constant
const int RELAY_PIN_1 = 2;
const int RELAY_PIN_2 = 3;
const int RELAY_PIN_3 = 4;
const int RELAY_PIN_4 = 5;
const int RELAY_PIN_5 = 6;

const int mswitch_1 = 7;
const int mswitch_2 = 8;
const int mswitch_3 = 9;
const int mswitch_4 = 10;
const int mswitch_5 = 11;
const int cmode = 12;
const int smode = 13;
int RECV_PIN = A0; // Pin connected to the IR Receiver
int LDR_PIN = A2;  // Pin connected to the LDR 
int DHTPIN  = A1;  // Pin connected to the DHT sensor 

// Uncomment the type of sensor in use:
#define DHTTYPE    DHT11     // DHT 11

//Define IR receiver and Result Objects
IRrecv irrecv(RECV_PIN);
decode_results results;


#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


DHT dht(DHTPIN, DHTTYPE);

char toggleState_1 = 'a'; //Define integer to remember the toggle state for switch 1
char toggleState_2 = 'b'; //Define integer to remember the toggle state for switch 2
char toggleState_3 = 'c'; //Define integer to remember the toggle state for switch 3
char toggleState_4 = 'd'; //Define integer to remember the toggle state for switch 4
char toggleState_5 = 'e'; //Define integer to remember the toggle state for switch 5

String pinStatus = "abcde000.0000.00";

int switchMode = 0;
int ModeFlag = 0;
float temperatureRead;
float humidityRead;
float ldrVal;
String displayText;

void relayOnOff(int relay){

    switch(relay){
      case 1: 
             if(toggleState_1 == 'a'){
              digitalWrite(RELAY_PIN_1, HIGH); // turn on relay 1
              toggleState_1 = 'A';
              }
             else{
              digitalWrite(RELAY_PIN_1, LOW); // turn off relay 1
              toggleState_1 = 'a';
              }
      break;
      case 2: 
             if(toggleState_2 == 'b'){
              digitalWrite(RELAY_PIN_2, HIGH); // turn on relay 2
              toggleState_2 = 'B';
              }
             else{
              digitalWrite(RELAY_PIN_2, LOW); // turn off relay 2
              toggleState_2 = 'b';
              }
      break;
      case 3: 
             if(toggleState_3 == 'c'){
              digitalWrite(RELAY_PIN_3, HIGH); // turn on relay 3
              toggleState_3 = 'C';
              }
             else{
              digitalWrite(RELAY_PIN_3, LOW); // turn off relay 3
              toggleState_3 = 'c';
              }
      break;
      case 4: 
             if(toggleState_4 == 'd'){
              digitalWrite(RELAY_PIN_4, HIGH); // turn on relay 4
              toggleState_4 = 'D';
              }
             else{
              digitalWrite(RELAY_PIN_4, LOW); // turn off relay 4
              toggleState_4 = 'd';
              }
      break;
      case 5: 
             if(toggleState_5 == 'e'){
              digitalWrite(RELAY_PIN_5, HIGH); // turn on relay 5
              toggleState_5 = 'E';
              }
             else{
              digitalWrite(RELAY_PIN_5, LOW); // turn off relay 5
              toggleState_5 = 'e';
              }
      break;
      default : break;      
      } 
      delay(200);
}
void changeMode(){      
  delay(200);
  if (switchMode == 0){
    switchMode = 1;
  }
  else if (switchMode == 1) {
    switchMode = 0;
  }
  //Serial.print("SwitchMode changed to: ");
  //Serial.println(switchMode);
}
String modeDecode(int count){
  if (count == 0){
    return " Manual Mode ";
  }
  else if (count == 1){
    return " Auto Mode ";
  }
}

void readSensor(){
    
  ldrVal = analogRead(LDR_PIN);
  
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  
  if (isnan(h) || isnan(t)) {
    //Serial.println("Failed to read from DHT sensor!");
    humidityRead = 0;
    temperatureRead = 0;
    return;
  }
  else {
    humidityRead = h;
    temperatureRead = t;
  }  
}

void displayData(){
  readSensor();
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(8,2);
  display.print(temperatureRead);
  display.print(" ");
  display.print("C    ");
  display.print(humidityRead); // display humidity
  display.print(" %");
  
  display.drawLine(0,18, display.width()-1,18, WHITE);
   
  display.setCursor(15,24);
  display.print(modeDecode(switchMode));   
  display.display();
}

void sendStatus(){  
  displayData();
  //Serial.print("PIN Status: ");
  //Serial.println(pinStatus);
  Serial.print("FromArduino:");
  Serial.print(pinStatus);
  Serial.print("\n");
  //Serial.print("LDR Value: ");
  //Serial.println(ldrVal, 2);  // 2 decimal places
}

void updateStatesFromServer(String message) {
  // Example format: "AbCDe132.3030.20"
  // Extracting each piece of information
  toggleState_1 = message[0];
  toggleState_2 = message[1];
  toggleState_3 = message[2];
  toggleState_4 = message[3];
  toggleState_5 = message[4];
  switchMode = message[5] - '0'; // Convert char '0' or '1' to int 0 or 1
  temperatureRead = message.substring(6, 11).toFloat();
  humidityRead = message.substring(12, 17).toFloat();

  // Update LEDs based on the states
  if (toggleState_1 == 'A') {
      digitalWrite(RELAY_PIN_1, HIGH);
  } else if (toggleState_1 == 'a') {
      digitalWrite(RELAY_PIN_1, LOW);
  }

  if (toggleState_2 == 'B') {
      digitalWrite(RELAY_PIN_2, HIGH);
  } else if (toggleState_2 == 'b') {
      digitalWrite(RELAY_PIN_2, LOW);
  }

  if (toggleState_3 == 'C') {
      digitalWrite(RELAY_PIN_3, HIGH);
  } else if (toggleState_3 == 'c') {
      digitalWrite(RELAY_PIN_3, LOW);
  }

  if (toggleState_4 == 'D') {
      digitalWrite(RELAY_PIN_4, HIGH);
  } else if (toggleState_4 == 'd') {
      digitalWrite(RELAY_PIN_4, LOW);
  }

  if (toggleState_5 == 'E') {
      digitalWrite(RELAY_PIN_5, HIGH);
  } else if (toggleState_5 == 'e') {
      digitalWrite(RELAY_PIN_5, LOW);
  }

}

void setup() {
  Serial.begin(9600);
  irrecv.enableIRIn(); // Enable the IR receiver
  
  pinMode(RELAY_PIN_1, OUTPUT);
  pinMode(RELAY_PIN_2, OUTPUT);
  pinMode(RELAY_PIN_3, OUTPUT);
  pinMode(RELAY_PIN_4, OUTPUT);
  pinMode(RELAY_PIN_5, OUTPUT);

  pinMode(smode, INPUT);
  pinMode(cmode, INPUT);

  pinMode(mswitch_1, INPUT);
  pinMode(mswitch_2, INPUT);
  pinMode(mswitch_3, INPUT);
  pinMode(mswitch_4, INPUT);
  pinMode(mswitch_5, INPUT);
 
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Address 0x3C for 128x32
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay(); // Clear display buffer
  dht.begin();

  // call the toggle_led function every 7500 millis (7.5 seconds)
  timer.every(7500, sendStatus);  
}

void loop() {
  
  if (digitalRead(cmode) == HIGH){ //change the Mode (Manual or Auto)
    //Serial.println("Cmode button pressed");
    changeMode();
  }
  else if (digitalRead(smode) == HIGH){
    digitalWrite(RELAY_PIN_1, LOW);  //Turn Off Relay 1
    toggleState_1 = 'a';
    digitalWrite(RELAY_PIN_2, LOW);  //Turn Off Relay 2
    toggleState_2 = 'b';
    digitalWrite(RELAY_PIN_3, LOW);  //Turn Off Relay 3
    toggleState_3 = 'c';
    digitalWrite(RELAY_PIN_4, LOW);  //Turn Off Relay 4
    toggleState_4 = 'd';
    digitalWrite(RELAY_PIN_5, LOW);  //Turn Off Relay 5
    toggleState_5 = 'e';
    switchMode = 0;
    pinStatus = ""; // Start with an empty string

    pinStatus += toggleState_1;
    pinStatus += toggleState_2;
    pinStatus += toggleState_3;
    pinStatus += toggleState_4;
    pinStatus += toggleState_5;

    pinStatus += String(switchMode);

    pinStatus += String(temperatureRead, 2);   // 2 decimal places
    pinStatus += String(humidityRead, 2);      // 2 decimal places

    //pinStatus = String(toggleState_1) + String(toggleState_2) + String(toggleState_3) + String(toggleState_4) + String(toggleState_5) + String(switchMode) + String(temperatureRead) + String(humidityRead);
  }     
  else{

    if (switchMode == 0){  // Manual Mode
      //Push Button Control
      if (digitalRead(mswitch_1) == HIGH){
        delay(200);
        relayOnOff(1);      
      }
      else if (digitalRead(mswitch_2) == HIGH){
        delay(200);
        relayOnOff(2);
      }
      else if (digitalRead(mswitch_3) == HIGH){
        delay(200);
        relayOnOff(3);
      }
      else if (digitalRead(mswitch_4) == HIGH){
        delay(200);
        relayOnOff(4);
      }
      else if (digitalRead(mswitch_5) == HIGH){
        delay(200);
        relayOnOff(5);
      }
   //WiFi Control
      if (Serial.available() > 0) {
        String message = Serial.readStringUntil('\n');
        if (message.length() == 17) { // Check if message length matches expected format
          updateStatesFromServer(message);
        }
        delay(500);
      }

     //IR Control
     if (irrecv.decode(&results)) {
      switch(results.value){
        case 0x80BF49B6: break; //relayOnOff(1);
        case 0x80BFC936: break; //relayOnOff(2);
        case 0x80BF33CC: break; //relayOnOff(3);
        case 0x80BF718E: break; //relayOnOff(4);
        case 0x80BFF10E: break; //relayOnOff(5);
        default : break;      
        }
      irrecv.resume(); // Receive the next value
      }
    }
    else if (switchMode == 1){  //Auto Mode
     // Serial.println(ldrVal);
      //LDR Control
      if (ldrVal < ldrOn){
          if(toggleState_3 == 'c'){
            digitalWrite(RELAY_PIN_3, HIGH); // turn on relay 3
            toggleState_3 = 'C';
          }
          if(toggleState_4 == 'd'){
            digitalWrite(RELAY_PIN_4, HIGH); // turn on relay 4
            toggleState_4 = 'D';
          }
      }
      else if (ldrVal >= ldrOff){
        if(toggleState_3 == 'C'){
            digitalWrite(RELAY_PIN_3, LOW); // turn off relay 3
            toggleState_3 = 'c';
          }
        if(toggleState_4 == 'D'){
            digitalWrite(RELAY_PIN_4, LOW); // turn off relay 4
            toggleState_4 = 'd';
          }
      }
      //DHT11 Control
      if (temperatureRead >= maxTemp){
        if(toggleState_1 == 'a'){
          digitalWrite(RELAY_PIN_1, HIGH); // turn on relay 1
          toggleState_1 = 'A';
        }
        else if(toggleState_2 == 'b'){
          digitalWrite(RELAY_PIN_2, HIGH); // turn on relay 2
          toggleState_2 = 'B';
        }
      }
      else if (temperatureRead < minTemp){
        if(toggleState_1 == 'A'){
            digitalWrite(RELAY_PIN_1, LOW); // turn on relay 1
            toggleState_1 = 'a';
          }
        else if(toggleState_2 == 'B'){
            digitalWrite(RELAY_PIN_2, LOW); // turn on relay 1
            toggleState_2 = 'b';
         }
       }
     }
     pinStatus = ""; // Start with an empty string
    
     pinStatus += toggleState_1;
     pinStatus += toggleState_2;
     pinStatus += toggleState_3;
     pinStatus += toggleState_4;
     pinStatus += toggleState_5;

     pinStatus += String(switchMode);

     pinStatus += String(temperatureRead, 2);   // 2 decimal places
     pinStatus += String(humidityRead, 2);      // 2 decimal places

     //pinStatus = String(toggleState_1) + String(toggleState_2) + String(toggleState_3) + String(toggleState_4) + String(toggleState_5) + String(switchMode) + String(temperatureRead) + String(humidityRead);
   }

  timer.tick(); // tick the timer
}

