#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <Wire.h>
#include <SD.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// GPS set pin
static const int RXPin = 2, TXPin = 16;   //  RX = D4, TX = D0
static const uint32_t GPSBaud = 9600;
TinyGPSPlus gps;
SoftwareSerial ss(RXPin, TXPin);

//JSN-SR04T Depth Sensor pins
// SCL = D1
// SDA = D2
#define TRIG 14 //Module pins
#define ECHO 12 

// GY-521
const int MPU_ADDR = 0x68; // I2C address of the MPU-6050. If AD0 pin is set to HIGH, the I2C address will be 0x69.

int16_t a_x, a_y, a_z; // variables for accelerometer raw data
int16_t gyro_x, gyro_y, gyro_z; // variables for gyro raw data
int16_t temperature; // variables for temperature data

char tmp_str[7]; // temporary variable used in convert function

char* convert_int16_to_str(int16_t i) { // converts int16 to string. Moreover, resulting strings will have the same length in the debug monitor.
  sprintf(tmp_str, "%6d", i);
  return tmp_str;
}

// SD set pin
const int CS_PIN = 15;  // D8 pin

//Global variables
String latt,lngt,txt,d;
int distance;


void setup() {
  Serial.begin(9600);
  // GPS start
  ss.begin(GPSBaud);
  Serial.println();

  // Initializing Trigger Output and Echo Input
  pinMode(TRIG, OUTPUT); 
  pinMode(ECHO, INPUT_PULLUP);

  // GY-521
  Wire.begin();
  Wire.beginTransmission(MPU_ADDR); // Begins a transmission to the I2C slave (GY-521 board)
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  // SD Cheack for file else create 
  File file = SD.open("/test.txt");
  if(!file) {
    Serial.println("File doens't exist");
    Serial.println("Creating file...");
    txt = "/test.txt";
    d = "Date,Time,latt,lngt \r\n";
    writetosd(txt.c_str(),d.c_str());
  } else {
    Serial.println("File already exists");  
  }
  file.close();
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
        Serial.println("GPS "+latt+","+lngt+"\r\n");
      } else {
        Serial.print("GPS data not valid: ");
        // DEBUG
        Serial.print(gps.location.lat(), 6);
        Serial.print(" ");
        Serial.print(gps.location.lng(), 6);
        Serial.print(" ");
        Serial.println(gps.location.isValid());
      }
    }
  }
  JSN_mod();
  Gyro_mod();
  String logtxt = latt+","+lngt+","+String(distance)+","+convert_int16_to_str(a_x)+","+convert_int16_to_str(a_y)+","+convert_int16_to_str(a_z)+","+temperature+","+convert_int16_to_str(gyro_x)+","+convert_int16_to_str(gyro_y)+","+convert_int16_to_str(gyro_z);
  writetosd("/test.txt",logtxt.c_str());
  Serial.println(logtxt);
  delay(1000);
}

void Gyro_mod() {
  // GY-521
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H) [MPU-6000 and MPU-6050 Register Map and Descriptions Revision 4.2, p.40]
  Wire.endTransmission(false); // the parameter indicates that the Arduino will send a restart. As a result, the connection is kept active.
  Wire.requestFrom(MPU_ADDR, 7*2, true); // request a total of 7*2=14 registers
  
  // "Wire.read()<<8 | Wire.read();" means two registers are read and stored in the same variable
  a_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x3B (ACCEL_XOUT_H) and 0x3C (ACCEL_XOUT_L)
  a_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x3D (ACCEL_YOUT_H) and 0x3E (ACCEL_YOUT_L)
  a_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x3F (ACCEL_ZOUT_H) and 0x40 (ACCEL_ZOUT_L)
  temperature = Wire.read()<<8 | Wire.read(); // reading registers: 0x41 (TEMP_OUT_H) and 0x42 (TEMP_OUT_L)
  temperature = temperature/340.00+36.53;
  gyro_x = Wire.read()<<8 | Wire.read(); // reading registers: 0x43 (GYRO_XOUT_H) and 0x44 (GYRO_XOUT_L)
  gyro_y = Wire.read()<<8 | Wire.read(); // reading registers: 0x45 (GYRO_YOUT_H) and 0x46 (GYRO_YOUT_L)
  gyro_z = Wire.read()<<8 | Wire.read(); // reading registers: 0x47 (GYRO_ZOUT_H) and 0x48 (GYRO_ZOUT_L)
  
  // print out data
  Serial.print(" | aX = "); Serial.print(convert_int16_to_str(a_x));
  Serial.print(" | aY = "); Serial.print(convert_int16_to_str(a_y));
  Serial.print(" | aZ = "); Serial.print(convert_int16_to_str(a_z));
  // the following equation was taken from the documentation [MPU-6000/MPU-6050 Register Map and Description, p.30]
  Serial.print(" | tmp = "); Serial.print(temperature);
  Serial.print(" | gX = "); Serial.print(convert_int16_to_str(gyro_x));
  Serial.print(" | gY = "); Serial.print(convert_int16_to_str(gyro_y));
  Serial.print(" | gZ = "); Serial.print(convert_int16_to_str(gyro_z));
  Serial.println();
}

void JSN_mod() {
  // JSN SR04-T
  digitalWrite(TRIG, LOW); // Set the trigger pin to low for 2uS 
  delayMicroseconds(2); 
    
  digitalWrite(TRIG, HIGH); // Send a 10uS high to trigger ranging 
  delayMicroseconds(20); 
    
  digitalWrite(TRIG, LOW); // Send pin low again 
  distance = pulseIn(ECHO, HIGH,26000); // Read in times pulse 
    
  distance= distance/58; //Convert the pulse duration to distance
                         //You can add other math functions to calibrate it well
                           
  Serial.print("Distance "+String(distance)+"cm");
}

void writetosd(const char* path, const char* message){
  if(SD.begin(CS_PIN)){
    File dataFile = SD.open(path, FILE_WRITE);

    // if the file is available, write to it:
    if(!dataFile) {
      Serial.println("Failed to open file for writing");
      return;
    }
    dataFile.seek(EOF);
    if(dataFile.write(message)) {
      Serial.println("File written");
      Serial.print("echo ");
      Serial.print(message);
      Serial.print(" >> ");
      Serial.println(path);
    } else {
      Serial.println("Write failed");
    }
    dataFile.flush();
    dataFile.close(); 
  } else {
    Serial.println("SD card not found.!");
  }
  SD.end();
}
