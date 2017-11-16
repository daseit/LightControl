/*
 Name:    Sketch1.ino
 Created: 12/7/2015 8:14:30 PM
 Author:  Daniel & Daniel
*/
#include <Wire.h>

#define LICHTSENSOR_I2C 0x23

// the setup function runs once when you press reset or power the board
void setup() {
  Wire.begin();
  Serial.begin(9600);
  lichtsensor_init();
  delay(1000); // Notwendig ?
}

// the loop function runs over and over again until power down or reset
void loop() {
  uint16_t wert = 0;
  if (lichtsensor_auslesen(&wert)) {
    Serial.print("Beleuchtung: ");
    Serial.print(wert / 1.2);
    Serial.println("lux");
  }
  else {
    Serial.println("Fehler beim auslesen");
  }
  delay(1000);
}

void lichtsensor_init() {
  Wire.beginTransmission(LICHTSENSOR_I2C); // I2C Adresse
  Wire.write(0x10); // H-Mode, also 1Lux resolution
  Wire.endTransmission();
}

bool lichtsensor_auslesen(uint16_t* ausgelesener_wert) {
  //Wire.beginTransmission(LICHTSENSOR_I2C); // Nicht notwendig?
  Wire.requestFrom(LICHTSENSOR_I2C, 2);

  uint8_t stelle0 = 0;
  uint8_t stelle1 = 0;

  if (Wire.available()) {
    stelle0 = Wire.read();
  }
  else {
    return false;
  }
  if (Wire.available()) {
    stelle1 = Wire.read();
  }
  else {
    return false;
  }
  
  *ausgelesener_wert = (stelle0 << 8) | stelle1;

  return true;
}
