/*
 levelSensor.ino measures liquid level or waveheight in a 
 small flume using the principle of conductivity. 
 The sensor is made up of three parallel plates of 1 cm 
 wide and 10.5 cm long. The plates are 1 mm thick epoxy
 board strips, covered with copper on one side.
 Two plates have copper strips across the full length. This
 is the main level sensor. The third plate has a 2 cm copper
 strip at the bottom. There is also a 2 cm short plate opposite,
 attached to the middle plate. This 2 cm cell is the reference 
 sensor. The conductivity of a cell can be written as:
 Y = Yw * A/L, where Yw is the conductivity of tap water
 (approx. 100 uS/cm), A is the plate area (in cm2) and L the 
 distance in cm. The resistance is the inverse and can be written
 as R = 1/(level*Yw) or level(in cm) = 1/(R*Yw)            (1) 
 The dependence on Yw can be eliminated by using a reference cell
 and creating a voltage divider, putting the level sensor and 
 reference in series: Vm = Vo*R/(R+Rr),                    (2)      
 where Vm is the measured voltage, Vo the supply voltage, 
 R the level sensor resistance and Rr the reference cell resistance. 
 Combining equations (1) and (2):
     level = 2*(Vo-Vm)/Vm, 
 where a reference level of 2 cm is used.
 The supply voltage is switched between Plus and Minus to avoid
 polarisation. This is achieved by using digital pins D2 and D3
 and writing HIGH and LOW in turns.
 When Plus and Minus are reversed:
     level = 2*Vm/(Vo-Vm)

           written 8 Dec 2017 by Wim der Kinderen
*/

const int Minus = 2;     // start minus on pin 2
const int Plus = 3;      // start plus on pin 3
const int Analog = 1;    // pick up measured voltage on A1  
float val, level;    

void setup() {
  Serial.begin(115200);      // set baudrate of serial link
  pinMode(Minus, OUTPUT);    // define power pins
  pinMode(Plus, OUTPUT);
}

void loop() {
  digitalWrite(Minus, LOW);    // set power
  digitalWrite(Plus, HIGH); 
  delay(20);
  val = analogRead(Analog);
  level = 2*(1023 - val)/val;  
  digitalWrite(Minus, HIGH);  // invert power
  digitalWrite(Plus, LOW); 
  delay(20);
  val = analogRead(Analog);
  level = 2*val/(1023 - val);
  Serial.print("-Vo:  Vm= ");
  Serial.print(val); 
  Serial.print(" level = ");
  Serial.print(level); 
  int printstars;
  printstars = int(level*20);
  for (int i=1; i<=printstars; i++) Serial.print("*");
  Serial.println(".");
}



