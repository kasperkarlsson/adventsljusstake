#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include "wifi_settings.h"
#define NUMBER_OF_PINS 7
#define PIN_NEOPIXEL D6
#define FAVICON_URL String("https://i.imgur.com/10tyXWl.png")
#define NUMBER_OF_MODES 6

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
  modeMemoryInt = (modeMemoryInt + 1) % num_colors;
  for (int i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, colors[(i+modeMemoryInt) % num_colors]);
  }
  strip.show();
  delay(500);
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
  // Step through colors
  modeMemoryInt = (modeMemoryInt + 1) % num_colors;
  set_all_wave(colors[modeMemoryInt], 200, false);
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

void mode_4() {
  // Change a single, random index to random color
  int index;
  // Avoid changing the same index twice in a row
  do {
    index = random(num_colors);
  } while (index == modeMemoryInt);
  modeMemoryInt = index;
  strip.setPixelColor(index, colors[random(num_colors-1)]);
  strip.show();
  delay(750);
}

void mode_5() {
  // Pulsing light
  int col;
  // Avoid repeating the same color twice in a row
  do {
    col = random(num_colors);
  } while (col == modeMemoryInt);
  modeMemoryInt = col;
  for (int brightness=0; brightness<255; brightness++) {
    set_all(colors[col], false);
    strip.setBrightness(brightness);
    strip.show();
    delay(5);
  }
  for (int brightness=255; brightness>0; brightness--) {
    set_all(colors[col], false);
    strip.setBrightness(brightness);
    strip.show();
    delay(5);
  }
}

String constantHeaders() {
  String headers = "\r\n";
  // Clickjacking protection
  headers += "X-Frame-Options: deny\r\n";
  headers += "X-XSS-Protection: 1; mode=block\r\n";
  headers += "Referrer-Policy: no-referrer\r\n";
  return headers;
}

String responseOkHtmlHeader() {
  String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html";
  return response;
}

String getParamValue(String params, String getName) {
  char c;
  String pName = "", pValue = "";
  bool isName = true;
  for (int i=0; i<params.length(); i++) {
    c = params.charAt(i);
    if (c == '&') {
      if (pName == getName) {
        // Found
        return pValue;
      }
      else {
        pName = "";
        pValue = "";
        isName = true;
      }
    }
    else if (isName && c == '=') {
      isName = false;
    }
    else {
      if (isName) {
        pName += c;
      }
      else {
        pValue += c;
      }
    }
  }
  if (pName == getName) {
    // Found
    return pValue;
  }
  return "";
}

void setColor(String color) {
  // Sets all LEDs to color index 'color'
  int col = color.toInt();
  if (0 <= col && col < num_colors) {
    set_all(colors[col], true);
    lightMode = -1;
  }
}

void handleHttpRequest(WiFiClient client, String request) {
  Serial.print("Request: '");
  Serial.print(request);
  Serial.println("'");
  // Parse request method and path
  int spaceIndexFirst = request.indexOf(" ");
  int spaceIndexSecond = request.indexOf(" ", spaceIndexFirst + 1);
  if (spaceIndexFirst == -1 || spaceIndexSecond == -1) {
    Serial.println("Invalid HTTP request: could not parse method and path");
    return;
  }
  String requestMethod = request.substring(0, spaceIndexFirst);
  String requestPath = request.substring(spaceIndexFirst + 1, spaceIndexSecond);
  String requestParams = "";
  int paramIndex = requestPath.indexOf("?");
  if (paramIndex != -1) {
    requestParams = requestPath.substring(paramIndex + 1);
    requestPath = requestPath.substring(0, paramIndex);
  }
  // Handle request
  String headers, body = "";
  if (requestPath.equals("/")) {
    // Main page
    // Headers
    headers = responseOkHtmlHeader();
    body = "<!DOCTYPE html>\r\n<html>\r\n";
    body += "<head>\r\n";
    body += "<script src='lib.js' async></script>\r\n";
    // Fancy button CSS
    body += "<style class='cp-pen-styles'>\r\n";
    body += "*{text-decoration:none;font-family:sans-serif;}\r\n";
    body += "a{border-radius:5px;background-color:green;padding:10px;color:white;width:200px}\r\n";
    body += "a:hover{background-color:lime;}</style>\r\n";
    
    body += "</head>\r\n<body>\r\n";
    body += "<br />\r\n";
    body += "<center>\r\n";
    body += "<div id='viewControl'>\r\n";
    body += "<a onclick='switchView()' style='display:block'>Mode type</a>\r\n";
    body += "</div>\r\n";
    body += "<br />\r\n";
    body += "<div id='modePicker' class='view' style='display:block'>\r\n";
    body += "<a onclick='request(\"/api/switch\")' style='display:block'>Switch mode</a>\r\n";
    body += "</div>\r\n";
    body += "<div id='colorPicker' class='view' style='display:none'>\r\n";
    body += "<a onclick='pickColor()' style='display:block'>Pick color</a>\r\n";
    body += "</div>\r\n";

    body += "<div id='status'>";
    body += "Current mode: ";
    body += lightMode;
    body += "</div>\r\n";
    
    body += "</center></body></html>";
  }
  else if (requestPath.equals("/lib.js")) {
    headers = "HTTP/1.1 200 OK\r\nContent-Type: application/javascript";
    body = "function setStatusMessage(msg) {\r\n";
    body += "  document.getElementById('status').innerHTML = msg;\r\n";
    body += "}\r\n\r\n";
    body += "function request(path) {\r\n";
    body += "  setStatusMessage('Sending...');\r\n";
    body += "  var xhttp = new XMLHttpRequest();\r\n";
    body += "  xhttp.onreadystatechange = function() {\r\n";
    body += "    if (this.readyState == 4) {\r\n";
    body += "       setStatusMessage(this.responseText);\r\n";
    body += "    }\r\n";
    body += "  };\r\n";
    body += "  xhttp.open('GET', path);\r\n";
    body += "  xhttp.send();\r\n";
    body += "}\r\n\r\n";
    body += "function switchView()\r\n";
    body += "{\r\n";
    body += "  views = document.getElementsByClassName('view');\r\n";
    body += "  for (i=0;i<views.length;i++) {\r\n";
    body += "    e = views[i].style;\r\n";
    body += "    if (e.display == 'none')\r\n";
    body += "      e.display = 'block';\r\n";
    body += "    else\r\n";
    body += "      e.display = 'none';\r\n";
    body += "  }\r\n";
    body += "}\r\n";
    body += "function pickColor()\r\n";
    body += "{\r\n";
    // TODO
    body += "  setStatusMessage('TODO: Pick color');\r\n";
    body += "}";
  }
  else if (requestPath.equals("/api/switch"))  {
    lightMode = (lightMode + 1) % NUMBER_OF_MODES;
    // Reset brightness to max, in case previous mode has changed it
    strip.setBrightness(255);
    headers = responseOkHtmlHeader();
    body = "Current mode: ";
    body += lightMode;
  }
  else if (requestPath.equals("/api/setcolor")) {
    headers = responseOkHtmlHeader();
    setColor(getParamValue(requestParams, "color"));
  }
  else if (requestPath.equals("/favicon.ico")) {
    // Redirect to external favicon
    headers = "HTTP/1.1 301 Redirect\r\n";
    headers += "Location: " + FAVICON_URL;
    body = "";
  }
  else {
    headers = "HTTP/1.1 404 Not found";
    body = "Not found";
  }
  // Add constant headers
  headers += constantHeaders();
  // Add Content-Length header
  headers += "Content-Length: " + (String)body.length();
  // Send response headers
  client.print(headers);
  // End headers, begin body
  client.print("\r\n\r\n");
  // Send body
  client.print(body);
  Serial.println("Response sent");
}


void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (client) {
    // Wait until the client sends some data
    Serial.println("Received request");
    long wait_start = millis();
    while (!client.available()) {
      // Stop waiting on timeout
      if ((millis() - wait_start) > 500L) {
        Serial.println("*** Stuck - timeout");
        client.flush();
        return;
      }
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
    case 4:
      mode_4();
      break;
    case 5:
      mode_5();
      break;
    default:
      delay(200);
      break;
  }
}

