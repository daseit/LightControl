/*
  Name:    Sketch1.ino
  Created: 12/7/2015 8:14:30 PM
  Author:  Daniel & Daniel
*/
#include <Wire.h>
#include <IRremote.h>
#include <BH1750.h>

// real rgb
#define  IR_BPlus  0xFF3AC5  // 
#define IR_BMinus 0xFFBA45  // 
#define IR_ON     0xFF02FD  // 
#define IR_OFF    0xFF02FD  // 
#define IR_PLAY    0xFF827D  // 
#define IR_R    0xFF1AE5  // 
#define IR_G    0xFF9A65  // 
#define IR_B      0xFFA25D  // 
#define IR_W    0xFF22DD  // 
#define IR_B1   0xFF2AD5  // 
#define IR_B2   0xFFAA55  // 
#define IR_B3   0xFF926D  // 
#define IR_B4   0xFF12ED  // 
#define IR_B5   0xFF0AF5  // 
#define IR_B6   0xFF8A75  // 
#define IR_B7   0xFFB24D  // 
#define IR_B8   0xF3F2CD  // 
#define IR_B9   0xFF38C7  // 
#define IR_B10    0xFFB847  // 
#define IR_B11    0xFF7887  // 
#define IR_B12    0xFFF807  // 
#define IR_B13    0xFF18E7  // 
#define IR_B14    0xFF9867  // 
#define IR_B15    0xFF58A7  // 
#define IR_B16    0xFFD827  // 
#define IR_UPR    0xFF28D7  // 
#define IR_UPG    0xFFA857  // 
#define IR_UPB    0xFF6897  // 
#define IR_QUICK  0xFFE817  // 
#define IR_DOWNR  0xFF08F7  // 
#define IR_DOWNG  0xFF8877  // 
#define IR_DOWNB  0xFF48B7  // 
#define IR_SLOW   0xFFC837  // 
#define IR_DIY1   0xFF30CF  // 
#define IR_DIY2   0xFFB04F  // 
#define IR_DIY3   0xFF708F  // 
#define IR_AUTO   0xFFF00F  // 
#define IR_DIY4   0xFF10EF  // 
#define IR_DIY5   0xFF906F  // 
#define IR_DIY6   0xFF50AF  // 
#define IR_FLASH  0xFFD02F  // 
#define IR_JUMP3  0xFF20DF  // 
#define IR_JUMP7  0xFFA05F  // 
#define IR_FADE3  0xFF609F  // 
#define IR_FADE7  0xFFE01F  // 
// cheap one
#define IR_ONCHEAP 0xFFB04F
//#define IR_OFF 0xFFF807

#define DIY1 0xFF052542FD
#define DIY2 0xFF0255252FD

// IR led
#define IRledPin 8
// PIR Input 1
#define Movement_1 11
// PIR Input 2
#define Movement_2 12
// IR input
// CHANGED: From 2 to 3, von vorne: links=eingang, mitte=ground, recht=vcc
#define IRrecPin 3
// Transistor output (for white led strip)
//Changed: From 9 to 5
#define WhiteLedPIN 5
// How long the light shall stay on
#define TIMER_ON 45000
// Under what lux the leds shall be turned on
int LightThreshold = 1000;

// Timestamp when motion was captured
unsigned long StartTime = 0;

bool isOn = false;

// State where settings can be changed (light threshold) or sensor input visualized
bool SetupModeEnabled = false;

IRsend irsend(IRledPin);
IRrecv irrecv(IRrecPin);
decode_results results;


BH1750 lightMeter;


// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(9600);
  //CHANGE
  analogWrite(WhiteLedPIN, 0);
  pinMode(IRledPin, OUTPUT);
  pinMode(Movement_1, INPUT);
  pinMode(Movement_2, INPUT);
  irrecv.enableIRIn();

  // Light measure
  lightMeter.begin(BH1750_CONTINUOUS_HIGH_RES_MODE_2);
  delay(2000);
  Serial.print("Start: ");
  SetupModeEnabled = false;
}




// the loop function runs over and over again until power down or reset
void loop()
{
  unsigned long elapsedTime = millis();

  bool movementDetected = checkMovement();
  if (movementDetected)
  {
    // Serial.print("Movement:");
   
    movementDetected = false;
    if (!SetupModeEnabled)
    {
      StartTime = elapsedTime;
      int lux = readLux();

//      Serial.print("Light: ");
//      Serial.print(lux);
//      Serial.println(" lx");

      if (!isOn && lux < LightThreshold)
      {
        TurnFullOn();
      }
    }
    else
    {
      FlashWhite(2);
      delay(2000);
    }
  }

  if (readIRCode())
  {
    evalIRCode();
  }

  // If TIMER_ON has elapsed (also handles rollover)
  if (((unsigned long)(elapsedTime - StartTime) > TIMER_ON) && isOn)
  {
    Serial.println("OFF");
    TurnWhiteOff();
    delay(1100);
    //if (!checkMovement())
    //{
    SendRGBOff();
    isOn = false;
    //}
    /*  else
      {
        TurnWhiteOn();
      }*/
  }

}

void TurnFullOn()
{
  delay(20);
  SendRGBOn();
  TurnWhiteOn();
  isOn = true;
}



uint16_t readLux()
{
  uint16_t lux = lightMeter.readLightLevel();
  //  Serial.print("Light: ");
  //  Serial.print(lux);
  //  Serial.println(" lx");
  return lux;
}

bool readIRCode()
{
  if (irrecv.decode(&results)) {
    if (results.decode_type == NEC) {
      Serial.print("Signal: NEC: ");
    }
    else if (results.decode_type == SONY) {
      Serial.print("SONY: ");
    }
    else if (results.decode_type == RC5) {
      Serial.print("RC5: ");
    }
    else if (results.decode_type == RC6) {
      Serial.print("RC6: ");
    }
    else if (results.decode_type == UNKNOWN) {
      Serial.print("UNKNOWN: ");
    }

    Serial.println(results.value, HEX);
    // Serial.println(results.rawbuf.Values);
    irrecv.resume(); // Receive the next value
    return true;
  }

  return false;
}

int evalIRCode()
{
  if (results.value == IR_PLAY)
  {
    Serial.println("Setup");
    SetupModeEnabled = !SetupModeEnabled;
    if (SetupModeEnabled)
    {
      TurnWhiteOff();
      SendRGBOn();
      isOn = false;
      FlashWhite(2);
    }
    else
    {
      FlashWhite(4);
    }
  }
  else if (SetupModeEnabled)
  {
    switch (results.value)
    {
      case IR_DIY3:
        IncreaseThreshold();
        break;
      case IR_DIY6:
        DecreaseThreshold();
        break;
      case IR_DIY4:
        // Show light level
        analogWrite(WhiteLedPIN, 0);
        FlashWhite(readLux());
        break;
      default:
        return 1;
        break;
    }
  }
  return 0;
}

bool checkMovement()
{
  int moveState_1 = digitalRead(Movement_1);
  int moveState_2 = digitalRead(Movement_2);
//  if (moveState_1 == HIGH)
//  {
//    Serial.print("PIR 1:");    
//    Serial.println(moveState_1);
//  }
//  if (moveState_2 == HIGH)
//  {
//    Serial.print("PIR 2:");
//    Serial.println(moveState_2);
//  }
  if (moveState_1 == HIGH || moveState_2 == HIGH)
  {
    // Serial.println("Movement detected");
    return true;
  }

  return false;
}

void TurnWhiteOn()
{
  Serial.println("Turn White ON");
  for (int fadeValue = 0; fadeValue <= 254; fadeValue += 2)
  {
    // sets the value (range from 0 to 255):
    analogWrite(WhiteLedPIN, fadeValue);
    // wait for 30 milliseconds to see the dimming effect
    delay(20);
  }
  analogWrite(WhiteLedPIN, 255);
}

void TurnWhiteOff()
{
  Serial.println("Turn White OFF");
  for (int fadeValue = 255; fadeValue >= 0; fadeValue -= 5) {
    // sets the value (range from 0 to 255):
    analogWrite(WhiteLedPIN, fadeValue);
    // wait for 30 milliseconds to see the dimming effect
    delay(30);
  }

}

// Send the on signal as long as it stays 'dark'
void SendRGBOn()
{
  // IR LED
  int lux1 = readLux();
  lux1 = readLux();
  int lux2 = 0;
  Serial.print("Sending ");
  Serial.print(lux1);
  Serial.print("  ");
  Serial.println(lux2);

  while (lux2 < lux1 + 15)
  {
    Serial.println("Sending RGB ON");
    sendHexNEC(IR_ON, 32, 1, 38);
    delay(200);
    lux2 = readLux();

    if (lux2 < lux1)
    {
      lux1 = lux2;
      Serial.print("new lux1: ");
      Serial.println(lux1);
    }

    delay(200);
  }

}

// Send the on signal as long as it stays 'bright'
void SendRGBOff()
{
  int lux1 = readLux();
  int lux2 = lux1;
  while (lux2 > lux1 - 10)
  {
    Serial.println("Sending RGB OFF");
    sendHexNEC(IR_ON, 32, 1, 38);
    delay(200);
    lux2 = readLux();

    if (lux2 > lux1)
    {
      lux1 = lux2;
      Serial.print("new lux1: ");
      Serial.println(lux1);
    }
    delay(200);
  }
  // Wait for motion sensor to calm down
  delay(10000);
}



// Send a signal 'code' to the rgb leds
void SendRGBOff(int code)
{
    Serial.print("Sending Code:");
    Serial.println(code);
    sendHexNEC(code, 32, 1, 38);
    delay(200);
}




void FlashWhite(int amount)
{

  for (int i = 0; i < amount; i++)
  {
    Serial.print("flash");
    analogWrite(WhiteLedPIN, 0);
    delay(200);
    analogWrite(WhiteLedPIN, 255);
    delay(200);
  }
  analogWrite(WhiteLedPIN, 0);
  Serial.println("");
}


void IncreaseThreshold()
{
  LightThreshold += 2;
  StartTime = 0;

  // Show input by flashing/disabling white leds
  FlashWhite(2);

  Serial.print("New threshold ");
  Serial.println(LightThreshold);
}

void DecreaseThreshold()
{
  if (LightThreshold > 2)
  {
    LightThreshold -= 2;
  }
  else
  {
    FlashWhite(5);
  }
  StartTime = 0;
  // Show input by flashing/disabling white leds
  FlashWhite(1);

  Serial.print("New threshold ");
  Serial.println(LightThreshold);

}


#define NEC_BIT_COUNT 32
unsigned char carrierFreq = 0; //default
unsigned char period = 0; //calculated once for each signal sent in initSoftPWM
unsigned char periodHigh = 0; //calculated once for each signal sent in initSoftPWM
unsigned char periodLow = 0; //calculated once for each signal sent in initSoftPWM
unsigned long sigTime = 0; //used in mark & space functions to keep track of time
unsigned long sigStart = 0; //used to calculate correct length of existing signal, to handle some repeats

void sendRawBuf(unsigned  int *sigArray, unsigned int sizeArray, unsigned char kHz) {

  if (carrierFreq != kHz)  initSoftPWM(kHz); //we only need to re-initialise if it has changed from last signal sent
  sigTime = micros(); //keeps rolling track of signal time to avoid impact of loop & code execution delays
  for (int i = 0; i < sizeArray; i++) {
    Serial.println(sizeArray);
    mark(sigArray[i++]); //also move pointer to next position
    if (i < sizeArray) { //check we have a space remaining before sending it
      space(sigArray[i]); //pointer will be moved by for loop
    }
  }
  Serial.println("Sent Raw.");
}



void sendHexNEC(unsigned long sigCode, byte numBits, unsigned char repeats, unsigned char kHz) {
  /*  A basic 32 bit NEC signal is made up of:
    1 x 9000 uSec Header Mark, followed by
    1 x 4500 uSec Header Space, followed by
    32 x bits uSec ( 1- bit 560 uSec Mark followed by 1690 uSec space; 0 - bit 560 uSec Mark follwed by 560 uSec Space)
    1 x 560 uSec Trailer Mark
    There can also be a generic repeat signal, which is usually not neccessary & can be replaced by sending multiple signals
  */
#define NEC_HEADER_MARK 9000
#define NEC_HEADER_SPACE 4500
#define NEC_ONE_MARK 560
#define NEC_ZERO_MARK 560
#define NEC_ONE_SPACE 1690
#define NEC_ZERO_SPACE 560
#define NEC_TRAILER_MARK 560

  unsigned long bitMask = (unsigned long)1 << (numBits - 1); //allows for signal from 1 bit up to 32 bits
  //
  if (carrierFreq != kHz)  initSoftPWM(kHz); //we only need to re-initialise if it has changed from last signal sent

  sigTime = micros(); //keeps rolling track of signal time to avoid impact of loop & code execution delays
  sigStart = sigTime; //remember for calculating first repeat gap (space), must end 108ms after signal starts
  // First send header Mark & Space
  mark(NEC_HEADER_MARK);
  space(NEC_HEADER_SPACE);

  while (bitMask) {
    if (bitMask & sigCode) { //its a One bit
      mark(NEC_ONE_MARK);
      space(NEC_ONE_SPACE);
    }
    else { // its a Zero bit
      mark(NEC_ZERO_MARK);
      space(NEC_ZERO_SPACE);
    }
    bitMask = (unsigned long)bitMask >> 1; // shift the mask bit along until it reaches zero & we exit the while loop
  }
  // Last send NEC Trailer MArk
  mark(NEC_TRAILER_MARK);

  //now send the requested number of NEC repeat signals. Repeats can be useful for certain functions like Vol+, Vol- etc
  /*  A repeat signal consists of
     A space which ends 108ms after the start of the last signal in this sequence
     1 x 9000 uSec Repeat Header Mark, followed by
     1 x 2250 uSec Repeat Header Space, followed by
     32 x bits uSec ( 1- bit 560 uSec Mark followed by 1690 uSec space; 0 - bit 560 uSec Mark follwed by 560 uSec Space)
     1 x 560 uSec repeat Trailer Mark
  */
  //First calcualte length of space for first repeat
  //by getting length of signal to date and subtracting from 108ms

  if (repeats == 0) return; //finished - no repeats
  else if (repeats > 0) { //first repeat must start 108ms after first signal
    space(108000 - (sigTime - sigStart)); //first repeat Header should start 108ms after first signal
    mark(NEC_HEADER_MARK);
    space(NEC_HEADER_SPACE / 2); //half the length for repeats
    mark(NEC_TRAILER_MARK);
  }

  while (--repeats > 0) { //now send any remaining repeats
    space(108000 - NEC_HEADER_MARK - NEC_HEADER_SPACE / 2 - NEC_TRAILER_MARK); //subsequent repeat Header must start 108ms after previous repeat signal
    mark(NEC_HEADER_MARK);
    space(NEC_HEADER_SPACE / 2); //half the length for repeats
    mark(NEC_TRAILER_MARK);
  }

}


void initSoftPWM(unsigned char carrierFreq) { // Assumes standard 8-bit Arduino, running at 16Mhz
  //supported values are 30, 33, 36, 38, 40, 56 kHz, any other value defaults to 38kHz
  //we will aim for a  duty cycle of circa 33%

  period = (1000 + carrierFreq / 2) / carrierFreq;
  periodHigh = (period + 1) / 3;
  periodLow = period - periodHigh;
  //  Serial.println (period);
  //  Serial.println (periodHigh);
  //  Serial.println (periodLow);
  // Serial.println (carrierFreq);

  switch (carrierFreq) {
    case 30: //delivers a carrier frequency of 29.8kHz & duty cycle of 34.52%
      periodHigh -= 6; //Trim it based on measurementt from Oscilloscope
      periodLow -= 10; //Trim it based on measurementt from Oscilloscope
      break;

    case 33: //delivers a carrier frequency of 32.7kHz & duty cycle of 34.64%
      periodHigh -= 6; //Trim it based on measurementt from Oscilloscope
      periodLow -= 10; //Trim it based on measurementt from Oscilloscope
      break;

    case 36: //delivers a carrier frequency of 36.2kHz & duty cycle of 35.14%
      periodHigh -= 6; //Trim it based on measurementt from Oscilloscope
      periodLow -= 11; //Trim it based on measurementt from Oscilloscope
      break;

    case 40: //delivers a carrier frequency of 40.6kHz & duty cycle of 34.96%
      periodHigh -= 6; //Trim it based on measurementt from Oscilloscope
      periodLow -= 11; //Trim it based on measurementt from Oscilloscope
      break;

    case 56: //delivers a carrier frequency of 53.8kHz & duty cycle of 40.86%
      periodHigh -= 6; //Trim it based on measurementt from Oscilloscope
      periodLow -= 12; //Trim it based on measurementt from Oscilloscope
      Serial.println(periodHigh);
      Serial.println(periodLow);

      break;


    case 38: //delivers a carrier frequency of 37.6kHz & duty cycle of 36.47%
    default:
      periodHigh -= 6; //Trim it based on measurementt from Oscilloscope
      periodLow -= 11; //Trim it based on measurementt from Oscilloscope
      break;
  }
}

void mark(unsigned int mLen) { //uses sigTime as end parameter
  sigTime += mLen; //mark ends at new sigTime
  unsigned long now = micros();
  unsigned long dur = sigTime - now; //allows for rolling time adjustment due to code execution delays
  if (dur == 0) return;
  while ((micros() - now) < dur) { //just wait here until time is up
    digitalWrite(IRledPin, HIGH);
    if (periodHigh) delayMicroseconds(periodHigh);
    digitalWrite(IRledPin, LOW);
    if (periodLow)  delayMicroseconds(periodLow);
  }
}

void space(unsigned int sLen) { //uses sigTime as end parameter
  sigTime += sLen; //space ends at new sigTime
  unsigned long now = micros();
  unsigned long dur = sigTime - now; //allows for rolling time adjustment due to code execution delays
  if (dur == 0) return;
  while ((micros() - now) < dur); //just wait here until time is up
}

