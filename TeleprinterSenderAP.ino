 /* 
TeleprinterSenderAP.ino is identical to TeleprinterSender_v1.ino, where it uses
WeMoS as accesspoint instead of the WiFi home network.                                      
Compiled using Arduino 1.8.19 on OS Monterey 12.7.4. Based on TeleprinterSender.ino 
and morseSender.ino. Program has been modified to interrupt the demo and the 
WiFi mode when using the keyboard. pin D6 is monitored. The input is LOW when 
receiving (default position) and goes HIGH during sending.

Note that Big Sur has the CH340 driver for Wemos included so DO NOT install a 
separate driver. Check the serial monitor baudrate when communicating.
This sketch is loaded on a Wemos D1 board. The board operates the relay of the
Creed 7b teleprinter using two TIP3055 transistors. Pin D1 activates the Mark
coil, pin D2 activates the Space coil. See schematic Creed 7b.pptx.

Pin A0 is used to set the mode of operation. On a HIGH (3.3V) the program transmits
a demo sentence: "THIS IS A CREED TELEPRINTER". The demo only runs once.
On a Low (0V) the program waits for input from a webpage.
Set the switch to WiFi/Type for either using the keyboard or to send a message
from a webpage. Select "Creed" as WiFi SSID from either PC or smartphone. The 
password is willem52, all lower case. The webpage is accessed from url
192.168.4.1. It may be required to reset the board by switching the power off/on. 
The program converts a message into Baudot code, where each bit has a duration 
of 20 ms. 
The Creed 7b teleprinter is always one character behind when printing. Therefore
it is recommended to start and stop with a space. 
The onboard LED lights up when the message is transmitted.

    by Wim der Kinderen 26 Nov 2024 
*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#ifndef APSSID
#define APSSID "Creed"
#define APPSK "willem52"  // minimum 8 characters
#endif

const char *ssid = APSSID;
const char *password = APPSK;

WiFiServer server(80);
 
int ledPin = 14;          // onboard LED is used to monitor transmission
int markPin = 5;          // Wemos D1 (GPIO5) sets the Mark code
int spacePin = 4;         // Wemos D2 (GPIO4) sets the Space code 
int pausePin = 12;        // Wemos D6 (GPIO12) sets the Pause status  
int i, index1, index2, unit, mark, space;
unsigned long gap;
char C;                  // C is the buffer for the message characters
String message;          // string to hold text
const int analogPin = A0;  // A0 is used to select operating mode  
int A0value = 0; 
int demoMode = 0;
bool pauseStatus = false;  // default status is no pause          

void setup() {
  Serial.begin(115200);    // use serial line to monitor status
  delay(10);  
  pinMode(ledPin, OUTPUT);  
  pinMode(markPin, OUTPUT);
  pinMode(spacePin, OUTPUT);
  pinMode(pausePin, INPUT);
  digitalWrite(ledPin, LOW);    // initiate transmitter status
  digitalWrite(markPin, LOW);
  digitalWrite(spacePin, LOW);
  mark = 1;
  space = 0;
  gap = 20;  // bit duration in msec. Creed 7b teleprinter runs at 50 Baud
 
   // Connect to WeMos AP
  WiFi.softAP(ssid, password);       // code for accesspoint
  IPAddress myIP = WiFi.softAPIP();
  Serial.println();                   
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  server.begin();                     // Start the server
  Serial.println("Server started"); 
  Serial.print("Use this URL : ");    // Print the IP address
  Serial.print("http://");
  Serial.println(myIP);
}
 
void loop() {
      // set interrupt on pausePin, jump to setPause routine
  attachInterrupt(digitalPinToInterrupt(pausePin),setPause,RISING);
  A0value = analogRead(analogPin); // A0 determines the mode of operation
  Serial.print(" A0 = ");
  Serial.println(A0value);        // A0value is between 0 (at 0V) and 1024 (at 3.3V)
  pauseStatus = digitalRead(pausePin);   // pause if keyboard is used
  Serial.print(" Pause Status is: ");
  Serial.println(pauseStatus);
  delay(1000);                    // check mode every second when not transmitting
              // run demo ONCE if A0 is connected to +3.3V and no pause interrupt
  if (A0value > 500 && demoMode == 0 && !pauseStatus) { 
    demoMode = 1;
    Serial.print(" run demo ");
    digitalWrite(ledPin, HIGH);
    digitalWrite(markPin, HIGH);    // switch on teletransmitter
    digitalWrite(spacePin, LOW);
    demo();
    digitalWrite(spacePin, LOW);    // end with a Mark
    digitalWrite(markPin, HIGH);
    delay(5*gap);  
    digitalWrite(ledPin, LOW);
    digitalWrite(markPin, LOW);     // switch off teletransmitter 
  }
          
  if (A0value < 500) {   // run WiFi or keybboard mode if A0 is connected to GND 
    demoMode = 0;                     // reset demoMode
    Serial.print(" keyboard/WiFi mode ");
     // Check if a client has connected
    WiFiClient client = server.available();
    if (!client || pauseStatus) { return; }
 
    // Wait until the client sends some data
    Serial.println("new client");
    while(!client.available()){ delay(1); }
 
    // Read the request. It looks like this: GET/?S=message HTTP/1.1
    // "message" will be extracted using positions of "=" and " " 
    message = "";                     // reset message content
    digitalWrite(ledPin, HIGH);
    digitalWrite(markPin, HIGH);      // switch on teletransmitter
    digitalWrite(spacePin, LOW);
    String request = client.readStringUntil('\r');  // read webpage message
    client.flush();                  // clear the serial buffer after reading
    Serial.println(request);         // send string to PC         
    index1 = request.indexOf("=");   // search for message delimiter characters
    index2 = request.indexOf(" ",7); // search for space from character 7 onwards
    Serial.print("Index1 = ");
    Serial.print(index1);            // index1 is position of "="
    Serial.print("  Index2 = ");
    Serial.print(index2);            // index2 is position of space (= end of message)
    Serial.print("  message = ");
    if (index1 == 7) {               // avoid the favicon message where it appears
      for (i = index1 + 1 ; i < index2; i++) {
        C = request.charAt(i);       // read message characters one by one
        Serial.print(C);             // send characters to PC
        // Transmit message to teleprinter. Only letters are used.
        // Characters are converted to Baudot code (+ is the marking of space in HTML)
        if (!pauseStatus) {
          if (C == '+') { transmit(space,space,mark,space,space);  } 
          else if (C == 'A' ||C == 'a') { transmit(mark,mark,space,space,space); }
          else if (C == 'B' ||C == 'b') { transmit(mark,space,space,mark,mark); }
          else if (C == 'C' ||C == 'c') { transmit(space,mark,mark,mark,space); }
          else if (C == 'D' ||C == 'd') { transmit(mark,space,space,mark,space); }
          else if (C == 'E' ||C == 'e') { transmit(mark,space,space,space,space); }
          else if (C == 'F' ||C == 'f') { transmit(mark,space,mark,mark,space); }
          else if (C == 'G' ||C == 'g') { transmit(space,mark,space,mark,mark); }
          else if (C == 'H' ||C == 'h') { transmit(space,space,mark,space,mark); }
          else if (C == 'I' ||C == 'i') { transmit(space,mark,mark,space,space); }
          else if (C == 'J' ||C == 'j') { transmit(mark,mark,space,mark,space); }
          else if (C == 'K' ||C == 'k') { transmit(mark,mark,mark,mark,space); }
          else if (C == 'L' ||C == 'l') { transmit(space,mark,space,space,mark); }
          else if (C == 'M' ||C == 'm') { transmit(space,space,mark,mark,mark); }
          else if (C == 'N' ||C == 'n') { transmit(space,space,mark,mark,space); }
          else if (C == 'O' ||C == 'o') { transmit(space,space,space,mark,mark); }
          else if (C == 'P' ||C == 'p') { transmit(space,mark,mark,space,mark); }
          else if (C == 'Q' ||C == 'q') { transmit(mark,mark,mark,space,mark); }
          else if (C == 'R' ||C == 'r') { transmit(space,mark,space,mark,space); }
          else if (C == 'S' ||C == 's') { transmit(mark,space,mark,space,space); }
          else if (C == 'T' ||C == 't') { transmit(space,space,space,space,mark); }
          else if (C == 'U' ||C == 'u') { transmit(mark,mark,mark,space,space); }
          else if (C == 'V' ||C == 'v') { transmit(space,mark,mark,mark,mark); }
          else if (C == 'W' ||C == 'w') { transmit(mark,mark,space,space,mark); }
          else if (C == 'X' ||C == 'x') { transmit(mark,space,mark,mark,mark); }
          else if (C == 'Y' ||C == 'y') { transmit(mark,space,mark,space,mark); }
          else if (C == 'Z' ||C == 'z') { transmit(mark,space,space,space,mark); }
        }
      }
    }
    Serial.println();
    digitalWrite(ledPin, LOW);
    digitalWrite(markPin, LOW);       // switch off teletransmitter
    digitalWrite(spacePin, LOW);

  // Build the webpage
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println(""); //  do not forget this one
    client.println("<!DOCTYPE HTML>");
    client.println("<html>");
    client.print("<body style='background-color:aqua' >");
    client.print("<h1 style='color:red; font-size:50px'><center>Teleprinter Sender</h1>");
    client.print("<h2 style='font-size:25px' >");
    client.println("<center>Transmit Baudot code to Creed 7b using WiFi");
    client.print("<h2 style='font-size:35px' >");
    client.println("Click on white box and enter message");
    client.println("<br>");
    client.print("<h2 style='font-size:25px' >");
    client.println("Use only letters. Start and finish with a space"); 
    client.println("</form>");
    client.println("<form action='' method='get'>");  //code for message box
    client.print("<input type='text' name='S' />"); //
    client.println("<input type='submit' value='SEND' style='font-size: 25px;' />");
    client.println("<br /h2>"); 
    client.println(message);
    client.println("<br /h2>"); 
    client.println("</html>");
    delay(1);
    Serial.println("Client disconnected");
    Serial.println("");
  }
}

// code sender, starts with space ends with mark, gap is pause in msec
void transmit(int a,int b,int c,int d, int e) { 
  if (!pauseStatus) {  
    digitalWrite(spacePin, HIGH);   // start bit is a Space
    digitalWrite(markPin, LOW);
    delay(gap); }
  if (!pauseStatus) {     
    if (a == 1) {     
      digitalWrite(markPin, HIGH);
      digitalWrite(spacePin, LOW); }
    if (a == 0) {
      digitalWrite(markPin, LOW);
      digitalWrite(spacePin, HIGH); }   
    delay(gap);}
  if (!pauseStatus) {
    if (b == 1) {     
      digitalWrite(markPin, HIGH);
      digitalWrite(spacePin, LOW); }
    if (b == 0) {
      digitalWrite(markPin, LOW);
      digitalWrite(spacePin, HIGH); }   
    delay(gap);}
  if (!pauseStatus) {
    if (c == 1) {     
      digitalWrite(markPin, HIGH);
      digitalWrite(spacePin, LOW); }
    if (c == 0) {
      digitalWrite(markPin, LOW);
      digitalWrite(spacePin, HIGH); }   
    delay(gap);}
  if (!pauseStatus) {
    if (d == 1) {     
      digitalWrite(markPin, HIGH);
      digitalWrite(spacePin, LOW); }
    if (d == 0) {
      digitalWrite(markPin, LOW);
      digitalWrite(spacePin, HIGH); }   
    delay(gap);}
  if (!pauseStatus) {
    if (e == 1) {     
      digitalWrite(markPin, HIGH);
      digitalWrite(spacePin, LOW); }
    if (e == 0) {
      digitalWrite(markPin, LOW);
      digitalWrite(spacePin, HIGH); }   
    delay(gap);}
  if (!pauseStatus) {  
    digitalWrite(markPin, HIGH);    // stop bit is a Mark
    digitalWrite(spacePin, LOW);
    delay(gap);                     // allow extra time after each character
    delay(gap);
  }  
}

void demo() {                  // start with two spaces
  transmit(space,space,mark,space,space);
  transmit(space,space,mark,space,space);
  transmit(space,space,space,space,mark);   // transmit "T"
  transmit(space,space,mark,space,mark);    // transmit 'H"
  transmit(space,mark,mark,space,space);    // transmit "I"
  transmit(mark,space,mark,space,space);    // transmit "S"
  transmit(space,space,mark,space,space);   // transmit a space
  transmit(space,mark,mark,space,space);    // transmit "I"
  transmit(mark,space,mark,space,space);    // transmit "S"
  transmit(space,space,mark,space,space);   // transmit a space
  transmit(mark,mark,space,space,space);    // transmit "A"
  transmit(space,space,mark,space,space);   // transmit a space
  transmit(space,mark,mark,mark,space);     // transmit "C"
  transmit(space,mark,space,mark,space);    // transmit "R"
  transmit(mark,space,space,space,space);   // transmit "E"
  transmit(mark,space,space,space,space);   // transmit "E"
  transmit(mark,space,space,mark,space);    // transmit "D"
  transmit(space,space,mark,space,space);   // transmit a space
  transmit(space,space,space,space,mark);   // transmit "T"
  transmit(mark,space,space,space,space);   // transmit "E"
  transmit(space,mark,space,space,mark);    // transmit "L"
  transmit(mark,space,space,space,space);   // transmit "E"
  transmit(space,mark,mark,space,mark);     // transmit "P"
  transmit(space,mark,space,mark,space);    // transmit "R"
  transmit(space,mark,mark,space,space);    // transmit "I"
  transmit(space,space,mark,mark,space);    // transmit "N"
  transmit(space,space,space,space,mark);   // transmit "T"
  transmit(mark,space,space,space,space);   // transmit "E"
  transmit(space,mark,space,mark,space);    // transmit "R"
  transmit(space,space,mark,space,space);   // finish with two spaces
  transmit(space,space,mark,space,space);  
}

IRAM_ATTR void setPause() {
  pauseStatus = digitalRead(pausePin);
}
