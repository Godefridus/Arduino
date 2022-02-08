
 /* - developed in Arduino 1.0.6 -
 EricssonAGF_v1.ino controls a single selector module of an Ericsson 500 
 telephone exchange. Normally, each selector in the exchange is operated 
 from a common driveshaft. Here we use a small DC motor to drive the large 
 gearwheel that rotates and translates the selector arm. The motor is 
 controlled from an L298N motor driver. Motor pins of the selector 
 connector are 5 (OUT1) and 18 (OUT2). The motor driver connects to a 
 12V power supply (SNP15-12VF-1). Connections to the Arduino are: 12V 
 to Vin, GND to GND, IN1 to D2, IN2 to D4, and ENA to D3. D3 is a PWM 
 pin that sets the motor speed.
 The rotation lock and translation lock relays are operated from a SONGLE 
 relais module HL-52S, switching the minus of a 24V power supply 
 (OTM-E30-24). The plus of the 24V supply connects to selector pin 9.
 
 Arduino D5 operates the rotation lock (IN1 and selector pin 14), 
 D6 operates the translation lock (IN2 and selector pin 6). Note that the 
 SONGLE relay modules are activated by writing LOW !! 
 Arduino D7 picks up the position of the rotation wheel from selector pin 19. 
 A closed contact sends +12V to an 8k2/5k6 divider. Both open and closed 
 contacts are counted. 
 Arduino D8 picks up the position of the rotation endstop from selector 
 pin 13. The contact opens at the endstop. During rotation D8 sees a HIGH 
 from +12V via a 12k/8k2 divider. Arduino D9 monitors the rotation homestop
 through selector pin 15. The contact opens at the stop. During rotation 
 D9 sees a HIGH from +12V via a 12k/8k2 divider.

 Analog inputs are used to monitor the translation end and home contacts
 because of the limited number of digital pins. Note that D0 and D1 cannot
 be used when the serial link is programmed for debugging.
 Arduino A2 sees a HIGH when the arm reaches the translation endstop, 
 using a 6k8/4k7 divider from +12V (selector pin 17). A3 sees a HIGH when 
 the arm reaches the retraction home stop, using a 8k2/5k6 divider from 
 +12V (selector pin 7). Both inputs are LOW during translation. The base 
 of the selector is connected to +12V via connector pin 16.
 Arm contact "c" is connected to A4 via selector pin 3. A4 goes HIGH when 
 the "c" contact meets 12V in the connector mat. A 4k7/6k8 voltage divider 
 reduces 12V to 5V input for A4.
 
 The ringer relay is connected to the "a" contact in the selector arm. 
 The relay is controlled from pin D10. The NO contact of the ringer relay 
 connects to the ringer module. A second relay is operated from the 
 same pin D10. That relay disconnects the A1 input from the "a" line 
 whilst the ringer sends its AC signal to the called phone. The NC contact 
 connects to A1 via a 3k9/15k divider to bring down the 24V line voltage to 
 5V for the A1 input. The NC contact of the ringer relay connects to the
 linefinder relay via a 2.2µF capacitor. At that point a 100nF capacitor 
 feeds the line with the various tones, generated from pin D12.
 
 The linefinder relay selects between phones 363 and 371, depending which
 handset is lifted. The linefinder relay is controlled by pin D11. The NC 
 contact connects to the "a" wire of phone 363 and to the corresponding
 "a" wire in the matrix. The NO contact connects to the "a" wire of 
 phone 371 and to the corresponding "a" wire in the matrix.  The "b" 
 contact of the arm (connector pin 2), and the "b" wires of the phones 
 and the matrix are connected to GND. The "a" wires of both phones
 connect to +24V via 680Ω resistors. Both phones have 3k9/15k voltage
 dividers. Phone 363 connects to A0, phone 371 connects to A5.
 
 OPERATION LOOP:
 At startup the arm is turned to the HOME position if it is not already 
 retracted. The linefinder relay selects the calling phone depending on 
 the voltage on pins A0 and A5. The voltage is about 5V when the handset
 is in its cradle (open contact). The voltage drops to about 1.5V when the
 handset is lifted. During dialing the voltage drops to zero when the dial
 contact closes. 
 Dial tone is sent to the line when a handset is lifted. Dialing can 
 commence. The dial tone is switched off and digits are counted by 
 monitoring contact closure. The number to call is built up from the digits 
 dialed. After 3 digits the number is complete and selecting of the 
 connection can begin. The number is set to the maximum of 399 if a higher 
 number has been dialed, or to 20 if a lower number is dialed.  
 The number of rotation steps is calculated by dividing the number to call
 (which is an integer) by 20 and adding one. The number of translation steps
 is then calculated as the remainder: numberToCall - (rotSteps - 1) * 20 + 1.
 The arm starts to rotate and stops at the calculated number of rotation 
 steps. The number of rotation steps is restricted to 21 by monitoring the 
 rotation endcontact using D8. 
 The arm switches over to translation. Steps are counted by monitoring 
 analog A4. A step is counted when A4 goes HIGH where the "c" contact 
 meets +12V. The voltage is brought down to 5V using a 4k7/6k8 divider.
 The translation endstop is monitored using A2, using a 4k7/6k8 divider.   
 On finding the dialed phone the "a" contact of the arm connects to the 
 "a" wire in the mat, and the "b" contact meets the "b" wire. The ringer 
 relay is then activated from D10. This puts the bell signal on the "a" 
 wire of the called phone. The sequencing of the ring signal is controlled 
 from D10. Another relay disconnects the called phone from A1 during 
 periods where the bell signal goes high. A voltage on A1 during those 
 silent moments reveals if the handset of the receiving phone is lifted. 
 A reading between 50 and 900 sets up the connection. A "no connection" 
 tone is sent to the line if no connection is found. A "busy" tone is
 sent if the called phone is engaged.
 The ringer relay switches over when a connection is established. It 
 disconnects the ring signal from the "a" wire. The "a" wire in the mat 
 is now connected to the "a" wire of the called phone. The call is 
 terminated when either handset is put down. The arm is retracted
 and returns HOME. 
 A short contact debounce delay has been added to ensure consistent step 
 counting.
 Note that command "if (a < x < b) {" is accepted by the compiler but 
 does not work. Use "if ((x > a) && (x < b)) {". 
 Voltage level on inputs A0 (phone 363) or A5 (phone 371) when calling:
 
       handset down                                       call ended    
 5V   ____________  handset           _   _               ___________
                  | lifted           | | | |   ring/call |
 ~ 1V             |________  dialing | | | |  ___________|
 0V                        |_________| |_| |_|                 
 
      created and tested OK by Wim der Kinderen 13 January 2022
 */

int phone363Pin = A0;     // calling phone 363 connected to analog pin A0
int calledPhonePin = A1;  // called phone connected to analog pin A1
int transEndPin = A2;     // A2 detects the endpoint of translation (end is HIGH)
int transHomePin = A3;    // A3 detects the homepoint of translation (home is HIGH)
int transCountPin = A4;   // A4 monitors the "c" contact to count translation steps
int phone371Pin = A5;     // second phone (371) connected to analog pin A5
int revPin = 3;           // D3 PWM output connects to ENA of the motor driver 
int motorPin1 = 2;        // D2 and D4 connect to IN1 and IN2 of the motor driver
int motorPin2 = 4;        //   clockwise rotation with IN1 HIGH and IN2 LOW
int rotLockPin = 5;       // D5 connects to SONGLE relay to control the rotation lock
int transLockPin = 6;     // D6 connects to SONGLE relay to control the translation lock
int rotCountPin = 7;      // D7 counts the rotation impulses (1 HIGH, 2 LOW, 3 HIGH etc)
int rotEndPin = 8;        // D8 detects the endpoint of rotation (end is LOW)
int rotHomePin = 9;       // D9 detects the homepoint of rotation (home is LOW)
int ringerPin = 10;       // D10 controls the ring sequence relay (ring is HIGH)
int phoneSelectorPin = 11;// D11 selects the first phone that dials as caller
int tonePin = 12;         // D12 sets dial, ring or unavailable tone on caller "a" wire
int ledPin = 13;          // use LED to show countPin status
int newCountStatus = 0;   // new count pickup status (0 or 1)
int prevCountStatus = 0;  // previous status
int countStatus = 0;      // status of translation count
int rotEndStatus = 1;     // rotation end position (open contact at the end)
int rotHomeStatus = 1;    // rotation homw position (open contact at home)
int transEndStatus = 0;   // translation end position (closed contact at the end)
int transHomeStatus = 0;  // translation home position (closed contact at home)
int revSpeedLow = 70;     // normal power to motor during rotation
int revSpeedHigh = 90;    // higher power during translation move
int rotCount = 0;         // rotation step counter 
int rotSteps = 19;        // number of rotation steps
int transSteps = 1;       // number of translation steps
int transCount = 0;       // counter for translation steps
int debounceDelay = 10;   // debounce delay in ms
int analogIn = 0;         // integer value of analog input (0 - 1024)
int dialReading = 0;      // 
int dialLastState = 0;    // required for dial debounce routine
int dialTrueState = 0;    // status of dial contact
long dialLastStateChangeTime = 0; // time when status last changed
int dialHasFinishedRotatingAfterMs = 100; // waiting time after dial rotation
int needToAddDigit = 0;   // need to add digit
int numberToCall = 0;     // phone number as integer
int val = 0;              // integer value of analog input (0 - 1024)
int count = 0;            // counters
int digit = 0;
int digitCount = 0;
String numberString;      // phone number as string
boolean rotate = false;     // flag to indicate that the arm is ready for rotation
boolean returnHome = false; // flag to indicate that the arm is ready for returning home
boolean transOut = false;   // flag to indicate that the arm is ready for translation
boolean transBack = false;  // flag to indicate that the arm is ready for retraction
boolean startUp = true;     // flag to indicate return home on startup if required
boolean firstStep = true;   // flag to indicate delay on first step
boolean callStart = false;  // flag to check who is going to call
boolean calling = false;    // flag to indicate ongoing call
boolean caller363 = false;  // flag to indicate that phone 363 is calling
boolean caller371 = false;  // flag to indicate that phone 371 is calling
boolean numberComplete = false; // flag to indicate that the number to call is built
boolean initDial = false;   // flag to indicate that dialing can commence
boolean printNumber = true; // flag to indicate that number can be sent to serial line
boolean ring = false;       // flag to mark that ringing is requested
boolean foundPhone = false; // flag to indicate that called number has been reached
boolean busy = false;       // flag to indicate that the called phone is engaged

void setup() {
  analogWrite(revPin, 0);
  pinMode(motorPin1, OUTPUT);
  pinMode(motorPin2, OUTPUT);
  pinMode(rotLockPin, OUTPUT);
  pinMode(transLockPin, OUTPUT);
  pinMode(ringerPin, OUTPUT);
  pinMode(tonePin, OUTPUT);
  pinMode(phoneSelectorPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(rotEndPin, INPUT);
  pinMode(rotHomePin, INPUT);
  pinMode(rotCountPin, INPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(rotLockPin, HIGH);         //   block rotation
  digitalWrite(transLockPin, HIGH);       //   block translation
  digitalWrite(ringerPin, HIGH);  // de-activate ringer relay (connects "a" wire)
  digitalWrite(phoneSelectorPin, HIGH); // de-activate linefinder relay (connects 363)
  Serial.begin(9600);             // only use when D0 and D1 are not required
  delay(1000); 
  Serial.println(" starting");
}

/******************************* MAIN LOOP ******************************************/
void loop() {
  if (startUp) goHome();                   // make sure selector arm starts from home
  if (callStart) checkPhoneStatus();       // check calling phone status
  if (initDial) dialNumber();              // build number to call on startup
  if (numberComplete) calcSelectorMoves(); // convert number to selector steps
  if (rotate) rotateClock();               // rotate clockwise for required steps
  if (transOut) translateOut();            // translate arm for required steps 
  if (foundPhone) connectPhone();          // connect when phone is found
  if (transBack) translateBack();          // fully retract arm after call
  if (returnHome) goHome();                // rotate counterclockwise to return home      
}

/*************************** CHECK PHONE STATUS ***********************************/
void checkPhoneStatus() {       // CHECK WHICH PHONE IS DIALING (363 or 371)
  val = analogRead(phone363Pin);//   READ PHONE PIN 363 (A0)
  delay(5);                     //   limit number of read cycles
  if (val > 900) {              //   above 4.5V: dial contact open
    calling = false;
    caller363 = false;          //   363 handset is down
    if (initDial) {             //   dialing starts after initial closed contact
      noTone(tonePin);          //   stop dialtone during dialing
      callStart = false;        //   no further checking of status required
      Serial.println("start dialing from 363");
    }
  }     
  if ((val > 50) && (val < 900)) {  // 0.5 - 4.5V: handset 363 lifted
    caller363 = true;                    // 363 ready for calling
    digitalWrite(phoneSelectorPin, HIGH); // connect 363 to arm (HIGH = relay NC)
    tone(tonePin, 150);                  // put dialtone on tonePin
  }
  if (val < 50) {                   // below 0.5 V: dial contact closed
    digitalWrite(phoneSelectorPin, HIGH); // connect 363 to selector arm
    caller363 = true;               // 363 started dialing
    calling = false;
    callStart = false;              // no further checking of status required
    initDial = true;                // dial has started rotating clockwise
    numberString = "";              // clear number buffer
    needToAddDigit = 0;
    digitalWrite(ringerPin, HIGH);  // de-activate ringer relay (connects "a" wire)   
  }
  if (!caller363) {              // 363 takes priority if both handsets are lifted
    val = analogRead(phone371Pin); // READ PHONE PIN 371 (A5)
    delay(5);                      // limit number of read cycles
    if (val > 900) {               // above 4.5V: dial contact open
    caller371 = false;           // 371 handset is down
      if (initDial) {              // dialing starts after initial closed contact
        noTone(tonePin);           // stop dialtone during dialing
        callStart = false;         // no further checking of status required
        Serial.println("start dialing from 371");
      }
    }     
    if ((val > 50) && (val < 900)) {   // 0.5 - 4.5V: handset 371 lifted
      caller371 = true;                     // 371 ready for dialing
      digitalWrite(phoneSelectorPin, LOW);  // connect 371 to arm (LOW = relay NO)
      tone(tonePin, 150);                   // put dialtone on tonePin
    }
    if (val < 50) {                   // below 0.5 V: dial contact closed
      digitalWrite(phoneSelectorPin, LOW); // connect 371 to selector arm
      caller371 = true;               // 371 started dialing
      calling = false;
      callStart = false;              // no further checking of status required
      initDial = true;                // dial has started rotating clockwise
      numberString = "";              // clear number buffer
      needToAddDigit = 0;
      digitalWrite(ringerPin, HIGH);  // de-activate ringer relay (connects "a" wire)   
    }
  }
} 

/********************************* DIAL THE NUMBER **********************************/
void dialNumber() {                        // dialing has started
  noTone(tonePin);                         // switch off dialtone
  if (caller363) {
    val = analogRead(phone363Pin);         // read pin of calling phone
  }
  if (caller371) {
    val = analogRead(phone371Pin);
  } 
  if (val < 900) dialReading = 0;         // convert analog reading to state
  if (val > 900) dialReading = 1;
  if (!numberComplete) {
    checkDialing();                            // build the number to call
    numberToCall = numberString.toInt();       // convert string to integer
    if (numberToCall > 399) numberToCall = 399; // limit numbers to range 20 - 399
  }
  if (numberComplete) {
    if (numberToCall < 20) numberToCall = 20;
    if (printNumber) {
      Serial.print("  number to call: ");
      Serial.println(numberToCall); 
      printNumber = false;     
    }
  }
}   

/**************************** DEAL WITH DIAL PULSES **********************************/
void checkDialing() {   
  if ((millis() - dialLastStateChangeTime) > dialHasFinishedRotatingAfterMs) {
    // the dial isn't being used, or dialing a digit has just finished
    if (needToAddDigit) {
      // if the dial has returned, the number is sent down the serial line and the
      //  count is reset. A count of 10 is modified because '0' will send 10 pulses
      digit = count % 10;
      digitCount++;
      Serial.print(digit, DEC);
      numberString = numberString + digit;  // build the number to call as a string
      needToAddDigit = 0;
      count = 0;
      if (digitCount == 3) numberComplete = true;
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
        // update the number with the new digit (once the dial has finished rotating)
        needToAddDigit = 1; 
      } 
    }
  }
  dialLastState = dialReading;
} 

/********************** CALCULATE ROTATION AND TRANSLATION STEPS *********************/
  // number to call is between 000 and 420 in 21 mats (000 - 019, 020 - 039 etc)
void calcSelectorMoves() {              
  rotSteps = (numberToCall / 20) + 1;   // selector arm can access 20 contacts per mat     
  if (rotSteps > 21) rotSteps = 21;     // rotation has been restricted to 21 steps
  transSteps = numberToCall - (rotSteps - 1) * 20 + 1;
  if (transSteps < 1) transSteps = 1;
  if (transSteps > 20) transSteps = 20;
  Serial.print(numberToCall);      // send number also to monitor
  Serial.print("  ");
  Serial.print(rotSteps);
  Serial.print("  ");
  Serial.println(transSteps); 
  rotate = true;                  // initiate selector rotation          
  numberString = "";
  digitCount = 0;
  callStart = false;              // block new call until current call is finished
  initDial = false;
  numberComplete = false;
  printNumber = true; 
  delay(1000);
}

/********************************* CONNECT PHONE  ********************************/
void connectPhone() {
  analogIn = analogRead(phone363Pin); // check both phones status
  val = analogRead(phone371Pin); 
  if (analogIn > 900 && val > 900) {  // return home when both handsets are down
      transBack = true;
      foundPhone = false;
      ring = false;
      busy = false;
      noTone(tonePin);                 //   switch off tones
  } 
  else {
    if (numberToCall == 371 || numberToCall == 363) {        
                                      // ONLY ACTIVATE RINGER ON 371 OR 363    
      digitalWrite(ringerPin, HIGH);  // de-activate ringer, connect line to selector
      delay(10);
      val = analogRead(calledPhonePin); // check CALLED PHONE status
      if ((val > 50) && (val< 900)) { // if called handset is off hook:
        if(!ring) {                   //   if user is busy when being connected:
          busy = true;                //   run busy tone sequence and wait
          busyTone();                 //     for either handset to be put down
        }
        else {                        // if handset is lifted when ringing:
        calling = true;               //   start conversation (ring remains true!)
        noTone(tonePin);              //   switch off ringtone
        }
      }    
      if (val > 900) {                // if handset is down or just put down:
        if (!calling) {  
          if (busy) {                 //   if this is from a busy tone status:                  
            busy = false;
            ring = false;
            noTone(tonePin);          //       switch off tones
            foundPhone = false;
            transBack = true;         //       return home    
          }
          else {                      //   if it has not yet been picked up:
            ring = true;
            ringBell();               //     generate 3 sec ring sequence
          }
        }
        else {                        // HANDSET IS PUT DOWN AFTER CALL:
          foundPhone = false;
          transBack = true;           //   call ended, return home
          ring = false;
          noTone(tonePin);            //   switch off ringtone
        }
      }   
    }
    else {                           // UNCONNECTED NUMBER BEING DIALED:
      tone(tonePin, 400);            //  put unobtainable tone on tonePin
      analogIn = analogRead(phone363Pin); // check both phones status
      val = analogRead(phone371Pin); 
      if (analogIn > 900 && val > 900) {  // return home when both handsets are down
        foundPhone = false;
        transOut = false;
        transBack = true;
        ring = false;
        calling = false;
        noTone(tonePin);    
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
    delay(2000);                           //   2 seconds OFF
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

/********************************* ROTATE CLOCKWISE ********************************/
void rotateClock() {                           // rotate clockwise
  transHomeStatus = analogRead(transHomePin);  // check translation home contact
  if (transHomeStatus < 900) {                 // if the arm is NOT retracted:
    digitalWrite(rotLockPin, HIGH);            //   block rotation
    digitalWrite(transLockPin, LOW);           //   allow translation
    transBack = true;                          //   raise flag for retracting arm
    returnHome = false;                        //   wait to return home
  }
  else {                                // if arm is retracted:    
    if (firstStep) {                    //   if this is the first step:
      digitalWrite(rotLockPin, HIGH);   //     block rotation
      digitalWrite(transLockPin, HIGH); //     block translation
      analogWrite(revPin, revSpeedLow);
      digitalWrite(motorPin1, LOW);     //     rotate motor left briefly to 
      digitalWrite(motorPin2, HIGH);    //       ensure translation lock is engaged
      delay(150);
      firstStep = false;
    }
    digitalWrite(transLockPin, HIGH);          // block translation
    digitalWrite(rotLockPin, LOW);             // allow rotation
    analogWrite(revPin, revSpeedLow);
    digitalWrite(motorPin1, HIGH);             // rotate motor right
    digitalWrite(motorPin2, LOW);
    while (rotCount < rotSteps) {     // until reaching required rotation steps: 
      newCountStatus = digitalRead(rotCountPin);   // check countPin status
      if (newCountStatus != prevCountStatus) {    // add 1 when status changes
        delay(5);                                  // debounce delay
        newCountStatus = digitalRead(rotCountPin);
        prevCountStatus = newCountStatus;
        rotCount++;
        if (rotCount == rotSteps) {            // when reaching number of steps:
          digitalWrite(rotLockPin, HIGH);      //  block rotation
          delay(100);                          //  ensure rotation lock engages
          analogWrite(revPin, 0);
          digitalWrite(motorPin1, LOW);        //  stop motor
          digitalWrite(motorPin2, LOW);
          rotate = false;
          transOut = true;                     //  set flag for arm translation
          firstStep = true;
          prevCountStatus = 0;                 //
        }
        Serial.println(rotCount);
        if (newCountStatus == 1) digitalWrite(ledPin, HIGH); 
        else digitalWrite(ledPin, LOW);        // flag even steps with LED
      }    
    }
    rotEndStatus = digitalRead(rotEndPin);    // check rotation end point
    if (rotEndStatus == 0) {
      analogWrite(revPin, 0);
      digitalWrite(motorPin1, LOW);           // stop motor
      digitalWrite(motorPin2, LOW);
      rotate = false; 
      transOut = false;
      returnHome = true;                      // prepare for home run
      Serial.println(" returning home"); 
      firstStep = true;
      rotCount = rotSteps;
    }    
  }
}

/********************************* TRANSLATE OUT ********************************/
void translateOut() {
  if (numberToCall > 359 && numberToCall < 380) { //if number is in the mat: 
    transEndStatus = analogRead(transEndPin);     // check trans end contact
    if (transEndStatus > 900) {                   // when reaching the end:
      digitalWrite(transLockPin, HIGH);           //   block translation
      analogWrite(revPin, 0);
      digitalWrite(motorPin1, LOW);               //   stop motor
      digitalWrite(motorPin2, LOW);
      transBack = true;                           // get ready to retract
      foundPhone = false;
    }
    else {                                  // until reaching translation end:
      digitalWrite(rotLockPin, HIGH);       //   block rotation
      digitalWrite(transLockPin, LOW);      //   allow translation
      if (firstStep) {                      //   wait at start of translation
        delay(500);
        firstStep = false;
      }    
      analogWrite(revPin, revSpeedHigh);    // need more power in the mat
      digitalWrite(motorPin1, HIGH);        // rotate motor right
      digitalWrite(motorPin2, LOW);
      while (transCount < transSteps) { // until reaching required trans steps: 
        val = analogRead(transCountPin);  // monitor "c" contact for +12V
        if (val > 900) countStatus = 1;    
        else countStatus = 0;
        newCountStatus = countStatus;            // check countPin status
        if (newCountStatus != prevCountStatus) { 
          delay(10);                             // debounce delay
          prevCountStatus = newCountStatus;
          if (countStatus == 1) transCount++;    // add 1 when +12V encountered
          if (transCount == transSteps) {        // when reaching number of steps:
            digitalWrite(rotLockPin, HIGH);      // block rotation
            digitalWrite(transLockPin, HIGH);    // block translation 
            delay(10);                           // ensure rotation lock engages
            analogWrite(revPin, 0);
            digitalWrite(motorPin1, LOW);        // stop motor
            digitalWrite(motorPin2, LOW);
            rotate = false;
            transOut = false;                    
            transBack = false;  
            foundPhone = true;                   // arrived at called phone
          }
          Serial.println(transCount);
            if (newCountStatus == 1) digitalWrite(ledPin, HIGH); 
          else digitalWrite(ledPin, LOW);        // flag even steps with LED
        }    
      }
    }
  }
  else {                                // if outside the mat:
    digitalWrite(rotLockPin, HIGH);     //   block rotation
    digitalWrite(transLockPin, LOW);    //   allow translation
    if (firstStep) {
      delay(500);
      firstStep = false;
    }
    analogWrite(revPin, revSpeedLow);   //   need less power outside mat
    digitalWrite(motorPin1, HIGH);      //   rotate motor right
    digitalWrite(motorPin2, LOW);       //   translate out to end stop
    transEndStatus = analogRead(transEndPin); // check translation end contact
    if (transEndStatus > 900) {         // when reaching the end:
      digitalWrite(transLockPin, HIGH); //   block translation
      analogWrite(revPin, 0);
      digitalWrite(motorPin1, LOW);     //   stop motor
      digitalWrite(motorPin2, LOW);
      tone(tonePin, 400);            //  put unobtainable tone on tonePin
      if (caller363) {
        val = analogRead(phone363Pin); // check CALLING PHONE status
      }
      if (caller371) {
        val = analogRead(phone371Pin);
      }
      if (val > 900) {         // return home when handset is put down
        foundPhone = false;
        transOut = false;
        transBack = true;
        ring = false;
        calling = false;
        noTone(tonePin);    
      }
    } 
  }
}

/************************************ RETRACT *********************************/
void translateBack() {
  digitalWrite(rotLockPin, HIGH);                // block rotation
  digitalWrite(transLockPin, LOW);               // allow translation
  if (firstStep) {
    delay(500);
    firstStep = false;
  }
  analogWrite(revPin, revSpeedHigh);
  digitalWrite(motorPin1, LOW);                  // rotate motor left
  digitalWrite(motorPin2, HIGH);
  transHomeStatus = analogRead(transHomePin);    // check translation home contact
  if (transHomeStatus > 900) {                   // if arm is retracted:
    analogWrite(revPin, 0);
    digitalWrite(motorPin1, LOW);                //   stop motor
    digitalWrite(motorPin2, LOW);
    transBack = false;                            
    returnHome = true;                           // raise flag to return home
    firstStep = true;
    delay(200); 
  }   
}

/********************************* RETURN HOME ********************************/
void goHome() {
  rotCount = 0;
  transCount = 0;
  noTone(tonePin); 
  transHomeStatus = analogRead(transHomePin);  // check translation home contact
  if (transHomeStatus < 900) {                 // if arm is NOT retracted:
    digitalWrite(rotLockPin, HIGH);            //   block rotation
    digitalWrite(transLockPin, LOW);           //   allow translation
    transBack = true;                          //   raise flag for translateBack
    returnHome = false;                        //   wait to return home
  }
  else {                                       // if arm is retracted:
    digitalWrite(transLockPin, HIGH);          //   block translation
    digitalWrite(rotLockPin, LOW);             //   allow rotation
    if (firstStep) {                           //   wait 0.5 seconds before first step
      delay(500);
      firstStep = false;
    }
    analogWrite(revPin, revSpeedLow);
    digitalWrite(motorPin1, LOW);              //   rotate motor left
    digitalWrite(motorPin2, HIGH);
  }
  rotHomeStatus = digitalRead(rotHomePin);     // check rotation home contact
  if (rotHomeStatus == 0) {                    // on returning home:
    analogWrite(revPin, 0);
    digitalWrite(motorPin1, LOW);              //   stop motor
    digitalWrite(motorPin2, LOW);
    digitalWrite(rotLockPin, HIGH);            //   block rotation
    digitalWrite(transLockPin, HIGH);          //   block translation
    returnHome = false;                        //   reset flags
    busy = false;
    noTone(tonePin);
    startUp = false;
    digitalWrite(phoneSelectorPin, HIGH); // connect 363 to selector arm (default)
    firstStep = true;
    prevCountStatus = 0;
    caller363 = false;
    caller371 = false;
    callStart = true;                          // allow dialing
    printNumber = true; 
    delay(1000);
  }  
}



