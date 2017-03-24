#include <i2c_t3.h>
#include <SPI.h>              //SPI
#include "SdFat.h"            //SD-Karte
#include "FreeStack.h"
#include "UserDataType.h"
#include "MAX30100_PulseOximeter.h"
#include <Adafruit_ST7735.h>  //Display
#include <Adafruit_GFX.h>
#include <gfxfont.h>
#include <Time.h>             //Zeit
#include <TimeLib.h>


//Klassendeklaration
SdFatSdio sd;
SdBaseFile binFile;
File myFile;
PulseOximeter pox;

//==============================================================================
// Start of configuration constants.
//==============================================================================
//Interval between data records in microseconds.
const uint32_t LOG_INTERVAL_USEC = 1000;

// File start time in micros.
uint32_t startMicros;

// File definitions.
//
// Maximum file size in blocks is limited to 65535.
// The program creates a contiguous file with FILE_BLOCK_COUNT 512 byte blocks.
// This file is flash erased using special SD commands.  The file will be
// truncated if logging is stopped early.
const uint32_t FILE_BLOCK_COUNT = 65500;

// log file base name.  Must be six characters or less.
#define FILE_BASE_NAME "data"
//------------------------------------------------------------------------------
// Buffer definitions.
//
// The logger will use SdFat's buffer plus BUFFER_BLOCK_COUNT additional
// buffers.
//
#ifndef RAMEND
// Assume ARM. Use total of nine 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 8;
//
#elif RAMEND < 0X8FF
#error Too little SRAM
//
#elif RAMEND < 0X10FF
// Use total of two 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 1;
//
#elif RAMEND < 0X20FF
// Use total of five 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 4;
//
#else  // RAMEND
// Use total of 13 512 byte buffers.
const uint8_t BUFFER_BLOCK_COUNT = 12;
#endif  // RAMEND

//==============================================================================
// End of configuration constants.
//==============================================================================

// Temporary log file.  Will be deleted if a reset or power failure occurs.
#define TMP_FILE_NAME FILE_BASE_NAME "##.bin"

// Size of file base name.  Must not be larger than six.
const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
const uint8_t FILE_NAME_DIM = BASE_NAME_SIZE + 7;
char binName[FILE_NAME_DIM] = FILE_BASE_NAME "00.bin"; //File_Base_Name=data

// Number of data records in a block.
const uint16_t DATA_DIM = (512 - 4) / sizeof(data_t);

//Compute fill so block size is 512 bytes.  FILL_DIM may be zero.
const uint16_t FILL_DIM = 512 - 4 - DATA_DIM * sizeof(data_t);

struct block_t {
  uint16_t count;
  uint16_t overrun;
  data_t data[DATA_DIM];    //Struct innerhalb eines Struct
  uint8_t fill[FILL_DIM];
};

const uint8_t QUEUE_DIM = BUFFER_BLOCK_COUNT + 2;

block_t* emptyQueue[QUEUE_DIM];
uint8_t emptyHead;
uint8_t emptyTail;

block_t* fullQueue[QUEUE_DIM];
uint8_t fullHead;
uint8_t fullTail;

// Advance queue index.
inline uint8_t queueNext(uint8_t ht) {
  return ht < (QUEUE_DIM - 1) ? ht + 1 : 0;
}

//==============================================================================
// Error messages stored in flash.
#define error(msg) errorFlash(F(msg))
//------------------------------------------------------------------------------
void errorFlash(const __FlashStringHelper* msg) {
  sd.errorPrint(msg);
}

#define TFT_CS        15
#define TFT_RST       34  // you can also connect this to the Arduino reset in which case, set this #define pin to 0!
#define TFT_DC        35
#define TFT_SCLK      13  // set these to be whatever pins you like!
#define TFT_MOSI      11  // set these to be whatever pins you like!

#define Taster        36   //Tasterbelegung

static volatile int t = 0;
static volatile int updated = 10000;
static volatile int timet = 0;
static int beendet = 0;
static int Dateien = 0;
static int DataBin = 0;
struct Zeit
{
  int Year;
  int Month;
  int Day;
  int Hour;
  int Minute;
  int Second;
};


//Initalisieren des SPI des Displays
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST); //SPI Setup für das Display


void setup() {

  analogReadResolution(10);
  //Initialisieren des Displays
  tft.initR(INITR_BLACKTAB);           // initialize a ST7735S chip, black tab
  tft.fillScreen(ST7735_BLACK);        //Bildschirm schwarz
  tft.setTextSize(1);                  //Schriftgröße
  tft.setTextColor(ST7735_WHITE);      //Weiße Schrift
  tft.setTextWrap(true);               //Schrift ist fortlaufend
  tft.setRotation(1);                  //Bildschirm wird um 90° gedreht

  //Initialisieren der Pins
  pinMode(Taster, INPUT);             //Taster wird als Input konfiguriert
  pinMode(21, OUTPUT);                //Anschalten des AD8232
  digitalWrite(21, HIGH);
  pinMode(18, INPUT);

  // initialize file system mit Fehlerabfrage
  if (!sd.begin()) {
    tft.fillScreen(ST7735_BLACK);
    tft.setTextSize(2);
    tft.setCursor(0, 0);
    tft.println("Keine SD-Karte");
    while (1);
  }

  //Ab hier werden Tastendrücke wahrgenommen
  noInterrupts();
  attachInterrupt(digitalPinToInterrupt(36), Schalter, FALLING);
  interrupts();
}

void loop() {
  static int min = 70;  //aktuelle Minute speichern
  static int h = 1000;  //Abfrage
  static int z = h;
  static int k = 0;

  //Anzeige der Uhrzeit. Nach jeder Minute erneuern, oder wenn zurück aus Unterfunktion
  if (min != minute() || k == 1)
  {
    setTime(Teensy3Clock.get());
    clockDisplay(); //Ausgabe der Uhrzeit
    tft.setTextSize(1);
    min = minute(); //neue Minute speichern
  }

  //Darstellen des Hauptmenüs, abhängig vom Tastendruck t
  if (h != t || k == 1)
  {
    tft.fillRect(0, 30, tft.width(), 50, ST7735_BLACK);
    tft.setTextSize(1);
    if ((t % 5) == 0)
    {
      tft.fillCircle(5, 33, 3, ST7735_MAGENTA); //Auswahlpunkt
      tft.setCursor(12, 30);
      tft.println("Keine Auswahl");
      tft.setCursor(0, 40);
      tft.println("Zeit einstellen");
      tft.setCursor(0, 50);
      tft.println("Daten loeschen");
      tft.setCursor(0, 60);
      tft.println("Daten aufnehmen");
      tft.setCursor(0, 70);
      tft.println("Daten uebertragen");

    }
    else if ((t % 5) == 1)
    {
      tft.setCursor(0, 30);
      tft.println("Keine Auswahl");
      tft.fillCircle(5, 43, 3, ST7735_MAGENTA);
      tft.setCursor(12, 40);
      tft.println("Zeit einstellen");
      tft.setCursor(0, 50);
      tft.println("Daten loeschen");
      tft.setCursor(0, 60);
      tft.println("Daten aufnehmen");
      tft.setCursor(0, 70);
      tft.println("Daten uebertragen");
    }
    else if ((t % 5) == 2)
    {
      tft.setCursor(0, 30);
      tft.println("Keine Auswahl");
      tft.setCursor(0, 40);
      tft.println("Zeit einstellen");
      tft.fillCircle(5, 53, 3, ST7735_MAGENTA);
      tft.setCursor(12, 50);
      tft.println("Daten loeschen");
      tft.setCursor(0, 60);
      tft.println("Daten aufnehmen");
      tft.setCursor(0, 70);
      tft.println("Daten uebertragen");
    }
    else if ((t % 5) == 3)
    {
      tft.setCursor(0, 30);
      tft.println("Keine Auswahl");
      tft.setCursor(0, 40);
      tft.println("Zeit einstellen");
      tft.setCursor(0, 50);
      tft.println("Daten loeschen");
      tft.fillCircle(5, 63, 3, ST7735_MAGENTA);
      tft.setCursor(12, 60);
      tft.println("Daten aufnehmen");
      tft.setCursor(0, 70);
      tft.println("Daten uebertragen");
    }
    else if ((t % 5) == 4)
    {
      tft.setCursor(0, 30);
      tft.println("Keine Auswahl");
      tft.setCursor(0, 40);
      tft.println("Zeit einstellen");
      tft.setCursor(0, 50);
      tft.println("Daten loeschen");
      tft.setCursor(0, 60);
      tft.println("Daten aufnehmen");
      tft.fillCircle(5, 73, 3, ST7735_MAGENTA);
      tft.setCursor(12, 70);
      tft.println("Daten uebertragen");
    }
    h = t;  //Aktuelle Auswahl speichern, damit nicht in jedem Durchlauf neu angezeigt wird
    k = 0;
  }

  //Wird die Auswahl länger als 2s gehalten, wird die entsprechende Unterfunktion aufgerufen
  if (z != h)
  {
    if ((t % 5) == 1 && millis() - timet > 2000)
    {
      Time();
      z = 0;
      k = 1;
    }
    else if ((t % 5) == 2 && millis() - timet > 2000)
    {
      loeschen();
      z = 0;
      k = 1;
    }
    else if ((t % 5) == 3 && millis() - timet > 2000)
    {
      Messung();
      z = 0;
      k = 1;
    }
    else if ((t % 5) == 4 && millis() - timet > 2000)
    {
      sendData();
      z = 0;
      k = 1;
    }
  }
}


//Interrupt, wenn der Schalter gedrückt wird
void Schalter ()
{
  updated = millis() - timet; //wie viel Zeit zwischen den Tastendrücken vergangen ist
  timet = millis();           //zu welcher Zeit der Taster gedrückt wurde

  t++;

}

void clockDisplay()
{
  tft.setTextSize(3);                                  //schriftgröße ändern
  tft.setCursor(0, 0);                                 //Cursor setzen

  //löschen der "alten" Uhrzeit
  tft.fillRect(0, 0, tft.width(), 30, ST7735_BLACK);

  //Ausgabe
  tft.print(hour());
  tft.print(":");
  if (minute() < 10)
    tft.print("0"); //führende 0
  tft.print(minute());
}
