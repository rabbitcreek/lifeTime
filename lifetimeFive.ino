#include <ESP32Servo.h>
#include <FastLED.h>
#include "RTClib.h"     // Total number of LEDs in the 6x90 matrix
#define WIDTH       6        // Matrix width
#define HEIGHT      90       // Matrix height
bool task = 1;
RTC_DS3231 rtc;
int bYear = 1953;
int bMonth = 9;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32
bool endcycle = 0;
int pos = 0;    // variable to store the servo position
int nRows = 0;
int nMonths = 0;
int servoPin = D7;
#define NUM_LEDS 540
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];
struct FallingPixel {
  float x;                   // X position
  float y;                   // Y position (fractional for smooth falling)
  float speed;               // Speed of falling
  bool active;               // Whether the pixel is falling
};
FallingPixel fallingPixels[NUM_LEDS];  // Array for the falling pixels
unsigned long startTime;               // Time when the process starts
struct Raindrop {
  float x;           // X position (fractional for smoothness)
  float y;           // Y position (fractional for smoothness)
  float speed;       // Speed of the raindrop
  float acceleration; // Acceleration of the raindrop (to simulate gravity)
  uint8_t fadeStart; // Where the drop starts fading (randomized per drop)
  bool active;       // Whether the raindrop is active
  CRGB color;        // Color of the raindrop
};
const int numDrops = 20;     // Number of raindrops on screen
Raindrop raindrops[numDrops]; // Raindrop array

// Water level
float waterLevel = 90;          // Start with no water (at the bottom)
float waterIncrement = 0.75;    // Increment for water rise
bool isMatrixEmptying = false;  // Flag to start emptying the matrix

// Function to get LED index from (x, y) coordinates for zigzag layout
int XY(int x, int y) {
  if (y % 2 == 0) { // Even row: left to right
    return (HEIGHT - 1 - y) * WIDTH + x;  // Reverse the row indexing for correct matrix layout
  } else { // Odd row: right to left
    return (HEIGHT - 1 - y) * WIDTH + (WIDTH - 1 - x);  // Reverse both row and column indexing
  }
}
uint8_t timeHour;
#define DATA_PIN2 D8
#define DATA_PIN D9
int colorBrray[550];
CRGB currentColor(250,25,25);
void setup() {
  Serial.begin();
if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
//rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  FastLED.addLeds<WS2812,DATA_PIN,GRB>(leds2,NUM_LEDS);
  FastLED.addLeds<WS2812,DATA_PIN2,GRB>(leds,NUM_LEDS);
	FastLED.setBrightness(30);
	// Allow allocation of all timers
	ESP32PWM::allocateTimer(0);
	ESP32PWM::allocateTimer(1);
	ESP32PWM::allocateTimer(2);
	ESP32PWM::allocateTimer(3);
	myservo.setPeriodHertz(50);    // standard 50 hz servo
	myservo.attach(servoPin, 500, 2500); // attaches the servo on pin 18 to the servo object
  FastLED.clear();  // Clear the matrix
   
  // Initialize all raindrops as inactive
  for (int i = 0; i < numDrops; i++) {
    raindrops[i].active = false;
  }
}
CRGBPalette16 nightPalette = CRGBPalette16(CRGB::DarkBlue, CRGB::Navy, CRGB::MidnightBlue, CRGB::DarkOliveGreen);
CRGBPalette16 morningPalette = CRGBPalette16(CRGB::Yellow, CRGB::LightYellow, CRGB::Red, CRGB::Orange);
CRGBPalette16 dayPalette = CRGBPalette16(CRGB::Red, CRGB::Yellow, CRGB::LightSkyBlue, CRGB::AntiqueWhite);
CRGBPalette16 eveningPalette = CRGBPalette16(CRGB::Red, CRGB::OrangeRed, CRGB::LightSalmon, CRGB::DarkOrange);
CRGBPalette16 duskPalette = CRGBPalette16(CRGB::Violet, CRGB::DarkOrange, CRGB::Purple, CRGB::Indigo);


void timerCalc(){
  DateTime now = rtc.now();
  int monthTotal = 1080 - (((now.year()- bYear) * 12)+(now.month() - bMonth));
  //Serial.print("total months =");
//Serial.println(monthTotal);
 nRows = float(monthTotal) / 12;
 nMonths = monthTotal % 12;
timeHour = now.hour();
}

void loop() {
  EVERY_N_MINUTES(5){
    openServo();
    delay(5000);
    endcycle = 0;
    resetMatrix();
  }
 timerCalc();
 if (waterLevel >0){
  clearMatrix();          // Clear the matrix for each frame
  updateRaindrops();      // Update raindrop positions
  drawWaterLevel();       // Draw the water level
  
  for (int i = 0; i < numDrops; i++) {
    if (raindrops[i].active) {
      drawRaindrop(raindrops[i]);  // Draw active raindrops
    }
  }
  startTime = millis();
 }  
 if(waterLevel <=0){
  if(millis() - startTime < 25000)removalNow();
  else if(leds[0] != CRGB(0,0,0)) {
     EVERY_N_MILLISECONDS(100) {
    // Try different fade values here and note how it changes the look.
    fadeToBlackBy(leds, NUM_LEDS, 10);  // 10/255 = 4%
    fadeToBlackBy(leds2, NUM_LEDS, 10);  // 10/255 = 4%    
  }
  }
}
 //if(waterLevel <=0 && leds[0] == CRGB(0,0,0))resetMatrix();
 if(waterLevel <=0 && leds[0] == CRGB(0,0,0) ){
  if(!endcycle){
     clearMatrix();
     FastLED.show();
  closeServo(); 
  delay(1000);
  }
 displayFlutteringBands(timeHour);
  endcycle = 1;
 }
  FastLED.show();         // Show the updated matrix
  delay(50);              // Delay to control the speed of the animation

 
}

void clearMatrix() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    leds2[i] = CRGB::Black;
  }
}
void initRaindrop(Raindrop &drop) {
  drop.x = random(0, WIDTH);    // Random starting X position
  drop.y = 0;                   // Start at the top (Y = 0)
  drop.speed = 0.1;             // Initial speed (slow start)
  drop.acceleration = 0.02;     // Acceleration for gravity-like effect
  drop.fadeStart = random(HEIGHT / 5, HEIGHT / 3);  // Fade starts between 1/5 and 1/3 of height
  drop.active = true;           // Set raindrop as active
  drop.color = CRGB::Aquamarine;     // Set the raindrop color
}

// Draw a 1-pixel raindrop
void drawRaindrop(Raindrop &drop) {
  int yInt = (int)drop.y;  // Get integer Y position

  if (yInt < HEIGHT) {  // Ensure it stays within bounds
    // Adjust brightness based on how close the drop is to its fade point
    if (yInt > drop.fadeStart) {
      uint8_t brightness = map(yInt, drop.fadeStart, HEIGHT / 2, 255, 0); // Fast fade to black
      leds[XY(drop.x, yInt)] = drop.color;
      leds[XY(drop.x, yInt)].fadeToBlackBy(255 - brightness);
      leds2[XY(drop.x, yInt)] = drop.color;
      leds2[XY(drop.x, yInt)].fadeToBlackBy(255 - brightness);
    } else {
      leds[XY(drop.x, yInt)] = drop.color;  // Normal brightness before fade starts
      leds2[XY(drop.x, yInt)] = drop.color;  // Normal brightness before fade starts
    }
  }
}

// Update raindrops: move them down based on their speed and acceleration
void updateRaindrops() {
  for (int i = 0; i < numDrops; i++) {
    if (raindrops[i].active) {
      raindrops[i].speed += raindrops[i].acceleration;  // Increase speed due to "gravity"
      raindrops[i].y += raindrops[i].speed;  // Move the raindrop down
      if (raindrops[i].y >= HEIGHT) {  // If it reaches the bottom
        raindrops[i].active = false;  // Mark it as inactive
        waterLevel -= waterIncrement;  // Increase the water level for each raindrop
      }
    } else {
      if (random(0, 100) < 10) {  // Random chance to create a new raindrop (more frequent)
        initRaindrop(raindrops[i]);
      }
    }
  }
}

// Draw the rising water level
void drawWaterLevel() {
  for (int y = 90; y > (int)waterLevel; y--) {
    CRGB color = CRGB(0, (y*2) +10, 0); // Dim light green
    
    for (int x = 0; x < WIDTH; x++) {
      leds[XY(x, y)] = color;
      leds2[XY(x, y)] = color;
    }
  }
   for (int x = 0; x < WIDTH; x++) {
      leds[XY(x, 0)] = CRGB::Black;
      leds2[XY(x, 0)] = CRGB::Black;
    }
}

// Reset the matrix when water level reaches the bottom
void resetMatrix() {
  if (waterLevel <= 0) {
    waterLevel = HEIGHT;          // Reset water level
    clearMatrix();           // Clear the matrix
    FastLED.show();          // Show cleared matrix
    delay(1000);             // Delay to allow for reset
  }
}
void initFallingPixel(int x, int y) {
  
  int index = XY(x, y);
  fallingPixels[index].x = x;
  fallingPixels[index].y = y;
  fallingPixels[index].speed = 01.0 + (random(0, 10) * 0.5); // Random falling speed
  fallingPixels[index].active = true;
  leds[index] = CRGB(20,0,0);
  leds2[index] = CRGB(20,0,0);// Change the color of the pixel to red
}
// Clear the falling pixels that have hit the bottom
void clearInactivePixels() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (fallingPixels[i].active && fallingPixels[i].y >= 90) {
      fallingPixels[i].active = false;    // Deactivate the pixel
      leds[XY(fallingPixels[i].x, 90)] = CRGB::Black;
       leds2[XY(fallingPixels[i].x, 90)] = CRGB::Black;// Turn off the LED when it reaches the bottom
    }
  }
}

// Update the falling pixels' positions based on their speed
void updateFallingPixels() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (fallingPixels[i].active) {
      int x = fallingPixels[i].x;
      int y = (int)fallingPixels[i].y; 
      leds[XY(x, y)] = CRGB::Black; 
      leds2[XY(x, y)] = CRGB::Black;  // Draw the red falling pixel
      fallingPixels[i].y += fallingPixels[i].speed;  // Update the Y position (fall down)

      if (fallingPixels[i].y > 90) {
        fallingPixels[i].y = 90;  // Clamp to the bottom of the matrix
      }
    }
  }
}

// Draw the falling pixels on the matrix
void drawFallingPixels() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (fallingPixels[i].active) {
      int x = fallingPixels[i].x;
      int y = (int)fallingPixels[i].y;  // Integer Y position

      if (y >= 0 && y < HEIGHT) {
        leds[XY(x, y)] = CRGB(20,0,0);
        leds2[XY(x, y)] = CRGB(20,0,0);  // Draw the red falling pixel
      }
    }
  }
}

// Function to start the random falling process
void startRandomFalling() {
  
   int boing=0;
   while(task) {
    int x = random(0, WIDTH);    // Random X position
    int y = random(0, HEIGHT); // Start from the top row
    if(leds[XY(x, y)] != CRGB(0,0,0)){
      task = 0;    
    initFallingPixel(x, y);
    }
    boing ++;
   if(boing >= 1000)task = 0;
    }
  
   }
void removalNow(){
  //fill_solid(leds, NUM_LEDS, CRGB::Green);  // Fill the matrix with green color
  FastLED.show(); 
task = 1;
  // Check if 30 seconds have passed
  // Start random falling
  startRandomFalling();
  
  // Update positions of falling pixels
  updateFallingPixels();
  
  // Clear pixels that reached the bottom
  clearInactivePixels();
  
  // Draw the falling pixels
  
   if(nMonths > 6 ){
fill_solid(leds, (nRows * 6) + 6, CRGB::DarkViolet );
fill_solid(leds2, (nRows * 6)+ (nMonths - 6), CRGB::DarkViolet );
} else {fill_solid(leds2, (nRows * 6), CRGB:: DarkViolet);
fill_solid(leds, (nRows * 6) + nMonths, CRGB::DarkViolet );
}
for(int i = 0; i <nRows *6; i++){
  leds[i] = CRGB(0, 0, 50 + i);
  leds2[i]= CRGB(0,0, 50 + i );
}
drawFallingPixels();
  FastLED.show();  // Show the updated matrix
  delay(50);       // Co
}
void openServo(){
  for (pos = 0; pos <= 90; pos = pos + 1) { // goes from 0 degrees to 180 degrees
		// in steps of 1 degree
		myservo.write(pos);    // tell servo to go to position in variable 'pos'
    //Serial.println(pos);
     delay(50);
  }
 
} 
void closeServo(){
  for (pos = 90; pos >= 0; pos = pos - 1) { // goes from 180 degrees to 0 degrees
		myservo.write(pos); 
    //Serial.println(pos);   // tell servo to go to position in variable 'pos'
		delay(50);             // waits 15ms for the servo to reach the position
 
}  
}
CRGBPalette16 getPaletteForHour(uint8_t hour) {
  if (hour >= 0 && hour <= 5) {
    return nightPalette;
  } else if (hour >= 6 && hour <= 11) {
    return morningPalette;
  } else if (hour >= 12 && hour <= 17) {
    return dayPalette;
  } else if (hour >= 18 && hour <= 20) {
    return eveningPalette;
  } else {
    return duskPalette;
  }
}
void displayFlutteringBands(uint8_t hour) {
  while(endcycle){
  static uint8_t shift = 0;
  static bool directionUp = true; // Determines whether bands move up or down
  static uint8_t flutterVariance = 0; // Variance for random speed

  CRGBPalette16 currentPalette = getPaletteForHour(hour); // Get palette based on timeHour

  // Draw color bands across the matrix
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      uint8_t colorIndex = (x * 16 / WIDTH) + shift; // Generate bands based on X position
      leds[XY(x, y)] = ColorFromPalette(currentPalette, colorIndex, 40);
      leds2[XY(x, y)] = ColorFromPalette(currentPalette, colorIndex, 40);  // 128 for half brightness
    }
  }

  FastLED.show();

  // Adjust shift direction and random variance
  if (directionUp) {
    shift += (1 + random(3)); // Move up with random speed variance
  } else {
    shift -= (1 + random(3)); // Move down with random speed variance
  }

  // Randomly switch direction for fluttering effect
  if (random(10) > 7) {
    directionUp = !directionUp;
  }

  // Add random delay to create fluttering timing
  flutterVariance = random(50, 150); // Random delay between 50 and 150 milliseconds
  delay(flutterVariance);
  }
}

