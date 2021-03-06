// This is a demo of SPACS user running as webserver with the Ether Card
// 2016-03-10 <md@dasdata.co> http://dasdata.co/spacs
/*   
  =======  HMC5883L compass  =========
 * SCL connection of the sensor attached to analog pin A5
 * SDA connection of the sensor attached to analog pin A4
 * GND connection of the sensor attached to ground
 * VCC connection of the sensor attached to +5V
*/
#include <Wire.h>
#include <HMC5883L.h>
HMC5883L compass;

/*  
 =======  ETH - ENC28J60  =========
  * SCK connection of the sensor attached pin 13
  * SO  connection of the sensor attached pin 12
  * SI  connection of the sensor attached pin 11
  * CS  connection of the sensor attached pin 8
  * VCC connection of the sensor attached to +3.3V
  * GND connection of the sensor attached to ground
*/
#include <EtherCard.h>
// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
static byte myip[] = { 192,168,0,109 };

byte Ethernet::buffer[500];
BufferFiller bfill;

/*  
 =======  SONAR - HC-SR04 ultrasonic rangefinder =========
 * ECHO connection of the sensor attached to digital pin 6
 * TRIG connection of the sensor attached to digital pin 7
 * GND connection of the sensor attached to ground
 * VCC connection of the sensor attached to +5V
*/
const int echoPin = 6;
const int trigPin = 7;
const int maxDist = 200; // maximum distance 



/*   ============================    SETUP   ========================    */
void setup () {
 // Init ethernet  
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
     Serial.println(F("Failed to access Ethernet controller"));
  ether.staticSetup(myip); 
  
  // Init compas 
 Wire.begin();
 compass = HMC5883L();
 compass.SetScale(1.3);
 compass.SetMeasurementMode(Measurement_Continuous);
}

/*   ============================    LOOP    ========================    */
void loop () {
  
  word len = ether.packetReceive();
  word pos = ether.packetLoop(len);

  if (pos)  // check if valid tcp data is received
    ether.httpServerReply(homePage()); // send web page data
}


/*   ============================    VOIDS    ========================    */
static word homePage() {
  // get compass values 
 MagnetometerRaw raw = compass.ReadRawAxis();
 MagnetometerScaled scaled = compass.ReadScaledAxis();
 float xHeading = atan2(scaled.YAxis, scaled.XAxis);
 float yHeading = atan2(scaled.ZAxis, scaled.XAxis);
 float zHeading = atan2(scaled.ZAxis, scaled.YAxis);
 if(xHeading < 0) xHeading += 2*PI;
 if(xHeading > 2*PI) xHeading -= 2*PI;
 if(yHeading < 0) yHeading += 2*PI; 
 if(yHeading > 2*PI) yHeading -= 2*PI;
 if(zHeading < 0) zHeading += 2*PI;
 if(zHeading > 2*PI) zHeading -= 2*PI;
 float xDegrees = xHeading * 180/M_PI;
 float yDegrees = yHeading * 180/M_PI;
 float zDegrees = zHeading * 180/M_PI;

  // establish variables for duration of the ping, 
  // and the distance result in inches and centimeters:
  long duration, inches, cm;

  // The sensor is triggered by a HIGH pulse of 10 or more microseconds.
  // Give a short LOW pulse beforehand to ensure a clean HIGH pulse:
  pinMode(trigPin, OUTPUT);
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(5);
  digitalWrite(trigPin, LOW);

  // Read the signal from the sensor: a HIGH pulse whose
  // duration is the time (in microseconds) from the sending
  // of the ping to the reception of its echo off of an object.
  pinMode(echoPin, INPUT);
  duration = pulseIn(echoPin, HIGH);

  // convert the time into a distance
  inches = microsecondsToInches(duration);
  cm = microsecondsToCentimeters(duration);
  
//  Serial.print(inches);
//  Serial.print("in, ");
  Serial.print(cm);
  Serial.print("cm");
  Serial.println();

 // delay(500);
 // if (maxDist > cm ) {
      bfill = ether.tcpOffset();
      bfill.emit_p(PSTR(
          "HTTP/1.0 200 OK\r\n"
          "Content-Type: text/html\r\n"
          "Pragma: no-cache\r\n"
          "\r\n"
          "<meta http-equiv='refresh' content='1'/>|"
          //  "<title>SPACS server</title>" 
          "$D | x:$D y:$D z:$D "), 
          cm, xDegrees, yDegrees, zDegrees);
     return bfill.position();
 //  }
}



/*  ===================         CONVERSIONS          =========================== */
long microsecondsToInches(long microseconds)
{
  // According to Parallax's datasheet for the PING))), there are
  // 73.746 microseconds per inch (i.e. sound travels at 1130 feet per
  // second).  This gives the distance travelled by the ping, outbound
  // and return, so we divide by 2 to get the distance of the obstacle.
  // See: http://www.parallax.com/dl/docs/prod/acc/28015-PING-v1.3.pdf
  return microseconds / 74 / 2;
}

long microsecondsToCentimeters(long microseconds)
{
  // The speed of sound is 340 m/s or 29 microseconds per centimeter.
  // The ping travels out and back, so to find the distance of the
  // object we take half of the distance travelled.
  return microseconds / 29 / 2;
}



