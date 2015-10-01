// 'wavface' example sketch for Adafruit I2C 8x8 LED backpacks
// and Wave Shield:
//
//  www.adafruit.com/products/870   www.adafruit.com/products/1049
//  www.adafruit.com/products/871   www.adafruit.com/products/1050
//  www.adafruit.com/products/872   www.adafruit.com/products/1051
//  www.adafruit.com/products/959   www.adafruit.com/products/1052
//                  www.adafruit.com/products/94
//
// Requires Adafruit_LEDBackpack, Adafruit_GFX libraries and WaveHC
// libraries.
//
// This sketch shows animation roughly synchronized to prerecorded
// speech.  It's fairly complex and may be overwhelming to novice
// programmers, who may want to start with the 'matrix8x8' example
// and then 'roboface' before working through this code.  Also, much
// of the comments relating to the face animation have been stripped
// here for brevity...refer to the 'roboface' sketch if you have any
// questions how that part works.
//
// Additional hardware required: sounds are triggered using three
// normally-open momentary buttons connected to Digital pins 6, 7, 8
// and GND. (e.g. www.adafruit.com/products/1009 )
//
// Adafruit invests time and resources providing this open source code,
// please support Adafruit and open-source hardware by purchasing
// products from Adafruit!
//
// Written by P. Burgess for Adafruit Industries, parts adapted from
// 'PiSpeakHC' sketch included with WaveHC library.
// BSD license, all text above must be included in any redistribution.

#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

// PROGMEM makes frequent appearances throughout this code, reason being that
// the SD card library requires gobs of precious RAM (leaving very little to
// our own sketch).  PROGMEM lets us put fixed data into program flash memory,
// which is considerably more spacious.  String tables are paritcularly nasty.
// See www.arduino.cc/en/Reference/PROGMEM for more info.


// Because the two eye matrices share the same address, only four
// matrix objects are needed for the five displays:
#define MATRIX_EYES         0
#define MATRIX_MOUTH_LEFT   1
#define MATRIX_MOUTH_MIDDLE 2
#define MATRIX_MOUTH_RIGHT  3
int tttalk = 0; //Talk Timer initialization
int tnow = 99; // is 99 no talking
int sayme = 20; // phrase number to say. 99 means say nothing
int btndwn = 0; //will be set with a number that determines how long to send the low signal to the FX board 
Adafruit_8x8matrix matrix[4] = { // Array of Adafruit_8x8matrix objects
  Adafruit_8x8matrix(), Adafruit_8x8matrix(),
  Adafruit_8x8matrix(), Adafruit_8x8matrix() };

// Rather than assigning matrix addresses sequentially in a loop, each
// has a spot in this array.  This makes it easier if you inadvertently
// install one or more matrices in the wrong physical position --
// re-order the addresses in this table and you can still refer to
// matrices by index above, no other code or wiring needs to change.
static const uint8_t PROGMEM matrixAddr[] = { 0x70, 0x71, 0x72, 0x73 };

static const uint8_t PROGMEM // Bitmaps are stored in program memory
  blinkImg[][8] = {    // Eye animation frames
  { B00111100,         // Fully open eye
    B01111110,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
  { B00000000,
    B01111110,
    B11111111,
    B11111111,
    B11111111,
    B11111111,
    B01111110,
    B00111100 },
  { B00000000,
    B00000000,
    B00111100,
    B11111111,
    B11111111,
    B11111111,
    B00111100,
    B00000000 },
  { B00000000,
    B00000000,
    B00000000,
    B00111100,
    B11111111,
    B01111110,
    B00011000,
    B00000000 },
  { B00000000,         // Fully closed eye
    B00000000,
    B00000000,
    B00000000,
    B10000001,
    B01111110,
    B00000000,
    B00000000 } },
  mouthImg[][24] = {                 // Mouth animation frames
  { B00000000, B00000000, B00000000, // Mouth position A (0)
    B01000000, B00000000, B00000010,
    B00100000, B00000000, B00000100,
    B00010000, B00000000, B00001000,
    B00001000, B00000000, B00010000,
    B00000110, B00000000, B01100000,
    B00000011, B11000011, B11000000,
    B00000000, B00111100, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position B (1)
    B00000000, B00000000, B00000000,
    B00111111, B11111111, B11111100,
    B00000111, B00000000, B11100000,
    B00000000, B11111111, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position C (2)
    B00000000, B00000000, B00000000,
    B00111111, B11111111, B11111100,
    B00001000, B00000000, B00010000,
    B00000110, B00000000, B01100000,
    B00000001, B11000011, B10000000,
    B00000000, B00111100, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position D (3)
    B00000000, B00000000, B00000000,
    B00111111, B11111111, B11111100,
    B00100000, B00000000, B00000100,
    B00010000, B00000000, B00001000,
    B00001100, B00000000, B00110000,
    B00000011, B10000001, B11000000,
    B00000000, B01111110, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position E (4)
    B00000000, B00111100, B00000000,
    B00011111, B11000011, B11111000,
    B00000011, B10000001, B11000000,
    B00000000, B01111110, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00111100, B00000000, // Mouth position F (5)
    B00000000, B11000011, B00000000,
    B00001111, B00000000, B11110000,
    B00000001, B00000000, B10000000,
    B00000000, B11000011, B00000000,
    B00000000, B00111100, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 },
  { B00000000, B00000000, B00000000, // Mouth position G (6)
    B00000000, B00000000, B00000000,
    B01111111, B11111111, B11111110,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000,
    B00000000, B00000000, B00000000 }};

// Animation sequences corresponding to each WAV.  First number in
// each pair is a mouth bitmap index.  Second number is the hold
// time (in frames).  255 marks end of list.
// There is no 'magic' here, the software is NOT deriving mouth
// position from the sound...the tables were determined by hand,
// just as animators do it.  Further explanation here:
// http://www.idleworm.com/how/anm/03t/talk1.shtml

static const uint8_t PROGMEM
  seq1[]  = { 5, 5,   1, 1,   2, 2,   3, 3, // "Well Hello"
              2, 2,   3, 3,   3, 3,   5, 5,
              5, 5,  255 },
  seq2[]  = { 3, 1,   3, 5,   0, 5,   1, 2, // "Happy Halloween"
              1, 2,   3, 2,   3, 4,   5, 3,
              2, 2,   255 },
  seq3[]  = { 5, 1,   3, 2,   3, 6,   5, 5, // "What are you supposed to be ?"
              5, 1,   1, 4,   0, 2,   5, 5,
              0, 6,   2, 4,    255 },
  seq4[]  = { 5, 5,   1, 1,   2, 2,   3, 3, // "Don't touch that"
              2, 2,    3, 3,   3, 3,   5, 5,
              5, 5,    255 },
  seq5[]  = { 5, 5,   4, 4,   5, 5,   4, 4, // "hoo hoo hoohoo ha"
              5, 5,   4, 4,   5, 5,   4, 4,
              3, 3,   3, 3,   3, 3,   2, 2, 
              3, 3,   4, 4,   3, 3,   2, 2, 
              3, 3,   4, 4,   3, 3,  3, 3,
              3, 3,   3, 3,   255 },
  seq6[]  = { 3, 3,   3, 3,   5, 5,   5, 5, // "OW"
              255 },
  seq7[]  = { 5, 5,   1, 1,   1, 1,   3, 3, // "Well aren't you cute"
              4, 4,   3, 3,   4, 4,   3, 3,
              4, 4,   5, 5,   3, 3,   4, 4, 
              5, 5,  255 },
 seq8[]  = {  1, 1,   2, 2,   3, 3,   5, 5, // "Did you see the witch ?"
              5, 5,   4, 4,   2, 2,   3, 3, 
              2, 2,   3, 3,   2, 2,   3, 3,
              5, 5,   4, 4,   3, 3,   2, 2,    255 },
 seq9[]  = {  4, 4,   4, 4,   3, 3,   3, 3, // "She was scary"
              3, 3,   5, 5,   3, 3,   3, 3,
              2, 2,   3, 3,   4, 4,   5, 5, 
              4, 4,   3, 3,   2, 2,   4, 4, 
              3, 3,   2, 2,   4, 4,   3, 3, 
              2, 2,   255 };
const uint8_t* const anim[] PROGMEM = { seq1, seq2, seq3, seq4, seq5, seq6, seq7, seq8, seq9 };

uint8_t
const  blinkIndex[] PROGMEM = { 1, 2, 3, 4, 3, 2, 1 }; // Blink bitmap sequence
uint8_t
  blinkCountdown = 100, // Countdown to next blink (in frames)
  gazeCountdown  =  75, // Countdown to next eye movement
  gazeFrames     =  50, // Duration of eye movement (smaller = faster)
  mouthPos       =   0, // Current image number for mouth
  mouthCountdown =  10, // Countdown to next mouth change
  newPos         = 255; // New mouth position for current frame
const uint8_t*  seq;                 // Animation sequence currently being played back
uint8_t
  idx,                  // Current array index within animation sequence
  prevBtn        = 99,  // Button # pressed on last loop() iteration
  btnCount       = 0,   // Number of iterations same button has been held

  eyeX = 3, eyeY = 3,   // Current eye position
  newX = 3, newY = 3,   // Next eye position
  dX   = 0, dY   = 0;   // Distance from prior to new position

void setup() {

  // Seed random number generator from an unused analog input:
  randomSeed(analogRead(A0));

  // Initialize each matrix object:
  for(uint8_t i=0; i<4; i++) {
    matrix[i].begin(pgm_read_byte(&matrixAddr[i]));
    // If using 'small' (1.2") displays vs. 'mini' (0.8"), enable this:
    // matrix[i].setRotation(3);
  }

  // Enable pull-up resistors on three button inputs.
  // Other end of each button then connects to GND.
  for(uint8_t i=6; i<=8; i++) {
    pinMode(i, INPUT);
    digitalWrite(i, HIGH); // Enable pullup
  }
  pinMode(4,OUTPUT); //set up voice trigger pin
  digitalWrite(4,HIGH); // set trigger pin high so no sound comes out of the FX board
  pinMode(10,OUTPUT); //reset FX board
  digitalWrite(4,HIGH); // set FX reset to high
  pinMode(11,INPUT); //set pin to detect sensor
 
}

void loop() {
//**************************** Not Talking *******************
while (sayme > 9){
  uint8_t i;

  // Draw eyeball in current state of blinkyness (no pupil).
  matrix[MATRIX_EYES].clear();
  matrix[MATRIX_EYES].setRotation(3);
  matrix[MATRIX_EYES].drawBitmap(0, 0,
    blinkImg[
      (blinkCountdown < sizeof(blinkIndex)) ?      // Currently blinking?
      pgm_read_byte(&blinkIndex[blinkCountdown]) : // Yes, look up bitmap #
      0                                            // No, show bitmap 0
    ], 8, 8, LED_ON);
  // Decrement blink counter.  At end, set random time for next blink.
  if(--blinkCountdown == 0) blinkCountdown = random(5, 180);

  if(--gazeCountdown <= gazeFrames) {
    // Eyes are in motion - draw pupil at interim position
    matrix[MATRIX_EYES].fillRect(
      newX - (dX * gazeCountdown / gazeFrames),
      newY - (dY * gazeCountdown / gazeFrames),
      2, 2, LED_OFF);
    if(gazeCountdown == 0) {    // Last frame?
      eyeX = newX; eyeY = newY; // Yes.  What's new is old, then...
      do { // Pick random positions until one is within the eye circle
        newX = random(7); newY = random(7);
        dX   = newX - 3;  dY   = newY - 3;
      } while((dX * dX + dY * dY) >= 10);      // Thank you Pythagoras
      dX            = newX - eyeX;             // Horizontal distance to move
      dY            = newY - eyeY;             // Vertical distance to move
      gazeFrames    = random(3, 15);           // Duration of eye movement
      gazeCountdown = random(gazeFrames, 120); // Count to end of next movement
    }
  } else {
    // Not in motion yet -- draw pupil at current static position
    matrix[MATRIX_EYES].fillRect(eyeX, eyeY, 2, 2, LED_OFF);
  }

  mouthPos = 6; // Mouth not in motion -- set to neutral position

  drawMouth(mouthImg[mouthPos]);

  // Refresh all matrices in one quick pass
  for(uint8_t i=0; i<4; i++) matrix[i].writeDisplay();

  delay(20); // sets animation at approx 50 FPS
  if (digitalRead(11) == HIGH){sayme = 0;digitalWrite(10, LOW); delay(20); digitalWrite(10, HIGH);}
}
// ************************** talking *********************************
while (sayme < 10){
   uint8_t i;

  // Draw eyeball in current state of blinkyness (no pupil).
  matrix[MATRIX_EYES].clear();
  matrix[MATRIX_EYES].setRotation(3);
  matrix[MATRIX_EYES].drawBitmap(0, 0,
    blinkImg[
      (blinkCountdown < sizeof(blinkIndex)) ?      // Currently blinking?
      pgm_read_byte(&blinkIndex[blinkCountdown]) : // Yes, look up bitmap #
      0                                            // No, show bitmap 0
    ], 8, 8, LED_ON);
  // Decrement blink counter.  At end, set random time for next blink.
  if(--blinkCountdown == 0) blinkCountdown = random(5, 180);

  if(--gazeCountdown <= gazeFrames) {
    // Eyes are in motion - draw pupil at interim position
    matrix[MATRIX_EYES].fillRect(
      newX - (dX * gazeCountdown / gazeFrames),
      newY - (dY * gazeCountdown / gazeFrames),
      2, 2, LED_OFF);
    if(gazeCountdown == 0) {    // Last frame?
      eyeX = newX; eyeY = newY; // Yes.  What's new is old, then...
      do { // Pick random positions until one is within the eye circle
        newX = random(7); newY = random(7);
        dX   = newX - 3;  dY   = newY - 3;
      } while((dX * dX + dY * dY) >= 10);      // Thank you Pythagoras
      dX            = newX - eyeX;             // Horizontal distance to move
      dY            = newY - eyeY;             // Vertical distance to move
      gazeFrames    = random(3, 15);           // Duration of eye movement
      gazeCountdown = random(gazeFrames, 120); // Count to end of next movement
    }
  } else {
    // Not in motion yet -- draw pupil at current static position
    matrix[MATRIX_EYES].fillRect(eyeX, eyeY, 2, 2, LED_OFF);
  }

  // The old code let you have buttons to decide what the pumpkin would say.
  // I left this code in because I don't really understand how the mouth gets animated.
i=tnow; // tnow holds either a 99 (no talk) or a number corresponding to the phrase that will be said.
  if(i < 10) {               // if it's time to talk is the phrase one of 9 that are loaded in the FX board
    if(i == prevBtn) {      // Same as last time we checked?  Good! 
      if(++btnCount == 3) { // 3 passes to 'debounce' button input
        
        // Look up animation sequence # corresponding to the phrase number on the FX board.
        seq            = (uint8_t *)pgm_read_word(&anim[i]); // i hold the animation sequence number (passed to it from "sayme")
        idx            = 0; // Begin at first byte of data
        newPos         = pgm_read_byte(&seq[idx++]); // Initial mouth pos
        mouthCountdown = pgm_read_byte(&seq[idx++]); // Hold time for pos
      }
    } else btnCount = 0; // Different button than before - start count over
    prevBtn = i;
  } else prevBtn = 99;   // No buttons pressed
// A pair of numbers A,B A= the address that contains the leds to light up in the segment. B= how long to stay on in frame numbers
// So set the proper leds to on (read all 1 and zeros intil 255 is reached.and turn segment on)using the "A" number
// then hold that led set on for "B" frames
// if we have not reached the end of the current mouth animation.
// if the mouth count down is not equal to zero.

  if(newPos != 255) { // Is the mouth in motion?
    if(--mouthCountdown == 0) { // Count down frames to next position
      newPos = pgm_read_byte(&seq[idx++]); // New mouth position
      if(newPos == 255) { // End of list?
        mouthPos = 0;     // Yes, set mouth to neutral position
        tnow = 99;
      } else {
        mouthPos       = newPos; // Set mouth to new position
        mouthCountdown = pgm_read_byte(&seq[idx++]); // Read hold time
      }
    }
  } else mouthPos = 0; // Mouth not in motion -- set to neutral position

  drawMouth(mouthImg[mouthPos]);

  // Refresh all matrices in one quick pass
  for(uint8_t i=0; i<4; i++) matrix[i].writeDisplay();

  delay(20); // sets animation at approx 50 FPS
 // tttalk controls the time between talking. At 200 it talks about every 7 seconds 500 equals about 15 seconds.
 // The FX board allows us to save a number of phrases in its static memory. We have 9 phrases.
 // The FX board says a different phrase each time a pin is pulled low (based on the file name see adafruit.com for more info on the FX board).
 // There is a maximum of 10 phrases per pin. There are 10 pins. We are only using 1 pin  so we can have 10 phrases. We have 9 phrases.
 //sayme holds the number of the phrase that the pumpkin will say. This is for use with the mouth animation.
 //When the time to talk (tttalk) time out its time to talk. We reset tttalk to zero to reset the time it wil talk again.
 // We set the FX board pin to low to tell it to say the next phrase. btndwn controls how long to hold the FX boards pin low. We reset btndwn to 20.
 // We check to see if we need to reset "sayme" to the first phrase. We place the contents to sayme into tnow in order to pass the correct phrase number to the mouth animation.
 // We increment sayme to make sure the pumpkin "mouths" the correct phrase the next time around.
  if (tttalk > 200) {tttalk =0; digitalWrite(4,LOW); btndwn = 20; if (sayme >= 9){sayme=20;}tnow = sayme;sayme++;} // pull pin 4 low to make Pumpkin talk 
  else {tttalk++;} // If its not time to talk then increment tttalk.
  if (sayme == 21) {btndwn = 0;}
  if (btndwn > 0){digitalWrite(4,LOW); --btndwn;} else {digitalWrite(4,HIGH);btndwn = 0;} // This controls how long we hold the FX card pin low.
}
}

// Draw mouth image across three adjacent displays
void drawMouth(const uint8_t *img) {
  for(uint8_t i=0; i<3; i++) {
    matrix[MATRIX_MOUTH_LEFT + i].clear();
    matrix[MATRIX_MOUTH_LEFT + i].drawBitmap(i * -8, 0, img, 24, 8, LED_ON);
  }
}



