## Description
This is an Internet of Things "adventsljusstake" - a traditional Swedish Christmas ornament - which can be connected to your WiFi. It is remotely controlled through a web browser on your computer or smartphone.

You can switch between multiple different modes, which animate the LEDs in various patterns. This brings that ~~horrible~~ lovely kitsch feeling to your window during December, so that people walking by outside can't miss the fact that Christmas is coming!

## Hardware
My implementation is based on a cheap, classical Swedish candelabrum of model "Adam", which is sold at [Clas Ohlson](https://www.clasohlson.com/se/Adventsljusstake-Adam/34-731) for 30 SEK (less than $4). Beware though - during November and December they usually increase the price to 60 SEK ~~since so many people want to build their own IoT adventsljusstake for Christmas~~.

The brain of this project is a [NodeMCU](https://en.wikipedia.org/wiki/NodeMCU) chip, but basically any Arduino-compatible ESP8266 chip should work just fine. These can typically be bought at e.g. [AliExpress](https://www.aliexpress.com/w/wholesale-nodemcu-v3-esp8266.html?CatId=400103) for less than $3.

Finally, you will ned some LEDs of model WS2811. Each LED has a built-in chip which allows them to be serially connected along a data bus. You can probably find them far cheaper from China/eBay/whatever, but I bought a 25-pack (enough LEDs for three full implementations and a few extra to spare) from Swedish [Lawicel-Shop](https://www.lawicel-shop.se/tillverkare/shiji-lighting/5mm-rgb-diffused-led-ws2811-25p) for 79 SEK (less than $10).

## Getting started
1. Solder the stuff together (or just use a short WS2811 LED strip, in case you want to try it out first).
2. Connect the first LED to pin `D6` of the NodeMCU. Also connect the `+5V`/`V_in` pin of the LED to `3v3` on the board and `GND` to `GND`.
3. Open up the code in the [Arduino IDE](https://www.arduino.cc/en/Main/Software)
4. Set your WiFi credentials in `wifi_settings.h`
5. Upload the sketch to the NodeMCU
6. Look at the serial monitor in the IDE. The NodeMCU will print its URL once connected to the WiFi.
7. Visit the URL from the previous step in a browser (from a computer or smartphone connected to the same LAN)

## Some notes on power usage
Yes - the 3.3V pin on the board is enough to power 5V LEDs.

Yes - you can power seven RGB LEDs through a NodeMCU this way. However, if you are using too many LEDs (probably 10+), the board will not be able to deliver enough current. Then you need to connect the `5V` and `GND` pins to an external power supply - just make sure to _also_ connect `GND` to the NodeMCU.

## Keep your WiFi credentials secure
If you build one of these and share your code, run the following command in order to avoid uploading your WiFi credentials to Github:

`git update-index --assume-unchanged wifi_settings.h`
