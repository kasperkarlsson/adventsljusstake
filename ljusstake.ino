#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include "wifi_settings.h"
#define NUMBER_OF_PINS 7
#define PIN_NEOPIXEL D6
#define FAVICON_URL String("https://i.imgur.com/10tyXWl.png")

#define SERVER_PORT 80

WiFiServer server(SERVER_PORT);

int ledPin = LED_BUILTIN;
int lightMode = 0;
int modeMemoryInt = 0;
bool modeMemoryBool = false;

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
  // Connect to WiFi
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);
 
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
 
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(ledPin, HIGH);
    delay(250);
    digitalWrite(ledPin, LOW);
    delay(250);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start HTTP server
  server.begin();
  Serial.println("Server started");
 
  // Print IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
}

void set_all(uint32_t color, bool show) {
  // Sets all pixels to 'color'
  for (int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
  }
  if (show) {
    strip.show();
  }
}

void mode_0() {
  // Shifting colors
  for (int j=0; j<num_colors; j++) {
    for (int i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, colors[(i+j) % num_colors]);
    }
    strip.show();
    delay(500);
  }
}

void back_and_forth(uint32_t background, uint32_t foreground, int duration, int iterations) {
  for (int iter=0; iter<iterations; iter++) {
    set_all(background, true);
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

void mode_1() {
  // Single pixel running back and forth
  int duration = 200;
  int col, col2;
  // Avoid repeating the same color twice in a row
  do {
    col = random(num_colors);
  } while (col == modeMemoryInt);
  modeMemoryInt = col;
  // Avoid getting same background and foreground color
  do {
    col2 = random(num_colors);
  } while (col2 == col);
   
  back_and_forth(colors[col], colors[col2], duration, 1);
  strip.setPixelColor(0, colors[col2]);
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

void mode_2() {
  // Wave
  int col;
  // Avoid repeating the same color twice in a row
  do {
    col = random(num_colors);
  } while (col == modeMemoryInt);
  modeMemoryInt = col;
  set_all_wave(colors[col], 200, false);
}

void mode_3() {
  // Wave back and forth
  int col;
  // Avoid repeating the same color twice in a row
  do {
    col = random(num_colors);
  } while (col == modeMemoryInt);
  modeMemoryInt = col;
  set_all_wave(colors[col], 200, modeMemoryBool);
  // Flip direction
  modeMemoryBool = !modeMemoryBool;
}

String responseOkHtmlHeader() {
  String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE html>\r\n";
  return response;
}

void handleHttpRequest(WiFiClient client, String request) {
  Serial.print("Request: '");
  Serial.print(request);
  Serial.println("'");
  String resp = "";
  if (request.startsWith("GET / HTTP/1.1")) {
    // Main page
    // Headers
    resp = responseOkHtmlHeader();
    resp += "<html>\r\n";
    resp += "<head>\r\n";
    // Fancy button CSS
    resp += "<style class='cp-pen-styles'>\r\n";
    resp += "*{text-decoration:none;font-family:sans-serif;}\r\n";
    resp += "a{border-radius:5px;background-color:green;padding:10px;color:white;text-decoration:none;font-family:sans-serif}\r\n";
    resp += "a:hover{background-color:lime;}</style>\r\n";
    resp += "</head>\r\n<body>\r\n";
    
    resp += "<br />\r\n";
    resp += "<a onclick='switchMode()'>Switch mode</a><br /><br />\r\n";

    resp += "<div id='current_mode'>";
    resp += "Current mode: ";
    resp += lightMode;
    resp += "</div>\r\n";
    
    resp += "<script>\r\n";
    resp += "function switchMode() {\r\n";
    resp += "  var xhttp = new XMLHttpRequest();\r\n";
    resp += "  xhttp.onreadystatechange = function() {\r\n";
    resp += "    if (this.readyState == 4) {\r\n";
    resp += "      document.getElementById('current_mode').innerHTML = this.responseText;\r\n";
    resp += "    }\r\n";
    resp += "  };\r\n";
    resp += "  xhttp.open('GET', '/?action=switch');\r\n";
    resp += "  xhttp.send();\r\n";
    resp += "}\r\n";
    resp += "</script>\r\n";
    
    resp += "</body></html>\r\n";
  }
  else if (request.indexOf("/?action=switch") != -1)  {
    lightMode = (lightMode + 1) % 4;
    resp = responseOkHtmlHeader();
    resp += "Current mode: ";
    resp += lightMode;
  }
  else if (request.indexOf("/favicon.ico") != -1) {
    // Redirect to external favicon
    resp = "HTTP/1.1 301 Redirect\r\n";
    resp += "Location: " + FAVICON_URL + "\r\n\r\n";
    Serial.print("Favicon redirect: '");
    Serial.print(resp);
    Serial.println("'");
  }
  else {
    resp = "HTTP/1.1 404 Not found\r\n\r\n";
    resp += "Not found";
  }
  // Send response
  client.print(resp);
  Serial.println("Response sent");
}


void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    // Wait until the client sends some data
    Serial.println("Received request");
    while(!client.available()){
      // TODO timeout
      delay(1);
    }
   
    // Read the first line of the request
    String request = client.readStringUntil('\r');
    client.flush();
   
    // Handle request and update lightMode if needed
    handleHttpRequest(client, request);
    
    // TODO Needed?
    delay(10);
    Serial.println("Client disconnected");
  }
  // Display selected animation mode
  Serial.print("Light mode: ");
  Serial.println(lightMode);
  switch (lightMode) {
    case 0:
      mode_0();
      break;
    case 1:
      mode_1();
      break;
    case 2:
      mode_2();
      break;
    case 3:
      mode_3();
      break;  
  }
}

