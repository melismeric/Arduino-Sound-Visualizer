#include <Adafruit_NeoPixel.h>  //Library to simplify interacting with the LED strip
#ifdef __AVR__
#include <avr/power.h>   //Includes the library for power reduction registers if your chip supports them. 
#endif  

#define LED_PIN   A3  //Pin for the pixel strand. Can be analog or digital.
#define LED_TOTAL 59  //Change this to the number of LEDs in your strand.
#define LED_HALF  LED_TOTAL/2
#define VISUALS   6   //Change this accordingly if you add/remove a visual in the switch-case in Visualize()

#define AUDIO_PIN A0  //Pin for the envelope of the sound detector
#define KNOB_PIN  A1 

Adafruit_NeoPixel strand = Adafruit_NeoPixel(LED_TOTAL, LED_PIN, NEO_GRB + NEO_KHZ800);  //LED strand objetcs
uint16_t gradient = 0; //Used to iterate and loop through each color palette gradually

uint16_t thresholds[] = {1529, 1019, 764, 764, 764, 1274};

uint8_t palette = 0;  //Holds the current color palette.
uint8_t visual = 0;   //Holds the current visual being displayed.
uint8_t volume = 0;   //Holds the volume level read from the sound detector.
uint8_t last = 0;     //Holds the value of volume from the previous loop() pass.

float maxVol = 15;    //Holds the largest volume recorded thus far to proportionally adjust the visual's responsiveness.
float knob = 1023.0;  //Holds the percentage of how twisted the trimpot is. Used for adjusting the max brightness.
float avgBump = 0;    //Holds the "average" volume-change to trigger a "bump."
float avgVol = 0;     //Holds the "average" volume-level to proportionally adjust the visual experience.
float shuffleTime = 0; 


bool shuffle = false;  //Toggles shuffle mode.
bool bump = false;     //Used to pass if there was a "bump" in volume

//For Traffic() visual
int8_t pos[LED_TOTAL] = { -2};    //Stores a population of color "dots" to iterate across the LED strand.
uint8_t rgb[LED_TOTAL][3] = {0};  //Stores each dot's specific RGB values.

//For Snake() visual
bool left = false;  //Determines the direction of iteration. Recycled in PaletteDance()
int8_t dotPos = 0;  //Holds which LED in the strand the dot is positioned at. Recycled in most other visuals.
float timeBump = 0; //Holds the time (in runtime seconds) the last "bump" occurred.
float avgTime = 0; 

void setup() {

  Serial.begin(9600); //Sets data rate for serial data transmission.

  strand.begin(); //Initialize the LED strand object.
  strand.show();  //Show a blank strand, just to get the LED's ready for use.

}

void loop() {
  volume = analogRead(AUDIO_PIN);       //Record the volume level from the sound detector
 if (volume < avgVol / 2.0 || volume < 15) volume = 0;
 else avgVol = (avgVol + volume) / 2.0; //If non-zeo, take an "average" of volumes.

   //If the current volume is larger than the loudest value recorded, overwrite
  if (volume > maxVol) maxVol = volume;

    if (gradient > thresholds[palette]) {
    gradient %= thresholds[palette] + 1;

    //Everytime a palette gets completed is a good time to readjust "maxVol," just in case
    //  the song gets quieter; we also don't want to lose brightness intensity permanently
    //  because of one stray loud sound.
    maxVol = (maxVol + volume) / 2.0;
  }

    //If there is a decent change in volume since the last pass, average it into "avgBump"
  if (volume - last > 10) avgBump = (avgBump + (volume - last)) / 2.0;

  //If there is a notable change in volume, trigger a "bump"
  //  avgbump is lowered just a little for comparing to make the visual slightly more sensitive to a beat.
  bump = (volume - last > avgBump * .9);  

  //If a "bump" is triggered, average the time between bumps
  if (bump) {
    avgTime = (((millis() / 1000.0) - timeBump) + avgTime) / 2.0;
    timeBump = millis() / 1000.0;
  }

  Visualize();   //Calls the appropriate visualization to be displayed with the globals as they are.

  gradient++;    //Increments gradient

  last = volume; //Records current volume for next pass

  delay(30);     //Paces visuals so they aren't too fast to be enjoyable
}

void Visualize() {
  switch (visual) {
    case 0: return Traffic();
    default: return Traffic();
  }
}

uint32_t ColorPalette(float num) {
  switch (palette) {
    case 0: return (num < 0) ? Rainbow(gradient) : Rainbow(num);
    default: return Rainbow(gradient);
  }
}

//TRAFFIC
//Dots racing into each other
void Traffic() {

  //fade() actually creates the trail behind each dot here, so it's important to include.
  fade(0.8);

  //Create a dot to be displayed if a bump is detected.
  if (bump) {

    //This mess simply checks if there is an open position (-2) in the pos[] array.
    int8_t slot = 0;
    for (slot; slot < sizeof(pos); slot++) {
      if (pos[slot] < -1) break;
      else if (slot + 1 >= sizeof(pos)) {
        slot = -3;
        break;
      }
    }

    //If there is an open slot, set it to an initial position on the strand.
    if (slot != -3) {

      //Evens go right, odds go left, so evens start at 0, odds at the largest position.
      pos[slot] = (slot % 2 == 0) ? -1 : strand.numPixels();

      //Give it a color based on the value of "gradient" during its birth.
      uint32_t col = ColorPalette(-1);
      gradient += thresholds[palette] / 24;
      for (int j = 0; j < 3; j++) {
        rgb[slot][j] = split(col, j);
      }
    }
  }

  //Again, if it's silent we want the colors to fade out.
  if (volume > 0) {

    //If there's sound, iterate each dot appropriately along the strand.
    for (int i = 0; i < sizeof(pos); i++) {

      //If a dot is -2, that means it's an open slot for another dot to take over eventually.
      if (pos[i] < -1) continue;

      //As above, evens go right (+1) and odds go left (-1)
      pos[i] += (i % 2) ? -1 : 1;

      //Odds will reach -2 by subtraction, but if an even dot goes beyond the LED strip, it'll be purged.
      if (pos[i] >= strand.numPixels()) pos[i] = -2;

      //Set the dot to its new position and respective color.
      //  I's old position's color will gradually fade out due to fade(), leaving a trail behind it.
      strand.setPixelColor( pos[i], strand.Color(
                              float(rgb[i][0]) * pow(volume / maxVol, 2.0) * knob,
                              float(rgb[i][1]) * pow(volume / maxVol, 2.0) * knob,
                              float(rgb[i][2]) * pow(volume / maxVol, 2.0) * knob)
                          );
    }
  }
  strand.show(); //Again, don't forget to actually show the lights!
}

//Fades lights by multiplying them by a value between 0 and 1 each pass of loop().
void fade(float damper) {

  //"damper" must be between 0 and 1, or else you'll end up brightening the lights or doing nothing.

  for (int i = 0; i < strand.numPixels(); i++) {

    //Retrieve the color at the current position.
    uint32_t col = strand.getPixelColor(i);

    //If it's black, you can't fade that any further.
    if (col == 0) continue;

    float colors[3]; //Array of the three RGB values

    //Multiply each value by "damper"
    for (int j = 0; j < 3; j++) colors[j] = split(col, j) * damper;

    //Set the dampened colors back to their spot.
    strand.setPixelColor(i, strand.Color(colors[0] , colors[1], colors[2]));
  }
}

uint8_t split(uint32_t color, uint8_t i ) {

  //0 = Red, 1 = Green, 2 = Blue

  if (i == 0) return color >> 16;
  if (i == 1) return color >> 8;
  if (i == 2) return color >> 0;
  return -1;
}

uint32_t Rainbow(unsigned int i) {
  if (i > 1529) return Rainbow(i % 1530);
  if (i > 1274) return strand.Color(255, 0, 255 - (i % 255));   //violet -> red
  if (i > 1019) return strand.Color((i % 255), 0, 255);         //blue -> violet
  if (i > 764) return strand.Color(0, 255 - (i % 255), 255);    //aqua -> blue
  if (i > 509) return strand.Color(0, 255, (i % 255));          //green -> aqua
  if (i > 255) return strand.Color(255 - (i % 255), 255, 0);    //yellow -> green
  return strand.Color(255, i, 0);                               //red -> yellow
}
