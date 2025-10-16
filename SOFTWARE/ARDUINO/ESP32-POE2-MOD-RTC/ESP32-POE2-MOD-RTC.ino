/*
   Olimex ESP32-POE2 + MOD-RTC (PCF8563T) Arduino IDE Demo
   ------------------------------------------------------
   ESP32-POE2 uses WROVER module, make sure to select PSRAM enabled (WROOM) from Tools

   After downloading to the board, go to Tools -> Serial Monitor and set baud to 115200
   
   Features:
     • Continuously reads and prints current date/time from the RTC every 1s.
     • Short press BUT1 → Syncs RTC with sketch compile time.
     • Long press BUT1 (5 sec) → Enter manual time setup via Serial.
   ------------------------------------------------------
   Hardware connections:
     SDA = GPIO13
     SCL = GPIO33
     BUT1 = GPIO34 (input only, active LOW)
*/

#include <Wire.h>

#define PCF8563_ADDR 0x51
#define SDA_PIN 13
#define SCL_PIN 33
#define BUTTON_PIN 34
#define LONG_PRESS_MS 5000
#define BUTTON_POLL_MS 50
#define PRINT_INTERVAL_MS 1000

uint8_t dec2bcd(uint8_t val) { return ((val / 10) << 4) | (val % 10); }
uint8_t bcd2dec(uint8_t val) { return (val & 0x0F) + ((val >> 4) * 10); }

unsigned long lastButtonPoll = 0;
unsigned long lastPrintTime = 0;
unsigned long buttonPressedTime = 0;
bool buttonHeld = false;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("\n=== Olimex ESP32-POE-2 + MOD-RTC (PCF8563) Demo ===");

  pinMode(BUTTON_PIN, INPUT);
  Wire.begin(SDA_PIN, SCL_PIN);

  Wire.beginTransmission(PCF8563_ADDR);
  if (Wire.endTransmission() != 0) {
    Serial.println("⚠️  PCF8563 not detected at address 0x51!");
    while (true);
  }
  Serial.println("RTC detected successfully.\n");
}

void loop() {
  unsigned long now = millis();

  // --- Non-blocking button handler (every 50 ms) ---
  if (now - lastButtonPoll >= BUTTON_POLL_MS) {
    handleButton();
    lastButtonPoll = now;
  }

  // --- Non-blocking RTC print (every 1 second) ---
  if (now - lastPrintTime >= PRINT_INTERVAL_MS) {
    printRTC();
    lastPrintTime = now;
  }
}

// ---------------------------------------------------------------------------
// BUTTON HANDLER
// ---------------------------------------------------------------------------
void handleButton() {
  int btnState = digitalRead(BUTTON_PIN);

  if (btnState == LOW) { // pressed
    if (buttonPressedTime == 0)
      buttonPressedTime = millis();

    if (!buttonHeld && (millis() - buttonPressedTime > LONG_PRESS_MS)) {
      buttonHeld = true;
      Serial.println("\n=== LONG PRESS detected ===");
      manualSetRTC();
    }
  } else { // released
    if (buttonPressedTime > 0 && !buttonHeld) {
      unsigned long pressDuration = millis() - buttonPressedTime;
      if (pressDuration < LONG_PRESS_MS && pressDuration > 30) {
        Serial.println("\n=== SHORT PRESS detected ===");
        setRTCtoCompileTime();
      }
    }
    buttonPressedTime = 0;
    buttonHeld = false;
  }
}

// ---------------------------------------------------------------------------
// SET RTC TO COMPILE TIME
// ---------------------------------------------------------------------------
void setRTCtoCompileTime() {
  const char *timeStr = __TIME__;
  const char *dateStr = __DATE__;

  uint8_t hour = (timeStr[0] - '0') * 10 + (timeStr[1] - '0');
  uint8_t min  = (timeStr[3] - '0') * 10 + (timeStr[4] - '0');
  uint8_t sec  = (timeStr[6] - '0') * 10 + (timeStr[7] - '0');

  char monthStr[4];
  memcpy(monthStr, dateStr, 3);
  monthStr[3] = '\0';
  const char *months = "JanFebMarAprMayJunJulAugSepOctNovDec";
  uint8_t mon = (strstr(months, monthStr) - months) / 3 + 1;
  uint8_t day = (dateStr[4] == ' ') ? (dateStr[5] - '0') : ((dateStr[4] - '0') * 10 + (dateStr[5] - '0'));
  uint16_t year = atoi(dateStr + 7) - 2000;

  writeRTC(sec, min, hour, day, mon, year);
  Serial.printf("RTC synced to compile time: %04u-%02u-%02u %02u:%02u:%02u\n\n",
                2000 + year, mon, day, hour, min, sec);
}

// ---------------------------------------------------------------------------
// MANUAL TIME SETUP VIA SERIAL
// ---------------------------------------------------------------------------
void manualSetRTC() {
  Serial.println("Enter new date and time in the following format:");
  Serial.println("YYYY-MM-DD HH:MM:SS");
  Serial.print("Example: 2025-10-16 18:45:00\n> ");

  String input = "";
  unsigned long timeout = millis();
  while ((millis() - timeout) < 60000) {
    if (Serial.available()) {
      input = Serial.readStringUntil('\n');
      input.trim();
      break;
    }
  }

  if (input.length() != 19) {
    Serial.println("Invalid format or timeout. Aborting.\n");
    return;
  }

  uint16_t year = input.substring(0, 4).toInt() - 2000;
  uint8_t mon   = input.substring(5, 7).toInt();
  uint8_t day   = input.substring(8, 10).toInt();
  uint8_t hour  = input.substring(11, 13).toInt();
  uint8_t min   = input.substring(14, 16).toInt();
  uint8_t sec   = input.substring(17, 19).toInt();

  if (mon < 1 || mon > 12 || day < 1 || day > 31 || hour > 23 || min > 59 || sec > 59) {
    Serial.println("Invalid values entered. Aborting.\n");
    return;
  }

  writeRTC(sec, min, hour, day, mon, year);
  Serial.printf("RTC manually set to: %04u-%02u-%02u %02u:%02u:%02u\n\n",
                2000 + year, mon, day, hour, min, sec);
}

// ---------------------------------------------------------------------------
// WRITE TO RTC
// ---------------------------------------------------------------------------
void writeRTC(uint8_t sec, uint8_t min, uint8_t hour, uint8_t day, uint8_t mon, uint16_t year) {
  Wire.beginTransmission(PCF8563_ADDR);
  Wire.write(0x02);
  Wire.write(dec2bcd(sec) & 0x7F);
  Wire.write(dec2bcd(min) & 0x7F);
  Wire.write(dec2bcd(hour) & 0x3F);
  Wire.write(dec2bcd(day) & 0x3F);
  Wire.write(0x00); // weekday = 0
  Wire.write(dec2bcd(mon) & 0x1F);
  Wire.write(dec2bcd(year));
  Wire.endTransmission();
}

// ---------------------------------------------------------------------------
// READ AND PRINT CURRENT RTC TIME
// ---------------------------------------------------------------------------
void printRTC() {
  Wire.beginTransmission(PCF8563_ADDR);
  Wire.write(0x02);
  Wire.endTransmission();

  Wire.requestFrom(PCF8563_ADDR, 7);
  if (Wire.available() < 7) return;

  uint8_t sec_bcd = Wire.read();
  uint8_t min_bcd = Wire.read();
  uint8_t hr_bcd  = Wire.read();
  uint8_t day_bcd = Wire.read();
  Wire.read(); // weekday
  uint8_t mon_bcd = Wire.read();
  uint8_t yr_bcd  = Wire.read();

  uint8_t sec = bcd2dec(sec_bcd & 0x7F);
  uint8_t min = bcd2dec(min_bcd & 0x7F);
  uint8_t hr  = bcd2dec(hr_bcd & 0x3F);
  uint8_t day = bcd2dec(day_bcd & 0x3F);
  uint8_t mon = bcd2dec(mon_bcd & 0x1F);
  uint16_t yr = 2000 + bcd2dec(yr_bcd);

  Serial.printf("%04u-%02u-%02u %02u:%02u:%02u\n", yr, mon, day, hr, min, sec);
}
