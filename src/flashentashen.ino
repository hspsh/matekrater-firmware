#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Adafruit_NeoPixel.h>
#include "WiFi_creds.h"

// Physical properties of the screen.
#define PIN 5
#define SCREEN_X 2*5
#define SCREEN_Y 2*4
#define SCREEN_LEDS SCREEN_X * SCREEN_Y

// WiFi properties
#define PORT              1337


WiFiUDP Udp;
uint8_t incomingPacket[SCREEN_LEDS * 4];


typedef struct Pixel {
  uint8_t r, g, b;
} Pixel;

typedef struct PPMScreen {
  unsigned int x, y;
  uint8_t depth;
  Pixel *pixels;
} PPMScreen;


class Screen {
  private:
    PPMScreen screen;

  public:
    Screen(unsigned int x, unsigned int y) {
      screen.x = x;
      screen.y = y;
      screen.pixels = new Pixel[x * y];
    }

    void consumePPM(uint8_t message[], const int len) {
      char *temp = new char[screen.x * screen.y * 3];
      int numbers[3];
      int j = 0;
      int k = 0;
      int l = 0;
      // Check magic number
      if (message[1] == '6') {
        for (int i = 3; i < len; ++i) {
          
          if (message[i] > 48 && message[i] < 58) {
            temp[j] = (char)message[i];
          } else {
            temp[j] = message[i];
          }
          ++j;
          
          if (message[i] == ' ' || message[i] == '\n' || message[i] == '\0') {
            if (temp[0] > 48 && temp[0] < 58) {
              j = 0;
              numbers[k] = atoi(temp);
              ++k;
            }
          }

          if(j == (screen.x * screen.y * 3)) {
              for (int i = 0; i < screen.x * screen.y * 3; ++i) {
                screen.pixels[l].r = temp[i];
                ++i;
                screen.pixels[l].g = temp[i];
                ++i;
                screen.pixels[l].b = temp[i];
                ++l;
              }
              break;
            }
        }
        Serial.println(numbers[0]);
        Serial.println(numbers[1]);
        Serial.println(numbers[2]);
      }
      delete[] temp;
    }

    /*
    void toZigZag() {
      int i = 0, j = 0, k = 0, y = 1;
      Pixel temp;

      while (i < screen.x * screen.y) {

        //Serial.println(i);
        if (y % 2 == 0 && y != 0) {
          // tu odwracamy rząd.
          j = y*screen.x - (i % screen.x) -1;
        } else j = i;
        
        if((i % screen.x) < (int)screen.x/2) {
          temp = screen.pixels[j];
          screen.pixels[j] = screen.pixels[i];
          screen.pixels[i] = temp;  
        }
        
        ++i;
        if (i % screen.x == 0 && i != 0) ++y;
      }
    }
    */

    int scrambler_4box(int pixelNo){
      
      int pixelX, pixelY;
      
      int boxPixel = pixelNo%20;
      int boxNo = pixelNo/20;

      int boxX = boxPixel/5;
      int boxY;
      if ((boxPixel/5)%2 == 0){
          boxY = boxPixel%5;
      }
      else{
          boxY = 4 - boxPixel%5;
      }

      if ((boxNo == 2)||(boxNo == 3)){
          boxX = 3-boxX;
          boxY = 4-boxY;
      }

      if (boxNo == 0){
          pixelX = boxX;
          pixelY = boxY + 5;
      }

      if (boxNo == 1){
          pixelX = boxX+4;
          pixelY = boxY + 5;
      }

      if (boxNo == 2){
          pixelX = boxX+4;
          pixelY = boxY;
      }

      if (boxNo == 3){
          pixelX = boxX;
          pixelY = boxY;
      }

    return pixelY*8+pixelX;
    }

    void display(Adafruit_NeoPixel *strip) {
      //toZigZag();
      for (int i = 0; i < screen.x * screen.y; ++i) {
        //Serial.printf("%02X %02X %02X ",  screen.pixels[i].r, screen.pixels[i].g, screen.pixels[i].b);
        //if (i % (screen.x-1) == 0 && i != 0) Serial.println();
        int pixelNo = scrambler_4box(i);
        strip->setPixelColor(i, screen.pixels[pixelNo].r, screen.pixels[pixelNo].g, screen.pixels[pixelNo].b);
        strip->show();
      }
    }
};

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(SCREEN_LEDS, PIN, NEO_RGB + NEO_KHZ800);
Screen flashentashen(SCREEN_X, SCREEN_Y);
unsigned int loops = 0;


void setup() {
  Serial.begin(115200);
  delay(10);

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(NETWORK_SSID);

  WiFi.begin(NETWORK_SSID, NETWORK_PASSWORD);
  //WiFi.config(IPAddress(192, 168, 1, 60), IPAddress(192, 168, 1, 1), IPAddress(255, 255, 255, 0));

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Udp.begin(PORT);

  strip.begin();
  strip.show(); // Show *nothing*
}

void loop() {
  ++loops;
  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    loops = 0;
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, SCREEN_LEDS * 4);

    flashentashen.consumePPM(incomingPacket, len);

    flashentashen.display(&strip);

  } else if (loops > 100000) {
    rainbowCycle(10);
    loops = 100000;
  };

  // Some example procedures showing how to display to the pixels:
  //colorWipe(strip.Color(255, 0, 0), 50); // Red
  //colorWipe(strip.Color(0, 255, 0), 50); // Green
  //colorWipe(strip.Color(0, 0, 255), 50); // Blue
  //rainbow(20);

}

// Fill the dots one after the other with a color
void colorWipe(uint32_t c) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.show();
  }
}


// Slightly different, this makes the rainbow equally distributed throughout
void rainbowCycle(uint8_t wait) {
  uint16_t i, j;

  for (j = 0; j < 256; j++) { // 5 cycles of all colors on wheel
    for (i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel(((i * 256 / strip.numPixels()) + j) & 255));
    }
    strip.show();
    delay(wait);
  }
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  if (WheelPos < 85) {
    return strip.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    return strip.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
    WheelPos -= 170;
    return strip.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
