#include <MemoryHexDump.h>

#include "Teensy41_Cardlike.h"
#include "flexio_teensy_mm.c"
#include "teensy41.c"

//#define use_spi
#if defined(use_spi)
#include <SPI.h>
#include <RA8876_t3.h>
#else
//#include <RA8876_t3.h>
#include <RA8876_t41_p.h>
#endif
//#include <math.h>

#if defined(use_spi)
#define RA8876_CS 10
#define RA8876_RESET 9
#define BACKLITE 5 //External backlight control connected to this Arduino pin
RA8876_t3 tft = RA8876_t3(RA8876_CS, RA8876_RESET); //Using standard SPI pins
#else
uint8_t dc = 13;
uint8_t cs = 11;
uint8_t rst = 12;
#define BACKLITE 5 //External backlight control connected to this Arduino pin
//RA8876_t41_p tft = RA8876_t41_p(dc,cs,rst); //(dc, cs, rst)
RA8876_t41_p tft = RA8876_t41_p(dc,cs,rst); //(dc, cs, rst)
#endif

uint32_t start = 0;
uint32_t end =  0;
uint8_t busSpeed = 24;

void setup() {
  while (!Serial && millis() < 3000) {} //wait for Serial Monitor
  if (CrashReport) {
    Serial.print(CrashReport);
    waitforInput();
  }

  Serial.printf("%c MicroMod Board and RA8876 parallel 8080 mode testing (8Bit/DMA)\n\n",12);
//  Serial.print(CrashReport);

  // Set 16bit mode
  //tft.setBusWidth(16);
  // DB5.0 WR pin, RD pin, D0 pin.
  tft.setFlexIOPins(53,52,40);

#if defined(use_spi)
  tft.begin(); 
#else
  tft.begin(busSpeed);// 20 is working in 8bit and 16bit mode on T41
#endif
  delay(100);

  Serial.print("Bus speed: ");
  Serial.print(busSpeed,DEC);
  Serial.println(" MHZ");
  Serial.print("Bus Width: ");
  Serial.print(tft.getBusWidth(),DEC);
  Serial.println("-bits");

  tft.graphicMode(true);
  tft.setRotation(0);
  tft.onCompleteCB(&frame_complete_callback);

}

volatile bool frame_active = false;
void frame_complete_callback() {
    Serial.println("\n*** Frame Complete Callback ***");
    frame_active = false;
}

uint16_t pixel_data[4000];

void loop() {
#if 1    
  tft.fillScreen(BLUE);
  frame_active = true;
  tft.pushPixels16bitAsync(teensy41_Cardlike,10,10,575,424);
  while(frame_active) {};
  delay(250); // make sure
  MemoryHexDump(Serial, teensy41_Cardlike, 128, true, "\nObject:\n", -1, 0);
  tft.readRect(10, 10, 575, 2, pixel_data);
  MemoryHexDump(Serial, pixel_data, 128, true, "\nReadBack:\n", -1, 0);

  waitforInput();
  tft.pushPixels16bitAsync(flexio_teensy_mm,0,0,480,320); // 480x320
  waitforInput();
  tft.pushPixels16bitAsync(teensy41,0,0,480,320); // 480x320
  waitforInput();
  tft.writeRect(10,10,575,424,teensy41_Cardlike);
  waitforInput();
  tft.writeRect(10,280,480,320,teensy41);
  waitforInput();
  tft.writeRect(530,0,480,320,flexio_teensy_mm);
  waitforInput();
#else
  tft.fillScreen(0x0010);
  start = micros();
  tft.pushPixels16bitDMA(teensy41,1,1,480,320);    // FLASHMEM buffer
  end = micros() - start;
  Serial.printf("Wrote %d bytes in %dus\n\n",480*320, end);
  waitforInput();
  tft.fillScreen(0x0010);
  start = micros();
  tft.pushPixels16bitDMA(teensy41_Cardlike,1,1,575,424);    // FLASHMEM buffer
  end = micros() - start;
  Serial.printf("Wrote %d bytes in %dss\n\n",575*424, end);
  waitforInput();
  tft.fillScreen(0x0010);
  start = micros();
  tft.pushPixels16bitDMA(flexio_teensy_mm,530,260,480,320); // FLASHMEM buffer
  end = micros() - start;
  Serial.printf("Wrote %d bytes in %dus\n\n",480*320, end);
  waitforInput();
#endif
}

void waitforInput()
{
  Serial.println("Press anykey to continue");
  while (Serial.read() == -1) ;
  while (Serial.read() != -1) ;
}
