/*
mobilePhone_v1 uses an Arduino Uno and a SIM900 shield.
The shield holds a SIMcard and connects to the O2 G2 network using Giffgaff.
The Giffgaff SIM number is 07955 xxxxxx. SIM900 only works with G2.

See also https://www.instructables.com/id/Interface-a-rotary-phone-dial-to-an-Arduino/
and https://www.instructables.com/id/Interface-a-rotary-phone-dial-to-an-Arduino/
https://lastminuteengineers.com/sim900-gsm-shield-arduino-tutorial/

The sketch allows making and receiving calls using a modified vintage 
GPO 250 Field Telephone. 

With an incoming call, a "RING" message is received from the provider.
This message is displayed via the serial monitor and the connection 
is made by sending: SIM900.print("ATA\r"). To hang up the code 
SIM900.println("ATH") is sent.

Digital pin D2 is connected to one of the DIAL contacts of the telephone via
a 470 Ohm resistor. The other side of that resistor is connected to 5V via a 
10 kOhm resistor. The second dial contact is connected to GND. The dial contacts
are normally closed, so pin D2 reads LOW. Pin D2 goes HIGH a number of times
according to the digit dialed.

Digital pin D3 is connected to the CALL BUTTON via a 470 Ohm resistor. The other
side of the resistor is connected to GND via 10 kOhm. The second contact of the 
call button connects to 5V.  A call is requested when the call button is 
pressed and pin D3 goes HIGH. The  callcommand is: 
SIM900.println("ATD + +" + numberToCall + ";"). The number has to include
the countrycode and starts with 44 in the UK.

Digital pin D4 is connected to the CRADLE switch, which is normally open when
the handset is down. The connection to the switch is via a 470 Ohm resistor. The
other side of that resistor connects to GND via 10 kOhm. The second contact
of the cradle switch connects to 5V. Pin D4 goes HIGH when the handset is lifted.  

        created by Wim der Kinderen on 19 March 2020
        TESTED OK
*/

#include <SoftwareSerial.h> // use Softserial to communicate with the shield

SoftwareSerial SIM900(7, 8); // set shield jumpers to D8(RX) and D7(TX)
char incoming_char=0;
int needToPrint = 0;
int count = 0;
int digit = 0;
int dialPin = 2; // Pin 2 goes HIGH when dial contact opens
int callPin = 3; // Pin 3 goes HIGH when call button is pressed
int cradlePin = 4; // Pin 4 goes HIGH when the handset is lifted 
int dialLastState = LOW; // required for dial debounce routine
int dialTrueState = LOW;
long dialLastStateChangeTime = 0;
int callLastState = LOW;  // required for call button debounce routine
int callTrueState = LOW;
long callLastStateChangeTime = 0;
int cradleLastState = LOW;  // required for cradle debounce routine
int cradleTrueState = LOW;
long cradleLastStateChangeTime = 0;
int cleared = 0;
int dialReading = 0;
int callReading = 0;
int cradleReading = 0;
String numberToCall;
boolean cradle = false; // cradle false means that handset is down
boolean calling = false;
boolean ringing = false;

// constants
int dialHasFinishedRotatingAfterMs = 100; // waiting time after dial rotation
int debounceDelay = 10; // debounce delay in ms

void setup() {
//  SIM900power(); // switch on SIM900                      TO BE ADDED LATER
  pinMode(dialPin, INPUT); // set D2, D3 and D4 as input
  pinMode(callPin, INPUT);
  pinMode(cradlePin, INPUT);
  SIM900.begin(19200); // initiate Arduino - SIM900 comms 
  Serial.begin(19200);  // serial monitor set at 19200
  delay(1000);
  Serial.println("Wait 20 seconds to find the network");
  delay(20000); // allow the shield to find the network
  Serial.println("Ready for calls");
  SIM900.print("AT+CLIP=1\r"); // turn on caller ID notification
  delay(100);
  numberToCall = "";
}

/* **************************** MAIN LOOP **********************************/

void loop() {
  dialReading = digitalRead(dialPin); // read dial status
  callReading = digitalRead(callPin); // read call button status
  cradleReading = digitalRead(cradlePin); // read cradle status 
  
  checkCradle();         // check cradle status
  if (!cradle) {         // monitor line for incoming call while handset is down
    checkIncomingCall(); // check for "RING" message and start ringing/lift handset
  }
  if (cradle) {          // allow dialling when handset is lifted
    checkDialing();      // build the number to call
  }
  checkCallbutton();     // check if call button is pressed, then call number
}  
   
/* ********************************* SUBROUTINES ****************************/

void checkCradle() {     // CHECK STATUS OF THE CRADLE SWITCH
  // reset debouncetimer when switch changes position
  if (cradleReading != cradleLastState) { 
    cradleLastStateChangeTime = millis();
  }
  if ((millis() - cradleLastStateChangeTime) > debounceDelay) {
    // wait for debounce to finish
    if (cradleReading != cradleTrueState) {
      // the switch has either just gone from closed->open or vice versa.
      cradleTrueState = cradleReading;
      if (cradleTrueState == HIGH) { // confirm that cradle switch closes
        Serial.println("Handset lifted");
        cradle = true; // handset is lifted
        calling = true; // call is possible
        if (ringing) {  // answer call when handset is lifted when phone rings
          SIM900.print("ATA\r");  // Send ATA command to answer the phone
          ringing = false; // stop ringing
        }
      }  
      if (cradleTrueState == LOW) { // confirm that cradle switch opens
        Serial.println("Handset is down");
        cradle = false; // handset is down
        if (calling) { 
          SIM900.println("ATH"); // hang up 
          delay(1000);
          Serial.println("Call ended"); 
          numberToCall = ""; 
        }     
      } 
    }
  }
  cradleLastState = cradleReading;  // remember last cradle position
}

void checkIncomingCall() { // DEAL WITH INCOMING CALL 
  if(SIM900.available() >0) {
    // Get characters from the cellular serial port when called
    incoming_char=SIM900.read();
    //  Check if the shield is sending a "RING" message
    if (incoming_char=='R') {
      delay(10);
      Serial.print(incoming_char);
      incoming_char=SIM900.read();
      if (incoming_char =='I') {
        delay(10);
        Serial.print(incoming_char);
        incoming_char=SIM900.read();
        if (incoming_char=='N') {
          delay(10);
          Serial.print(incoming_char);
          incoming_char=SIM900.read();
          if (incoming_char=='G') {
            delay(10);
            Serial.println(incoming_char);
            // Send ringtone if the message received from the shield is RING
            ringing = true;    // start ringtone
          }
        }
      }
    }
  }
} 

void checkDialing() { // DEAL WITH DIAL PULSES 
  if ((millis() - dialLastStateChangeTime) > dialHasFinishedRotatingAfterMs) {
    // the dial isn't being dialed, or has just finished being dialed.
    if (needToPrint) {
      // if it's only just finished being dialed, we need to send the number down the serial
      // line and reset the count. We mod the count by 10 because '0' will send 10 pulses.
    digit = count % 10;
    Serial.print(digit, DEC);
    numberToCall = numberToCall + digit;  // build up the number to call as a string
    needToPrint = 0;
    count = 0;
//    cleared = 0;
    }
  } 
  if (dialReading != dialLastState) {
    dialLastStateChangeTime = millis();
  }
  if ((millis() - dialLastStateChangeTime) > debounceDelay) {
    // wait for debounce to finish
    if (dialReading != dialTrueState) {
      // the switch has either just gone from closed->open or vice versa.
      dialTrueState = dialReading;
      if (dialTrueState == HIGH) {
        // increment the count of dialpulses if it's gone high.
        count++; 
        // we'll need to print this number (once the dial has finished rotating)
        needToPrint = 1; 
      } 
    }
  }
  dialLastState = dialReading;
} 

void checkCallbutton() { // DEALING WITH THE CALL BUTTON
  if (callReading != callLastState) { // reset debouncetimer when button is pressed
    callLastStateChangeTime = millis();
  }
  if ((millis() - callLastStateChangeTime) > debounceDelay) {
    // wait for debounce to finish
    if (callReading != callTrueState) {
      // the switch has either just gone from closed->open or vice versa.
      callTrueState = callReading;
      if (callTrueState == HIGH) { // confirm that call button has been pressed
        Serial.println(" ");
        Serial.println("Call Requested");
        calling = true;
        callSomeone();
      } 
    }
  }
  callLastState = callReading; // remember last call button status  
}

void callSomeone() { // CALL A NUMBER
  Serial.println("Calling " + numberToCall);
  SIM900.println("ATD + +" + numberToCall + ";"); // call number, start with 44 in UK
  delay(100);
//  SIM900.println();
  checkCradle();  // check if handset is put back on cradle to hang up the call
  if (!cradle) {
    SIM900.println("ATH"); // hang up
    delay(1000);
    Serial.println("Call ended"); 
    numberToCall = ""; 
  }    
}

void SIM900power() {  // switch on power to SIM900
  pinMode(9, OUTPUT); 
  digitalWrite(9,LOW);
  delay(1000);
  digitalWrite(9,HIGH);
  delay(2000);
  digitalWrite(9,LOW);
  delay(3000);
}
