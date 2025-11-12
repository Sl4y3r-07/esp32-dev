#include <Arduino.h>
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "FS.h"
#include "LittleFS.h"

#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
#endif

void VextON(void) { pinMode(Vext, OUTPUT); digitalWrite(Vext, LOW); }
void VextOFF(void) { pinMode(Vext, OUTPUT); digitalWrite(Vext, HIGH); }

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("\n=== Heltec ESP32 OLED File Display (LittleFS) ===");

  VextON();
  delay(200);

  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.display();

  // Mount LittleFS
  if (!LittleFS.begin()) {
    Serial.println(" LittleFS Mount Failed!");
    display.drawString(0, 0, "LittleFS Mount Failed!");
    display.display();
    return;
  }
  Serial.println(" LittleFS mounted successfully!\n");


  Serial.println(" Files on LittleFS:");
  File root = LittleFS.open("/");
  File file = root.openNextFile();
  while (file) {
    Serial.printf("  %s\t%u bytes\n", file.name(), file.size());
    file = root.openNextFile();
  }


  File msgFile = LittleFS.open("/message.txt", "r");
  if (!msgFile) {
    Serial.println(" /message.txt not found!");
    display.clear();
    display.drawString(0, 0, "message.txt not found!");
    display.display();
    return;
  }

  if (msgFile.size() == 0) {
    Serial.println(" message.txt is empty!");
    display.clear();
    display.drawString(0, 0, "message.txt empty!");
    display.display();
    msgFile.close();
    return;
  }

  Serial.println("\n Reading /message.txt...");
  display.clear();

  // Print raw file content to Serial
  Serial.println("------ FILE CONTENT ------");
  while (msgFile.available()) {
    Serial.write(msgFile.read());
  }
  Serial.println("\n--------------------------");

  // Rewind for OLED display
  msgFile.seek(0);

  int lineNum = 0;
  String line = "";

  while (msgFile.available()) {
    char c = msgFile.read();
    if (c == '\n' || c == '\r') {
      if (line.length() > 0) {
        display.drawString(0, lineNum * 10, line);
        line = "";
        lineNum++;
        if (lineNum >= 6) break; 
      }
    } else {
      line += c;
    }
  }

  if (line.length() > 0 && lineNum < 6) {
    display.drawString(0, lineNum * 10, line);
  }

  display.display();
  msgFile.close();

  Serial.println("File read complete!");
  Serial.println(" Displaying content for 10 seconds...");
  delay(10000);

  display.clear();
  display.display();
  Serial.println("Display cleared after 10 seconds.");
}

void loop() {}