#include "Arduino.h"
#include <Wire.h>
#include "HT_SSD1306Wire.h"
#include "LoRaWan_APP.h"
#include "FS.h"
#include "LittleFS.h"

// ================== OLED SETUP ==================
#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
#endif

void VextON(void) { pinMode(Vext, OUTPUT); digitalWrite(Vext, LOW); }
void VextOFF(void) { pinMode(Vext, OUTPUT); digitalWrite(Vext, HIGH); }

// ================== LORA CONFIG ==================
#define RF_FREQUENCY            915000000 // Hz
#define TX_OUTPUT_POWER         5         // dBm
#define LORA_BANDWIDTH          0         // [0: 125 kHz]
#define LORA_SPREADING_FACTOR   7         // [SF7..SF12]
#define LORA_CODINGRATE         1         // [1: 4/5]
#define LORA_PREAMBLE_LENGTH    8
#define LORA_FIX_LENGTH_PAYLOAD false
#define LORA_IQ_INVERSION_ON    false
#define BUFFER_SIZE             30        // Payload size

char txpacket[BUFFER_SIZE + 1];
bool lora_idle = true;

static RadioEvents_t RadioEvents;
void OnTxDone(void);
void OnTxTimeout(void);

// ================== DISPLAY HELPER ==================
void display_tx_status(const char* packet, bool success) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  
  display.drawString(0, 0, success ? "Status: TX Done" : "Status: TX Timeout");
  display.drawString(0, 14, "Packet:");
  
  String packetStr = String(packet);
  int maxLen = 30;
  display.drawString(0, 28, packetStr.substring(0, maxLen));
  if (packetStr.length() > maxLen)
    display.drawString(0, 42, packetStr.substring(maxLen));
  
  display.display();
}

// ================== GLOBAL FILE BUFFER ==================
File msgFile;
bool fileEnded = false;

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  // --- OLED Setup ---
  VextON();
  delay(200);
  display.init();
  display.clear();
  display.setFont(ArialMT_Plain_16);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.drawString(display.getWidth()/2, display.getHeight()/2 - 8, "LoRa File Sender");
  display.display();
  delay(1500);

  // --- Mount LittleFS ---
  if (!LittleFS.begin()) {
    Serial.println("❌ LittleFS Mount Failed!");
    display.clear();
    display.drawString(0, 0, "LittleFS Mount Failed!");
    display.display();
    while (true);
  }
  Serial.println("✅ LittleFS mounted successfully!");

  // --- Open message.txt ---
  msgFile = LittleFS.open("/message.txt", "r");
  if (!msgFile) {
    Serial.println("⚠️  /message.txt not found!");
    display.clear();
    display.drawString(0, 0, "message.txt not found!");
    display.display();
    while (true);
  }
  if (msgFile.size() == 0) {
    Serial.println("⚠️  message.txt is empty!");
    display.clear();
    display.drawString(0, 0, "message.txt empty!");
    display.display();
    while (true);
  }

  Serial.printf("✅ Opened /message.txt (%u bytes)\n", msgFile.size());

  // --- LoRa Setup ---
  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetTxConfig(MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                    LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                    LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD,
                    true, 0, 0, LORA_IQ_INVERSION_ON, 3000);
}

// ================== LOOP ==================
void loop() {
  if (lora_idle && !fileEnded) {
    // Read next packet-sized chunk
    int i = 0;
    while (msgFile.available() && i < BUFFER_SIZE) {
      txpacket[i++] = msgFile.read();
    }
    txpacket[i] = '\0';

    if (i == 0) {
      fileEnded = true;
      Serial.println("✅ File transmission complete!");
      display.clear();
      display.drawString(0, 0, "File sent!");
      display.display();
      msgFile.close();
      return;
    }

    Serial.printf("\r\nSending packet: \"%s\" (len=%d)\r\n", txpacket, i);

    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);
    display.drawString(0, 0, "Status: Sending...");
    display.drawString(0, 14, "Packet:");
    String s = String(txpacket);
    int maxLen = 30;
    display.drawString(0, 28, s.substring(0, maxLen));
    if (s.length() > maxLen)
      display.drawString(0, 42, s.substring(maxLen));
    display.display();

    Radio.Send((uint8_t *)txpacket, strlen(txpacket));
    lora_idle = false;
  }

  Radio.IrqProcess();
}

// ================== CALLBACKS ==================
void OnTxDone(void) {
  Serial.println("✅ TX done");
  display_tx_status(txpacket, true);
  lora_idle = true;
}

void OnTxTimeout(void) {
  Serial.println("⚠️ TX timeout");
  Radio.Sleep();
  display_tx_status(txpacket, false);
  lora_idle = true;
}