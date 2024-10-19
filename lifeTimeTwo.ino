#include <ESP32Servo.h>
#include <FastLED.h>
#include "RTClib.h"

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
#define NUM_LEDS 550
CRGB leds[NUM_LEDS];
CRGB leds2[NUM_LEDS];

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
	// using default min/max of 1000us and 2000us
	// different servos may require different min/max settings
	// for an accurate 0 to 180 sweep
  delay(70);
}
void fadeall() { 
   for(int i = 0; i < NUM_LEDS; i++){ 
    leds[i].nscale8(250); 
    leds2[i].nscale8(250); 
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
 FastLED.clear();
 FastLED.show();
	for (pos = 0; pos <= 90; pos = pos + 1) { // goes from 0 degrees to 180 degrees
		// in steps of 1 degree
		myservo.write(pos);    // tell servo to go to position in variable 'pos'
    //Serial.println(pos);
		delay(35);             // waits 15ms for the servo to reach the position
	}
  //myservo.detach();
  /*
  delay(10000);
  static uint8_t hue = 0;
  for(int i = 0; i < NUM_LEDS; i++) {
		// Set the i'th led to red 
		leds[i] = CHSV(hue++, 255, 255);
    leds2[i] = CHSV(hue++, 255, 255);
		// Show the leds
		FastLED.show(); 
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();
		// Wait a little bit before we loop around and do it again
		delay(10);
	}
	Serial.print("x");

	// Now go in the other direction.  
	for(int i = (NUM_LEDS)-1; i >= 0; i--) {
		// Set the i'th led to red 
		leds[i] = CHSV(hue++, 255, 255);
    leds2[i] = CHSV(hue++, 255, 255);
		// Show the leds
		FastLED.show();
		// now that we've shown the leds, reset the i'th led to black
		// leds[i] = CRGB::Black;
		fadeall();
		// Wait a little bit before we loop around and do it again
		delay(10);
	}
  /*
  //myservo.attach(servoPin, 1000, 4000);
	for (pos = 90; pos >= 0; pos = pos - 1) { // goes from 180 degrees to 0 degrees
		myservo.write(pos); 
    //Serial.println(pos);   // tell servo to go to position in variable 'pos'
		delay(50);             // waits 15ms for the servo to reach the position
	}
  //myservo.detach();
   
  //myservo.detach();
  */
fill_solid(leds,NUM_LEDS, CRGB::DarkGreen);
fill_solid(leds2,NUM_LEDS, CRGB::DarkGreen);
FastLED.show();
randomArray();
delay(10000);
 
  //for(int i = 0; i<540; i++)Serial.print(colorBrray[i]);

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
fill_solid(leds, (nRows * 6) + 6, CRGB::DarkRed );
fill_solid(leds2, (nRows * 6)+ (nMonths - 6), CRGB::DarkRed );
} else {fill_solid(leds2, (nRows * 6), CRGB::DarkRed );
fill_solid(leds, (nRows * 6) + nMonths, CRGB::DarkRed );
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