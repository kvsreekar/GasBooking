#include <Arduino.h>
#include "WiFi.h"
#include <LiquidCrystal_I2C.h>
#define WIFI_NETWORK "RSSS"
#define WIFI_PASSWORD "idontknow"
#define WIFI_TIMEOUT_MS 20000
#include "HX711.h"
#define DOUT  5
#define CLK  4
 
HX711 scale(DOUT, CLK);

LiquidCrystal_I2C lcd(0x27, 16, 2);

float calibration_factor = -109525; 

void connectToWiFi(){
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_NETWORK,WIFI_PASSWORD);
  
  unsigned long startAttemptTime = millis();

  while(WiFi.status()!=WL_CONNECTED && millis()-startAttemptTime < WIFI_TIMEOUT_MS)
  {
    Serial.print(".");
    delay(100);
  }
  if(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
  }
  else
  {
    Serial.print("Connected");
    Serial.println(WiFi.localIP());
  }
}
const int a = A0;
int R= 0;
double t = 0;
int Buzzer = 32;        // used for ESP32
int Gas_analog = 39;    // used for ESP32

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  lcd.begin();
  lcd.clear();
  lcd.print("Welcome");
  delay(2000);
  pinMode(Buzzer, OUTPUT); 
  
  Serial.println("HX711 Calibration");
  Serial.println("Remove all weight from scale");
  Serial.println("After readings begin, place known weight on scale");
  Serial.println("Press a,s,d,f to increase calibration factor by 10,100,1000,10000 respectively");
  Serial.println("Press z,x,c,v to decrease calibration factor by 10,100,1000,10000 respectively");
  Serial.println("Press t for tare");
  scale.set_scale();
  scale.tare(); //Reset the scale to 0
  long zero_factor = scale.read_average(); //Get a baseline reading
  Serial.print("Zero factor: "); //This can be used to remove the need to tare the scale. Useful in permanent scale projects.
  Serial.println(zero_factor);   
    
}

void loop() {
  int gassensorAnalog = analogRead(Gas_analog);
  R = analogRead(a);
  t = R / 9.31;
  Serial.print(" Temperature in C = ");
  Serial.print(t,1);
  lcd.clear();
  lcd.setCursor(0,1);
  lcd.print("Temp: ");
  lcd.setCursor(6,1);
  lcd.print(t);
  Serial.print("\t Gas Sensor(Analog): ");
  Serial.println(gassensorAnalog);
  lcd.setCursor(0,0);
  lcd.print("Gas: ");
  lcd.setCursor(5,0);
  lcd.print(gassensorAnalog);
  delay(1000);
  if(t>50 || gassensorAnalog >1000)
  {
    lcd.clear();
  }
  if(t>50)
  {
    Serial.println("High Temperature");
    lcd.setCursor(0,1);
    lcd.print("High Temp");
    digitalWrite(Buzzer,HIGH);
    delay(1000);
    digitalWrite(Buzzer,LOW);
  }
  if (gassensorAnalog > 1000) {
    Serial.println("!WARNING! GAS DETECTED");
    lcd.setCursor(0,0);
    lcd.print("GAS DETECTED");
    digitalWrite (Buzzer, HIGH) ; //send tone
    delay(1000);
    digitalWrite (Buzzer, LOW) ;  //no tone
  }

  delay(100);

  scale.set_scale(calibration_factor); //Adjust to this calibration factor
 
  Serial.print("Reading: ");
  Serial.print(scale.get_units());
  Serial.print(" kg"); //Change this to kg and re-adjust the calibration factor if you follow SI units like a sane person
  lcd.clear();
  lcd.print(scale.get_units() ,3);
  lcd.print(" kg");
  Serial.print(" \t calibration_factor: ");
  Serial.print(calibration_factor);
  Serial.println();
  if(Serial.available())
  {
    char temp = Serial.read();
    if(temp == '+' || temp == 'a')
      calibration_factor += 10;
    else if(temp == '-' || temp == 'z')
      calibration_factor -= 10;
    else if(temp == 's')
      calibration_factor += 100;  
    else if(temp == 'x')
      calibration_factor -= 100;  
    else if(temp == 'd')
      calibration_factor += 1000;  
    else if(temp == 'c')
      calibration_factor -= 1000;
    else if(temp == 'f')
      calibration_factor += 10000;  
    else if(temp == 'v')
      calibration_factor -= 10000;  
    else if(temp == 't')
      scale.tare();  //Reset the scale to zero
  }
  delay(2000);

  
}
