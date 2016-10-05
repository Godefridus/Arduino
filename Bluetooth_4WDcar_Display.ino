/*
Bluetooth_4WDcar_Display.ino
Follows from Bluetooth_4WDcar.ino
Added I2C LCD to display speed, see
https://arduino-info.wikispaces.com/LCD-Blue-I2C
Supports full 4WD using second L298N motor controller. 
Uses revcounter module HCMODU0025 with added debounce hardware
(CD40106BC Schmitt trigger, tau = R * C = 10k * 100nF = 1 msec)

Based on Mega2560 and Adafruit's nRF8001 Bluetooth LE breakout:
https://learn.adafruit.com/getting-started-with-the-nrf8001-bluefruit-le-breakout
This works with the Bluefruit LE app on iPhone and iPad.
StandardFirmata sketch has been modified to report back
which Control Pad key is touched. Keys are used to
control 4 motors on the robot car: 

Key '1' changes gear to NEUTRAL
Key '2' changes gear to LOW SPEED
Key '3' changes gear to MEDIUM SPEED
Key '4' changes gear to HIGH SPEED
Key 'Up' turns wheels forward
Key 'Down' turns wheels backward
Key 'Left' sets lower turnspeed on left wheels
Key 'Right' sets lower turnspeed on right wheels

Key codes received from Bluetooth device are in HEX:
Key 1:     21 42 31 31 3A 21 42 31 30 38
Key 2:     21 42 32 31 39 21 42 32 30 3A
Key 3:     21 42 33 31 38 21 42 33 30 39
Key 4:     21 42 34 31 37 21 42 34 30 38
Key Up;    21 42 35 31 36 21 42 35 30 37
Key Down:  21 42 36 31 35 21 42 36 30 36
Key Left:  21 42 37 31 34 21 42 37 30 35
Key Right: 21 42 38 31 33 21 42 38 30 34
Several parts of the code are identical but the third 
number identifies a specific key for follow-up action.
Note that in this sketch the decimal equivalent of the
HEX numbers is used.

PINOUTS:
Pins 2, 9, 10, 50, 51 and 52: Bluetooth breakout
(on Mega: CLK = 52, MISO = 50 and MOSI = 51)
Pins 33, 35, 37 and 39: rear u/s module (5V to 39, GND to 33)
Pins 11 and 12: front u/s module
Pins 7 and 8: left and right blinking LEDs (using 560 ohm resistor)
Pin 6: speaker (horn) using 220 ohm resistor
Pin 18: interrupt from revcounter

Pin 3: ENA (PWM) orange: front right motor (RF)
Pin 24: IN1 red: front right motor (+)
Pin 26: IN2 brown: front right motor
Pin 25: IN3 black: rear right motor (+)
Pin 27: IN4 white: rear right motor (RR)
Pin 46: ENB (PWM) grey: rear right motor
Pin 44: ENB (PWM) red: rear left motor
Pin 42: IN4 brown: rear left motor (LR)
Pin 40: IN3 black: rear left motor(+)
Pin 41: IN2 white: front left motor (LF)
Pin 43: IN1 grey: front left motor (+)
Pin 45: ENA (PWM) purple: front left motor

The horn sounds when the vehicle is reversing, The LEDs blink
when vehicle is turning. 5V supply for the Mega is taken from
a stepper motor controller

   Wim der Kinderen  1 Mar 2016
*/

#include <Wire.h>                // required for I2C display
#include <SPI.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_BLE_Firmata.h>  // Bluetooth library
#include <Adafruit_BLE_UART.h>

#define AUTO_INPUT_PULLUPS true // switch on internal pull-up resistors
 
#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2   // must be an interrupt pin
#define ADAFRUITBLE_RST 9

Adafruit_BLE_UART BLEserial = Adafruit_BLE_UART(ADAFRUITBLE_REQ, 
                                  ADAFRUITBLE_RDY, ADAFRUITBLE_RST);
// make one instance for the user
Adafruit_BLE_FirmataClass BLE_Firmata(BLEserial);

// Set the LCD I2C address and pins for the LCM1602-LCD connection
// (addr, en,rw,rs,d4,d5,d6,d7,bl,blpol)
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE); 

// Motor RF (right front wheel)
#define dir1PinRF 24  // (+) IN/OUT1 Red: forward HIGH, backward LOW
#define dir2PinRF 26  // (-) IN/OUT2: Brown: forward LOW, backward HIGH
#define speedPinRF 3  // Orange: PWM pin to control motor speed

// Motor LF (left front wheel)
#define dir1PinLF 43  // (+) IN/OUT3 Grey : forward HIGH, backward LOW
#define dir2PinLF 41  // (-) IN/OUT4 (-) White: forward HIGH, backward LOW
#define speedPinLF 45 // Purple: PWM pin to control motor speed

// Motor LR (left rear wheel)
#define dir1PinLR 40  // (+) IN/OUT1 Black: forward HIGH, backward LOW
#define dir2PinLR 42  // (-) IN/OUT2: Brown: forward LOW, backward HIGH
#define speedPinLR 44 // Red: PWM pin to control motor speed

// Motor RR (right rear wheel)
#define dir1PinRR 25  // (+) IN/OUT3 Black : forward HIGH, backward LOW
#define dir2PinRR 27  // (-) IN/OUT4 (-) White: forward HIGH, backward LOW
#define speedPinRR 46 // Grey: PWM pin to control motor speed

#define frontTrigPin 12  // front u/s module transmitter
#define frontEchoPin 11  // front u/s module receiver
#define rearTrigPin 37
#define rearEchoPin 35
#define powerPin 39   // 5V for rear u/s module taken from digital pin
#define groundPin 33
#define horn 6        // this is the reversing buzzer   

int gearOne = 80;        // PWM settings to control speed
int gearTwo = 100;
int gearThree = 150;
int gear = 0;
int trigPin, echoPin;

int turnspeedLow = 70;   // different wheel speeds when turning
int turnspeedHigh = 130;

const int leftLED = 8;     // left blinker
const int rightLED = 7;    // right blinker

unsigned long duration, distance, currentMillis, previousMillis = 0; 
const int minDistance = 20;    // user-defined u/s sensor limits
const int maxDistance = 150;

int BLEdata;           // Bluetooth data
int count = 0;         // counter for Bluetooth key codes
int blinkcounter = 0;  // counter for left/right LEDs and horn
float revCounter = 0;  // counter for wheel revs
int interval = 1;      // display update interval in seconds
char dir = 'N';        // direction N(eutral), F(orward), R(everse), 
                          // L(eft), R(ight)
boolean rBlink = true;
boolean lBlink = true;

aci_evt_opcode_t lastBTLEstatus, BTLEstatus; // status code Nordic chip

/* ------------------------ SETUP ---------------------- */
void setup() {
  //Define L298N Dual H-Bridge Motor Controller Pins
  pinMode(dir1PinRF, OUTPUT);
  pinMode(dir2PinRF, OUTPUT);
  pinMode(dir1PinLF, OUTPUT);
  pinMode(dir2PinLF, OUTPUT);
  pinMode(dir1PinLR, OUTPUT);
  pinMode(dir2PinLR, OUTPUT);
  pinMode(dir1PinRR, OUTPUT);
  pinMode(dir2PinRR, OUTPUT);  
  pinMode(speedPinRF, OUTPUT);
  pinMode(speedPinLF, OUTPUT); 
  pinMode(speedPinLR, OUTPUT);
  pinMode(speedPinRR, OUTPUT); 
   
  pinMode(leftLED, OUTPUT);
  pinMode(rightLED, OUTPUT);
  pinMode(powerPin, OUTPUT);     // rear u/s module power pin
  pinMode(groundPin, OUTPUT);
  digitalWrite(powerPin, HIGH);
  digitalWrite(groundPin, LOW); 
  digitalWrite(speedPinRF, 0);
  digitalWrite(speedPinLF, 0);
  digitalWrite(speedPinLR, 0);
  digitalWrite(speedPinRR, 0);

  Serial.begin(9600);  // use serial line for debugging 
  lcd.begin(16,2);   // initialize LCD for 16 chars 2 lines
  lcd.backlight();   // turn backlight on   
  lcd.setCursor(0,0); 
  lcd.print("Wim's Robot Car"); 
  Serial.println(F("Bluefruit LE Robot Car Controller")); 
  BLEserial.begin();  
  BTLEstatus = lastBTLEstatus = ACI_EVT_DISCONNECTED;
  
  // increase revcounter when speed pin (18) goes HIGH
  attachInterrupt(5, docount, RISING);  // Pin 18 = interrupt 5
}

/* ------------------------ MAIN LOOP ---------------------- */
void loop() {  
  BLEserial.pollACI(); // check the BTLE link
  BTLEstatus = BLEserial.getState();  // Link status check
  if (BTLEstatus != lastBTLEstatus) { // Check if something has changed
    if (BTLEstatus == ACI_EVT_DEVICE_STARTED) {     // print it out!
        Serial.println(F("* Switch iPhone/iPad Bluetooth ON"));
        Serial.println(F("* Select BLuefruit LE app, then"));
        Serial.println(F("* connect to UART device and"));
        Serial.println(F("* select Controller"));
    }
    if (BTLEstatus == ACI_EVT_CONNECTED) {
        Serial.println(F("* Bluetooth device connected"));
        Serial.println(F("* Select Control Pad"));
        BLE_Firmata.begin();
    }
    if (BTLEstatus == ACI_EVT_DISCONNECTED) {
        Serial.println(F("* Disconnected or device timed out"));
    }   
    lastBTLEstatus = BTLEstatus; // OK reset the last status change
  }
  if (BTLEstatus != ACI_EVT_CONNECTED) {  // if not connected... bail
    delay(100);
    return;
  }  
  // For debugging, see if there's data on the serial console, 
         // and forward it to BLE
  if (Serial.available()) {
    BLEserial.write(Serial.read());
  }  
  // check if data has arrived, then call process module      
  if (BLE_Firmata.available())  {
    BLEdata = BLE_Firmata.processInput();  // pick up Bluetooth input
//    Serial.print(F("  BLEdata =  "));  // print for debugging         
//    Serial.print(BLEdata);   
//    Serial.print(F("  count =  "));             
//    Serial.println(count);
    count++ ;
    if (count == 3) {  // third number holds control key data 
      Serial.print(F(" Button "));
      if (BLEdata == 49) {                // decimal 49 = HEX 31        
        Serial.println(F("1: Stop"));
        gear = 0;
        caseZero();   
      }
      if (BLEdata == 50) {
        Serial.println(F("2: First Gear"));
        gear = gearOne; 
        caseForward();
      }
      if (BLEdata == 51) {
        Serial.println(F("3: Second Gear"));
        gear = gearTwo;
        caseForward();  
      }     
      if (BLEdata == 52) {
        Serial.println(F("4: Third Gear"));
        gear = gearThree;
        caseForward(); 
      }                
      if (BLEdata == 53) {
        Serial.println(F("Up")); //control forward
        caseForward();
      }  
      if (BLEdata == 54) {
        Serial.println(F("Down")); //control reverse
        caseReverse();
      }
      if (BLEdata == 55) {
        Serial.println(F("Left"));
        caseLeft();
      }
      if (BLEdata == 56) {
        Serial.println(F("Right"));
        caseRight();
      }
      delay(100);
    }
  }
  // reset counter after processing (note '!' character)
  if (!BLE_Firmata.available()) count = 0; 
  
  if (dir == 'R') {  // turning Right
    digitalWrite(leftLED, LOW);  // switch off left LED
    frontPulse();                // find distance
    if (distance < minDistance) caseReverse();  
    blinkcounter--;
    if (blinkcounter < 0) {   
      if (rBlink) digitalWrite(rightLED, HIGH); 
      else digitalWrite(rightLED, LOW);
      blinkcounter = 20;        // flash right LED
      rBlink = !rBlink;
    }
  }
  if (dir == 'L')  {   // turning Left
    digitalWrite(rightLED, LOW);  // switch off right LED
    frontPulse();
    if (distance < minDistance) caseReverse();
    blinkcounter--;
    if (blinkcounter < 0) {   
      if (lBlink) digitalWrite(leftLED, HIGH); 
      else digitalWrite(leftLED, LOW);
      blinkcounter = 20; // use blinkcounter to set blinking cycle
      lBlink = !lBlink;
    }
  }
  if (dir == 'F')  {   // going Forward 
    blinkcounter = 0; 
    digitalWrite(rightLED, LOW);  // switch off both LEDs
    digitalWrite(leftLED, LOW);
    frontPulse();
    if (distance < minDistance) caseReverse(); // reverse if too close
  }
  if (dir == 'B')  {   // going Backwards
    digitalWrite(rightLED, HIGH);  // switch on both LEDs
    digitalWrite(leftLED, HIGH);
    rearPulse();
    if (distance < minDistance) caseZero(); // stop if too close
    blinkcounter--;
    if (blinkcounter < 0) {   // use blinkcounter to repeatedly 
      tone(horn,1000,300);    // activate horn for 300 ms 
      blinkcounter = 20;      // with 1000 Hz tone
    }  
  } 
  if (dir == 'N' ) {   // Neutral
    blinkcounter = 0;
    lBlink = true;
    rBlink = true;
    digitalWrite(rightLED, LOW);
    digitalWrite(leftLED, LOW);    
  }
  currentMillis = millis();
  if (currentMillis - previousMillis > interval*1000) {
    updateDisplay();     // update display every interval
    previousMillis = millis();
    revCounter = 0;     //  reset revcounter to zero
  }
}         // end of loop

/* ------------------------ FUNCTIONS ---------------------- */
void caseZero() {
  dir = 'N';
  analogWrite(speedPinRF, 0);
  analogWrite(speedPinLF, 0);
  analogWrite(speedPinLR, 0);
  analogWrite(speedPinRR, 0); 
  digitalWrite(dir1PinRF, LOW);
  digitalWrite(dir2PinRF, LOW);
  digitalWrite(dir1PinLF, LOW);
  digitalWrite(dir2PinLF, LOW); 
  digitalWrite(dir1PinLR, LOW);
  digitalWrite(dir2PinLR, LOW);
  digitalWrite(dir1PinRR, LOW);
  digitalWrite(dir2PinRR, LOW);    
}
void caseForward() {
  dir = 'F';
  analogWrite(speedPinRF, gear);
  analogWrite(speedPinLF, gear); 
  analogWrite(speedPinLR, gear);
  analogWrite(speedPinRR, gear);  
  digitalWrite(dir1PinRF, HIGH);
  digitalWrite(dir2PinRF, LOW);
  digitalWrite(dir1PinLF, HIGH);
  digitalWrite(dir2PinLF, LOW);
  digitalWrite(dir1PinLR, HIGH);
  digitalWrite(dir2PinLR, LOW);
  digitalWrite(dir1PinRR, HIGH);
  digitalWrite(dir2PinRR, LOW);
}
void caseReverse() {
  dir = 'B';
  if (gear > 0) {  // only reverse when gear is selected
    analogWrite(speedPinRF, gearOne);
    analogWrite(speedPinLF, gearOne);
    analogWrite(speedPinLR, gearOne);
    analogWrite(speedPinRR, gearOne);
    digitalWrite(dir1PinRF, LOW);
    digitalWrite(dir2PinRF, HIGH);
    digitalWrite(dir1PinLF, LOW);
    digitalWrite(dir2PinLF, HIGH); 
    digitalWrite(dir1PinLR, LOW);
    digitalWrite(dir2PinLR, HIGH);
    digitalWrite(dir1PinRR, LOW);
    digitalWrite(dir2PinRR, HIGH);
  }
  else caseZero(); 
}
void caseLeft() {
  dir = 'L';
  if (gear > 0) {  // only turn when gear is selected
    analogWrite(speedPinRF, turnspeedHigh); // right wheels
    analogWrite(speedPinLF, turnspeedLow);  // faster
    analogWrite(speedPinLR, turnspeedLow);
    analogWrite(speedPinRR, turnspeedHigh);
    digitalWrite(dir1PinRF, HIGH);
    digitalWrite(dir2PinRF, LOW);
    digitalWrite(dir1PinLF, LOW);
    digitalWrite(dir2PinLF, HIGH);
    digitalWrite(dir1PinLR, LOW);
    digitalWrite(dir2PinLR, HIGH);
    digitalWrite(dir1PinRR, HIGH);
    digitalWrite(dir2PinRR, LOW);
  }
  else caseZero();
}
void caseRight() {
  dir = 'R';
  if (gear > 0) {
    analogWrite(speedPinRF, turnspeedLow);
    analogWrite(speedPinLF, turnspeedHigh); 
    analogWrite(speedPinLR, turnspeedHigh);
    analogWrite(speedPinRR, turnspeedLow); 
    digitalWrite(dir1PinRF, LOW);
    digitalWrite(dir2PinRF, HIGH);
    digitalWrite(dir1PinLF, HIGH);
    digitalWrite(dir2PinLF, LOW);
    digitalWrite(dir1PinLR, HIGH);
    digitalWrite(dir2PinLR, LOW);
    digitalWrite(dir1PinRR, LOW);
    digitalWrite(dir2PinRR, HIGH);
  }
  else caseZero();
}

/* ----------- MEASURE DISTANCE ---------------------- */
void frontPulse() { // activate front u/s module
  trigPin = frontTrigPin;
  echoPin = frontEchoPin;
  sendPulse();  
}
void rearPulse() {  // activate rear u/s module
  trigPin = rearTrigPin;
  echoPin = rearEchoPin;
  sendPulse();    
}
void sendPulse() {
  /* Sensor is triggered by a HIGH pulse of 10 or more microseconds;
  // give a short LOW pulse beforehand to ensure a clean HIGH pulse */
  if (gear > 0) {  // only activate u/s modules when moving
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    /* Read the signal from the sensor: a HIGH pulse whose
       duration is the time (in microseconds) from the sending
       of the ping to the reception of its echo off of an object */
    duration = pulseIn(echoPin, HIGH);  // in microseconds
    distance = duration/58;          // distance in cm
//    Serial.println(distance);
    if (distance > maxDistance) distance = maxDistance;
    delay(20);
  }
}

/* --------------- UPDATE DISPLAY ----------------------- */
void updateDisplay()
{
  Serial.print("Wheel speed: "); 
  // divide number of interrupts by holes in disc times interval
  float revs = (revCounter / (20 * interval));  
  Serial.print(revs,1);  
  Serial.println(" revs per second"); 
  // wheel diameter = 6.5 cm, one revolution = 20.5 cm travel
  //   speed = 0.036*travel*revs in km/h
  Serial.print("Car speed: "); 
  float carSpeed = 0.738 * revs; 
  if (dir == 'B') Serial.print("-");
  Serial.print(carSpeed,1);  
  Serial.println(" km/h"); 
  Serial.print("Ping distance: ");  
  Serial.print(distance);
  Serial.println(" cm"); 
  lcd.setCursor(0,1);
  if (dir == 'N') lcd.print("   stop ");
  if (dir == 'F') lcd.print("forward ");
  if (dir == 'L') lcd.print("   left ");
  if (dir == 'R') lcd.print("  right ");
  if (dir == 'B') lcd.print("reverse ");
  lcd.print(carSpeed,1);
  lcd.print(" km/h");  
}
void docount()  {  // count interrupts from speed sensor
  revCounter++; 
} 

