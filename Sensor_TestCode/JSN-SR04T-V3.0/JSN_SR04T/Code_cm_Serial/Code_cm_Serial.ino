/* This code works with JSN SR04 T ultrasound module
 * It measures the distance and shows it on the Serial monitor
 * Refer to www.SurtrTech. com or SurtrTech YT channel for more informations 
 */

#define TRIG 11 //Module pins
#define ECHO 12 

void setup() { 
  
  Serial.begin(9600); // Serial monitoring 
  pinMode(TRIG, OUTPUT); // Initializing Trigger Output and Echo Input 
  pinMode(ECHO, INPUT_PULLUP);
  } 
  
  void loop() { 
    
    digitalWrite(TRIG, LOW); // Set the trigger pin to low for 2uS 
    delayMicroseconds(2); 
    
    digitalWrite(TRIG, HIGH); // Send a 10uS high to trigger ranging 
    delayMicroseconds(20); 
    
    digitalWrite(TRIG, LOW); // Send pin low again 
    int distance = pulseIn(ECHO, HIGH,26000); // Read in times pulse 
    
    distance= distance/58; //Convert the pulse duration to distance
                           //You can add other math functions to calibrate it well
                           
    Serial.print("Distance ");
    Serial.print(distance);
    Serial.println("cm");
   
    delay(50);
    
}

