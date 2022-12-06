
 /* - developed in Arduino 1.0.6 -  TESTED OK 4 Dec 22
 EricssonAHD_v1.ino controls a single selector module of an Ericsson AHD
 telephone exchange. The sketch is similar to EricssonAGF_v1.ino which
 controls an AGF selector. 
 
 Arduino establishes a connection between two phones with a rotary 
 dial. The phonenumbers 53 and 57 are chosen arbitrarily. They both 
 have a 5 as the first digit. This means that their contacts have  
 the same X coordinate in the wirebank. Otherwise it would be
 necessary to build two lines of contact wires in the wirebank. 
 
 Arduino output D5 operates electromagnet XM (IN3 of SONGLE relay 
 module and selector pin 14), output D6 operates electromagnet YM 
 (IN2 and selector pin 13). XM controls the movement of the contact 
 carriage in the X direction, YM steps the a/b and c/d wipers into 
 the contact wire bank (Y direction). The electromagnets XM and YM 
 have a DC resistance of 9 ohm. They run at 24V and draw about 2 Amps 
 when activated. 24V is supplied from selector pins 11 and 12.
 Arduino output D4 operates electromagnet ZM (IN1 and selector pin 6).
 ZM activates the release mechanism that returns the 
 contacts carriage to the home position. This electromagnet has a 
 DC resistance of 86 ohm. ZM can operate only when the contacts 
 carriage is out of its home position. The minus lead of the magnet 
 coil (selector pin 6) passes through the springset that operates 
 on carriage movement. 
 Note that the SONGLE relay modules are activated by writing LOW !! 

 Arduino input pin D8 picks up the position of the endstop of either 
 the X or Y movement, via selector pin 15. The pin goes from 0V to 5V 
 when the endstop spring closes. The carriage is sent home when the 
 endstop is reached. 
 Arm wiper "c" is connected to input D7 via selector pin 3. D7 goes 
 HIGH when the "c" wiper meets 5V in the contacts mat. This indicates 
 that the wipers have reached a connected telephone. 
 
 The ringer relay is connected to the "a" wiper of the selector arm 
 via selector pin 1. The relay is controlled from Arduino pin D10 (IN5). 
 The NO contact of the ringer relay connects to the ringer module. The 
 NC contact of the ringer relay connects to the linefinder relay via a 
 1µF capacitor. At that point a 100nF capacitor feeds the line with 
 the various tones, generated from pin D12. 
 A second relay is operated from the same pin D10 (IN4). That relay 
 disconnects the analog input A1 from the "a" line whilst the ringer 
 sends its high voltage AC signal to the called phone. The NC contact 
 connects to A1 via a 3k9/15k divider to bring down the 24V line voltage 
 to 5V for the A1 input. The voltage on A1 indicates the status of the 
 called phone. The voltage increases from 0V to a value between 1V and 
 5 V when the handset of the called phone is lifted, provided that the 
 wipers have made contact with that phone.
  
 The linefinder relay selects between phones 53 and 57, depending which
 handset is lifted. The linefinder relay is controlled by pin D11 (IN6). 
 The NC contact connects to the "a" wire of phone 57 and to the 
 corresponding "a" wire in the matrix. The NO contact connects to the 
 "a" wire of phone 53 and to the corresponding "a" wire in the matrix. 
 The "b" wiper of the arm (selector pin 2), and the "b" wires of the 
 phones and the matrix are connected to GND. The "a" wires of both phones
 connect to +24V via 680 ohm resistors. Both phones have 3k9/15k voltage
 dividers to bring voltages in the range of analog inputs A0 and A5. 
 Phone 53 connects to A0, phone 57 connects to A5.
 
 OPERATING LOOP:
 At startup the contacts carriage is forced to the HOME position once, 
 if it is not already retracted. The linefinder relay selects the calling 
 phone depending on the voltage on pins A0 and A5. The voltage is about 
 5V when the handset is on its cradle (open contact for DC). The voltage 
 drops to about 1.5V when the handset is lifted. The line is then 
 terminated by the impedance of the inductor coil of the telephone. 
 The DC resistance is about 500 Ohm.  
 The line voltage drops to zero when the dial is turned clockwise. 
 Contacts in the dial create a short circuit around the inductor coil
 to prevent contact pulses to reach the receiver.
 
 The dialcontact opens and closes when the dial returns to its home 
 position. The line is at 5V when the contact is open and at 0V when 
 the contact is closed. The voltage returns to about 1.5V in the home 
 position. The number of contact openings corresponds with the digit 
 dialled. 
 
 Dial tone is sent to the line when a caller handset is lifted. Dialling 
 can now commence. The dial tone is switched off and the dial contact
 openings of the first digit are counted. On each count the XM magnet is 
 activated to move the contact carriage in the X direction. The program
 uses a timing routine to detect the end of contact openings for each 
 digit dialled. 
 The second digit is dialled. Now the YM magnet is activated to move the
 wipers into the wirebank. This brings the contacts to the wires of the
 called number. This is checked by the status of pin D7. D7 is LOW on non
 existing numbers. This sends a "no connection" tone to the caller. A low 
 voltage on either A0 or A5 reveals that the called number is engaged. 
   
 On finding the dialled phone the "a" wiper of the arm connects to the 
 "a" wire in the bank, and the "b" wiper meets the "b" wire. The ringer 
 relay is then sequenced from D10. This puts the bell signal on the "a" 
 wire of the called phone. Another relay disconnects the called phone 
 from A1 during periods where the bell signal goes high. 
 The voltage on A1 during those silent moments reveals if the handset of 
 the receiving phone is lifted. An analog reading between 50 and 900 
 sets up the connection. 
 A "no connection" tone of 400 Hz is sent to the line if no connection 
 is found, as detected by a reading above 900 on A1 and a LOW reading 
 on D7.
 A "busy" tone of intermittent 400 Hz is sent if the called phone is 
 engaged, i.e. when the reading on A1 is below 900 and D7 is HIGH.
 The ringer relay switches over when a connection is established. It 
 disconnects the ring signal from the "a" wire. The "a" wire in the bank 
 is now connected to the "a" wire of the called phone. The call is 
 terminated when either handset is put down. The arm is retracted
 and returns HOME. 
 A short contact debounce delay has been added to ensure consistent step 
 counting.
 Note that command "if (a < x < b) {" is accepted by the compiler but 
 does not work. Use "if ((x > a) && (x < b)) {". 
 Voltage level on inputs A0 (phone 53) or A5 (phone 57) when calling:
 
       handset down                                       call ended    
 5V   ____________  handset           _   _               ___________
                  | lifted           | | | |   ring/call |
 ~ 1.5V             |________  dialing | | | |_____________|
 0V                        |_________| |_|                  
 
      created by Wim der Kinderen 4 December 2022
 */

int phone53Pin = A0;      // calling phone 53 connected to analog pin A0
int phone57Pin = A5;      // second phone (57) connected to analog pin A5
int calledPhonePin = A1;  // called phone connected to analog pin A1
int XmagnetPin = 5;       // D5 connects to SONGLE relay to step X magnet
int YmagnetPin = 6;       // D6 connects to SONGLE relay to step Y magnet
int ZmagnetPin = 4;       // D4 connects to SONGLE relay to activate Z magnet
int connectPin = 7;       // D7 monitors the c spring; goes HIGH on connected phone
int endPin = 8;           // D8 detects the X or Y endpoint (end is HIGH)
int ringerPin = 10;       // D10 controls the ring sequence relay (ringing is LOW)
int phoneSelectorPin = 11;// D11 selects the first phone that dials as caller (linefinder)
int tonePin = 12;         // D12 sets dial, ring or unavailable tone on caller "a" wire
int ledPin = 13;          // use LED to show countPin status
int endStatus = 0;        // X or Y end position (end contact gives 5V on endPin)
int debounceDelay = 10;   // debounce delay in ms
int activationTime = 30;  // time to activate XM and YM magnets
int analogInA0 = 0;       // integer value of analog input A0 (0 - 1024)
int analogInA5 = 0;       // integer value or analog input A5 (0 - 1024)
int val = 0;              // 
int dialReading = 0;      // 
int dialLastState = 0;    // required for dial debounce routine
int dialTrueState = 0;    // status of dial contact
long dialLastStateChangeTime = 0; // time when status last changed
int dialHasFinishedRotatingAfterMs = 100; // waiting time after dial rotation
int needToAddDigit = 0;   // need to add digit if number is not complete
int numberToCall = 0;     // phone number as integer
int count = 0;            // counters to build the number to call
int digit = 0;
int digitCount = 0;
String numberString;      // phone number as string
boolean stepXmagnet = false;
boolean stepYmagnet = false;
boolean returnHome = false; // flag to indicate that the carriage is ready for returning home
boolean startUp = true;     // flag to indicate return home on startup if required
boolean callStart = false;  // flag to check who is going to call
boolean calling = false;    // flag to indicate ongoing call
boolean caller53 = false;   // flag to indicate that phone 53 is calling
boolean caller57 = false;   // flag to indicate that phone 57 is calling
boolean numberComplete = false; // flag to indicate that the number to call is built
boolean initDial = false;   // flag to indicate that dialing can commence
boolean printNumber = true; // flag to indicate that number can be sent to serial line
boolean ring = false;       // flag to mark that ringing is requested
boolean foundPhone = false; // flag to indicate that called number has been reached
boolean busy = false;       // flag to indicate that the called phone is engaged

void setup() {
  pinMode(XmagnetPin, OUTPUT);
  pinMode(YmagnetPin, OUTPUT);
  pinMode(ZmagnetPin, OUTPUT);
  pinMode(ringerPin, OUTPUT);
  pinMode(tonePin, OUTPUT);
  pinMode(phoneSelectorPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(connectPin, INPUT);
  pinMode(endPin, INPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(XmagnetPin, HIGH);         //   block X movement
  digitalWrite(YmagnetPin, HIGH);         //   block Y movement
  digitalWrite(ZmagnetPin, HIGH);         //   de-activate return magnet
  digitalWrite(ringerPin, HIGH);  // de-activate ringer relay (connects "a" wire to ringer)
  digitalWrite(phoneSelectorPin, HIGH); // de-activate linefinder relay (connects 57)
  Serial.begin(9600);                // only use when D0 and D1 are not required
  delay(1000); 
  Serial.println(" starting");
}

/******************************* MAIN LOOP ******************************************/
void loop() {
  if (startUp) goHome();                   // make sure selector arm starts from home
  if (callStart) checkPhoneStatus();       // check calling phone status
  if (initDial) dialNumber();              // build number to call on startup
  if (!numberComplete) stepCarriage();
  if (numberComplete) connectPhone();      // connect when number is complete
  if (returnHome) goHome();                // rotate counterclockwise to return home      
}

/*************************** CHECK PHONE STATUS ***********************************/
                                // CHECK WHICH PHONE STARTS THE CALL (53 or 57)
void checkPhoneStatus() {       // initDial IS TRUE IF DIALLING HAS COMMENCED
  analogInA0 = analogRead(phone53Pin); // read phonePin of 53 (A0)
  delay(5);                     // limit number of read cycles when idle
  Serial.print("phone53 = ");
  Serial.println(val);
  if (analogInA0 > 900) {       // above 4.5V: handset down or dial contact open
    calling = false;            // no call is being made yet
    caller53 = false;           
    if (initDial) {             // dialling starts after initial closed contact
      noTone(tonePin);          // stop dialtone during dialling
      callStart = false;        // no further checking of status required
      Serial.println("start dialing from 53");
    }
  }     
  if ((analogInA0 > 50) && (analogInA0 < 900)) { // 0.5 - 4.5V: handset 53 lifted
    caller53 = true;               // 53 is ready for dialling
    caller57 = false;
    digitalWrite(phoneSelectorPin, LOW); // connect 53 to arm (HIGH = relay NC)
    tone(tonePin, 150);            // put dialtone on tonePin
  }
  if (analogInA0 < 50) {            // below 0.5 V: dial contact closed
    digitalWrite(phoneSelectorPin, LOW); // connect 53 to selector arm
    caller53 = true;                // 53 started dialling
    caller57 = false;
    calling = false;                // no conversation yet
    callStart = false;              // no further checking of status required
    initDial = true;                // DIAL OF 53 HAS STARTED TURNING CLOCKWISE
    numberString = "";              // clear number buffer
    needToAddDigit = 0;             // reset digit count
    digitalWrite(ringerPin, HIGH);  // de-activate ringer relay (connects "a" wire)   
  }
  if (!caller53) {                 // 57 takes priority if both handsets are lifted
    analogInA5 = analogRead(phone57Pin);  // READ PHONE PIN 57 (A5)
    delay(5);                      // limit number of read cycles
    Serial.print("phone57 = ");
    Serial.println(val);
    if (analogInA5 > 900) {        // above 4.5V: dial contact open
      caller57 = false;            // 57 is ready for calling
      if (initDial) {              // dialling starts after initial closed contact
        noTone(tonePin);           // stop dialtone during dialling
        callStart = false;         // no further checking of status required
        Serial.println("start dialing from 57");
      }
    }     
    if ((analogInA5 > 50) && (analogInA5 < 900)) {  // 0.5 - 4.5V: handset 57 lifted
      caller57 = true;                // 57 ready for dialing
      caller53 = false;
      digitalWrite(phoneSelectorPin, HIGH);  // connect 57 to arm (LOW = relay NO)
      tone(tonePin, 150);             // put dialtone on tonePin
    }
    if (analogInA5 < 50) {            // below 0.5 V: dial contact closed
      digitalWrite(phoneSelectorPin, HIGH); // connect 57 to selector arm
      caller57 = true;                // 57 started dialing
      caller53 = false;
      calling = false;                // no conversation yet
      callStart = false;              // no further checking of status required
      initDial = true;                // DIAL OF 57 HAS STARTED TURNING CLOCKWISE
      numberString = "";              // clear number buffer
      needToAddDigit = 0;
      digitalWrite(ringerPin, HIGH);  // de-activate ringer relay (connects "a" wire)   
    }
  }
} 

/********************************* DIAL THE NUMBER **********************************/
void dialNumber() {                    // DIALLING HAS STARTED WHEN initDial IS TRUE 
  noTone(tonePin);                     // switch off dialtone
  if (caller53) {
    val = analogRead(phone53Pin);     // read pin of calling phone
  }
  if (caller57) {
    val = analogRead(phone57Pin);
  } 
  if (val < 900) dialReading = 0;     // convert analog reading to state
  if (val > 900) dialReading = 1;
  if (!numberComplete) {              // go around the loop until number is complete
    checkDialing();                            // build the number to call
    numberToCall = numberString.toInt();       // convert string to integer
    if (numberToCall > 99) numberToCall = 99; // limit numbers to range 10 - 99
  }
  if (numberComplete) {
    initDial = false;
    if (numberToCall < 10) numberToCall = 10;
    if (printNumber) {
      Serial.print("  number to call: ");
      Serial.println(numberToCall); 
      printNumber = false;     
    }
  }
}   

/**************************** DEAL WITH DIAL PULSES **********************************/
                    // convert dialled digits to pulses for the magnets
void checkDialing() {   
  if ((millis() - dialLastStateChangeTime) > dialHasFinishedRotatingAfterMs) {
    // the dial isn't being used, or dialing a digit has just finished
    if (needToAddDigit) {
      // if the dial has returned, the number is sent down the serial line and the
      //  count is reset. A count of 10 is modified because '0' will send 10 pulses
      digit = count % 10;
      digitCount++;
  //    Serial.print(digit, DEC);  ADDED comment slashes 16/11
      numberString = numberString + digit;  // build the number to call as a string
      needToAddDigit = 0;
      count = 0;
      if (digitCount == 2) numberComplete = true; // stop after 2 digits
    }
  } 
  if (dialReading != dialLastState) {      // check if the dial contacts changed
    dialLastStateChangeTime = millis();
  }
  if ((millis() - dialLastStateChangeTime) > debounceDelay) {
    // wait for debounce to finish
    if (dialReading != dialTrueState) {
      // the dialswitch has either just gone from closed -> open or vice versa.
      dialTrueState = dialReading;
      if (dialTrueState == 1) {
        // increment the count of dialpulses if it's gone high.
        count++;
        if (digitCount == 0) {
          stepXmagnet = true; 
          stepYmagnet = false; 
        }
        if (digitCount == 1) {
          stepXmagnet = false; 
          stepYmagnet = true; 
        }       
        // update the number with the new digit (once the dial has finished rotating)
        needToAddDigit = 1; 
      } 
    }
  }
  dialLastState = dialReading;
} 

/********************** MOVE CONTACTS CARRIAGE IN X OR Y DIRECTION *******************/
void stepCarriage() {
  endStatus = digitalRead(endPin);
  if (endStatus == 1) goHome();           // return carriage if endstop is reached
  if (stepXmagnet) {
    digitalWrite(YmagnetPin, HIGH);       // block Y movement
    digitalWrite(XmagnetPin, LOW);        // make one step in X direction
    delay(activationTime);                // activate magnet for x ms (was 30)
    digitalWrite(XmagnetPin, HIGH);      
    stepXmagnet = false;
  }
  if (stepYmagnet) {
    digitalWrite(XmagnetPin, HIGH);       // block X movement
    digitalWrite(YmagnetPin, LOW);        // make one step in Y direction
    delay(activationTime);
    digitalWrite(YmagnetPin, HIGH);       
    stepYmagnet = false;
  }  
}

/********************************* CONNECT PHONE  ********************************/
void connectPhone() {
  analogInA0 = analogRead(phone53Pin);  // check status of both phones
  analogInA5 = analogRead(phone57Pin); 
  if (analogInA0 > 900 && analogInA5 > 900) returnHome = true; // return home when 
          // both handsets are on cradle or one handset is put down during a call                      
  if ((analogInA0 > 900 || analogInA5 > 900) && calling) returnHome = true; 
  if ((analogInA0 > 900 || analogInA5 > 900) && busy) returnHome = true;
  else {
    foundPhone = digitalRead(connectPin);  // check if c wiper sees 5V
    if (foundPhone) {
      val = analogRead(calledPhonePin); // check CALLED PHONE status
      Serial.print("calledPhoneStatus = ");
      Serial.println(val);
      if (val > 900 && !calling) {    // if handset is on cradle and no call active:
        ring = true;                  // run ring sequence
        ringBell();
      }
      if ((val > 50) && (val< 900)) { // if called handset is off hook:
        if(!ring && !calling) {       //   if user is busy when being connected:
          busy = true;                //   run busy tone sequence and wait
          busyTone();                 //     for handsets to be put down
        }
        else {                        // if handset is lifted when ringing:
          ring = false;               //   stop ring sequence
          calling = true;             //   start conversation
          noTone(tonePin);            //   switch off ringtone
          digitalWrite(ringerPin, HIGH); // de-activate ringer, connect line to selector
        }     
      }
    }  
    else {                           // UNCONNECTED NUMBER BEING DIALLED:
      tone(tonePin, 400);            // put unobtainable tone (normally 400 Hz)
      analogInA0 = analogRead(phone53Pin); // check both phones status
      analogInA5 = analogRead(phone57Pin); 
      if (analogInA0 > 900 && analogInA5 > 900) { // return home when both handsets 
                                                  //   are down
        foundPhone = false;
        calling = false;
        returnHome = true;                 
      }  
    }
  }
}

/*************************** GENERATE RING SEQUENCE ******************************/
void ringBell() {
  if (ring) {                              // if ringing is requested:
    tone(tonePin, 150);                    // put 150Hz ringtone on tonePin
    digitalWrite(ledPin, HIGH);            // switch on LED    
    digitalWrite(ringerPin, LOW);          // activate ring sequence:
    delay(400);                            //   400 ms ON
    noTone(tonePin);                       //   switch off tone
    digitalWrite(ledPin, LOW);             //   switch off LED    
    digitalWrite(ringerPin, HIGH);         //   de-activate relay
    delay(200);                            //   200 ms OFF
    tone(tonePin, 150);
    digitalWrite(ledPin, HIGH);            //   switch on LED    
    digitalWrite(ringerPin, LOW);          //   activate relay 
    delay(400);                            //   400 ms ON
    noTone(tonePin);
    digitalWrite(ledPin, LOW);             //   switch off LED    
    digitalWrite(ringerPin, HIGH);         //   de-activate relay 
    delay(100);                            // let relay settle
    if (caller53) {
      analogInA0 = analogRead(phone53Pin); // check calling phone status
      if (analogInA0 > 900) goHome();      // return home on handset put down
    }
    if (caller57) {   
      analogInA5 = analogRead(phone57Pin); 
      if (analogInA5 > 900) goHome();
    }
    delay(1900);                           // in total 2 seconds OFF
  }
}

/*************************** GENERATE BUSY TONE SEQUENCE ***************************/
void busyTone() {
  if (busy) {
    tone(tonePin, 400);                    // send 400 Hz to tonePin
    digitalWrite(ledPin, HIGH);            // switch on LED 
    delay(750);
    noTone(tonePin);                       // switch off tone 
    digitalWrite(ledPin, LOW);             // switch off LED 
    delay(750);
  }
}

/********************************* RETURN HOME ********************************/
void goHome() {
  returnHome = false; 
  callStart = true;                      // allow dialing
  startUp = false;
  initDial = false;
  numberComplete = false;
  ring = false;
  busy = false;
  caller53 = false;
  caller57 = false;
  printNumber = true;   
  stepXmagnet = false;
  stepYmagnet = false; 
  calling = false;    
  foundPhone = false; 
  needToAddDigit = 0;                    // reset digit count
  count = 0;
  digitCount = 0;
  numberString = "";
  dialReading = 0;    
  dialLastState = 0;   
  dialTrueState = 0;  
  dialLastStateChangeTime = 0; // time when status last changed
  noTone(tonePin);
  digitalWrite(ZmagnetPin, LOW);         // activate Z magnet to return carriage
  delay(150);                            // was 100 ms
  digitalWrite(ZmagnetPin, HIGH);        // de-activate Z magnet 
  digitalWrite(phoneSelectorPin, HIGH);  // connect 57 as default to the line
  digitalWrite(XmagnetPin, HIGH);        //   block X movement
  digitalWrite(YmagnetPin, HIGH);        //   block Y movement
  digitalWrite(ringerPin, HIGH);  // de-activate ringer relay (connects "a" wire to line)
  delay(500);  // was 1000
}



