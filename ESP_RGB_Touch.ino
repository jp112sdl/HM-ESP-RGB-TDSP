#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <Arduino.h>
#include <ArduinoOTA.h>
#include <UTFT.h>
#include <SPI.h>
#include <XPT2046.h>
#include <FS.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>

#define ESP_SPI_FREQ 8000000L

extern unsigned  wifiicon32[1024];
extern unsigned  RGBScale[0x3690];
extern unsigned  RGBhorz[55896];


UTFT myGLCD ( ILI9341_S5P, 15, 5, 2);
XPT2046 myTouch(/*cs=*/ 4, /*irq=*/ 5);

HTTPClient http;

char Hostname[] = "ESP-HM-RC-TC-WIFI";

bool isDimTouching = false;
bool isRGBTouching = false;
int posX = 20; // Start x
int posY = 20; // Start y
int posEndX = 274; // Breite
int posEndY = 75; // Höhe
int percentX = 312;
int percentY = 28;
int wifiIconX = 210;
int wifiIconY = 108;
int RGBstartX = 20;
int RGBstartY = 145;
int RGBheight = 274;
int RGBwidth = 25;
int RGBbarCount = 3;

const boolean hasUsedColor = true; // Farbe für genutzen Anteil ?
const word usedColor = VGA_WHITE; // Farbe f. genutzen Anteil oder 0, ignoriert wenn hasUsedColor=false
const boolean hasUnusedColor = true; // Farbe für ungenutzen Anteil ?
const word unusedColor = VGA_BLACK; // Farbe f. ungenutzen Anteil oder 0, ignoriert wenn hasUnusedColor=false

boolean isFrameSet = false; // Wert wird true wenn Frame erstmalig geschrieben - nicht manuell anpassen!
extern uint8_t BigFont[];
int intAusgabe;

bool Debug = false;

//don't touch
bool shouldSaveConfig =       false;
bool createNewSystemConfig =  false;
String configJsonFile =       "config.json";
bool wifiManagerDebugOutput = true;
char ccuip[15]   = "0.0.0.0";
char ip[15]      = "0.0.0.0";
char netmask[15] = "0.0.0.0";
char gw[15]      = "0.0.0.0";
char ise_LEVEL[10] = "";
char ise_COLOR[10] = "";
bool isRGBavailable = false;

void setup() {
  SPI.setFrequency(ESP_SPI_FREQ);
  myGLCD.InitLCD(LANDSCAPE);
  myGLCD.clrScr();
  myGLCD.setFont(BigFont);
  myGLCD.setColor(255, 255, 255);
  Serial.begin(115200);
  myTouch.begin((uint16_t)myGLCD.getDisplayYSize(), (uint16_t)myGLCD.getDisplayXSize());  // Must be done before setting rotation
  myTouch.setCalibration(206, 1735, 1728, 246);
  myTouch.setRotation(myTouch.ROT270);

  //formatFS();
  createNewSystemConfig = !loadSystemConfig();
  
  myGLCD.print("Verbinde WLAN", 175, 10, 90);
  myGLCD.drawBitmap (wifiIconX, wifiIconY, 32, 32, wifiicon32, 90 , 16 , 16); yield();
  if (doWifiConnect() == false) {
    myGLCD.clrScr();
    myGLCD.setFont(BigFont);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("  Fehler bei ", 175, 00, 90);
    myGLCD.print("WLAN-Verbindung", 155, 00, 90);
  }
  else {
    slider_init();
  }
}

void loop() {
  ArduinoOTA.handle();
  if (WiFi.status() == WL_CONNECTED) {
    slider_read();
    yield();
  }
}

void slider_init() {
  myGLCD.clrScr();
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRect (posX, posY, posX + posEndX, posY + posEndY);
  bool CCUerror = false;
  //aktuellen LEVEL holen und anzeigen
  String currentLEVELValue = getStateFromCCU(String(ise_LEVEL));
  if (currentLEVELValue != "error") {
    currentLEVELValue = currentLEVELValue.substring(currentLEVELValue.indexOf("value='"));
    currentLEVELValue = currentLEVELValue.substring(7, currentLEVELValue.indexOf("'/>"));
    float flValue = currentLEVELValue.toFloat() * 100;
    int percentValue = (int)flValue;
    String displayPercent = "   " + String(percentValue);
    displayPercent = displayPercent.substring(displayPercent.length() - 3) + "%";
    myGLCD.setColor(255, 255, 255);
    myGLCD.print(displayPercent, percentX, percentY, 90);

    for (int tPosY = posY; tPosY <= posY + posEndY; tPosY++)
      myGLCD.drawLine(posX, tPosY, posX + (percentValue * posEndX / 100), tPosY);

    yield();
  } else {
    CCUerror = true;
  }

  if (isRGBavailable) {
    String currentCOLORValue = getStateFromCCU(String(ise_COLOR));
    if (currentCOLORValue != "error") {
      currentCOLORValue = currentCOLORValue.substring(currentCOLORValue.indexOf("value='"));
      currentCOLORValue = currentCOLORValue.substring(7, currentCOLORValue.indexOf("'/>"));
      myGLCD.setColor(255, 255, 255);

      int knobPosX = (((float)currentCOLORValue.toInt() / 200.0) + ((float)RGBstartX / (float)RGBheight)) * (float)RGBheight;
      draw_knob(knobPosX);
      yield();
    } else {
      CCUerror = true;
    }
  }

  if (CCUerror) {
    myGLCD.clrScr();
    myGLCD.setFont(BigFont);
    myGLCD.setColor(255, 255, 255);
    myGLCD.print("  Fehler bei ", 175, 00, 90);
    myGLCD.print("CCU2-Verbindung", 155, 00, 90);
    myGLCD.setColor(255, 0, 0);
    myGLCD.drawLine(319, 0, 319, 240);
    myTouch.powerDown();
  } else {
    myGLCD.setColor(0, 255, 0);
    myGLCD.drawLine(319, 0, 319, 240);
  }
  myGLCD.setColor(0, 0, 0);
  myGLCD.fillRect (wifiIconX, wifiIconY, wifiIconX + 32, wifiIconY + 32); yield();
}

// Slider auslesen und Werte zurückgeben
int slider_read() {
  float tPosX;
  float tPosY;
  float tAusgabe;
  if (myTouch.isTouching())
  {
    //Serial.println("Touch detected.");
    uint16_t x, y;
    int xCumul = 0;
    int yCumul = 0;
    for (int touchCount = 0; touchCount < 100; touchCount++)
    {
      myTouch.getPosition(x, y);
      xCumul += x;
      yCumul += y;
    }
    tPosX = xCumul / 100;
    tPosY = yCumul / 100;
    Serial.println("tPosX = " + String(tPosX) + "; tPosY = " + String(tPosY));
    if (tPosX >= posX && tPosX <= posX + posEndX && tPosY >= posY && tPosY <= posY + posEndY) {
      isDimTouching = true;
      draw_used_unused(tPosX, tPosY);
      tAusgabe = ((tPosX - posY) / posEndX) * 100;
      intAusgabe = (int) tAusgabe;
      String displayPercent = "   " + String(intAusgabe);
      displayPercent = displayPercent.substring(displayPercent.length() - 3) + "%";
      myGLCD.setColor(255, 255, 255);
      myGLCD.print(displayPercent, percentX, percentY, 90);
    }
    //RGB
    if (isRGBavailable && tPosX >= RGBstartX && tPosX <= RGBstartX + RGBheight && tPosY >= RGBstartY && tPosY <= RGBstartY +  (RGBwidth * RGBbarCount)) {
      draw_knob(tPosX);
      isRGBTouching = true;
      tAusgabe = ((tPosX - RGBstartX) / RGBheight) * 200;
      intAusgabe = (int) tAusgabe;
    }
  }
  else {
    if (isDimTouching) {
      if (intAusgabe < 4) intAusgabe = 0;
      String val = "00" + String(intAusgabe);
      val = "0." + val.substring(val.length() - 2);
      Serial.println("Setze CCU Wert " + String(ise_LEVEL) + " = " + val);
      setStateCCU(String(ise_LEVEL), val);
      isDimTouching = false;
    }
    //RGB
    if (isRGBTouching) {
      Serial.println("Setze CCU Wert " + String(ise_COLOR) + " = " + String(intAusgabe));
      setStateCCU(String(ise_COLOR), String(intAusgabe));
      isRGBTouching = false;
    }
  }
}


// zeichnet einen Schieberegler
void draw_knob(int tPosX) {
  word tColor;
  for (int barCount = 0; barCount < RGBbarCount; barCount++)
    myGLCD.drawBitmap (RGBstartX, RGBstartY + (RGBwidth * barCount), RGBheight, RGBwidth, RGBhorz, 1);
  tColor = myGLCD.getColor();
  myGLCD.setColor(255, 255, 255);

  myGLCD.fillRect(tPosX - 1, RGBstartY + ( (RGBwidth * RGBbarCount) / 2) - ( (RGBwidth * RGBbarCount) / 4), tPosX, RGBstartY + ( (RGBwidth * RGBbarCount) / 2) + ( (RGBwidth * RGBbarCount) / 4));
  // myGLCD.drawLine(posX, RGBstartY + ( (RGBwidth * RGBbarCount) / 2), tPosX, RGBstartY + ( (RGBwidth * RGBbarCount) / 2));

  myGLCD.setColor(tColor);

}

void draw_used_unused(int tPosX, int tPosY) {
  word tColor;
  if ((hasUsedColor == true) || (hasUnusedColor == true)) tColor = myGLCD.getColor();
  if (hasUsedColor == true) myGLCD.setColor(usedColor);
  for (int tPosY = posY; tPosY <= posY + posEndY; tPosY++) myGLCD.drawLine(posX, tPosY, tPosX, tPosY);
  myGLCD.setColor(tColor);
  if (hasUnusedColor == true) myGLCD.setColor(unusedColor);
  for (int tPosY = posY; tPosY <= posY + posEndY; tPosY++) myGLCD.drawLine(tPosX, tPosY, posX + posEndX, tPosY);
  myGLCD.setColor(255, 255, 255);
  myGLCD.drawRect (posX, posY, posX + posEndX, posY + posEndY);
  myGLCD.setColor(tColor);
}

void formatFS() {
  Serial.println("Formatiere FS...");
  SPIFFS.begin();
  SPIFFS.format();
  SPIFFS.end();
  Serial.println("Formatieren beendet.");
}

