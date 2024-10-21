#include <FastLED.h>

#define LED_PIN     D9        // Pin where the NeoPixel matrix is connected (D9)
#define NUM_LEDS    540      // Total number of LEDs in the 6x90 matrix
#define WIDTH       6        // Matrix width
#define HEIGHT      90       // Matrix height
#define COLOR_ORDER GRB      // Color order for your NeoPixels
#define CHIPSET     WS2812B  // Adjust to match your specific LED strip/matrix type

CRGB leds[NUM_LEDS];         // LED array for the matrix

// Raindrop structure to hold position, speed, and acceleration
struct Raindrop {
  float x;           // X position (fractional for smoothness)
  float y;           // Y position (fractional for smoothness)
  float speed;       // Speed of the raindrop
  float acceleration; // Acceleration of the raindrop (to simulate gravity)
  uint8_t fadeStart; // Where the drop starts fading (randomized per drop)
  bool active;       // Whether the raindrop is active
  CRGB color;        // Color of the raindrop
};

// Number of raindrops and array to hold them
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

// Clear the LED matrix
void clearMatrix() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
  }
}

// Initialize a raindrop
void initRaindrop(Raindrop &drop) {
  drop.x = random(0, WIDTH);    // Random starting X position
  drop.y = 0;                   // Start at the top (Y = 0)
  drop.speed = 0.1;             // Initial speed (slow start)
  drop.acceleration = 0.02;     // Acceleration for gravity-like effect
  drop.fadeStart = random(HEIGHT / 5, HEIGHT / 3);  // Fade starts between 1/5 and 1/3 of height
  drop.active = true;           // Set raindrop as active
  drop.color = CRGB::DarkRed;     // Set the raindrop color
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
    } else {
      leds[XY(drop.x, yInt)] = drop.color;  // Normal brightness before fade starts
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
    CRGB color = CRGB(0, 10, 0); // Dim light green
    
    for (int x = 0; x < WIDTH; x++) {
      leds[XY(x, y)] = color;
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

void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.clear();  // Clear the matrix

  // Initialize all raindrops as inactive
  for (int i = 0; i < numDrops; i++) {
    raindrops[i].active = false;
  }
}

void loop() {
  clearMatrix();          // Clear the matrix for each frame

  updateRaindrops();      // Update raindrop positions
  drawWaterLevel();       // Draw the water level
  
  for (int i = 0; i < numDrops; i++) {
    if (raindrops[i].active) {
      drawRaindrop(raindrops[i]);  // Draw active raindrops
    }
  }

  resetMatrix();          // Check if water level has reached the bottom and reset

  FastLED.show();         // Show the updated matrix
  delay(50);              // Delay to control the speed of the animation
}
