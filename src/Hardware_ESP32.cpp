#include "Hardware_ESP32.h"
#include "tDeckPro.h"
#include "GxEPD2_BW.h"
#include "JetBrainsMono_Bold8pt7b.h"

#include <string>

using namespace std;


static GxEPD2_BW<GxEPD2_310_GDEQ031T10, GxEPD2_310_GDEQ031T10::HEIGHT> display(
  GxEPD2_310_GDEQ031T10(BOARD_SPI_CS, BOARD_SPI_DC, BOARD_SPI_RST, BOARD_SPI_BUSY));


void PimHardware::init()
{
  Serial.begin(115200);
  while (!Serial) { delay(10); }
  Serial.println("Serial up.");

  pinMode(BOARD_LORA_CS, OUTPUT);
  digitalWrite(BOARD_LORA_CS, LOW); // low to disable

  pinMode(BOARD_LORA_RST, OUTPUT);
  digitalWrite(BOARD_LORA_RST, LOW);

  pinMode(BOARD_SD_CS, OUTPUT);
  digitalWrite(BOARD_SD_CS, HIGH);

  pinMode(BOARD_EPD_CS, OUTPUT);
  digitalWrite(BOARD_EPD_CS, HIGH);

  pinMode(BOARD_KEYBOARD_LED, OUTPUT);
  digitalWrite(BOARD_KEYBOARD_LED, LOW);

  // --------------- SPI ---------------
  SPI.begin(BOARD_SPI_SCK, BOARD_SPI_MISO, BOARD_SPI_MOSI, BOARD_SD_CS);
  delay(500);

  // --------------- Display ---------------
  display.init(115200, true, 2, false);
  delay(500);
  display.setFullWindow();
  display.setRotation(0);
  display.setFont(&JetBrainsMono_Bold8pt7b);
  display.setTextColor(0x000);
  display.clearScreen();
  display.fillScreen(0xFFF);
  display.setCursor(20, 20);
  display.print("-- setup --");
  display.display();
}

void PimHardware::loop()
{
  try
  {
    Serial.println("-- loop --");
  }
  catch (const exception &e)
  {
    PimHardware::handle_exception( e );
  }
}


void PimHardware::log(const string &message)
{
  Serial.println(message.c_str());
}

void PimHardware::handle_exception( const exception &e )
{
  Serial.println("-- exception --");
  Serial.println(e.what());
}