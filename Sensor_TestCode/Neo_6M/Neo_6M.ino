#include <SoftwareSerial.h>
#include <TinyGPS++.h>

// GPS set pin
static const int RXPin = 2, TXPin = 16;   //  RX = D4, TX = D0
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

//Global variables
String latt;
String lngt;

void setup() {
  Serial.begin(9600);
  // GPS start
  ss.begin(GPSBaud);
  Serial.println();
}

bool checkGPS() {
  if (gps.charsProcessed() < 10) {
    Serial.println(F("No GPS detected: check wiring."));
    return true;
  } else {
    return false;
  }
}

void loop()
{
  if (checkGPS()== true) {
      delay(10000);
    }
  if (ss.available() > 0) {
    if (gps.encode(ss.read())) {
      if (gps.location.isValid()) {
        // get latt and lngt
        latt = String(float(gps.location.lat()),6);
        lngt = String(float(gps.location.lng()),6);

        Serial.println(latt+","+lngt+"\r\n");
      } else {
        Serial.print("GPS data not valid: ");
        Serial.print(gps.location.lat(), 6);
        Serial.print(" ");
        Serial.print(gps.location.lng(), 6);
        Serial.print(" ");
        Serial.println(gps.location.isValid());
      }
    }
  } 
}
