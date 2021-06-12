/*
ESPboy FMradio module firmware. Using SI4705 chipset
www.espboy.com project
by RomanS 12.06.2021
Install to your Arduino IDE "ESPboyRadio_lib" library from the "lib" folder
*/


#include "lib/ESPboyInit.h"
#include "lib/ESPboyInit.cpp"
#include "radio.h"
#include "SI4703.h"
#include "RDSParser.h"
#include <ESP_EEPROM.h>

#define radioSDApin       SDA 
#define radioRESETpin     3


struct ESP_EEPROM{
  uint16_t freq;
  uint8_t vol;
  uint8_t stereo;
  uint8_t bass;
};


ESP_EEPROM esp_eeprom;
static uint8_t esp_eeprom_needsaving = 0;

ESPboyInit myESPboy;
SI4703 radio(radioSDApin, radioRESETpin);    
RDSParser rds;
RADIO_INFO radioinfo;

void RDS_process(uint16_t block1, uint16_t block2, uint16_t block3, uint16_t block4) {
  rds.processData(block1, block2, block3, block4);
}


void DisplayServiceName(char *name){
  if (name[0]){
    myESPboy.tft.fillRect(0, 58, 128, 16, TFT_BLACK);
    myESPboy.tft.setTextSize(2);
    myESPboy.tft.setTextColor(TFT_RED);
    myESPboy.tft.setCursor((128-strlen(name)*12)/2, 58);
    myESPboy.tft.print(name);
  }
} 


void DisplayTime(uint8_t hour, uint8_t minute) {
  myESPboy.tft.fillRect(0, 14, 128, 10, TFT_BLACK),
  myESPboy.tft.setTextSize(1);
  myESPboy.tft.setTextColor(TFT_YELLOW);
  String toPrint="";
  if (hour < 10) toPrint+="0";
  toPrint+=hour;
  toPrint+=":";
  if (minute < 10) toPrint+="0";
  toPrint+=minute;
  myESPboy.tft.drawString(toPrint, 50, 14); 
}


void redrawtft(){
 uint8_t part1freq, part2freq;
 uint16_t freq;
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
  freq = radio.getFrequency();
  part1freq = radio.getFrequency()/100;
  part2freq = freq - part1freq * 100;
  if (part2freq > 99) part2freq /= 10;
  if (part2freq > 9) part2freq /= 10;
  if (part1freq >99) myESPboy.tft.setCursor(18, 30);
  else myESPboy.tft.setCursor(28, 30);
  myESPboy.tft.print(part1freq);
  myESPboy.tft.print(".");
  myESPboy.tft.print(part2freq); 
//draw Vol
  myESPboy.tft.setTextSize(1);
  myESPboy.tft.setTextColor(TFT_YELLOW);
  myESPboy.tft.setCursor(0, 80);
  myESPboy.tft.print("Vol ");
  for (int i=0; i<radio.getVolume(); i++) myESPboy.tft.print("|");
//draw MONO
  myESPboy.tft.setCursor(0, 90);
  if (!radio.getMono()) myESPboy.tft.print ("Stereo"); else myESPboy.tft.print ("Mono");
//draw BASS
  myESPboy.tft.setCursor(0, 100);
  if (radio.getBassBoost()) myESPboy.tft.print ("Bass on"); else myESPboy.tft.print ("Bass off");
//draw RSSI
   myESPboy.tft.setCursor(0, 110);
   radio.getRadioInfo(&radioinfo); 
   myESPboy.tft.print ("RSSI "); myESPboy.tft.print (radioinfo.rssi);
//draw RDS
   radio.checkRDS();
}



void runButtonsCommand(uint8_t bt){
     esp_eeprom_needsaving++;
     myESPboy.playTone(600, 20);
     myESPboy.myLED.setRGB(0,0,10);
     if (bt&PAD_RIGHT) { 
         myESPboy.myLED.setRGB(10,0,0);
         radio.seekUp(false); 
         myESPboy.myLED.setRGB(0,10,0);
         myESPboy.playTone(1000, 20);
         delay(200);
         }
     if (bt&PAD_LEFT) { 
         myESPboy.myLED.setRGB(10,0,0);
         radio.seekDown(false); 
         myESPboy.myLED.setRGB(0,10,0);
         myESPboy.playTone(1000, 20);
         delay(200);}
     if (bt&PAD_UP) { if (radio.getVolume() < 15) radio.setVolume(radio.getVolume() + 1); }
     if (bt&PAD_DOWN) { if (radio.getVolume() > 0) radio.setVolume(radio.getVolume() - 1); }
     if (bt&PAD_ACT) { 
         radio.setMono(! radio.getMono()); }
     if (bt&PAD_ESC) { 
         radio.setBassBoost(!radio.getBassBoost()); }
    
     delay(200);
     redrawtft(); 
     myESPboy.myLED.setRGB(0,0,0);
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
 
//Init ESPboy
  myESPboy.begin("FM radio");

//radio init  
  radio.init();
  delay (100);
  radio.attachReceiveRDS(RDS_process);
  rds.attachServicenNameCallback(DisplayServiceName);
  rds.attachTimeCallback(DisplayTime);

//load last radio state from eeprom
  EEPROM.begin(sizeof (esp_eeprom));
  esp_eeprom_load();

//draw radio interface
  redrawtft();
}


void loop() {
 static unsigned long counter;

  radio.checkRDS();
  uint8_t bt = myESPboy.getKeys();
  Serial.println(bt);
  if (bt) runButtonsCommand(bt);
  if (millis() > (counter + 2000)){
     counter = millis();
     if (esp_eeprom_needsaving){
        esp_eeprom_save();
        esp_eeprom_needsaving = 0;
     }
  }
  
  delay (100);
} 
