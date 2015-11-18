/*-------------------------------------------------------------------------
  Spark Core and Photon library to control WS2811/WS2812 based RGB
  LED devices such as Adafruit NeoPixel strips.
  Currently handles 800 KHz and 400kHz bitstream on Spark Core and Photon,
  WS2812, WS2812B and WS2811.

  Also supports:
  - Radio Shack Tri-Color Strip with TM1803 controller 400kHz bitstream.
  - TM1829 pixels

  PLEASE NOTE that the NeoPixels require 5V level inputs
  and the Spark Core and Photon only have 3.3V level outputs.
  Level shifting is necessary, but will require a fast device such as one
  of the following:

  [SN74HCT125N]
  http://www.digikey.com/product-detail/en/SN74HCT125N/296-8386-5-ND/376860

  [SN74HCT245N]
  http://www.digikey.com/product-detail/en/SN74HCT245N/296-1612-5-ND/277258

  Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.
  Modified to work with Spark Core and Photon by Technobly.
  Contributions by PJRC and other members of the open source community.

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing products
  from Adafruit!
  --------------------------------------------------------------------*/

/* ======================= includes ================================= */

#include "application.h"
#include "neopixel.h"
#include "httpclient.h"

SYSTEM_MODE(AUTOMATIC);

/* ======================= prototypes =============================== */

boolean gPowerState = true;
float gHue = 1;
float gSaturation = 1;
float gBrightness = 1;

void colorAll(uint32_t c, uint8_t wait);
void colorWipe(uint32_t c, uint8_t wait);
void rainbow(uint8_t wait);
void rainbowCycle(uint8_t wait);
uint32_t Wheel(byte WheelPos);

/* ======================= extra-examples.cpp ======================== */

// IMPORTANT: Set pixel COUNT, PIN and TYPE
#define PIXEL_COUNT 60
#define PIXEL_PIN D6
#define PIXEL_TYPE WS2812B

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
//               note: if not specified, D2 is selected for you.
// Parameter 3 = pixel type [ WS2812, WS2812B, WS2811, TM1803 ]
//               note: if not specified, WS2812B is selected for you.
//               note: RGB order is automatically applied to WS2811,
//                     WS2812/WS2812B/TM1803 is GRB order.
//
// 800 KHz bitstream 800 KHz bitstream (most NeoPixel products ...
//                         ... WS2812 (6-pin part)/WS2812B (4-pin part) )
//
// 400 KHz bitstream (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//                   (Radio Shack Tri-Color LED Strip - TM1803 driver
//                    NOTE: RS Tri-Color LED's are grouped in sets of 3)

Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIXEL_COUNT, PIXEL_PIN, PIXEL_TYPE);


HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
      { "Accept" , "application/json" },
  //  { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;
unsigned int nextTime = 0;


// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

void setup() {
  Serial.begin(9600);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(50);

  RGB.control(true);

  //updateLED();

  //Exposed internal functions as cloud API
  Spark.function("powerState", powerState);
  Spark.function("hue", hue);
  Spark.function("saturation", saturation);
  Spark.function("brightness", brightness);
}

//void loop() {
//  if (nextTime > millis()) {
//        return;
//  }

  // Get weather request
  /*request.hostname = "http://api.openweathermap.org/";
  request.port = 80;
  request.path = "/data/2.5/weather?q=hatten,de&units=metric";*/

  /*http.get(request, response, headers);
  Serial.println(response.body);*/

  //switch leds with current color
  //colorWipe(strip.Color(0, 255, 0), 50); // Green


  //nextTime = millis() + 10000;
  // Some example procedures showing how to display to the pixels:
  // Do not run more than one of these at a time, or the b/g tasks
  // will be blocked.
  //--------------------------------------------------------------

  //strip.setPixelColor(0, strip.Color(255, 0, 255));
  //strip.show();

  //colorWipe(strip.Color(255, 0, 0), 50); // Red

  //colorWipe(strip.Color(0, 255, 0), 50); // Green

  //colorWipe(strip.Color(0, 0, 255), 50); // Blue

  //rainbow(20);

  //rainbowCycle(20);

  //colorAll(strip.Color(0, 255, 255), 50); // Cyan
//}

/* Cloud methods */

int powerState(String value) {

  Serial.println("powerState function called with" + value);
int result = -1;

    if (value.length() > 0) {
        gPowerState = (value.toInt() == 1);
        result = gPowerState;
    }

    updateLED();

    return result;
}

int hue(String value) {
    Serial.println("hue function called with" + value);

    int result = -1;

    if (value.length() > 0) {
        gHue = (float)value.toInt() / 100.0;
        result = value.toInt();
    }

    updateLED();

    return result;
}

int saturation(String value) {
  Serial.println("saturation function called with" + value);

    int result = -1;

    if (value.length() > 0) {
        gSaturation = (float)value.toInt() / 100.0;
        result = value.toInt();
    }

    updateLED();

    return result;
}

int brightness(String value) {
    Serial.println("brightness function called with" + value);
    int result = -1;

    if (value.length() > 0) {
        gBrightness = (float)value.toInt() / 100.0;
        result = value.toInt();
    }

    updateLED();

    return result;
}

/* LED color manipulation methods */

float hueToRGB(float p, float q, float t) {
    if(t < 0.0) t += 1.0;

    if(t < 1.0/6.0) return p + (q - p) * 6.0 * t;
    if(t < 1.0/2.0) return q;
    if(t < 2.0/3.0) return p + (q - p) * (2.0/3.0 - t) * 6.0;

    return p;
}

void applyHSL(float h, float s, float l) {
    float r = 0;
    float g = 0;
    float b = 0;

    if(s == 0) {
        r = g = b = l;
    } else {
        float q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
        float p = 2.0 * l - q;
        r = hueToRGB(p, q, h + 1.0/3.0);
        g = hueToRGB(p, q, h);
        b = hueToRGB(p, q, h - 1.0/3.0);
    }

    Serial.println("applyHSL function called");

    // switch neopixel
    colorAll(strip.Color((int)(r*255.0), (int)(g*255.0), (int)(b*255.0)), 50);
    strip.setBrightness((int)(l*255.0));

    RGB.color((int)(r*255.0), (int)(g*255.0), (int)(b*255.0));
    RGB.brightness((int)(l*255.0));
}

void updateLED() {
    Serial.println("updateLED function called");

    if (gPowerState) {
        applyHSL(gHue, gSaturation, gBrightness);
    } else {
        applyHSL(0, 0, 0);
    }
}

// Set all pixels in the strip to a solid color, then wait (ms)
void colorAll(uint32_t c, uint8_t wait) {
  uint16_t i;

  for(i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
  }
  strip.show();
  delay(wait);
}

// Fill the dots one after the other with a color, wait (ms) after each one
void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
    delay(wait);
  }
}

void rainbow(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) {
    for(i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel((i+j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Slightly different, this makes the rainbow equally distributed throughout, then wait (ms)
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for(j=0; j<256; j++) { // 1 cycle of all colors on wheel
    for(i=0; i< strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if(WheelPos < 85) {
   return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170;
   return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
