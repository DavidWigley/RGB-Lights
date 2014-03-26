#include "FastLED.h"
//author David

// Number of RGB LEDs in strand (this happens to be the same for
// all the left and right strands, logo is different)
//need to count # of LEDs
#define nLEDsUnder 22
#define nLEDsArms 22
#define nLEDsLUpright 22
#define nLEDsRUpright 22


// Various data pins for the strands' data and clock
//int dataPin  = 2;
//int clockPin = 3;
//*************************************
//all pins #s should be fixed
#define lUprightClockPin 8
#define lUprightDataPin 11
#define rUprightClockPin 9
#define rUprightDataPin 12
#define armDataPin 14
#define armClockPin 15
#define underDataPin 20
#define underClockPin 21
// Input pins that together make up a 4 bit number to pass in mode
//doing 5 dont care
#define modeInputPin1 2
#define modeInputPin2 3
#define allianceInputPin 5
#define secretInputPin 6

// Input pins that analog signals are sent to via PWM
#define pwmInputPinLeft 6
#define pwmInputPinRight 7
#define pwmInputAutoState 8

// How many leds in your strip?
#define NUM_LEDS 21

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 7
#define CLOCK_PIN 6
#define MAX_BRIGHTNESS 255


// Define the array of leds
CRGB leds[NUM_LEDS];
CRGB lUpLeds[nLEDsLUpright];
CRGB rUpLeds[nLEDsRUpright];
CRGB armLeds[nLEDsArms];
CRGB underLeds[nLEDsUnder];


void setup() { 
    FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS);
    FastLED.addLeds<WS2801, lUprightDataPin, lUprightClockPin, RGB>(lUpLeds, nLEDsLUpright);
    FastLED.addLeds<WS2801, rUprightDataPin, rUprightClockPin, RGB>(rUpLeds, nLEDsRUpright);
    FastLED.addLeds<WS2801, armDataPin, armClockPin, RGB>(armLeds, nLEDsArms);
    FastLED.addLeds<WS2801, underDataPin, underClockPin, RGB>(underLeds, nLEDsUnder);
    pinMode(modeInputPin1, INPUT);
    digitalWrite(modeInputPin1, HIGH);
    pinMode(modeInputPin2, INPUT);
    digitalWrite(modeInputPin2, HIGH);
    pinMode(allianceInputPin, INPUT);
    digitalWrite(allianceInputPin, HIGH);
    pinMode(secretInputPin, INPUT);
    digitalWrite(secretInputPin, HIGH);
  
    pinMode(pwmInputPinLeft, INPUT);
    pinMode(pwmInputPinRight, INPUT);
    pinMode(pwmInputAutoState, INPUT);
}

void loop() { 
  // Turn the LED on, then pause
//  for(int i =0; i < NUM_LEDS ;i++){
//  leds[i].red = 0;
//  leds[i].blue = 0;
//  leds[i].green = 127--;
//  }
  // Get the PWM inputs from the respective pins
  // a -1.0 PWM from the cRIO has a pulse width of 0.68 ms (680 us)
  // a 1.0 PWM from the cRIO has a pulse width of 2.29 ms (2290 us)
  // Because these functions hold up the program, we limit them to 
  // timeout at 25 ms (25000 us) because the signal is a 50mHz
  // just in case the input doesn't happen, we don't sit around for 3 minutes
  int pwmInputLeft = pulseIn(pwmInputPinLeft, HIGH, 25000);
  int pwmInputRight = pulseIn(pwmInputPinRight, HIGH, 25000);
  int pwmInputAuto = pulseIn(pwmInputAutoState, HIGH, 25000);
  
  float scaledInputLeft = scaleInput(pwmInputLeft);
  float scaledInputRight = scaleInput(pwmInputRight);  
  float scaledInputAuto = scaleInput(pwmInputAuto);
  // Construct the mode variable from the various input pins
  //this is a binary shift. Effectively pin one determines 1 or 0. Then pin 2 is 2 or 0.
  //This gives you 4 modes: 0,1,2,3
  int mode = digitalRead(modeInputPin1)
           | (digitalRead(modeInputPin2) << 1);
  int allianceChoice = digitalRead(allianceInputPin);
  int secret = digitalRead(secretInputPin);
  
  switch(mode) {
    case 0: // BLINK EVERYTHING
      tj(&lUpLeds);
      tj(&rUpLeds);
      tj(&armLeds);
      tj(&underLeds);
      break;
    case 1: // Autonomous   
      // Left blue, right yellow
      //max color is 127 and collor is rgb in code but actually is grb
      chase(0,127,0,30,&armLeds, nLEDsArms);
      //r,g,b,wait,strip,#leds
      if (allianceChoice == 0) {
        //strip, length, r,g,b
       fill(&underGlowStrip, nLEDsUnder, 0,0,127); 
       fill(&lUprightStrip, nLEDsLUpright, 0,0,127);
       fill(&rUprightStrip, nLEDsRUpright, 0,0,127);
      } else if (allianceChoice == 1) {
       fill(&underGlowStrip, nLEDsUnder, 127,0,0); 
       fill(&lUprightStrip, nLEDsLUpright, 127,0,0);
       fill(&rUprightStrip, nLEDsRUpright, 127,0,0);
      }

  
      break;
    case 2: // Drive dependent fill. runs in teleop
      //uint32_t color;
      int r;
      int g;
      int b =0;  
      if (scaledInputLeft < 0) {
        // If the input is negative, set the color accordingly and then make it
        // positive because we can't pass a negative number of LEDs
        scaledInputLeft *= -1;
        r=127;
        g=0;
        //color = Color(127, 0, 0);
      } else {
        // Otherwise, just set it to the forward color
        color = Color(0, 127, 0);
        r=0;
        g=127;
      }
      fill(&lUprightStrip, (int)(scaledInputLeft * nLEDsLUpright), color);
      if (scaledInputRight < 0) {
        scaledInputRight *= -1;
        r=127;
        g=0;
        //color = Color(127, 0, 0);
      } else {
        r= 0;
        g=127;
        //color = Color(0, 127, 0);
      }
      fill(&rUprightStrip, (int)(scaledInputRight * nLEDsRUpright), r, g, b);
      chase(0,127,0,30,&armLeds, nLEDsArms);
      //fill(&armStrip, (nLEDsArm -5) , 4,  Color(0,127,0));
      if (allianceChoice == 0) {
       fill(&underGlowStrip, nLEDsUnder, 0,0,127); 
      } else if (allianceChoice == 1) {
       fill(&underGlowStrip, nLEDsUnder, 127,0,0); 
      }
      break;
    case 3: //ask David
      if (secret == 1) {
          fill(&lUprightStrip, nLEDs, 0,127,0);
          fill(&rUprightStrip, nLEDs, 0,127,0);
          fill(&armStrip, nLEDs, 0,127,0);
          fill(&underGlowStrip, nLEDs, 0,127,0);  
      }
      else if (secret == 0) { 
        if (count <=5) {
          fill(&lUprightStrip, nLEDsLUpright, 64,0,64);
          fill(&rUprightStrip, nLEDsRUpright, 64,0,64);
          fill(&armStrip, nLEDsArms, 64,0,64);
          fill(&underGlowStrip, nLEDsUnder, 64,0,64);        
        } else {
          fill(&lUprightStrip, nLEDsLUpright, 127,127,0);
          fill(&rUprightStrip, nLEDsRUpright, 127,127,0);
          fill(&armStrip, nLEDsArms, 127,127,0);
          fill(&underGlowStrip, nLEDsUnder, 127,127,0); 
          count = 0;
        }
        count++;
      }
      break;
    case 0xF: // No input, robot must be disabled or otherwise not sending input. Why didnt we do default? - David
      // So we just make it all rainbow and stuff
      rainbowCycle(&lUprightStrip, nLEDsLUpright);
      rainbowCycle(&armStrip, nLEDsArms);
      rainbowCycle(&underGlowStrip, mLEDsUnder);
      rainbowCycle(&rUprightStrip, nLEDsRUpright);
      break;
  }
  ledPosition++;
  delay(15);
  FastLED.show();
}

float scaleInput(int input) {
  static int LOWER_INPUT = 650;
  static int UPPER_INPUT = 2350;
  static int MAX_RANGE = UPPER_INPUT - LOWER_INPUT;
  // First, correct the input to be in our expected range, because sometimes
  // it doesn't happen... (like after a PulseIn timeout, input will be 0)
  if (input < LOWER_INPUT) {
    input = LOWER_INPUT;
  } else if (input > UPPER_INPUT) {
    input = UPPER_INPUT;
  }
  // Reduce the input to start at 0 instead of something like 650.
  double output = input - LOWER_INPUT;
  // Normalize the input to be based equally around 0.  (ie, -850 to 850)
  output -= (MAX_RANGE / 2);
  // Scale the input to be on a range of -1 to 1.
  output /= (MAX_RANGE / 2);
  return output;
}

void clearPixels(int ledNum, CRGB strip){
  for (int i = 0; i < ledNum; i++) {
    strip[i] = 0;
    //kendall says -> is basically a . in java so reference to an object
   //can also do FastLED.clear(); but it clears all I think? 
  }
}
//These are the functions we have defined to do chase patterns.  They are actually called inside the  loop() above
//They are meant to demonstrate things such as setting LED colors, controlling brightness
void chase(int r, int g, int b, uint8_t wait, CRGB strip, int numLEDs, int chaseLength){
        clearPixels(numLEDs, strip);
       	//clear() turns all LED strips off
	//FastLED.clear();
	//The brightness ranges from 0-255
	//Sets brightness for all LEDS at once
	FastLED.setBrightness(MAX_BRIGHTNESS);

	for(int led_number = 0; led_number < numLEDs; led_number++){
                //bug with chase lengths > 1
                for (int length = 0; length < chaseLength; length++) {
		  strip[led_number + length].r = red;
                  strip[led_number + length].g = green;
                  strip[led_number + length].b = blue;
                }
		// Show the leds (only one of which is has a color set, from above
		// Show turns actually turns on the LEDs
		FastLED.show();

		// Wait a little bit
		delay(wait);
		// Turn our current led back to black for the next loop around
              for (int length = 0; length < chaseLength; length++) {
                  strip[led_number + length] = CRGB::Black;
		}
	}
	return;
}

//unimplemented have to reimplement logic
//void chaseFade(CRGB *strip, int chaseLength, int fadeLength, int r, int g, int b) {
//  clearPixels(strip);
//  // Note, ledPosition is the last pixel in the chase itself, and therefore
//  // it is also the pixel immediately before the fade
//  // iterate through all the pixels we need for the fade chase
//  for (int j = 0; j < fadeLength; j++) {
//    // Calculate how much we need to fade by
//    float fadeFactor = 1 - ((float)j / fadeLength);
//    // Set the pixel
//    strip->setPixelColor((ledPosition - j) % strip->numPixels(), 
//                        scaleColor(color, fadeFactor));
//  }
//  // The chase itself is nothing fancy, and we can reuse the same chase function.
//  chase(strip, chaseLength, color, false);
//}
//
//// Take a color and a scaling factor (between 0 and 1) and scale the
//// brightness to it, while retaining the color.
//uint32_t scaleColor(uint32_t color, float scale) {
//  int g = (color >> 16) & 0x7f;
//  int r = (color >> 8)  & 0x7f;
//  int b = color         & 0x7f;
//  return Color(r * scale, g * scale, b * scale);
//}


// Sets every pixel to r, g and b to each be either totally on or off.
void tj(CRGB strip, int numLEDs) {
  uint32_t color = 0;
  for (int i = 0; i < numLEDs; i++) {
    int c = random(1, 8);
    // no purple
    if(c==5) { c = 4; }
    // if white, try again (i.e. low frequency of white)
    if(c==7) { c = random(1, 8); }
    //what is this shifting?
    int r = 64  * (c>>2 & 1);
    int g = 64 * (c>>1 & 1);
    int b = 64 * (c & 1);
    //strip[i]::color;
//    strip[i].red = r;
//    strip[i].green = g;
//    strip[i].blue = b;
  }
}

// Fills the strip to a certain length of a solid color
void fill(CRGB strip, int length, int r, int g, int b) {
  clearPixels(strip, length);
  //redundancy check. Taking it out 
//  // Limit the length of the fill to be the length of the strip
//  if (length > strip->numPixels()) {
//    length = strip->numPixels();
//  }
  
  // Set each pixel in the length to be the desired color
  for (int i = 0; i < length; i++) {
    strip[i].r = r;
    strip[i].g = g;
    strip[i].b = b;
  }
}

void rainbowCycle(CRGB strip, int numLEDs) {
  uint16_t i, j;
  j = ledPosition % 384;
  for (i=0; i < numLEDs; i++) {
    // tricky math! we use each pixel as a fraction of the full 384-color wheel
    // (thats the i / strip->numPixels() part)
    // Then add in j which makes the colors go around per pixel
    // the % 384 is to make the wheel cycle around
    strip[i].r= (i, WheelR( ((i * 384 / numLEDs + j) % 384) );
    strip[i].g= (i, WheelG( ((i * 384 / numLEDs + j) % 384) );
    strip[i].b= (i, WheelB( ((i * 384 / numLEDs + j) % 384) );
    
  } 
}

// Input a value 0 to 384 to get a color value.
// The colours are a transition r - g -b - back to r
uint32_t WheelR(uint16_t WheelPos)
{
  byte r;
  switch(WheelPos / 128)
  {
    case 0:
      r = 127 - WheelPos % 128;   // Red down
      break; 
    case 1:
      r = 0;                      // Red off
      break; 
    case 2:
      r = WheelPos % 128;         // Red up
      break; 
  }
  return(r);
}

// Input a value 0 to 384 to get a color value.
// The colours are a transition r - g -b - back to r
uint32_t WheelG(uint16_t WheelPos)
{
  byte g;
  switch(WheelPos / 128)
  {
    case 0:
      g = WheelPos % 128;         // Green up
      break; 
    case 1:
      g = 127 - WheelPos % 128;   // Green down
      break; 
    case 2:
      g = 0;                      // Green off
      break; 
  }
  return(g);
}

// Input a value 0 to 384 to get a color value.
// The colours are a transition r - g -b - back to r
uint32_t WheelB(uint16_t WheelPos)
{
  byte b;
  switch(WheelPos / 128)
  {
    case 0:
      b = 0;                      // Blue off
      break; 
    case 1:
      b = WheelPos % 128;         // Blue up
      break; 
    case 2:
      b = 127 - WheelPos % 128;   // Blue down 
      break; 
  }
  return(b);
}


