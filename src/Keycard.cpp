#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <MFRC522.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SS_PIN   5
#define RST_PIN  4    // use -1 if RST is wired to 3V3

#define GREEN_LED 15
#define RED_LED   2

#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
#define OLED_ADDRESS  0x3C   // try 0x3D if the screen stays blank

// The card that is allowed in
byte ALLOWED_UID[] = {0x00, 0xF5, 0xE2, 0x5C};

MFRC522 mfrc522(SS_PIN, RST_PIN);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

bool isAllowed() {
  if (mfrc522.uid.size != sizeof(ALLOWED_UID)) return false;
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] != ALLOWED_UID[i]) return false;
  }
  return true;
}

void drawCenteredText(const char *text, int y) {
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextSize(2);
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((SCREEN_WIDTH - w) / 2, y);
  display.print(text);
}

void showWelcome() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  // Thick checkmark
  for (int o = -1; o <= 1; o++) {
    display.drawLine(44 + o, 20, 56 + o, 34, SSD1306_WHITE);
    display.drawLine(56 + o, 34, 84 + o, 4, SSD1306_WHITE);
  }
  drawCenteredText("Welcome", 46);
  display.display();
}

void showDenied() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  // Thick X
  for (int o = -1; o <= 1; o++) {
    display.drawLine(50 + o, 4, 78 + o, 36, SSD1306_WHITE);
    display.drawLine(78 + o, 4, 50 + o, 36, SSD1306_WHITE);
  }
  drawCenteredText("Denied", 46);
  display.display();
}

void showReady() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  drawCenteredText("Scan card", 24);
  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);

  Wire.begin(21, 22);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println("OLED not found");
  }

  SPI.begin(18, 19, 23, 5);   // SCK, MISO, MOSI, SS
  mfrc522.PCD_Init();
  delay(50);

  byte v = mfrc522.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.print("Reader version: 0x");
  Serial.println(v, HEX);

  showReady();
}

void loop() {
  if (!mfrc522.PICC_IsNewCardPresent()) return;
  if (!mfrc522.PICC_ReadCardSerial()) return;

  Serial.print("Card UID: ");
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    if (mfrc522.uid.uidByte[i] < 0x10) Serial.print("0");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    if (i < mfrc522.uid.size - 1) Serial.print(":");
  }
  Serial.println();

  if (isAllowed()) {
    Serial.println("ACCESS GRANTED");
    digitalWrite(GREEN_LED, HIGH);
    showWelcome();
  } else {
    Serial.println("ACCESS DENIED");
    digitalWrite(RED_LED, HIGH);
    showDenied();
  }

  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();

  delay(2000);     // keep the result on screen and LED on

  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  showReady();
}