/* This code works with JSN SR04 T ultrasound module and LCD i²c screen
 * It measures the distance and shows it on the LCD screen in cm
 * Refer to www.SurtrTech. com or SurtrTech YT channel for more informations 
 */

#include <LiquidCrystal_I2C.h> //Lcd library

#define I2C_ADDR 0x27 //or(0x3F) I2C adress, you should use the code to scan the adress first (0x27) here
#define BACKLIGHT_PIN 3 // Declaring LCD Pins
#define En_pin 2
#define Rw_pin 1
#define Rs_pin 0
#define D4_pin 4
#define D5_pin 5
#define D6_pin 6
#define D7_pin 7

#define TRIG 11   //Module pins
#define ECHO 12 

LiquidCrystal_I2C lcd(I2C_ADDR,En_pin,Rw_pin,Rs_pin,D4_pin,D5_pin,D6_pin,D7_pin);

void setup() { 
  
  pinMode(TRIG, OUTPUT); // Initializing Trigger Output and Echo Input 
  pinMode(ECHO, INPUT_PULLUP);  
  lcd.begin (16,2);
  lcd.setBacklightPin(BACKLIGHT_PIN,POSITIVE);
  lcd.setBacklight(HIGH); //Lighting backlight
  lcd.home ();
  } 
  
  void loop() { 
    
    digitalWrite(TRIG, LOW); // Set the trigger pin to low for 2uS 
    delayMicroseconds(2); 
    
    digitalWrite(TRIG, HIGH); // Send a 10uS high to trigger ranging 
    delayMicroseconds(20); 
    
    digitalWrite(TRIG, LOW); // Send pin low again 
    float distance = pulseIn(ECHO, HIGH,26000); // Read in times pulse 
    
    distance = distance/58;//Convert the pulse duration to distance
                           //You can add other math functions to calibrate it well
       
    lcd.clear();
    lcd.print("Distance:");
    lcd.setCursor(0,1);
    lcd.print(distance,1);
    lcd.setCursor(6,1);
    lcd.print("cm");
    delay(500);
    
}

