/*
ESPboy FMradio module firmware. Using SI4703 chipset
www.espboy.com project 
by RomanS 12.06.2021
Install to your Arduino IDE library https://github.com/pu2clr/SI470X
*/


#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"
#include <SI470X.h>
#include <ESP_EEPROM.h>

#define radioSDApin       SDA 
#define radioRESETpin     D8
#define I2C_ADDR 0x10

#define MAX_DELAY_RDS 20
#define MAX_DELAY_SAVE 2000  

struct ESP_EEPROM{
  uint16_t freq;
  uint8_t vol;
  uint8_t stereo;
};


String RDStext, RDStime, RDSprevText, RDSprevTime;
uint16_t prevRSSI;

ESP_EEPROM esp_eeprom;
static uint8_t esp_eeprom_needsaving = 0;

ESPboyInit myESPboy;
SI470X radio;


void displayRDStext(char *nme){
  RDSprevText = RDStext;
  myESPboy.tft.fillRect(0, 58, 128, 16, TFT_BLACK);
  myESPboy.tft.setTextSize(2);
  myESPboy.tft.setTextColor(TFT_RED);
  nme[10]=0;
  String nmestr = (String)nme;
  nmestr.trim();
  myESPboy.tft.setCursor((128-strlen(nme)*12)/2, 58);
  myESPboy.tft.print(nmestr.c_str());
} 


void displayRDStime(char *tme) {
  RDSprevTime = RDStime;
  myESPboy.tft.fillRect(0, 14, 128, 10, TFT_BLACK),
  myESPboy.tft.setTextSize(1);
  myESPboy.tft.setTextColor(TFT_YELLOW);
  myESPboy.tft.drawString(tme, 50, 14); 
}


void redrawtft(){
//clear TFT
  myESPboy.tft.fillScreen(TFT_BLACK);
//draw ESPboy radio
  myESPboy.tft.setTextSize(1);
  myESPboy.tft.setTextColor(TFT_MAGENTA);
  myESPboy.tft.setCursor(10, 0);
  myESPboy.tft.print(F("ESPboy FM radio v2"));
//draw freq
  myESPboy.tft.setTextSize(3);
  myESPboy.tft.setTextColor(TFT_GREEN);
  uint16_t freq = radio.getFrequency();
  uint16_t fstfreq = freq/100;
  uint16_t scdfreq = freq - fstfreq*100;
  String toPrint = (String)(fstfreq);
  toPrint += ".";
  toPrint += (String)(scdfreq/10);
  myESPboy.tft.drawString(toPrint, 18,30);
//draw Volume
  myESPboy.tft.setTextSize(1);
  myESPboy.tft.setTextColor(TFT_YELLOW);
  myESPboy.tft.setCursor(0, 80);
  myESPboy.tft.print(F("Vol "));
  for (int i=0; i<radio.getVolume(); i++) myESPboy.tft.print("|");
//draw MONO/STEREO
  myESPboy.tft.setCursor(0, 90);
  if (radio.isStereo()) myESPboy.tft.print (F("Stereo")); 
  else myESPboy.tft.print (F("Mono"));
//draw RSSI
   myESPboy.tft.setCursor(0, 100);
   myESPboy.tft.print ("RSSI "); myESPboy.tft.print (radio.getRssi());
//draw RDS
  displayRDStext((char*)RDStext.c_str());
  displayRDStime((char*)RDStime.c_str());
}



void runButtonsCommand(uint8_t bt){
     esp_eeprom_needsaving++;
     myESPboy.playTone(600, 20);
     myESPboy.myLED.setRGB(0,0,10);
     if (bt&PAD_RIGHT) { 
         myESPboy.tft.setTextSize(3);
         myESPboy.tft.setTextColor(TFT_GREEN,TFT_BLACK);
         myESPboy.tft.drawString(">>>>>", 18,30);
         myESPboy.myLED.setRGB(10,0,0);
         radio.seek(SI470X_SEEK_WRAP, SI470X_SEEK_UP);
         myESPboy.myLED.setRGB(0,10,0);
         myESPboy.playTone(1000, 20);
         radio.setRds(true);
         radio.setRdsMode(0);
         RDStext="";
         RDStime="";
         delay(200);
         }
     if (bt&PAD_LEFT) { 
         myESPboy.tft.setTextSize(3);
         myESPboy.tft.setTextColor(TFT_GREEN,TFT_BLACK);
         myESPboy.tft.drawString("<<<<<", 18,30);
         myESPboy.myLED.setRGB(10,0,0);
         radio.seek(SI470X_SEEK_WRAP, SI470X_SEEK_DOWN);
         myESPboy.myLED.setRGB(0,10,0);
         myESPboy.playTone(1000, 20);
         radio.setRds(true);
         radio.setRdsMode(0);
         RDStext="";
         RDStime="";
         delay(200);}
     if (bt&PAD_UP) radio.setVolumeUp();
     if (bt&PAD_DOWN) radio.setVolumeDown();
     if (bt&PAD_ACT) {if(radio.isStereo()) radio.setMono(true); else radio.setMono(false);}
    
     delay(200);
     redrawtft(); 
     myESPboy.myLED.setRGB(0,0,0);
}


boolean esp_eeprom_save(){
  esp_eeprom.freq = radio.getFrequency();
  esp_eeprom.vol = radio.getVolume();
  esp_eeprom.stereo = radio.isStereo();
  EEPROM.put (0, esp_eeprom);
  return (EEPROM.commit());
}


void esp_eeprom_load(){
    EEPROM.get(0, esp_eeprom);
    radio.setFrequency(esp_eeprom.freq);
    if (esp_eeprom.stereo) radio.setMono(false); else radio.setMono(true);
    radio.setVolume(esp_eeprom.vol);
}


void setup() {
 
//Init ESPboy
  myESPboy.begin("FM radio");
  Wire.begin();
  
//detect FMradio module
  pinMode(D8,OUTPUT);
  digitalWrite(D8, HIGH);
  Wire.beginTransmission(I2C_ADDR);
  if (Wire.endTransmission()){
    myESPboy.tft.setTextColor(TFT_RED);
    myESPboy.tft.drawString("FM-radio module", 0, 2);
    myESPboy.tft.drawString("not found!", 0, 12);
    while(1)delay(1000);
  }

//radio init  
  radio.setup(radioRESETpin, radioSDApin);

// Enables SDR
  radio.setRds(true);
  radio.setRdsMode(0); 
  radio.setSeekThreshold(30); // Sets RSSI Seek Threshold (0 to 127)

//load last radio state from eeprom
  EEPROM.begin(sizeof (esp_eeprom));
  esp_eeprom_load();

//draw radio interface
  redrawtft();
}


void loop() {
 static unsigned long cnt_rds, cnt_save;

  uint8_t bt = myESPboy.getKeys();
  if (bt) runButtonsCommand(bt);
  
  if (millis() - cnt_save > MAX_DELAY_SAVE){
     cnt_save = millis();
     if (esp_eeprom_needsaving){
        esp_eeprom_save();
        esp_eeprom_needsaving = 0;
     }
  }

  if (radio.getRdsReady() && (millis() - cnt_rds) > MAX_DELAY_RDS){ 
      cnt_rds = millis();
      RDStime = (String)radio.getRdsTime();
      RDStext = (String)radio.getRdsText();
      if (RDStime != RDSprevTime || RDStext != RDSprevText) redrawtft();
  }

  if(prevRSSI != radio.getRssi()){
   prevRSSI=radio.getRssi();
   myESPboy.tft.setTextColor(TFT_YELLOW, TFT_BLACK);
   String toPrint = (String)radio.getRssi();
   toPrint+="   ";
   myESPboy.tft.drawString(toPrint,5*6,100);
  }
  delay (5);
} 
