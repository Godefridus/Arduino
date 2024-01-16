
 /* - developed in Arduino IDE 2.1.0 - TESTED OK 21 Nov 2023
 
 EricssonCrossbar_v2.ino controls a single module of an Ericsson Crossbar
 exchange. The module has 10 selector relays and 4 hold relays,
 similar to the crossbar switch of an Ericsson ARD 624 Private
 Automatic Exchange (PAX). The selector relays operate 5 selector bars, 
 often referred to as the horizontals. The hold relays operate the 
 holding bars or verticals. The ARD624, however, has the switch turned 
 90 degrees inside the cabinet, so the horizontals point upwards. 
 The ARD PAX has 16 extensions and 2 connecting circuits
 (see documentation). 

 Here we simulate the ARD PAX to some extent. Control of our switch
 is by an Arduino Uno microcontroller and 11 Songle relays. Only two 
 lines are connected, where each extension can call the other line. 
 Extension numbers 4 and 17 are chosen arbitrarily to operate 
 different horizontal selectors. Using all numbers would require 6 
 more Songle relays. Two vertical contact banks are sufficient when 
 only two phones are connected. It would be possible to fully simulate 
 the operation of the ARD PAX by adding more relays and using the 
 Arduino Mega instead of the Uno.

 In principle it is not necessary to use two of the horizontal selectors
 to switch between two verticals, because all vertical contact bars 
 have fixed endpoint connections. However, this setup mimicks the 
 configuration of the ARD 624. 
 The horizontal selectors 9 and 0 are used for switching between the
 two extensions when either of them makes the call. Horizontal selectors 
 9 and 0 are therfore not available for line connections. This means 
 that the numbering of the horizontals and the extensions is somewhat 
 odd. Extension numbers 1 and 10 are not used. Extensions 2 to 8 connect 
 to the corresponding horizontal selectors, extension 9 connects to 
 selector 1. Extensions 11 to 18 connect to selectors 1 to 8. The 
 connections are normally multipled between all verticals but that thas 
 been omitted here.
 Each extension is associated with 3 contact springs in the
 switch, called 'a' and 'b' for the talk path, and 'c' for control 
 purposes. Normally, both the 'a' and 'b' wires are switched in an 
 exchange. For simplicity we only switch the 'a' wire and ground the 'b' 
 wire. 

 Arduino pins D2 to D7 are used to operate 5V Songle relays. They  
 connect the respective selector coils to ground in order to 
 activate the switch magnets. The other end of the coils is connected
 to a 24V power supply. D2 operates the V1 hold magnet, D3 operates V0, 
 D4 operates horizontal selector magnet 4, D5 controls magnet 7, D6 
 turns the fifth horizontal selector bar to magnet 0, and D7 turns it 
 to magnet 9. These latter two magnets control the switching between 
 the two extensions, as they connect to verticals V0 and V1. They are 
 referred to here as HA and HB, respectively. 
 The horizontal selectors operate the fingers that move the 
 spring contacts of the vertical banks when the associated vertical 
 selector magnet is activated. The vertical magnet holds the contact 
 springs in place during a connection, where the horizontal selector 
 is released to create another link if required.  
 Digital pin D8 selects the extension that is being called, in order to
 pass the ring sequence and to connect analog input A2. This latter input
 is used to detect the status of the called phone, whether it is free
 or engaged. Input A2 is disconnected by the ring sequence relay during 
 periods where the ring voltage is high. This protects that input.
 The default position of the V select relay (normally closed NC) connects
 line 17.
 Pin D9 is used for connecting V0 and V1 to create the communication path.
 Pin D10 operates the ring sequence relay. The ringer module produces 
 a 16Hz ring signal continuously. This signal is interrupted by the 
 ring sequence relay to produce the British standar ring sequence. 
 Simultaneous with the ring sequence, the Arduino sends a sequenced 
 150 Hz tone to pin D12 via a 100nF capacitor. 
 Dial selector pin D11 controls the tone relay and connects the calling
 line. The default position (normally closed NC) connects line 4. Dial
 tone is 150 Hz continuous.
 Note that digital pins D0 and D1 cannot be used when the serial link 
 is programmed for debugging.

 Arduino analog inputs accept voltages between 0 and 5V. These voltages
 are converted to integers, ranging from 0 to 1024. Inputs A0 and A1 read
 approx 5V when both handsets are on their cradle, A2, A3 and A4 are at 0V. 
 To allow for some variation the integer values are interpreted as above 
 900, below 50 or between 50 and 900. 
 Analog input A0 is permanently connected to line 4, using a 4k7/18k 
 voltage divider, to bring the 24V line voltage down to 5V. A 5V6 zener
 protects the input when the ring signal is put on the line. The same
 configuration is used for line 17, which is connected to analog A1. 
 A0 and A1 identify the caller, ext 4 or ext 17 respectively.
 A 2.2µF capacitor connects the extensions once a connection is establised.
 A not-available (N/A) tone of 400 Hz is put on the calling line when
 neither a 4 or 17 have been dialled. 
 Arduino analog inputs A3 and A4 are used to detect if the 'c' wire of
 a called extension sees 5V in either V0 or V1. This confirms that an
 available number has been dialled.
 
 OPERATION LOOP:     
 The Arduino program has a main loop that calls a number of functions 
 depending on the value of boolean parameters. On (callStart), the function
 checkPhoneStatus() waits for a handset to be lifted. It operates the 
 selectors and holds the calling line in function connectCaller(). It 
 sets (initCall) to true. This calls the function dialNumber(). On 
 (numberComplete) the called number selectors are operated in function 
 ringNumber(). The called line is tested on availability and the ringsignal 
 is put through. On (foundPhone) the talk connection is established in 
 function connectTalk().

 At startup all parameters are set to initial conditions. This also happens
 when a call is ended. All magnets are de-activated, the tone relay connects
 the tone output to V0a, the ringer is disconnected and the Vsel relay 
 connects to V1a. Note that digital outputs are set to LOW to activate the 
 Songle relays, i.e. to switch from normally open (NO) to normally closed
 (NC). 
 Arduino detects a voltage drop on A0 when extension 4 lifts the handset.
 This activates selector magnets HA and 4, followed by hold magnet V0. 
 The 'a' wire of ext 4 is now connected to the 'a' wire of HA, 
 called V0a. Dial tone is sent to ext 4 from V0a. A0 drops to 0 as soon 
 as the caller turns the dial clockwise. This initiates the analysis of the
 dialled number. The transitions between 0 and 5V are counted to find 
 the digits.  A short contact debounce delay has been added to ensure
 consistent step counting. If the first digit is not '1', the number is 
 complete and it has only one digit. If the first digit is '1' the 
 analysis continues to find the second digit. Only '17' is a valid number 
 if ext 4 is calling. A non-available tone is sent to ext 4 if another 
 number has been dialled.
 Selectors HB and 7 are activated when the number to call is complete, 
 followed by hold magnet V1. This connects the 'a' wire of ext 17 to 
 the 'a' wire of HB, here called V1a.
 The called line is now tested on availability. Busy tone is sent if 
 the handset is off its cradle at the moment of connection. If the line is 
 available, the ring sequence is started and ringtone sent to ext 4. The
 communication link is established when ext 17 lifts the handset. The call 
 ends when both handsets are put down.
 The same procedure is followed when ext 17 calls ext 4. Lifting the
 handset of 17 activates selectors HB and 7, followed by hold magnet V1.
 Selector relay Vsel connects the tone signal to V1a. Pulses are now
 counted on A1 to find the number to be called, where only '4' is valid.
 A non-available tone is sent otherwise. Selectors HA and 4 are activated
 followed by hold magnet V0. The line of ext 4 is tested on availability.
 The communication link is establised when ext 4 lifts the handset.

 Note that command "if (a < x < b) {" is accepted by the compiler but 
 does not work. Use "if ((x > a) && (x < b)) {". 
 Voltage level on inputs A0 (phone 4) or A1 (phone 17) when calling:
 
       handset down                                       call ended    
 5V   ____________  handset           _   _               ___________
                  | lifted           | | | |   ring/call |
 ~ 1V             |________  dialing | | | |  ___________|
 0V                        |_________| |_| |_|                 
 
      created by Wim der Kinderen 21 November 2023
 */

int phone4Pin = A0;       // phone 4 connects to analog pin A0
int phone17Pin = A1;      // phone 17 connects to A1
int calledPhonePin = A2;  // called phone connects to A2
int holdV1Pin = 2;        // vertical V1 hold relay connects to digital D2
int holdV0Pin = 3;        // V0 hold relay connects to D3
int select4Pin = 4;       // horizontal selector 4 relay connects to D4
int select7Pin = 5;       // selector 7 relay connects to D5
int selectHAPin = 6;      // selector HA connects to D6
int selectHBPin = 7;      // selector HB connects to D7
int selectVPin = 8;       // V select relay connects to D8
int talkPin = 9;          // talk relay connects to D9
int ringerPin = 10;       // D10 controls the ring sequence relay (ring is LOW)
int dialSelectorPin = 11; // D11 connects dial tone to caller
int tonePin = 12;         // D12 sends dial, ring or unavailable tone to caller 
int ledPin = 13;          // LED to show countPin status
int debounceDelay = 10;   // debounce delay of dial pulses in ms
int analogIn4 = 0;        // integer value of analog input A0 (0 - 1024)
int analogIn17 = 0;       // integer value of analog input A1 (0 - 1024)
int dialReading = 0;      // 
int dialLastState = 0;    // required for dial debounce routine
int dialTrueState = 0;    // status of dial contact
long dialLastStateChangeTime = 0; // time when status last changed
int dialHasFinishedRotatingAfterMs = 100; // waiting time after dial rotation
int needToAddDigit = 0;   // need to add digit
int numberToRing = 17;    // phone number to ring as integer
int val = 0;              // integer value of analog input (0 - 1024)
int count = 0;            // counters
int digit = 0;
int digitCount = 0;
String numberString;        // phone number as string
boolean startUp = true;     // flag to indicate return home on startup
boolean callStart = true;   // flag to check who is going to call
boolean calling = false;    // flag to indicate ongoing call
boolean caller4 = false;    // flag to indicate if phone 4 is calling
boolean caller17  = false;  // flag to indicate if phone 17 is calling
boolean numberComplete = false; // flag to indicate that number to call is built
boolean initDial = false;   // flag to indicate that dialing can commence
boolean printNumber = true; // flag to indicate that number can be sent to serial line
boolean callerFlag = true;  // flag to indicate that caller can be connected
boolean printFlag = true;
boolean holdLine = false;   // flag to hold the vertical selectors
boolean ring = false;       // flag to mark that ringing is requested
boolean foundPhone = false; // flag to indicate that called number has been reached
boolean busy = false;       // flag to indicate that the called phone is engaged

/******************************* STARTUP **********************************/
void setup() {
  pinMode (holdV1Pin, OUTPUT); 
  pinMode (holdV0Pin, OUTPUT);
  pinMode (select4Pin, OUTPUT);
  pinMode (select7Pin, OUTPUT);
  pinMode (selectHAPin, OUTPUT);
  pinMode (selectHBPin, OUTPUT);
  pinMode (selectVPin, OUTPUT);        // selects the called phone
  pinMode (talkPin, OUTPUT);           // establishes communication
  pinMode (ringerPin, OUTPUT);         // generates the ring sequence
  pinMode (tonePin, OUTPUT);           // sends the various tones
  pinMode (dialSelectorPin, OUTPUT);   // selects the calling phone
  pinMode (ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  digitalWrite(holdV1Pin, HIGH);       // de-activate all switch relays
  digitalWrite(holdV0Pin, HIGH);
  digitalWrite(select4Pin, HIGH);
  digitalWrite(select7Pin, HIGH);
  digitalWrite(selectHAPin, HIGH);
  digitalWrite(selectHBPin, HIGH);
  digitalWrite(selectVPin, HIGH);      // V0/V1 select pin to V1a by default
  digitalWrite(talkPin, HIGH);         // talk link is open by default 
  digitalWrite(ringerPin, HIGH);       // ringer is disconnected by default     
  digitalWrite(dialSelectorPin, HIGH); // tone is connected to V0a by default 
  Serial.begin(9600);          // only use when D0 and D1 are not required
  delay(1000); 
  Serial.print(" - starting ");
}
/******************************* MAIN LOOP **********************************/
void loop() {    
  if (callStart) checkPhoneStatus();     // check calling phone status
  if (analogIn4 > 900 && analogIn17 > 900 && !initDial) endCall(); // end the
                      // call when caller returns the handset before dialling 
  if (initDial) dialNumber();            // build number to call on initDial
  if (numberComplete) ringNumber();      // call extension
  if (foundPhone) connectTalk();         // connect when phone is available
}
/*************************** CHECK PHONE STATUS *****************************/
void checkPhoneStatus() {       // CHECK WHICH PHONE IS DIALING (4 or 17)
  analogIn4 = analogRead(phone4Pin);  //   READ PHONE 4 PIN  (A0)
  delay(5);                     //   limit number of read cycles
  if (analogIn4 > 900) {        //   above 4.5V: dial contact open
    calling = false;            //    handset 4 is down      
    caller4 = false;            //    4 is not dialling nor calling
    if (initDial) {             //   dialing starts after initial closed contact
      noTone(tonePin);          //   stop dialtone during dialing
      callStart = false;        //   no further checking of status required 
    }
  }     
  if ((analogIn4 > 50) && (analogIn4 < 900)) { // 0.5 - 4.5V: handset 4 lifted
    caller4 = true;                            // 4 is ready for dialling
    digitalWrite(dialSelectorPin, HIGH); // connect tone relay to V0a (default)
    tone(tonePin, 150);                  // put dialtone on tonePin
    if (callerFlag) {                    // operate switch to hold V0
      Serial.print(" - start dialing from 4  "); 
      connectCaller();
      callerFlag = false;                // allow connecting caller once
    } 
  }
  if (analogIn4 < 50) {            // below 0.5 V: dial contact closes when 
                                   //   dial is turned clockwise
    digitalWrite(dialSelectorPin, HIGH); // connect tone relay to V0a
    caller4 = true;                // 4 has started dialing
    calling = false;               // no communication yet
    callStart = false;             // no further checking of status required
    initDial = true;               // dial has started rotating clockwise
    numberString = "";             // clear number buffer
    needToAddDigit = 0;
    digitalWrite(ringerPin, HIGH); // de-activate ringer relay (default) 
  }
  if (!caller4) {              // 4 takes priority if both handsets are lifted
    analogIn17 = analogRead(phone17Pin);  // READ PHONE 17 PIN  (A1)
    delay(5);                  // limit number of read cycles
    if (analogIn17 > 900) {    // above 4.5V: dial contact open
    calling = false;
    caller17 = false;          // 17 handset is down
      if (initDial) {          // dialing starts after initial closed contact
        noTone(tonePin);       // stop dialtone during dialing
        callStart = false;     // no further checking of status required
      }
    }     
    if ((analogIn17 > 50) && (analogIn17 < 900)) {   // handset 17 lifted
      caller17 = true;                     // 17 is ready for dialing
      digitalWrite(dialSelectorPin, LOW);  // connect tone relay to V1a
      tone(tonePin, 150);                  // put dialtone on tonePin
      if (callerFlag) {
        Serial.print(" - start dialing from 17  "); // allow connecting caller once
        connectCaller();
        callerFlag = false;
      } 
    }
    if (analogIn17 < 50) {            // below 0.5 V: dial contact closed
      digitalWrite(dialSelectorPin, LOW); // connect 17 to tone relay
      caller17 = true;               // 17 has started dialing
      calling = false;
      callStart = false;             // no further checking of status required
      initDial = true;               // dial has started rotating clockwise
      numberString = "";             // clear number buffer
      needToAddDigit = 0;
      digitalWrite(ringerPin, HIGH); // de-activate ringer relay  
    }
  }
} 
/************************ CONNECT CALLING PHONE  ***************************/
void connectCaller() {
  numberComplete = false;                 // stop ring a number
  digitalWrite(ringerPin, HIGH);          // de-activate ring relay
    if (caller4) {                        // if number 4 is calling:
      digitalWrite(selectVPin, HIGH);     //   connect ringer to 17
      digitalWrite(selectHAPin, LOW);     // activate relay HA (horizontal A)   
      digitalWrite(select4Pin, LOW);      // activate relay 4 (horizontal 4)
      delay(300);                         // let selector finger settle
      digitalWrite(holdV0Pin, LOW);       // activate relay V0 (vertical V0)
      delay(100);                         // 
      digitalWrite(selectHAPin, HIGH);    // release relay HA   
      digitalWrite(select4Pin, HIGH);     // release relay 7, V0 holds
      analogIn4 = analogRead(phone4Pin);  // check if 4 has put handset down
      if (analogIn4 > 900) endCall();
    }
    if (caller17) {                       // if number 17 is calling: 
      digitalWrite(selectVPin, LOW);      // connect ringer to 4
      digitalWrite(selectHBPin, LOW);     // activate relay HB (horizontal B)   
      digitalWrite(select7Pin, LOW);      // activate relay 7 (horizontal 7)
      delay(300);
      digitalWrite(holdV1Pin, LOW);       // activate relay V1 (vertical V1)
      delay(100);                         // 
      digitalWrite(selectHBPin, HIGH);    // release relay HB   
      digitalWrite(select7Pin, HIGH);     // release relay 7, V1 holds
      analogIn17 = analogRead(phone17Pin);  // check if 17 has put handset down
      if (analogIn17 > 900) endCall();
    }
}
/********************************* DIAL THE NUMBER ****************************/
void dialNumber() {                        // dialing has started
  noTone(tonePin);                         // switch off dialtone
  if (caller4) {
    val = analogRead(phone4Pin);           // read pin of calling phone
  }
  if (caller17) {
    val = analogRead(phone17Pin);
  } 
  if (val < 900) dialReading = 0;         // convert analog reading to state
  if (val > 900) dialReading = 1;
  if (!numberComplete) {
    checkDialing();                           // build the number to call
    numberToRing = numberString.toInt();      // convert string to integer
    if (numberToRing > 20) numberToRing = 18; // limit numbers to range 0 - 19
  }
  if (numberComplete) {
    if (printNumber) {
      Serial.print(" - Number to ring: ");
      Serial.println(numberToRing); 
      printNumber = false;  
      initDial = false;   
    }
  }
}   
/**************************** DEAL WITH DIAL PULSES *************************/
void checkDialing() {   
  if ((millis() - dialLastStateChangeTime) > dialHasFinishedRotatingAfterMs) {
    // the dial isn't being used, or dialing a digit has just finished
    if (needToAddDigit) {
      // if the dial has returned, the number is sent down the serial line and
      //  the count is reset. A count of 10 is modified because '0' will send
      // 10 pulses
      digit = count % 10;
      digitCount++;
      numberString = numberString + digit;  // build the number as a string
      if (numberString != "1") { 
          numberComplete = true;
      }
      needToAddDigit = 0;
      count = 0;
      if (digitCount == 2) numberComplete = true;
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
/********************************* RING NUMBER  ***************************/
void ringNumber() {
  if (!holdLine) {                        // to prevent multiple activations
    if (numberToRing == 17) {             // if number to call is 17
      digitalWrite(selectVPin, HIGH);     // connect ringer to 17  -  MOVED
      digitalWrite(selectHBPin, LOW);     // activate relay HB (horizontal B)   
      digitalWrite(select7Pin, LOW);      // activate relay 7 (horizontal 7)
      delay(300);                         // wait for selector fingers to settle
      digitalWrite(holdV1Pin, LOW);       // activate relay V1 (vertical V1)
      delay(100);
      digitalWrite(selectHBPin, HIGH);    // release relay HB   
      digitalWrite(select7Pin, HIGH);     // release relay 7, V1 holds
    }
    if (numberToRing == 4) {              // if number to call is 4
      digitalWrite(selectVPin, LOW);      // connect ringer to 4
      digitalWrite(selectHAPin, LOW);     // activate relay HA (horizontal A)   
      digitalWrite(select4Pin, LOW);      // activate relay 4 (horizontal 4)
      delay(300);
      digitalWrite(holdV0Pin, LOW);       // activate relay V0 (vertical V0)
      delay(100);
      digitalWrite(selectHAPin, HIGH);    // release relay HA   
      digitalWrite(select4Pin, HIGH);     // release relay 4, V0 holds
    }
    holdLine = true;                      // hold the called line
  }
  if (numberToRing == 17 || numberToRing == 4) { // only nrs 4 and 17 allowed
    val = analogRead(calledPhonePin);       // check CALLED PHONE status (A2)
    if (printFlag) {
      Serial.print(" - calledPhonePin = "); // print value of A2 input once
      Serial.print(val);
      printFlag = false;
    }
    if (val > 900) {                 // if called handset is down
      if (!calling) {                // if not talking:
        if (busy) {                  //  if this is from a busy tone status:
          Serial.print(" - handset put down after phone busy  "); 
          endCall();                 // end the call
        }
        else {                       // if it has not yet been picked up:
          ring = true;               //   generate 3 sec ring sequence
          ringBell();                
        }
      }
      else {                         // HANDSET IS PUT DOWN AFTER CALL:
        Serial.print(" - called handset is put down after call ");
        endCall();
      }
    }  
    if ((val > 50) && (val< 900)) {   // if called handset is off hook:
      if(!ring) {                     //  if engaged when being connected:
        busy = true;                  //   run busy tone sequence and wait
        busyTone();                   //     for either handset to be put down
      }
      else {                          // if handset is lifted when ringing:
        foundPhone = true;            //   ready to connect to caller
        numberComplete = false;
        noTone(tonePin);              //   switch off ringtone
        Serial.println(" - ready to connect ");
      }
    } 
  }
  if (numberToRing != 17 && numberToRing != 4) { // number other than 4 or 17:
    tone(tonePin, 400);                 //  put unobtainable tone on tonePin
    digitalWrite(ringerPin, HIGH);      //  de-activate ring relay
    analogIn4 = analogRead(phone4Pin);  // check calling phone status
    analogIn17 = analogRead(phone17Pin);
    if (analogIn4 > 900 && analogIn17 > 900) {
      Serial.print(" - wrong number  "); // after handsets have been put down
      endCall();
    }   
  }
}

/*************************** GENERATE RING SEQUENCE ***********************/
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
    if (caller4) {
      analogIn4 = analogRead(phone4Pin);   // check calling phone status
      if (analogIn4 > 900) endCall();  
    }
    if (caller17) {   
      analogIn17 = analogRead(phone17Pin); 
      if (analogIn17 > 900) endCall();
    }
    delay(1900);                           // in total 2 seconds OFF
  }  
}
/******************* GENERATE BUSY TONE SEQUENCE *********************/
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
/************************ CONNECT CALLED PHONE  ***************************/
void connectTalk() {
  calling = true;                       // ready for talk link
  digitalWrite(talkPin, LOW);           // establish communication
  analogIn4 = analogRead(phone4Pin);    // check both phones status
  analogIn17 = analogRead(phone17Pin);  // end call when both handsets are down
  if (analogIn4 > 900 && analogIn17 > 900) {
    Serial.print(" - both handsets down  ");
    endCall();
  }
}
/********************************* END CALL ************************/
void endCall() {
  dialReading = 0;              // 
  dialLastState = 0;            // required for dial debounce routine
  dialTrueState = 0;            // status of dial contact
  dialLastStateChangeTime = 0;  // time when status last changed
  needToAddDigit = 0;           // need to add digit
  val = 0;                      // integer value of analog input (0 - 1024)
  count = 0;                    // counters
  digit = 0;
  digitCount = 0;
  numberComplete = false;
  initDial = false;
  foundPhone = false;
  holdLine = false;
  ring = false;
  busy = false;
  startUp = false;
  caller4 = false;
  caller17 = false;
  calling = false;
  callStart = true;                // allow dialing
  callerFlag = true;
  printFlag = true;
  printNumber = true; 
  noTone(tonePin);                 //   switch off tones
  digitalWrite(holdV0Pin, HIGH);   // release relay V0 (vertical V0)
  digitalWrite(holdV1Pin, HIGH);   // release relay V1 (vertical V1)
  digitalWrite(selectVPin, HIGH);  // connect ringer to 17 (default)
  digitalWrite(dialSelectorPin, HIGH);  // connect 4 to V0a (default)
  digitalWrite(talkPin, HIGH);     // stop communication
  delay(500); 
}




