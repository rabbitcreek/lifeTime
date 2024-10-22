#include <ESP32Servo.h>
#include <FastLED.h>
#include "RTClib.h"
//#define LED_PIN     D9        // Pin where the NeoPixel matrix is connected (D9)
//#define NUM_LEDS    540      // Total number of LEDs in the 6x90 matrix
#define WIDTH       6        // Matrix width
#define HEIGHT      90       // Matrix height
//#define COLOR_ORDER GRB      // Color order for your NeoPixels
//#define CHIPSET     WS2812B  // Adjust to match your specific LED strip/matrix type

//CRGB leds[NUM_LEDS];         // LED array for the matrix
RTC_DS3231 rtc;
int bYear = 1953;
int bMonth = 9;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
Servo myservo;  // create servo object to control a servo
// 16 servo objects can be created on the ESP32

int pos = 0;    // variable to store the servo position
int nRows = 0;
int nMonths = 0;
int servoPin = D7;
#define NUM_LEDS 540
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];
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
float waterIncrement = 0.5;    // Increment for water rise
bool isMatrixEmptying = false;  // Flag to start emptying the matrix

// Function to get LED index from (x, y) coordinates for zigzag layout
int XY(int x, int y) {
  if (y % 2 == 0) { // Even row: left to right
    return (HEIGHT - 1 - y) * WIDTH + x;  // Reverse the row indexing for correct matrix layout
  } else { // Odd row: right to left
    return (HEIGHT - 1 - y) * WIDTH + (WIDTH - 1 - x);  // Reverse both row and column indexing
  }
}

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


void timerCalc(){
  DateTime now = rtc.now();
  int monthTotal = 1080 - (((now.year()- bYear) * 12)+(now.month() - bMonth));
  Serial.print("total months =");
Serial.println(monthTotal);
 nRows = float(monthTotal) / 12;
 nMonths = monthTotal % 12;
Serial.print("rows = ");
Serial.println(nRows);
Serial.print("leftovermonths =");
Serial.println(nMonths);
}

void loop() {
 timerCalc();
 //FastLED.clear();
 //FastLED.show();
 if (waterLevel >0){
 clearMatrix();          // Clear the matrix for each frame

  updateRaindrops();      // Update raindrop positions
  drawWaterLevel();       // Draw the water level
  
  for (int i = 0; i < numDrops; i++) {
    if (raindrops[i].active) {
      drawRaindrop(raindrops[i]);  // Draw active raindrops
    }
  }
 } 
  //resetMatrix();          // Check if water level has reached the bottom and reset
 
 if((90- nRows) >= waterLevel){
  if(nMonths > 6 ){
fill_solid(leds, (nRows * 6) + 6, CRGB::Aquamarine );
fill_solid(leds2, (nRows * 6)+ (nMonths - 6), CRGB::Aquamarine );
} else {fill_solid(leds2, (nRows * 6), CRGB:: Aquamarine);
fill_solid(leds, (nRows * 6) + nMonths, CRGB::Aquamarine );
}
 }
 
  FastLED.show();         // Show the updated matrix
  delay(50);              // Delay to control the speed of the animation

 /*
	for (pos = 0; pos <= 90; pos = pos + 1) { // goes from 0 degrees to 180 degrees
		// in steps of 1 degree
		myservo.write(pos);    // tell servo to go to position in variable 'pos'
    //Serial.println(pos);
		delay(35);             // waits 15ms for the servo to reach the position
	}
 
 
	
 
  
	for (pos = 90; pos >= 0; pos = pos - 1) { // goes from 180 degrees to 0 degrees
		myservo.write(pos); 
    //Serial.println(pos);   // tell servo to go to position in variable 'pos'
		delay(50);             // waits 15ms for the servo to reach the position
	}
  
  //myservo.detach();
  
fill_solid(leds,NUM_LEDS, CRGB::DarkGreen);
fill_solid(leds2,NUM_LEDS, CRGB::DarkGreen);
FastLED.show();
randomArray();
delay(10000);
 
  //for(int i = 0; i<540; i++)Serial.print(colorBrray[i]);
*/
}
void randomArray(){
  for(int i = 0; i < 541; i++) colorBrray[i]= i;
  for (  int i = 0; i <539; i++)
{
    size_t j = random(0, 539 - i);

    int t = colorBrray[i];
    colorBrray[i] =colorBrray[j];
    colorBrray[j] = t;
}
//Serial.print(colorBrray);
for(int i = 0; i < 540; i++){
leds[colorBrray[i]].fadeToBlackBy(250);
//leds[colorBrray[i]] = CRGB::DarkRed;
leds2[colorBrray[i]].fadeToBlackBy(250);
//nMonths = 3;
if(nMonths > 6 ){
fill_solid(leds, (nRows * 6) + 6, CRGB::Aquamarine );
fill_solid(leds2, (nRows * 6)+ (nMonths - 6), CRGB::Aquamarine );
} else {fill_solid(leds2, (nRows * 6), CRGB::Aquamarine);
fill_solid(leds, (nRows * 6) + nMonths, CRGB::Aquamarine );
}

 FastLED.show();
 //fadeall();
 delay(100);

}
delay(5000);
fill_solid(leds,NUM_LEDS, CRGB::Black);
fill_solid(leds2,NUM_LEDS, CRGB::Black);
FastLED.show();
delay(5000);
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
    // Set the water color gradient from darker to lighter green
    //CRGB color = CRGB(0, 0, 0); // Start with black (no color)
    int clar = (90/(y + 1)) + 10;
    CRGB color = CRGB(0, clar, 0); // Dim light green
    
    for (int x = 0; x < WIDTH; x++) {
      leds[XY(x, y)] = color;
      leds2[XY(x, y)] = color;
    }
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