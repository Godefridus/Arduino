
 /* - developed in Arduino IDE 2.3.3
 GDKconverter.ino controls the converter that translates the keystrokes
 of three DC signalling keypads into dialpulses for e.g. a Strowger
 telephone exchange. The keypads are of a) a GPO SA4252 telephone,
 b) a first generation Philips GDK (GDK1), and c) a second 
 generation Philips GDK (GDK2). A rotary switch allows selection 
 between the keypads. The Arduino monitors analog input A2 for 
 this selection. A2 is 0V (0 counts) for SA4252, 2.5V for GDK1 
 (512) and 5V for GDK2 (1023). 
 The SA4252 and GDK2 have full telephone circuitry, the GDK1 has 
 only just the keypad. For the SA4252 and GDK2 the converter 
 allows an incoming call to come through. To make a call, the 
 caller lifts the handset and pushes the "key start" button on 
 the converter. This switches the telephone "a" line to the 
 converter and interrupts the "a" line to the exchange. At the 
 same time the exchange is held with a 500 Ohm coil from a bell 
 circuit. The keypad circuit is now powered from the 5V supply of 
 the Arduino. The call ends when the caller returns the handset.
 For GDK1 there is no communication path because there is no 
 telephone circuit. Here the connection ends automatically
 after 5 seconds. 

 A small modification has been made to the SA4252 to make it work
 with the Arduino. One 1k resistor has been added between the "a"
 wire and the keypad circuit, another 1k resistor between the "b" 
 line and the circuit, and a 33k resistor between Earth and "b", 
 see schematic. These modifications are required to allow 
 sufficient margin on the A1 voltage when a key is released.
 GDK1 is operated with its yellow and blue wire connected. This 
 provides a voltage of 5V on the Earth wire when no key is pushed.

 The SA4252 and GDK1 use the Earth wire for signalling. The voltage 
 on that Earth wire (E) is monitored by Arduino analog input A1. 
 Arduino switches the supply between +5V on "a" and GND on "b", 
 and +5V on "b" and GND on "a". The Earth wire is connected
 to the "a" wire with a 10k resistor and to the "b" wire with 22k.
 A relay has been included to protect A1 from high voltage 
 on the Earth wire when buttons are pushed while the phone is 
 connected to the exchange. 

 The SA4252 and GDK1 now have similar keypad circuitry, the key
 voltage combinations being different. Therefore the Arduino 
 programme has different "getNumber" subroutines for each of them.
 Each digit on the keyboard has a different combination of voltages.
 The GDK2 does not use the Earth wire. Arduino monitors analog input
 A0 for voltage changes when a key is pushed and the polarity of the
 a and b wires is alternated. In fact, A0 measures the current 
 through a 500 Ohm resistor in series with the 5V supply.

 For the SA4252:
 When "a" is positive, the voltage for keys 1 and 3 is 884mV
 (181 counts on A1), for 2, 5 and 9 it is 2.86V (585) counts), for
 4 and 6 its is 424mV (87 counts), for 7 and # it is 2.52V
 (516 counts) for 8 and 0 it is 4.69V (960 counts), for * it is 
 2.23V (456 counts). 
 When "a" is negative the voltages are:
 keys 1,7 and 0: 2.17V (444), keys 2,3: 824mV (169), keys 4,*: 4.58V 
 (937), keys 5,#: 2.49V (509), key 6: 2.79V (571), key 8,9:
 317mV (65). 
 These combinations are tested  after a key is pushed. The next key 
 is only available after a short period of release, where the voltage 
 on A1 is 3.45V (707 counts) when "a" is positive and 1.55V (317) 
 when "a" is negative.

 For GDK1: 
 When "a" is positive, the voltage for keys 1 and 7 is 3.45V (706 
 counts on A1), for 2,3,8 and 9 it is 940mV (192 counts), for 4 and 0 
 its is 2.59V (530 counts), for 5,6,+ and - it is 4.33V (884 counts).
 When "a" is negative the voltages are:
 keys 1,2 and +: 660mV (135 counts), keys 3,4,5: 1.55V (317), 
 keys 6,9 and 0: 4.05V (829), keys 7,8 and -: 2.43V (497)
 These combinations are tested after a key is pushed. The next key 
 is only available after a short period of release, where the voltage 
 on A1 is 5V (1023) when "a" is positive and 0V (0) when "a" is negative. 
 These combinations are tested  after a key is pushed. The next key 
 is only available after a short period of release, where the voltage 
 on A1 is 5V (1023 counts) when "a" is positive.

 for GDK2:
 When "a" is positive, the voltage for keys 1, 2 and 3 is 870mV
 (178 counts on A0), for 4,5 and 6 it is 4.3V (880 counts), for
 7, 8 and 9 its is 4.72V (966 counts), and for the red, 0 and white
 key it is 5V (1023 counts). When "b" is positive the voltages are:
 keys 1,4,7,red: 5V (1023), keys 2,5,8,0: 4.72V (966) and
 keys 3,6,9,white: 4.32V (884). These combinations are tested for 
 after a key is pushed. The next key is only available after a
 short period of release, where the voltage on A0 depends on the 
 resistance of the phone circuit which is nominally 500 Ohm. 
 The resistance depends on the position of the microphone in the 
 handset. A wide margin is assumed to capture the key release.

 Once THREE digits have been counted the keyboard is disabled
 and the number is being dialled. The interrupt relay is activated
 following the order of the digits. The contact is open for 67 msec
 and closed for 33 msec. The line is held for a short moment
 between the digits.
    
    created by Wim der Kinderen 16 September 2025
 */

int ledPin = 13;
int dialstartPin = 2;     // D2 default at 5V; switched to GND for dialling
int polarityPin = 3;      // polarity switch relay connects to digital D3
int lineswitchPin = 4;    // D4 switches between Strowger and converter
                          // and connects the Earth wire to analog A1
int interruptPin = 5;     // D5 generates the dialling pulses
int holdPin = 6;          // D6 holds Strowger during dialling
int analogPin0 = 0;       // A0 reads the linevoltage when using GDK2  
int analogIn0;            // integer value of analog A0 (0 - 1023)
int analogPin1 = 1;       // A1 reads the voltage on the Earth wire 
int analogIn1;
int analogPin2 = 2;       // A2 reads the voltage on the phoneselector
                          // switch: 0V = SA4252, 2.5V = GDK1, 5V = GDK2
int analogIn2;
int phone = 2;            // default selected phone is GDK1
int dialStart;
int keyReading;
int digitCount = 0;
int i;
int digit1 = 0;
int digit2 = 0;
int digit3 = 0;
boolean readyforKeys = false;
boolean alreadyPrinted = false;
boolean keyRelease = false;
boolean dialReady = false;

/******************************* SETUP ********************************/
void setup() {
  pinMode(dialstartPin, INPUT);    // pushbutton in the schematic
  pinMode(polarityPin, OUTPUT);    // relays P 
  pinMode(lineswitchPin, OUTPUT);  // relays L and E
  pinMode(interruptPin, OUTPUT);   // relay D 
  pinMode(holdPin, OUTPUT);        // relay H 
  digitalWrite(ledPin, LOW);
  digitalWrite(polarityPin, HIGH);    // P de-activated is default
  digitalWrite(lineswitchPin, HIGH);  // L de-activated is default
  digitalWrite(interruptPin, HIGH);   // D de-activated is default
  digitalWrite(holdPin, HIGH);        // H de-activated is default
  alreadyPrinted = false;
  keyRelease = true;
  Serial.begin(115200);           
  Serial.println(" starting ");
  delay(1000); 
}
/******************************* MAIN LOOP ************************/
void loop() { 
  digitalWrite(polarityPin, HIGH);        // start polarity a+
  analogIn2 = analogRead(analogPin2);     // select keypad
  if (analogIn2 < 100) phone = 1;         // 0V = SA4252
  else if (analogIn2 > 950) phone = 3;    // 5V = GDK2
  else phone = 2;                         // default GDK1
  dialStart = digitalRead(dialstartPin);  // check start button
  if (dialStart == LOW) {                 // start button pushed
    readyforKeys = true;                  // ready for keypad
    digitCount = 0;                       // reset digit counter
    dialReady = false;                    // disable dialling
    Serial.print("ready for keypad ");
    Serial.println(phone); 
    if ((phone == 1) || (phone == 3)) digitalWrite(holdPin, HIGH);  
                       // release holdpin when using SA4252 or GDK2
    delay(500);
  }  
  if ((readyforKeys) && (digitCount < 3) && (phone == 1)) SA4252();
  if ((readyforKeys) && (digitCount < 3) && (phone == 2)) GDK1();
  if ((readyforKeys) && (digitCount < 3) && (phone == 3)) GDK2();
          // jump to respective subroutine to collect digits
  if (dialReady) dialNumber();            // send pulses when 3
                                          // digits enetered
}
/**************************** DIAL THE NUMBER ************************/
void dialNumber() {
  readyforKeys = false;                 // disable keyboard
  Serial.println(100*digit1 + 10*digit2 + digit3);
  if (digit1 == 0) digit1 = 10;
  else if (digit2 == 0) digit2 = 10;
  else if (digit3 == 0) digit3 = 10;
  delay(500);
  digitalWrite(interruptPin, LOW);    // close dialcontact
  digitalWrite(holdPin, HIGH);        // release hold
  delay(500);
  for (i = 0; i < digit1; i++) {
    digitalWrite(interruptPin, HIGH); // open dialcontact
    delay(67);
    digitalWrite(interruptPin, LOW);  // close dialcontact
    delay(33);
  }
  digitalWrite(holdPin, LOW);         // hold the exchange
  digitalWrite(interruptPin, HIGH);  // open dialcontact
  delay(500);
  digitalWrite(interruptPin, LOW);   // close dialcontact
  digitalWrite(holdPin, HIGH);       // release hold
  delay(300);
  for (i = 0; i < digit2; i++) {
    digitalWrite(interruptPin, HIGH); // open dialcontact
    delay(67);
    digitalWrite(interruptPin, LOW); // close dialcontact
    delay(33);
  }
  digitalWrite(holdPin, LOW);       // hold the exchange
  digitalWrite(interruptPin, HIGH); // open dialcontact
  delay(500);
  digitalWrite(interruptPin, LOW);  // close dialcontact
  digitalWrite(holdPin, HIGH);      // release hold
  delay(300);
  for (i = 0; i < digit3; i++) {
    digitalWrite(interruptPin, HIGH); // open dialcontact
    delay(67);
    digitalWrite(interruptPin, LOW); // close dialcontact
    delay(33);
  }
  digitalWrite(holdPin, LOW);       // hold the exchange
  digitalWrite(interruptPin, HIGH); // open dialcontact
  delay(300);
  digitalWrite(lineswitchPin, HIGH); // switch line to phone
  if (phone == 2) delay(5000);      // hold exchange for 
                              // 5 seconds when using GDK1
  digitalWrite(holdPin, HIGH);    // release hold
  dialReady = false;
  digitCount = 0;
}
/******************* GET NUMBER TO CALL ON SA4252 *************/
void SA4252() {  
  dialReady = false;                    // disable dialling
  digitalWrite(holdPin, LOW);           // engage/hold the exchange
  digitalWrite(lineswitchPin, LOW);     // switch line to converter
  delay(100);                           // essential delay
  analogIn1 = analogRead(analogPin1); 
  Serial.println(analogIn1); 
  if ((analogIn1 > 687) and (analogIn1 < 727)) {
        // A1 is approx 3.45V when handset is lifted
    if (alreadyPrinted == false) {
      Serial.println("waiting for key"); // print once
      alreadyPrinted = true;
    }
  }
  if ((analogIn1 > 161) && (analogIn1 < 201)) { // nominal 0.884V
        // = 181 counts; could be 1 or 3 at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 418) && (analogIn1 < 458)) {
        // nominal -2.14 = 438 counts for key 1 at a-
      keyReading = 1;
      Serial.println("Key 1 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 149) && (analogIn1 < 189)) {
        // nominal -824mV = 169 counts for key 2 at a-
      keyReading = 3;
      Serial.println("Key 3 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    digitCount++;
    if (digitCount == 1) digit1 = keyReading;
    else if (digitCount == 2) digit2 = keyReading;
    else if (digitCount == 3) digit3 = keyReading;
  }
  while (!keyRelease) {
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 687) and (analogIn1 < 727)) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn1 = analogRead(analogPin1);
  }
  if ((analogIn1 > 555) && (analogIn1 < 605)) { // nominal 2.86V
        // = 585 or 575 counts; could be 2, 5 or 9 at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 149) && (analogIn1 < 189)) {
        // nominal -824mV = 169 counts for key 2 at a-
      keyReading = 2;
      Serial.println("Key 2 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 489) && (analogIn1 < 529)) {
        // nominal -2.49V = 509 counts for key 5 at a-
      keyReading = 5;
      Serial.println("Key 5 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 45) && (analogIn1 < 85)) {
        // nominal -317mV = 65 counts for key 9 at a-
      keyReading = 9;
      Serial.println("Key 9 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    digitCount++;
    if (digitCount == 1) digit1 = keyReading;
    else if (digitCount == 2) digit2 = keyReading;
    else if (digitCount == 3) digit3 = keyReading;
  }
  while (!keyRelease) {
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 687) and (analogIn1 < 727)) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn1 = analogRead(analogPin1);
  }
  if ((analogIn1 > 67) && (analogIn1 < 107)) { // nominal 424mV
        // = 87 counts; could be 4 or 6 at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 917) && (analogIn1 < 957)) {
        // nominal -4.58V = 937 counts for key 4 at a-
      keyReading = 4;
      Serial.println("Key 4 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 551) && (analogIn1 < 591)) {
        // nominal -2.79V = 571 counts for key 6 at a-
      keyReading = 6;
      Serial.println("Key 6 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    digitCount++;
    if (digitCount == 1) digit1 = keyReading;
    else if (digitCount == 2) digit2 = keyReading;
    else if (digitCount == 3) digit3 = keyReading;
  }
  while (!keyRelease) {
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 687) and (analogIn1 < 727)) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn1 = analogRead(analogPin1);
  }
  if ((analogIn1 > 496) and (analogIn1 < 538)) { // nominal 2.52V 
        // = 516 counts, could be 7 or # at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 418) && (analogIn1 < 458)) {
        // nominal -2.14V = 438 counts for key 7 at a-
      keyReading = 7;
      Serial.println("Key 7 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 487) && (analogIn1 < 527)) {
        // nominal -2.48V = 507 counts for key # at a-
      keyReading = 11;
      Serial.println("Key # pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if (keyReading == 7) {
      digitCount++;
      if (digitCount == 1) digit1 = keyReading;
      else if (digitCount == 2) digit2 = keyReading;
      else if (digitCount == 3) digit3 = keyReading;
    }
  }
  while (!keyRelease) {
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 687) and (analogIn1 < 727)) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn1 = analogRead(analogPin1);
  }
  if ((analogIn1 > 940) && (analogIn1 < 980)) { // nominal 4.69V
        // = 960 counts; could be 8 or 0 at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 45) && (analogIn1 < 85)) {
        // nominal -317mV = 65 counts for key 8 at a-
      keyReading = 8;
      Serial.println("Key 8 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 424) && (analogIn1 < 464)) {
        // nominal -2.17V = 444 counts for key 0 at a-
      keyReading = 0;
      Serial.println("Key 0 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    digitCount++;
    if (digitCount == 1) digit1 = keyReading;
    else if (digitCount == 2) digit2 = keyReading;
    else if (digitCount == 3) digit3 = keyReading;
  }
  while (!keyRelease) {
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 687) and (analogIn1 < 727)) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn1 = analogRead(analogPin1);
  }
  if ((analogIn1 > 436) && (analogIn1 < 476)) { // nominal 2.23V
        // = 456 counts; key * at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 917) && (analogIn1 < 957)) {
        // nominal -4.58V = 937 counts for key * at a-
      keyReading = 12;
      Serial.println("Key * pushed");   // reset key
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = true;
      digitalWrite(lineswitchPin, HIGH);  // switch line to phone
      digitalWrite(holdPin, HIGH);        // release hold
      digitCount = 0;
      readyforKeys = false;
      dialReady = false;
    }
  }
  while (!keyRelease) {
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 687) and (analogIn1 < 727)) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn1 = analogRead(analogPin1);
  }
  if (digitCount == 3) {
    dialReady = true;
    digitCount = 0;
  }
}
/********************* GET NUMBER TO CALL ON GDK1 *****************/
void GDK1() {  
  dialReady = false;                    // disable dialling
  digitalWrite(holdPin, LOW);           // engage/hold the exchange
  digitalWrite(lineswitchPin, LOW);     // switch line to converter
                                        // and Earth wire to A1
  delay(100);                           // essential delay
  analogIn1 = analogRead(analogPin1); 
  Serial.println(analogIn1); 
  if (analogIn1 > 1003) {
        // A1 is approx 5V when no key is pushed
    if (alreadyPrinted == false) {
      Serial.println("waiting for key"); // print once
      alreadyPrinted = true;
    }
  }
  if ((analogIn1 > 686) && (analogIn1 < 726)) { // nominal 3.45V
        // = 706 counts; could be 1 or 7 at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 115) && (analogIn1 < 155)) {
        // nominal -0.66V = 135 counts for key 1 at a-
      keyReading = 1;
      Serial.println("Key 1 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 477) && (analogIn1 < 517)) {
        // nominal -2.43V = 497 counts for key 7 at a-
      keyReading = 7;
      Serial.println("Key 7 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    digitCount++;
    if (digitCount == 1) digit1 = keyReading;
    else if (digitCount == 2) digit2 = keyReading;
    else if (digitCount == 3) digit3 = keyReading;
  }
  while (!keyRelease) {
    analogIn1 = analogRead(analogPin1);
    if (analogIn1 > 1003) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn1 = analogRead(analogPin1);
  }
  if ((analogIn1 > 172) && (analogIn1 < 212)) { // nominal 0.94V
        // = 192 counts; could be 2, 3, 8 or 9 at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 115) && (analogIn1 < 155)) {
        // nominal -660mV = 135 counts for key 2 at a-
      keyReading = 2;
      Serial.println("Key 2 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 297) && (analogIn1 < 337)) {
        // nominal -1.55V = 317 counts for key 3 at a-
      keyReading = 3;
      Serial.println("Key 3 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 477) && (analogIn1 < 517)) {
        // nominal -2.43V = 497 counts for key 8 at a-
      keyReading = 8;
      Serial.println("Key 8 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 809) && (analogIn1 < 849)) {
        // nominal -4.05V = 829 counts for key 9 at a-
      keyReading = 9;
      Serial.println("Key 9 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    digitCount++;
    if (digitCount == 1) digit1 = keyReading;
    else if (digitCount == 2) digit2 = keyReading;
    else if (digitCount == 3) digit3 = keyReading;
  }
  while (!keyRelease) {
    analogIn1 = analogRead(analogPin1);
    if (analogIn1 > 1003) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn1 = analogRead(analogPin1);
  }
  if ((analogIn1 > 510) && (analogIn1 < 550)) { // nominal 2.59V
        // = 530 counts; could be 4 or 0 at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 297) && (analogIn1 < 337)) {
        // nominal -1.55V = 317 counts for key 4 at a-
      keyReading = 4;
      Serial.println("Key 4 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 809) && (analogIn1 < 849)) {
        // nominal -4.05V = 829 counts for key 0 at a-
      keyReading = 0;
      Serial.println("Key 0 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    digitCount++;
    if (digitCount == 1) digit1 = keyReading;
    else if (digitCount == 2) digit2 = keyReading;
    else if (digitCount == 3) digit3 = keyReading;
  }
  while (!keyRelease) {
    analogIn1 = analogRead(analogPin1);
    if (analogIn1 > 1003) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn1 = analogRead(analogPin1);
  }
  if ((analogIn1 > 866) and (analogIn1 < 906)) { // nominal 4.33V 
        // = 886 counts, could be 5,6,+ or - at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn1 = analogRead(analogPin1);
    if ((analogIn1 > 297) && (analogIn1 < 337)) {
        // nominal -1.55V = 317 counts for key 5 at a-
      keyReading = 5;
      Serial.println("Key 5 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 809) && (analogIn1 < 849)) {
        // nominal -4.05V = 829 counts for key 6 at a-
      keyReading = 6;
      Serial.println("Key 6 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 115) && (analogIn1 < 155)) {
        // nominal -660mV = 135 counts for key + at a-
      keyReading = 11;
      Serial.println("Key + pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn1 > 477) && (analogIn1 < 517)) {
        // nominal -2.43V = 497 counts for key - at a-
      keyReading = 12;
      Serial.println("Key - pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
      // do not add digit if + or - are pushed
    if ((keyReading == 5) || (keyReading == 6))  {
      digitCount++;
      if (digitCount == 1) digit1 = keyReading;
      else if (digitCount == 2) digit2 = keyReading;
      else if (digitCount == 3) digit3 = keyReading;
    }
  }
  while (!keyRelease) {
    analogIn1 = analogRead(analogPin1);
    if (analogIn1 > 1003) keyRelease = true;  
  }
  if (digitCount == 3) {
    dialReady = true;
    digitCount = 0;
  }
}
/*********************** GET NUMBER TO CALL on GDK2 *****************/
void GDK2() {  
  dialReady = false;                    // disable dialling
  digitalWrite(holdPin, LOW);           // engage/hold the exchange
  digitalWrite(lineswitchPin, LOW);     // switch line to converter
  delay(100);                           // essential delay
  analogIn0 = analogRead(analogPin0);  
  Serial.println(analogIn0); 
  if ((analogIn0 > 300) and (analogIn0 < 700)) {
        // A0 is between 1.5 and 3.5 V when handset is lifted
    if (alreadyPrinted == false) {
      Serial.println("waiting for key"); // print once
      alreadyPrinted = true;
    }
  }
  if ((analogIn0 > 158) && (analogIn0 < 198)) { // nominal 0.87V
        // = 178 counts; could be 1, 2 or 3 at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn0 = analogRead(analogPin0);
    if (analogIn0 > 1000) {
        // nominal -5V = 1023 counts for key 1 at a-
      keyReading = 1;
      Serial.println("Key 1 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn0 > 946) && (analogIn0 < 986)) {
        // nominal -4.72V = 966 counts for key 2 at a-
      keyReading = 2;
      Serial.println("Key 2 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn0 > 864) && (analogIn0 < 904)) {
        // nominal -4.32V = 884 counts for key 3 at a-
      keyReading = 3;
      Serial.println("Key 3 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    digitCount++;
    if (digitCount == 1) digit1 = keyReading;
    else if (digitCount == 2) digit2 = keyReading;
    else if (digitCount == 3) digit3 = keyReading;
  }
  while (!keyRelease) {
    analogIn0 = analogRead(analogPin0);
    if ((analogIn0 > 300) and (analogIn0 < 700)) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn0 = analogRead(analogPin0);
  }
  if ((analogIn0 > 860) && (analogIn0 < 900)) { // nominal 4.3V
        // = 880 counts; could be 4, 5, or 6 at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn0 = analogRead(analogPin0);
    if (analogIn0 > 1000) {
        // nominal -5V = 1023 counts for key 4 at a-
      keyReading = 4;
      Serial.println("Key 4 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn0 > 946) && (analogIn0 < 986)) {
        // nominal -4.72V = 966 counts for key 5 at a-
      keyReading = 5;
      Serial.println("Key 5 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn0 > 864) && (analogIn0 < 904)) {
        // nominal -4.32V = 884 counts for key 6 at a-
      keyReading = 6;
      Serial.println("Key 6 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    digitCount++;
    if (digitCount == 1) digit1 = keyReading;
    else if (digitCount == 2) digit2 = keyReading;
    else if (digitCount == 3) digit3 = keyReading;
  }
  while (!keyRelease) {
    analogIn0 = analogRead(analogPin0);
    if ((analogIn0 > 300) and (analogIn0 < 700)) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn0 = analogRead(analogPin0);
  }
  if ((analogIn0 > 946) && (analogIn0 < 986)) { // nominal 4.72V
        // = 966 counts; could be 7, 8 or 9 at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn0 = analogRead(analogPin0);
    if (analogIn0 > 1000) {
        // nominal -5V = 1023 counts for key 7 at a-
      keyReading = 7;
      Serial.println("Key 7 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn0 > 946) && (analogIn0 < 986)) {
        // nominal -4.72V = 966 counts for key 8 at a-
      keyReading = 8;
      Serial.println("Key 8 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn0 > 864) && (analogIn0 < 904)) {
        // nominal -4.32V = 884 counts for key 9 at a-
      keyReading = 9;
      Serial.println("Key 9 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    digitCount++;
    if (digitCount == 1) digit1 = keyReading;
    else if (digitCount == 2) digit2 = keyReading;
    else if (digitCount == 3) digit3 = keyReading;
  }
  while (!keyRelease) {
    analogIn0 = analogRead(analogPin0);
    if ((analogIn0 > 300) and (analogIn0 < 700)) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn0 = analogRead(analogPin0);
  }
  if (analogIn0 > 1000) { // nominal 5V = 1023 counts
        // could be red, 0 or white at polarity a+
    delay(10);
    digitalWrite(polarityPin, LOW);  // switch polarity to a-
    delay(30);
    analogIn0 = analogRead(analogPin0);
    if (analogIn0 > 1000) {
        // nominal -5V = 1023 counts for key red at a-
      keyReading = 11;
      Serial.println("Key red pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn0 > 946) && (analogIn0 < 986)) {
        // nominal -4.72V = 966 counts for key 0 at a-
      keyReading = 0;
      Serial.println("Key 0 pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if ((analogIn0 > 864) && (analogIn0 < 904)) {
        // nominal -4.32V = 884 counts for key white at a-
      keyReading = 12;
      Serial.println("Key white pushed"); 
      delay(10);
      digitalWrite(polarityPin, HIGH); // return polarity
      delay(30);
      keyRelease = false;
    }
    if (keyReading == 0) {
      digitCount++;
      if (digitCount == 1) digit1 = keyReading;
      else if (digitCount == 2) digit2 = keyReading;
      else if (digitCount == 3) digit3 = keyReading;
    }
  }
  while (!keyRelease) {
    analogIn0 = analogRead(analogPin0);
    if ((analogIn0 > 300) and (analogIn0 < 700)) keyRelease = true;  
  }
  if (keyRelease) {
    analogIn0 = analogRead(analogPin0);
  }
  if (digitCount == 3) dialReady = true;
}
























