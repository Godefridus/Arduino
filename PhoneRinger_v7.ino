/*
PhoneRinger_v7 is a modified version of v6, specifically designed to work with
Final Selector SAW81532 (Strowger demonstrator).

PhoneRinger _v7.ino controls a Songle 5V relay to generate the ringtone sequence
for the telephone exchange. The relay interrupts the 50V 50Hz signal from a mains 
transformer. The relay is controlled by Arduino Pin D10. The normally-
open (NO) contacts of the relay are used. The contacts close on D10 LOW(!). 
The relay connects selector pin U32 to one side of the mains transformer 
secondary winding. The other side of the transformer secondary is connected to
selector pin U11 (ETH). A 10k resistor connects U32 and U30 to provide the ringing
signal to tone relay A.

The ring tone sequencing is the UK standard pattern: 
0.4 seconds ON, 0.2 seconds OFF, 0.4 seconds ON, 2 seconds OFF.
To start the sequencing a signal is picked up from selector pin U3 (RINGER START)
which connects to Arduino D3. Ringing is initiated when this pin is earthed. 
Arduino Pin D3 is connected to +5V by a 6k8 resistor. Input D3 HIGH means
no ringing.

The sketch also provides DIALTONE. A 350Hz squarewave is provided on Arduino
Pin D6. That Pin D6 is connected to Selector pin U3 via a 10nF capacitor 
to provide isolation. Note that Arduino GND must be connected to Selector ETH. 
The selector automatically connects the dial signal to tone relay A when relay 
DT is activated (internally by the selector).

The sketch was developed in IDE 1.0.6 and runs on an Arduino Nano clone.
(make sure to update the serial port driver, see CH341SER_MAC)

The ringer relay closes the ringing loop in the final selector circuit: 
Earth (= +V on 50Vdc supply), 50Vac supply, ringer relay, U connector
point 32 (INT RING), 300Ω F relay, contact E5,  contacr F3, contact H3,
upper wiper, bank, called phone, bank, lower wiper, contact H2, contact F2, 
300Ω R6, RING RET BATT (= -V on 50Vdc supply).
Arduino Pin D10 is switched between LOW and HIGH to produce the UK standard 
ringing pattern: 0.4 seconds ON, 0.2 seconds OFF, 0.4 seconds ON, 
2 seconds OFF.
Pin D13 is used as control pin. It is internally connected to the onboard LED.
The LED lights up when RINGER START has been activated.

            modified 1 april 2019
            by Wim der Kinderen
*/

int val = 0;                // variable to store the read value of relay 1
//int val2 = 0;                // variable to store the read value of relay 2
boolean ringer = false;      // ringer is OFF on startup

void setup() {
  Serial.begin(9600);
  pinMode(3, INPUT);        // GND trigger signal on input pin D3
  pinMode(13, OUTPUT);      // signalling LED on pin D13
  pinMode(10, OUTPUT);      // ringer relay control signal on D10
  digitalWrite(10, HIGH);   // switch ringer OFF (5V means relay inactive)
  tone(6, 350);             // put a 350Hz squarewave on D6
}

void loop() {
  val = digitalRead(3);        // read pin D3 (= RINGER START signal on U3)
  Serial.print(val);           // report val on Serial Line for debugging
  delay(50);
  if (val == 0) ringer = true; // enable ringer if D3 value is LOW
  else ringer = false;         // else switch ringer OFF 
  
  if (ringer) {                // if ringer status is TRUE:
      digitalWrite(13, HIGH);  //  switch LED ON to show INT RING activated
      digitalWrite(10, LOW);   //  turn ringer ON (0V activates relay)                        
      delay(400);              //  wait for 0.4 seconds
      digitalWrite(10, HIGH);  //  turn ringer OFF
      delay(200);              //  wait for 0.2 seconds
      digitalWrite(10, LOW);   //  turn ringer ON
      delay(400);              //  wait for 0.4 seconds
      digitalWrite(10, HIGH);  //  turn ringer OFF
      delay(1950);             //  wait for 2 seconds and return
    }
  else {                       // if ringer status is FALSE: 
    digitalWrite(13, LOW);     // switch LED OFF to show INT RING inactive
    digitalWrite(10, HIGH);    // switch ringer OFF  
  }
}  

