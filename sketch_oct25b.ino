#include <SPI.h>
#include <MFRC522.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

#define SS_PIN 10
#define RST_PIN 9

#define RED_LED 2
#define GREEN_LED 3
#define BUZZER 8

MFRC522 rfid(SS_PIN, RST_PIN);
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

// Student Database
struct Student {
  String cardID;
  String name;
};

Student students[] = {
  {"5E 03 FD 06", "Irfan"},
  {"DC FC 0A E1", "Nur"},
  {"DB C2 E9 06", "Alam"},
  {"34 78 2B 07", "Thank You"}
};

const int totalStudents = sizeof(students) / sizeof(students[0]);

void setup() {

  Serial.begin(9600);

  SPI.begin();
  rfid.PCD_Init();

  lcd.init();
  lcd.backlight();

  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);

  // RTC Initialization
  if (!rtc.begin()) {
    Serial.println("RTC NOT FOUND");
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RTC NOT FOUND");
    while (1);
  }

  // SET RTC TIME ONCE THEN COMMENT THIS LINE
  //rtc.adjust(DateTime(2026, 6, 23, 22, 40,30));

  showWelcomeMessage();

  Serial.println("RFID Attendance System Ready");
  Serial.println("Scan Your Card...");
}

void loop() {

  if (!rfid.PICC_IsNewCardPresent()) {
    return;
  }

  if (!rfid.PICC_ReadCardSerial()) {
    return;
  }

  String cardID = "";

  for (byte i = 0; i < rfid.uid.size; i++) {

    if (rfid.uid.uidByte[i] < 0x10) {
      cardID += "0";
    }

    cardID += String(rfid.uid.uidByte[i], HEX);

    if (i < rfid.uid.size - 1) {
      cardID += " ";
    }
  }

  cardID.toUpperCase();

  Serial.print("Card UID: ");
  Serial.println(cardID);

  bool found = false;

  for (int i = 0; i < totalStudents; i++) {

    if (cardID == students[i].cardID) {

      found = true;

      DateTime now = rtc.now();

      char timeBuffer[9];
      sprintf(timeBuffer,
              "%02d:%02d:%02d",
              now.hour(),
              now.minute(),
              now.second());

      // Green LED ON
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);

      // Success Beep
      tone(BUZZER, 1000);
      delay(200);
      noTone(BUZZER);

      Serial.print("Attendance Marked: ");
      Serial.println(students[i].name);

      Serial.print("Time: ");
      Serial.println(timeBuffer);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Welcome");

      lcd.setCursor(0, 1);
      lcd.print(students[i].name);

      delay(2000);

      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Present");

      lcd.setCursor(0, 1);
      lcd.print(timeBuffer);

      delay(3000);

      digitalWrite(GREEN_LED, LOW);

      break;
    }
  }

  if (!found) {

    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);

    // Error Beeps
    tone(BUZZER, 500);
    delay(300);
    noTone(BUZZER);

    delay(100);

    tone(BUZZER, 500);
    delay(300);
    noTone(BUZZER);

    Serial.println("Unauthorized Card");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Unknown Card");

    lcd.setCursor(0, 1);
    lcd.print("Access Denied");

    delay(2000);

    digitalWrite(RED_LED, LOW);
  }

  showWelcomeMessage();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

void showWelcomeMessage() {

  DateTime now = rtc.now();

  char timeBuffer[9];
  sprintf(timeBuffer,
          "%02d:%02d:%02d",
          now.hour(),
          now.minute(),
          now.second());

  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("RFID Attendance");

  lcd.setCursor(0, 1);
  lcd.print(timeBuffer);
}