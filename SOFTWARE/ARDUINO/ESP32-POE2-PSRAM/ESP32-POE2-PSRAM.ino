/*
  ESP32-POE2 PSRAM Demonstration
  -------------------------------------------
  This sketch:
   - Checks if PSRAM is available
   - Prints PSRAM and heap stats every 15 seconds
   - Demonstrates storing and retrieving data in PSRAM

  Tested on: OLIMEX ESP32-POE2 (ESP32-WROVER-E-N4R8)
  Board: "ESP32 Dev Module" or "OLIMEX ESP32-POE"
  Remember to enable PSRAM - Tools → PSRAM → Enabled
  Remember to select serial port of the device
*/

#define REPORT_INTERVAL 15000  // milliseconds
#define BUFFER_SIZE (1024 * 50) // 50 KB for demo

uint8_t *psramBuffer = nullptr;
unsigned long reportCounter = 0;

void printMemoryReport() {
  Serial.println("\n--------------------------------------------");
  Serial.print("Memory Report #");
  Serial.println(++reportCounter);
  Serial.println("--------------------------------------------");

  if (psramFound()) {
    Serial.println("* PSRAM detected and enabled");
    Serial.print("  Total PSRAM: ");
    Serial.print(ESP.getPsramSize() / 1024);
    Serial.println(" KB");

    Serial.print("  Free PSRAM:  ");
    Serial.print(ESP.getFreePsram() / 1024);
    Serial.println(" KB");
  } else {
    Serial.println("! PSRAM not found or not enabled");
  }

  Serial.println("\nHeap info:");
  Serial.print("  Total Heap: ");
  Serial.print(ESP.getHeapSize() / 1024);
  Serial.println(" KB");

  Serial.print("  Free Heap:  ");
  Serial.print(ESP.getFreeHeap() / 1024);
  Serial.println(" KB");

  Serial.println("--------------------------------------------\n");
}

void storeAndRetrieveDemo() {
  Serial.println("Demonstrating PSRAM data storage...");

  // Allocate buffer in PSRAM
  psramBuffer = (uint8_t *)ps_malloc(BUFFER_SIZE);
  if (psramBuffer == nullptr) {
    Serial.println("! Failed to allocate PSRAM buffer");
    return;
  }

  Serial.print("* Allocated ");
  Serial.print(BUFFER_SIZE / 1024);
  Serial.println(" KB buffer in PSRAM");

  // Fill buffer with a recognizable pattern
  for (size_t i = 0; i < BUFFER_SIZE; i++) {
    psramBuffer[i] = (i % 256);
  }
  Serial.println("* Wrote test pattern to PSRAM");

  // Show first 16 bytes
  Serial.print("  First 16 bytes: ");
  for (int i = 0; i < 16; i++) {
    Serial.printf("%02X ", psramBuffer[i]);
  }
  Serial.println();

  // Verify data
  bool valid = true;
  for (size_t i = 0; i < BUFFER_SIZE; i++) {
    if (psramBuffer[i] != (i % 256)) {
      valid = false;
      break;
    }
  }

  if (valid)
    Serial.println("* Data verified correctly in PSRAM");
  else
    Serial.println("! Data mismatch detected in PSRAM");

  // Release buffer
  free(psramBuffer);
  psramBuffer = nullptr;
  Serial.println("* PSRAM buffer released\n");
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); } // Wait for Serial Monitor
  delay(1000);

  Serial.println("\n=== ESP32-POE2 PSRAM Test ===");

  if (!psramFound()) {
    Serial.println("! PSRAM not detected. Check Tools → PSRAM → Enabled");
  } else {
    Serial.println("* PSRAM detected successfully");
  }

  // Immediate visible output before loop
  printMemoryReport();
  storeAndRetrieveDemo();
}

void loop() {
  delay(REPORT_INTERVAL);
  printMemoryReport();
  storeAndRetrieveDemo();
}
