#include <Adafruit_NeoPixel.h>
#define NUMBER_OF_PINS 10
#define PIN_NEOPIXEL D6


int ledPin = LED_BUILTIN;
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_OF_PINS, PIN_NEOPIXEL, NEO_RGB + NEO_KHZ800);

uint32_t red = strip.Color(150, 0, 0),
         green = strip.Color(0, 150, 0),
         blue = strip.Color(0, 0, 150),
         yellow = strip.Color(150, 150, 0),
         purple = strip.Color(150, 0, 150),
         orange = strip.Color(255, 50, 0),
         white = strip.Color(150, 150, 150),
         teal = strip.Color(0, 150, 150);

uint32_t colors[] = {red, green, blue, yellow, purple, orange, white, teal};
int num_colors = (sizeof(colors)/sizeof(uint32_t));

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.print("Initializing NeoPixel LED strip... ");
  strip.begin();
  strip.show();
  Serial.println("Done!");
}

void set_all(uint32_t color, bool show) {
  // Sets all pixels to 'color'
  Serial.print("set_all - color ");
  Serial.println(color);
  for (int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  if (show) {
    Serial.println("set_all - show");
    strip.show();
  }
}

void mode_1(int iterations) {
  // Shifting colors
  for (int iter=0; iter<iterations; iter++) {
    for (int j=0; j<num_colors; j++) {
      Serial.print("Color offset ");
      Serial.println(j);
      for (int i=0; i<strip.numPixels(); i++) {
        strip.setPixelColor(i, colors[(i+j) % num_colors]);
      }
      strip.show();
      delay(500);
    }
  }
}

void back_and_forth(uint32_t background, uint32_t foreground, int duration, int iterations) {
  for (int iter=0; iter<iterations; iter++) {
    set_all(background, true);
    Serial.println(strip.numPixels());
    for (int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, foreground);
      strip.show();
      delay(duration);
      strip.setPixelColor(i, background);
    }
    // Only flip pixel 0 on the way back during the last iteration,
    // since it will otherwise be the first flipped during next iteration
    int offset;
    if (iter < iterations-1)
      offset = 2;
    else
      offset = 1;
    for (int i=strip.numPixels()-offset; i>0; i--) {
      strip.setPixelColor(i, foreground);
      strip.show();
      delay(duration);
      strip.setPixelColor(i, background);
    }
  }
}

void mode_2(int iterations) {
  // Single pixel running back and forth
  int duration = 200;
  for (int iter=0; iter<iterations; iter++) {
    back_and_forth(purple, white, duration, 3);
    back_and_forth(green, teal, duration, 2);
    back_and_forth(blue, orange, duration, 1);
  }
  Serial.print("mode_2 - white 0");
  strip.setPixelColor(0, white);
  strip.show();
  delay(duration);
}

void set_all_wave(uint32_t color, int delay_per_pixel, bool reverse) {
  for (int i=0; i<strip.numPixels(); i++) {
    if (reverse) {
      strip.setPixelColor(i, color);
    }
    else {
      strip.setPixelColor(strip.numPixels()-i-1, color);
    }
    strip.show();
    delay(delay_per_pixel);
  }
}

void mode_3(int iterations) {
  // Wave
  for (int iter=0; iter<iterations; iter++) {
    Serial.print("mode_3 - iter ");
    Serial.println(iter);
    for (int col; col<=num_colors; col++) {
      set_all_wave(colors[col], 200, false);
    }
  }
}

void mode_4(int iterations) {
  // Wave back and forth
  for (int iter=0; iter<iterations; iter++) {
    Serial.print("mode_4 - iter ");
    Serial.println(iter);
    bool flip = false;
    for (int col; col<=num_colors; col++) {
      flip = !flip;
      set_all_wave(colors[col], 400, flip);
    }
  }
}

void mode_5(int iterations) {
  for (int iter=0; iter<iterations; iter++) {
    
  }
}

void loop() {
  mode_1(5);
  mode_2(3);
  mode_3(2);
  mode_4(2);
}

