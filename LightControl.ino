/*
Name:    Sketch1.ino
Created: 12/7/2015 8:14:30 PM
Author:  Daniel & Daniel
*/
#include <Wire.h>
#include <IRremote.h>
#include <BH1750.h>
#include <FastLED.h>

// real rgb
#define IR_BPlus  0xFF3AC5  // 
#define IR_BMinus 0xFFBA45  // 
#define IR_ONOFF     0xFF02FD  // 
#define IR_PLAY    0xFF827D  // 
#define IR_Red    0xFF1AE5  // 
#define IR_Green    0xFF9A65  // 
#define IR_Blue      0xFFA25D  // 
#define IR_White    0xFF22DD  // 
#define IR_NearlyRed   0xFF2AD5  // 
#define IR_LightGreen 0xFFAA55  // 
#define IR_LightBlue   0xFF926D  // 
#define IR_LightPink   0xFF12ED  // 
#define IR_RedOrange   0xFF0AF5  // 
#define IR_SeaBlue   0xFF8A75  // 
#define IR_Violet   0xFFB24D  // 
#define IR_SortofPink   0xF3F2CD  // 
#define IR_Orange   0xFF38C7  // 
#define IR_BlueTurquois    0xFFB847  // 
#define IR_DarkViolet    0xFF7887  // 
#define IR_lighterBlue    0xFFF807  // 
#define IR_Yellow    0xFF18E7  // 
#define IR_GreenBlue    0xFF9867  // 
#define IR_Pink    0xFF58A7  // 
#define IR_SomeBlue    0xFFD827  // 
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


//PWM PINS
// D10, D9, D6, D5

// IR led
#define IRledPin 8
// PIR Input 1: CHANGED 
#define Movement_1 15
// PIR Input 2
#define Movement_2 16
// IR input
// von vorne: links=eingang, mitte=ground, recht=vcc
#define IRrecPin 14
// Transistor output (for white led strip)
#define WhiteLedPIN 10

// RGB Outputs:
#define RedLED 9
#define BlueLED 6
#define GreenLED 5

#define SIZE    255
#define DELAY    20
#define HUE_MAX  255
#define HUE_DELTA 0.01
#define RAND_MAX  255;


// WhiteLED
uint8_t lastBrightness = 0;
uint8_t whiteBrightness = 255;

// RGB Color in HSV
CHSV currentColor;

// How long the light in ms shall stay on
#define DEFAULT_TIMER_ON 30000
float TimerExtension = 0;

// Below what lux the leds shall be turned on
uint16_t LightThreshold = 10;

// Timestamp of first movement (when lamp was off)
unsigned long StartTime = 0;
// Timestamp when motion was captured
unsigned long LastDetectionTime = 0;

// Is the thing on?
bool isOn = false;

// are the rgb leds on?
bool isRGBon = false;

// State where settings can be changed (light threshold) or sensor input visualized
bool SetupModeEnabled = false;


enum colormode { staticColor, flashing, rainbow, randomcolor };
colormode currentMode;
// if any timed color thingy is on
bool interruptEnabled = false;
// duration between animation in ms
int animationspeed = 100;
// last animation at 
unsigned long lastanimation = 0;
// strobo status
int isStroboOn = false;
// current hue (for rainbow animation)
int rainbowHue = 0;

IRsend irsend(IRledPin);
IRrecv irrecv(IRrecPin);
decode_results results;

BH1750 lightMeter;


// the setup function runs once when you press reset or power the board
void setup() {

	Serial.begin(9600);
	bitSet(TCCR1B, WGM12);

	analogWrite(WhiteLedPIN, 0);
	analogWrite(RedLED, 0);
	analogWrite(GreenLED, 0);
	analogWrite(BlueLED, 0);

	currentMode = staticColor;
	//pinMode(Movement_1, INPUT);
	//pinMode(Movement_2, INPUT);
	irrecv.enableIRIn();

	// Light measure
	lightMeter.begin(BH1750_CONTINUOUS_HIGH_RES_MODE);
	delay(2000);
	Serial.print("Start: ");
	SetupModeEnabled = false;
	currentColor.setHSV(50, 100, 100);
	randomSeed(analogRead(0));
}

// the loop function runs over and over again until power down or reset
void loop()
{
	unsigned long elapsedTime = millis();

	bool movementDetected = checkMovement();

	// Normal Mode
	if (movementDetected && !SetupModeEnabled)
	{
		movementDetected = false;

		LastDetectionTime = elapsedTime;
		Serial.print("NewTime: ");
		Serial.println(LastDetectionTime);

		// if the LEDS are off...
		if (!isOn)
		{
			uint16_t lux = readLux();
			// .. and it's dark enough:
			if (lux <= LightThreshold)
			{
				TurnFullOn();
				StartTime = elapsedTime;
			}
		}
		else
		{
			unsigned long diff = elapsedTime - StartTime;
			if (diff <= 15000)
			{
				TimerExtension = 0;
			}
			// between 15 and 60 seconds activity: f(x)= 2/3x + 20000
			else if (diff > 15000 && diff <= 60000)
			{
				TimerExtension = ((float)(2.0f / 3.0f)* diff) - 10000;
			}
			// between 1 minute and 20 minutes use f(x) = 2/7x + 5/7*60000
			else if (diff >= 60000 && diff <= 1200000)
			{
				TimerExtension = ((float)(2.0f / 7.0f)* diff) + ((float)(5.0f / 7.0f) * 60000) - 30000;
			}
			//Serial.print("Diff:");
			//Serial.print(diff);

			//Serial.print(" -> TimerExtension update:");
			//Serial.println(TimerExtension);


			// if an animated color mode is active
			if (currentColor != staticColor && elapsedTime > lastanimation + animationspeed)
			{
				switch (currentMode)
				{
				case flashing:
					if (isStroboOn)
					{
						Serial.println("STROBO AUS");
						analogWrite(WhiteLedPIN, 0);
						ChangeRGBColor(HUE_RED, 255, 0);
						isStroboOn = false;
					}
					else
					{
						Serial.println("STROBO AN");
						analogWrite(WhiteLedPIN, 255);
						ChangeRGBColor(HUE_RED, 255, 255);
						isStroboOn = true;
					}
					break;
				case randomcolor:
					ChangeRGBColor(random(0, 255), 255, 255);
					break;
				case rainbow:
					rainbowHue %= 255;
					rainbowHue += 1;
					ChangeRGBColor(rainbowHue, 255, 255);
					break;
				default:
					break;
				}
				lastanimation = elapsedTime;
			}
		}
	}
	// If TIMER_ON has elapsed (no movement detected for the TIMER_ON duration) turn the whole thing off (also handles rollover)
	else if (((unsigned long)(elapsedTime - LastDetectionTime) > DEFAULT_TIMER_ON + TimerExtension) && isOn)
	{
		TurnFullOff();
	}



	if (readIRCode())
	{
		evalIRCode();
	}
}


// toggles both RGB and white LEDs
void ToggleFull()
{
	if (isOn)
	{
		TurnFullOff();
	}
	else
	{
		TurnFullOn();
	}


}

void TurnFullOn()
{
	SetRGBtoCurrColor();
	FadeWhite();
	isOn = true;
}

void TurnFullOff()
{
	Serial.println("OFF");
	int tempBrightness = whiteBrightness;
	TurnWhiteOff();
	delay(2000);
	if (!checkMovement())
	{
		TurnRGBOff();
		isOn = false;
	}
	else
	{
		whiteBrightness = tempBrightness;
		FadeWhite();
	}
}


uint16_t readLux()
{
	uint16_t lux = lightMeter.readLightLevel();
	Serial.print("Light: ");
	Serial.print(lux);
	Serial.println(" lx");
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

	switch (results.value)
	{

	case IR_BPlus:
		if (whiteBrightness <= 223)
			whiteBrightness += 32;
		else if (whiteBrightness >= 224)
			whiteBrightness = 255;
		FadeWhite();
		break;
	case IR_BMinus:
		if (whiteBrightness > 31)
			whiteBrightness -= 32;
		else
			whiteBrightness = 0;
		FadeWhite();
		break;
	case IR_ONOFF:
		ToggleFull();
		break;
	case IR_PLAY:
		currentMode = staticColor;
		ChangeRGBColor(HUE_ORANGE, 255, 255); break;
		FadeWhite();
		break;
	case IR_Red:
		ChangeRGBColor(HUE_RED, 255, 255); break;
	case IR_Green:
		ChangeRGBColor(HUE_GREEN, 255, 255); break;
	case IR_Blue:
		ChangeRGBColor(HUE_BLUE, 255, 255); break;
	case IR_White:
		ChangeRGBColor(0, 0, 255); break;
	case IR_NearlyRed:
		ChangeRGBColor(8, 255, 255); break;
	case IR_RedOrange:
		ChangeRGBColor(14, 255, 255); break;
	case IR_Orange:
		ChangeRGBColor(HUE_ORANGE, 255, 255); break;
	case IR_Yellow:
		ChangeRGBColor(HUE_YELLOW, 255, 255); break;
	case IR_LightGreen:
		ChangeRGBColor(HUE_GREEN + 16, 255, 255); break;
	case IR_SeaBlue:
		ChangeRGBColor(HUE_GREEN + 32, 255, 255); break;
	case IR_BlueTurquois:
		ChangeRGBColor(HUE_GREEN + 48, 255, 255); break;
	case IR_GreenBlue:
		ChangeRGBColor(HUE_GREEN + 56, 255, 255); break;
	case IR_LightBlue:
		ChangeRGBColor(HUE_BLUE + 16, 255, 255); break;
	case IR_Violet:
		ChangeRGBColor(HUE_BLUE + 32, 255, 255); break;
	case IR_DarkViolet:
		ChangeRGBColor(HUE_BLUE + 40, 255, 255); break;
	case IR_Pink:
		ChangeRGBColor(HUE_PINK, 255, 255); break;
	case IR_LightPink:
		ChangeRGBColor(HUE_PINK + 8, 255, 255); break;
	case IR_SortofPink:
		ChangeRGBColor(HUE_PINK + 16, 255, 255); break;
	case IR_lighterBlue:
		ChangeRGBColor(HUE_PINK + 24, 255, 255); break;
	case IR_SomeBlue:
		ChangeRGBColor(HUE_PINK + 31, 255, 255); break;
	case IR_UPR:
		//tempTimerExtension = 900000;
		//FlashWhite(1);
		break;
	case IR_UPG: break;
	case IR_UPB: break;
	case IR_QUICK:
		if (animationspeed - 10 > 1)
			animationspeed -= 10;
		else
			animationspeed = 1;
		break;
	case IR_DOWNR: break;
		//tempTimerExtension = 0;
		//FlashWhite(2);
		break;
	case IR_DOWNG: break;
	case IR_DOWNB: break;
	case IR_SLOW: break;
		if (animationspeed + 10 < 200)
			animationspeed += 10;
		else
			animationspeed = 200;
		break;
	case IR_DIY1:
		ToggleRGB();
		break;
	case IR_DIY2:
		// increase start time by 20 minutes, so timer_on is set to max
		StartTime -= 1200000;
		break;
	case IR_DIY3: break;
	case IR_AUTO:
		currentMode = randomcolor;
		break;
	case IR_DIY4: break;
	case IR_DIY5: break;
	case IR_DIY6: break;
	case IR_FLASH:
		currentMode = flashing;
		isStroboOn = false;
		break;
	case IR_JUMP3: break;
	case IR_JUMP7:
		break;
	case IR_FADE3: break;
	case IR_FADE7:
		currentMode = rainbow;

		break;

	}


	return 0;
}

bool checkMovement()
{
	int moveState_1 = digitalRead(Movement_1);
	int moveState_2 = digitalRead(Movement_2);

	if (moveState_1 == HIGH || moveState_2 == HIGH)
	{
		if (moveState_1 == HIGH)
			Serial.println("MotionOn1");
		if (moveState_2 == HIGH)
			Serial.println("MotionOn2");

		return true;
	}

	return false;
}

void FadeWhite()
{
	Serial.println("Turn White ON");

	int step = 2;

	if (whiteBrightness > lastBrightness)
	{
		for (int fadeValue = lastBrightness; fadeValue <= whiteBrightness; fadeValue += step)
		{
			// sets the value (range from 0 to 255):
			analogWrite(WhiteLedPIN, fadeValue);
			Serial.println(fadeValue);
			// wait for some milliseconds to see the dimming effect
			delay(20);
			if (fadeValue >= 254)
				break;
		}
	}
	else
	{
		for (int fadeValue = lastBrightness; fadeValue >= whiteBrightness; fadeValue -= step)
		{
			analogWrite(WhiteLedPIN, fadeValue);
			Serial.println(fadeValue);
			delay(20);
			if (fadeValue <= 2)
			{
				analogWrite(WhiteLedPIN, 0);
				whiteBrightness = 0;
				break;
			}
		}
	}
	lastBrightness = whiteBrightness;
}

void TurnWhiteOff()
{
	Serial.println("Turn White OFF");

	uint8_t tempBrightness = whiteBrightness;
	whiteBrightness = 0;
	FadeWhite();
	whiteBrightness = tempBrightness;

}

void ChangeRGBColor(uint8_t hue, uint8_t saturation, uint8_t value)
{
	currentColor.setHSV(hue, saturation, value);
	SetRGBtoCurrColor();
}


void ToggleRGB()
{
	if (!isRGBon)
		SetRGBtoCurrColor();
	else
		TurnRGBOff();
}

void TurnRGBOff()
{
	analogWrite(RedLED, 0);
	analogWrite(GreenLED, 0);
	analogWrite(BlueLED, 0);
	isRGBon = false;
}

void SetRGBtoCurrColor()
{
	CRGB rgbvalue;
	Serial.print("HSV:");

	Serial.print(currentColor.hue);
	Serial.print(", ");
	Serial.print(currentColor.sat);
	Serial.print(", ");
	Serial.println(currentColor.val);


	hsv2rgb_rainbow(currentColor, rgbvalue);
	Serial.print("Turn RGB to:");
	Serial.print(rgbvalue.red);
	Serial.print(", ");
	Serial.print(rgbvalue.green);
	Serial.print(", ");
	Serial.println(rgbvalue.blue);



	analogWrite(RedLED, rgbvalue.red);
	analogWrite(GreenLED, rgbvalue.green);
	analogWrite(BlueLED, rgbvalue.blue);

	isRGBon = true;
}



// Send a signal 'code' to the rgb leds
void SendRGBCode(int code)
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
	analogWrite(WhiteLedPIN, whiteBrightness);
	Serial.println("");
}


void IncreaseThreshold()
{
	LightThreshold += 2;
	LastDetectionTime = 0;

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
	LastDetectionTime = 0;
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

