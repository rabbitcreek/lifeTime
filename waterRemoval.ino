#include <FastLED.h>

#define LED_PIN     D9        // Pin where the NeoPixel matrix is connected (D9)
#define NUM_LEDS    540      // Total number of LEDs in the 6x90 matrix
#define WIDTH       6        // Matrix width
#define HEIGHT      90       // Matrix height
#define COLOR_ORDER GRB      // Color order for your NeoPixels
#define CHIPSET     WS2812B  // Adjust to match your specific LED strip/matrix type
bool task = 1;
CRGB leds[NUM_LEDS];         // LED array for the matrix

// Structure for falling pixels
struct FallingPixel {
  float x;                   // X position
  float y;                   // Y position (fractional for smooth falling)
  float speed;               // Speed of falling
  bool active;               // Whether the pixel is falling
};

FallingPixel fallingPixels[NUM_LEDS];  // Array for the falling pixels
unsigned long startTime;               // Time when the process starts

// Function to get LED index from (x, y) coordinates for zigzag layout
int XY(int x, int y) {
  if (y % 2 == 0) { // Even row: left to right
    return (HEIGHT - 1 - y) * WIDTH + x;  // Reverse the row indexing for correct matrix layout
  } else { // Odd row: right to left
    return (HEIGHT - 1 - y) * WIDTH + (WIDTH - 1 - x);  // Reverse both row and column indexing
  }
}

// Initialize a falling pixel at the selected (x, y) position
void initFallingPixel(int x, int y) {
  
  int index = XY(x, y);
  fallingPixels[index].x = x;
  fallingPixels[index].y = y;
  fallingPixels[index].speed = 01.0 + (random(0, 10) * 0.5); // Random falling speed
  fallingPixels[index].active = true;
  leds[index] = CRGB::BlanchedAlmond;  // Change the color of the pixel to red
}

// Clear the falling pixels that have hit the bottom
void clearInactivePixels() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (fallingPixels[i].active && fallingPixels[i].y >= 90) {
      fallingPixels[i].active = false;    // Deactivate the pixel
      leds[XY(fallingPixels[i].x, 90)] = CRGB::Black; // Turn off the LED when it reaches the bottom
    }
  }
}

// Update the falling pixels' positions based on their speed
void updateFallingPixels() {
  for (int i = 0; i < NUM_LEDS; i++) {
    if (fallingPixels[i].active) {
      int x = fallingPixels[i].x;
      int y = (int)fallingPixels[i].y; 
      leds[XY(x, y)] = CRGB::Black;  // Draw the red falling pixel
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
        leds[XY(x, y)] = CRGB::Aquamarine;  // Draw the red falling pixel
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
    if(leds[XY(x, y)] == CRGB::Green){
      task = 0;    
    initFallingPixel(x, y);
    }
    boing ++;
   if(boing >= 1000)task = 0;
    }
  
   }
   
    
 


void setup() {
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.clear();    // Clear the matrix
  fill_solid(leds, NUM_LEDS, CRGB::Green);  // Fill the matrix with green color
  FastLED.show();
  startTime = millis();  // Record the start time
}

void loop() {
  //FastLED.clear();  // Clear the matrix for each frame
  task = 1;
  // Check if 30 seconds have passed
  if (millis() - startTime >= 30000) {
    FastLED.clear(); // Clear everything when 30 seconds are up
    FastLED.show();
    delay(1000);
    
  }

  // Start random falling
  startRandomFalling();
  
  // Update positions of falling pixels
  updateFallingPixels();
  
  // Clear pixels that reached the bottom
  clearInactivePixels();
  
  // Draw the falling pixels
  drawFallingPixels();

  FastLED.show();  // Show the updated matrix
  delay(50);       // Control speed of the animation
}
