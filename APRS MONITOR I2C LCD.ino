/*
Project: Monitor APRS Signal
By: 9W2KEY
Hardware: I2C LCD display
Koding asal https://github.com/chokelive/aprs_tnc 
*/
#include <LibAPRS.h>
//#include "SSD1306Ascii.h" // OLED Config
//#include "SSD1306AsciiAvrI2c.h" // OLED Config
#include <LiquidCrystal_I2C.h>
#include<Wire.h>
int LED_RX= 8; // rx dan proses LED indicator 

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display

/*  I2C LCD     ARDUINO (NANO/UNO)
    VCC         5V
    GND         GND
    SDA         A4
    SCL         A5
*/

// APRS Configulation
#define CALL_SIGN "9W2KEY" // sila bubuh callsign sendiri
#define CALL_SIGN_SSID 9

#define ADC_REFERENCE REF_5V
#define OPEN_SQUELCH false

// OLED Configulation

//#define I2C_ADDRESS 0x3C
//#define RST_PIN -1
//SSD1306AsciiAvrI2c oled;

// APRS Global Variable
boolean gotPacket = false;
AX25Msg incomingPacket;
uint8_t *packetData;


void setup() {
  // Set up serial port
  pinMode(LED_RX, OUTPUT);
  Serial.begin(115200);

  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  // Initialise APRS library - This starts the modem
  APRS_init(ADC_REFERENCE, OPEN_SQUELCH);
  APRS_setCallsign(CALL_SIGN, CALL_SIGN_SSID);
  APRS_printSettings();
  Serial.print(F("Free RAM:     ")); Serial.println(freeMemory());
/*
#if RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
#else // RST_PIN >= 0
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
#endif // RST_PIN >= 0

  oled.setFont(Adafruit5x7);
  oled.clear();
  oled.set1X();
  oled.println(F("  APRS Decoder V1.0"));
  oled.println(F("by 9W2KEY (Dec, 2022)"));
  oled.println(F("  mzakiab@gmail.com"));
  oled.println(F("~~~~~~~~~~~~~~~~~~~~~"));
  oled.println(F("Waiting for signal..."));
  oled.println(F("Make sure radio is ON"));
  oled.println(F("ATN is good and ready"));
  oled.println(F("   to RX RF signal   "));
  */
  lcd.setCursor(0,0); 
  lcd.print("APRS Decoder LCD");
  lcd.setCursor(4,1);
  lcd.print("by 9W2KEY");
}


void loop() {
  
  delay(500);
  processPacket();
}


void processPacket() {
  char sentence[150];
  if (gotPacket) {
    gotPacket = false;
    
    Serial.print(F("Received APRS packet. SRC: "));
    Serial.print(incomingPacket.src.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.src.ssid);
    Serial.print(F(". DST: "));
    Serial.print(incomingPacket.dst.call);
    Serial.print(F("-"));
    Serial.print(incomingPacket.dst.ssid);
    Serial.print(F(". Data: "));

    for (int i = 0; i < incomingPacket.len; i++) {
      Serial.write(incomingPacket.info[i]);
    }
    Serial.println("");

    digitalWrite(LED_RX, HIGH); // ON LED 
    delay(500); // tunggu kejap nak proses data yg masuk
    lcd.clear(); // clear display dulu
    lcd.setCursor(0, 0); 
    lcd.print(incomingPacket.src.call);
    lcd.print(F("-"));
    lcd.print(incomingPacket.src.ssid);
    lcd.print(F(" >"));
    lcd.print(incomingPacket.dst.call);
    //lcd.println();
    //lcd.set1X();
    //lcd.println();
    lcd.setCursor(0, 1);
    for (int i = 0; i < incomingPacket.len; i++) {
    if(i%20==0) lcd.println();
      lcd.write(incomingPacket.info[i]);
      lcd.blink(); // cursor kelip kelip
    }
    // lcd.println("");
    digitalWrite(LED_RX, LOW); // OFF LED
    free(packetData);

    // Serial.print(F("Free RAM: ")); Serial.println(freeMemory());
  }
}

void aprs_msg_callback(struct AX25Msg *msg) {
  if (!gotPacket) {
    gotPacket = true;

    memcpy(&incomingPacket, msg, sizeof(AX25Msg));

    if (freeMemory() > msg->len) {
      packetData = (uint8_t*)malloc(msg->len);
      memcpy(packetData, msg->info, msg->len);
      incomingPacket.info = packetData;
    } else {
      // We did not have enough free RAM to receive
      // this packet, so we drop it.
      gotPacket = false;
    }
  }
}