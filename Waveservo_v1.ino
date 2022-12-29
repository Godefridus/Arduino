/* 
 Waveservo_v1.uno operates an Emax ES3104 waveservo connected to an Uno.
 Developed using IDE 1.0.6
 Three potentiometers (4.7k, 4.7k and 4.2k) are connected to
 5V, GND and analog A0, A1 and A2, respectively. A0 controls the frequency
 or timestep, A1 controls the skew of the movement, i.e. the ratio of
 forward to backward stepping and A2 sets the maximum angle.
 A special pattern of decreasing amplitude is selected by turning
 pot A1 fully counterclockwise.
 The analog values are mapped, so the rampUp and rampDown steps are
 between 1 and 10, and the max angle can be set between 0 and 75 around
 th midpoint. 
 The waveservo is connected to 5V, GND and control pin 9. 

        written 7 Dec 2017 by Wim der Kinderen
 */
 
#include <Servo.h>          // servo library
Servo waveServo;            // refers to the servomotor
const int frequencyPot=0;   // frequency potentiometer on A0
const int rampRatioPot=1;   // ramp ratio potentiometer on A1
const int ampPot=2;         // amplitude potentiometer on A2
int frequency, rampRatio, amp;
int i = 1;

float angle = 0;             // position of servo
float midPoint = 75;         // zero position
float maxAngle, skew, stepSize, forwardStep, backwardStep; 
boolean forward = true;

void setup() {
  waveServo.attach(9);      // servo attached to digital pin 9
  Serial.begin(115200);     // set up serial link
  for (int i=1; i <= midPoint; i++) {  
    waveServo.write(i);     // move servo to mid position
    delay(10); 
  } 
}
void loop() {
  frequency = analogRead(frequencyPot);    // read frequency value
  rampRatio = analogRead(rampRatioPot);    // read rampdown value
  amp = analogRead(ampPot);                // read amplitude value
  maxAngle = map(amp, 0, 1023, 0, 75);     // map analog value to angle
  stepSize = map(frequency, 0, 1023, 2, 10);
  skew = map(rampRatio, 0, 1023, 1, 9);    // skew 9 max forward, 2 max backward
                                           // 1 = special pattern
  forwardStep = stepSize*skew/10;             
  backwardStep = stepSize*(10-skew)/10;        

/**** EXECUTE SPECIAL PATTERN IF SKEW POT IS TURNED FULLY COUNTERCLOCKWISE ****/  
  if (skew == 1) {    
    forwardStep = stepSize;
    backwardStep = stepSize;    
    if (i < 4) {              // decrease amplitude in steps
      if (forward)  {            
        angle = angle + (maxAngle/i)*forwardStep/100; 
        if (angle > midPoint + maxAngle/i) { 
          angle = midPoint + maxAngle/i;
          forward = false;
        }
      }
      else {
        angle = angle - (maxAngle/i)*backwardStep/100;
        if (angle < midPoint - maxAngle/i) { 
          angle = midPoint - maxAngle/i;
          forward = true;
          i++;
        }  
      }
    }
    else i = 1;
  }
  
/**** EXECUTE NORMAL PATTERN IF SKEW POT IS TURNED UP CLOCKWISE ****/  
  else {
    if (forward)  {
      angle = angle + maxAngle*forwardStep/100;
      if (angle > midPoint + maxAngle) { 
        angle = midPoint + maxAngle;
        forward = false;
      }
    }
    else {
      angle = angle - maxAngle*backwardStep/100;
      if (angle < midPoint - maxAngle) { 
        angle = midPoint - maxAngle;
        forward = true;
      }  
    }
  }
 if (maxAngle < 1) angle = midPoint; 
 /* Serial.print("frequency= ");
 Serial.print(frequency);
 Serial.print(" rampRatio= ");
 Serial.print(rampRatio);
 Serial.print(" amplitude= ");
 Serial.print(amp);
 Serial.print(" maxAngle= ");
 Serial.print(maxAngle);
 Serial.print(" stepSize= ");
 Serial.println(stepSize);
 Serial.print(" skew= ");
 Serial.print(skew);
 Serial.print(" forwardStep= ");
 Serial.print(forwardStep); 
 Serial.print(" backwardStep= ");
 Serial.print(backwardStep); 
 Serial.print(" angle= ");
 Serial.println(angle);
 */ 
 waveServo.write(angle);      // control the servomotor
 delay(2);                    // allow servo time to move
}

