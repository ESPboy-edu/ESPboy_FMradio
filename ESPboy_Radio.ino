#include <Adafruit_NeoPixel.h>
#include <Adafruit_MCP23017.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_GFX.h>
#include <ESP8266WiFi.h> 
#include <radio.h>
#include <SI4703.h>
#include <RDSParser.h>
#include "ESPboyLogo.h"
#include <ESP_EEPROM.h>

#define LEDquantity 1
#define MCP23017address 0 //actually it's 0x20 but in <Adafruit_MCP23017.h> there is (x|0x20)

//PINS
#define LEDpin            D4
#define SOUNDpin          D3
#define csTFTMCP23017pin  8 //chip select pin on the MCP23017 for TFT display
#define radioSDApin       SDA 
#define radioRESETpin     3

//buttons
#define UP_BUTTON       1
#define DOWN_BUTTON     2
#define LEFT_BUTTON     0
#define RIGHT_BUTTON    3
#define ACT_BUTTON      4
#define ESC_BUTTON      5

//SPI for LCD
#define TFT_RST       -1
#define TFT_DC        D8
#define TFT_CS        -1

uint8_t buttonspressed[8];
char servicename[10];

struct ESP_EEPROM{
  uint16_t freq;
  uint8_t vol;
  uint8_t stereo;
  uint8_t bass;
};

ESP_EEPROM esp_eeprom;
static uint8_t esp_eeprom_needsaving = 0;

Adafruit_MCP23017 mcp;
SI4703 radio(radioSDApin, radioRESETpin);    
RDSParser rds;
RADIO_INFO radioinfo;
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(LEDquantity, LEDpin, NEO_GRB + NEO_KHZ800);


void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
}


void DisplayServiceName(char *name){
  if (name[0]){
    tft.fillRect(0, 58, 128, 16, ST77XX_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_RED);
    tft.setCursor((128-strlen(name)*12)/2, 58);
    tft.print(name);
  }
} 


void batVoltageDraw(){
  uint16_t volt;
  volt = analogRead(A0);
  volt = map(volt, 820, 1024, 0, 100);
  tft.fillRect(75, 120, 128-75, 8, ST77XX_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_WHITE);
  tft.setCursor(75, 120);
  tft.print("Bat ");
  tft.print(volt);
  tft.print("%");
}



void DisplayTime(uint8_t hour, uint8_t minute) {
  tft.fillRect(0, 14, 128, 10, ST77XX_BLACK),
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(50, 14); 
  if (hour < 10) tft.print('0');
  tft.print(hour);
  tft.print(':');
  if (minute < 10) tft.print('0');
  tft.println(minute);
}


void redrawtft(){
 uint8_t part1freq, part2freq;
 uint16_t freq;
//clear TFT
  tft.fillScreen(ST77XX_BLACK);
//draw ESPboy radio
  tft.setTextSize(1);
  tft.setTextColor( 0x2222);
  tft.setCursor(10, 0);
  tft.print("ESPboy FM radio v1");
//draw freq
  tft.setTextSize(3);
  tft.setTextColor(ST77XX_GREEN);
  freq = radio.getFrequency();
  part1freq = radio.getFrequency()/100;
  part2freq = freq - part1freq * 100;
  if (part2freq > 99) part2freq /= 10;
  if (part2freq > 9) part2freq /= 10;
  if (part1freq >99) tft.setCursor(18, 30);
  else tft.setCursor(28, 30);
  tft.print(part1freq);
  tft.print(".");
  tft.print(part2freq); 
//draw Vol
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(0, 80);
  tft.print("Vol ");
  for (int i=0; i<radio.getVolume(); i++) tft.print("|");
//draw MONO
  tft.setCursor(0, 90);
  if (!radio.getMono()) tft.print ("Stereo"); else tft.print ("Mono");
//draw BASS
  tft.setCursor(0, 100);
  if (radio.getBassBoost()) tft.print ("Bass on"); else tft.print ("Bass off");
//draw RSSI
   tft.setCursor(0, 110);
   radio.getRadioInfo(&radioinfo); 
   tft.print ("RSSI "); tft.print (radioinfo.rssi);
//draw RDS
   radio.checkRDS();
   DisplayServiceName(&servicename[0]);
//draw bat
   batVoltageDraw();
}



uint8_t checkbuttons(){
  uint8_t check = 0;
  for (int i = 0; i < 6; i++){
    if(!mcp.digitalRead(i)) { 
       buttonspressed[i] = 1; 
       check++;
       delay(10);} // to avoid i2c bus ovelflow during long button keeping pressed
    else buttonspressed[i] = 0;
  }
  return (check);
}


void runButtonsCommand(){
     esp_eeprom_needsaving++;
     tone (SOUNDpin, 600, 20);
     pixels.setPixelColor(0, pixels.Color(0,0,10));
     pixels.show();
     if (buttonspressed[RIGHT_BUTTON]) { 
         pixels.setPixelColor(0, pixels.Color(10,0,0)); 
         pixels.show(); 
         radio.seekUp(false); 
         pixels.setPixelColor(0, pixels.Color(0,10,0)); 
         pixels.show(); 
         tone (SOUNDpin, 1000, 20);
         delay(200);
         }
     if (buttonspressed[LEFT_BUTTON]) { 
         pixels.setPixelColor(0, pixels.Color(10,0,0)); 
         pixels.show(); 
         radio.seekDown(false); 
         pixels.setPixelColor(0, pixels.Color(0,10,0)); 
         pixels.show(); 
         tone (SOUNDpin, 1000, 20);
         delay(200);}
     if (buttonspressed[UP_BUTTON]) { if (radio.getVolume() < 15) radio.setVolume(radio.getVolume() + 1); }
     if (buttonspressed[DOWN_BUTTON]) { if (radio.getVolume() > 0) radio.setVolume(radio.getVolume() - 1); }
     if (buttonspressed[ACT_BUTTON]) { 
         radio.setMono(! radio.getMono()); }
     if (buttonspressed[ESC_BUTTON]) { 
         radio.setBassBoost(!radio.getBassBoost()); }
     
     while (checkbuttons());
     delay(200);
     redrawtft(); 
     pixels.setPixelColor(0, pixels.Color(0,0,0));
     pixels.show();
}


boolean esp_eeprom_save(){
  esp_eeprom.freq = radio.getFrequency();
  esp_eeprom.vol = radio.getVolume();
  esp_eeprom.stereo = radio.getMono();
  esp_eeprom.bass = radio.getBassBoost();
  EEPROM.put (0, esp_eeprom);
  return (EEPROM.commit());
}


void esp_eeprom_load(){
    EEPROM.get(0, esp_eeprom);
    radio.setBandFrequency(RADIO_BAND_FM, esp_eeprom.freq);
    radio.setMono(esp_eeprom.stereo);
    radio.setVolume(esp_eeprom.vol);
    radio.setBassBoost(esp_eeprom.bass);
}


void setup() {
  Serial.begin(9600);
  delay (100);
  WiFi.mode(WIFI_OFF); // to safe some battery power

//LED init
  pinMode(LEDpin, OUTPUT);
  pixels.begin();
  delay(100);
  pixels.setPixelColor(0, pixels.Color(0,0,0));
  pixels.show();

//buttons on mcp23017 init
  mcp.begin(MCP23017address);
  delay (100);
  for (int i=0;i<6;i++){  
     mcp.pinMode(i, INPUT);
     mcp.pullUp(i, HIGH);}

//TFT init     
  mcp.pinMode(csTFTMCP23017pin, OUTPUT);
  mcp.digitalWrite(csTFTMCP23017pin, LOW);
  tft.initR(INITR_144GREENTAB);
  delay (100);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);
  
//draw ESPboylogo  
  tft.drawXBitmap(30, 24, ESPboyLogo, 68, 64, ST77XX_YELLOW);
  tft.setTextSize(1);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setCursor(42,102);
  tft.print ("FM radio");

//reset radio module
  pinMode (radioRESETpin, OUTPUT);
  digitalWrite(radioRESETpin, LOW);
  delay(50);
  digitalWrite(radioRESETpin, HIGH);
  delay(50);

//global vars init
  memset(buttonspressed, 0, sizeof(buttonspressed));
  memset(servicename, 0, sizeof(servicename));
  
//sound init and test
  pinMode(SOUNDpin, OUTPUT);
  tone(SOUNDpin, 200, 100);
  delay(100);
  tone(SOUNDpin, 100, 100);
  delay(100);
  noTone(SOUNDpin);
  
//BAT voltage measure init
  pinMode(A0, INPUT);

//radio init  
  radio.init();
  delay (100);
  radio.attachReceiveRDS(RDS_process);
  rds.attachServicenNameCallback(DisplayServiceName);
  rds.attachTimeCallback(DisplayTime);

//load last radio state from eeprom
  EEPROM.begin(sizeof (esp_eeprom));
  esp_eeprom_load();

//clear TFT
  delay(2000);
  tft.fillScreen(ST77XX_BLACK);

//draw radio desktop
  redrawtft();
}


void loop() {
 static unsigned long counter;
  radio.checkRDS();
  if (checkbuttons()) runButtonsCommand();
  if (millis() > (counter + 2000)){
     counter = millis();
     DisplayServiceName(&servicename[0]);
     batVoltageDraw();    
     if (esp_eeprom_needsaving){
        esp_eeprom_save();
        esp_eeprom_needsaving = 0;
     }
  }
  
  delay (20);
} 
