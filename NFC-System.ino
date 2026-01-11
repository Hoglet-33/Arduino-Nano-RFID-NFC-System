#include <Wire.h>
#include <Adafruit_PN532.h>
#include <EEPROM.h>

#define PN532_IRQ   2
#define PN532_RESET 3

#define GREEN_LED 6
#define RED_LED   7
#define BUZZER_PIN 9
#define PROGRAM_BUTTON 4
#define LED_PIN 13

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

// -------- Alarm system --------
unsigned long alarmStart = 0;
bool alarmActive = false;

// -------- Brute-force protection --------
uint8_t lastBadUID[7];
uint8_t lastBadUIDLen = 0;
int badCount = 0;

// -------- Sound effects --------

void authorizedBeep() {
  int melody[] = {2000, 2600, 3200, 3800};
  for (int i = 0; i < 4; i++) {
    tone(BUZZER_PIN, melody[i]);
    delay(80);
  }
  noTone(BUZZER_PIN);
}

void deniedBeep() {
  tone(BUZZER_PIN, 500);
  delay(200);
  noTone(BUZZER_PIN);
}

void savedBeep() {
  int melody[] = {3000, 3400, 3800};
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, melody[i]);
    delay(80);
  }
  noTone(BUZZER_PIN);
}

void deletedBeep() {
  tone(BUZZER_PIN, 1200);
  delay(120);
  tone(BUZZER_PIN, 600);
  delay(200);
  noTone(BUZZER_PIN);
}

void programmingJingle() {
  int melody[] = {1200, 1600, 2000};
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, melody[i]);
    delay(100);
  }
  noTone(BUZZER_PIN);
}

void alarmMode() {
  for (int i = 0; i < 15; i++) {
    digitalWrite(RED_LED, HIGH);
    tone(BUZZER_PIN, 800 + (i % 2) * 1200);
    delay(150);
    digitalWrite(RED_LED, LOW);
    tone(BUZZER_PIN, 2000 - (i % 2) * 1000);
    delay(150);
  }
  noTone(BUZZER_PIN);
}

// -------- Helper: Compare two UIDs --------
bool sameUID(uint8_t *a, uint8_t alen, uint8_t *b, uint8_t blen) {
  if (alen != blen) return false;
  for (int i = 0; i < alen; i++) {
    if (a[i] != b[i]) return false;
  }
  return true;
}

// -------- EEPROM Tag Database --------

int findTag(uint8_t *uid, uint8_t uidLength) {
  int i = 0;
  while (i < EEPROM.length()) {
    byte len = EEPROM.read(i);
    if (len == 0xFF) return -1;

    if (len == uidLength) {
      bool match = true;
      for (int j = 0; j < len; j++) {
        if (EEPROM.read(i + 1 + j) != uid[j]) {
          match = false;
          break;
        }
      }
      if (match) return i;
    }
    i += 1 + len;
  }
  return -1;
}

void saveTag(uint8_t *uid, uint8_t uidLength) {
  int i = 0;
  while (i < EEPROM.length()) {
    if (EEPROM.read(i) == 0xFF) {
      EEPROM.write(i, uidLength);
      for (int j = 0; j < uidLength; j++) {
        EEPROM.write(i + 1 + j, uid[j]);
      }
      EEPROM.write(i + 1 + uidLength, 0xFF);
      return;
    }
    i += 1 + EEPROM.read(i);
  }
}

void deleteTag(int index) {
  int len = EEPROM.read(index);
  int next = index + 1 + len;
  while (next < EEPROM.length()) {
    EEPROM.write(index++, EEPROM.read(next++));
  }
  EEPROM.write(index, 0xFF);
}

// -------- Setup --------

void setup() {
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(PROGRAM_BUTTON, INPUT_PULLUP);
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  Wire.begin();

  nfc.begin();
  if (!nfc.getFirmwareVersion()) {
    Serial.println("PN532 not found");
    while (1);
  }

  nfc.SAMConfig();
  Serial.println("NFC Access System Ready");
  Serial.println("Hold button + scan to add/delete tags");
}

// -------- Main Loop --------

void loop() {
  if (alarmActive) {
    if (millis() - alarmStart > 10000) {
      alarmActive = false;
      badCount = 0;
      lastBadUIDLen = 0;
      Serial.println("Alarm cleared");
    } else {
      return;
    }
  }

  uint8_t uid[7];
  uint8_t uidLength;

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {

    int tagIndex = findTag(uid, uidLength);

    if (digitalRead(PROGRAM_BUTTON) == LOW) {
      digitalWrite(RED_LED, HIGH);
      programmingJingle();

      if (tagIndex == -1) {
        Serial.println("Adding tag");
        saveTag(uid, uidLength);
        savedBeep();
      } else {
        Serial.println("Deleting tag");
        deleteTag(tagIndex);
        deletedBeep();
      }

      for (int i = 0; i < 3; i++) {
        digitalWrite(GREEN_LED, HIGH);
        delay(150);
        digitalWrite(GREEN_LED, LOW);
        delay(150);
      }

      digitalWrite(RED_LED, LOW);
      badCount = 0;           // Reset bad count after programming
      lastBadUIDLen = 0;
    }
    else {
      if (tagIndex != -1) {
        Serial.println("AUTHORIZED");
        authorizedBeep();
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(LED_PIN, HIGH);
        delay(300);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(LED_PIN, LOW);

        badCount = 0;
        lastBadUIDLen = 0;
      } else {
        Serial.println("NOT AUTHORIZED");

        digitalWrite(RED_LED, HIGH);
        deniedBeep();
        delay(300);
        digitalWrite(RED_LED, LOW);

        if (sameUID(uid, uidLength, lastBadUID, lastBadUIDLen)) {
          badCount++;
        } else {
          // Different UID scanned, reset count
          badCount = 1;
          for (int i = 0; i < uidLength; i++) lastBadUID[i] = uid[i];
          lastBadUIDLen = uidLength;
        }

        Serial.print("Bad scan count for this UID: ");
        Serial.println(badCount);

        if (badCount == 2) {
          Serial.println("Warning beep");
          // Angry warning beep (two short beeps)
          tone(BUZZER_PIN, 1000);
          delay(100);
          noTone(BUZZER_PIN);
          delay(100);
          tone(BUZZER_PIN, 1000);
          delay(100);
          noTone(BUZZER_PIN);
        } else if (badCount >= 3) {
          Serial.println("!!! ALARM MODE !!!");
          alarmActive = true;
          alarmStart = millis();
          alarmMode();
        }
      }
    }

    delay(1200);
  }
}
