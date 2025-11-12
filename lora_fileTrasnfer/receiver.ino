#include "LoRaWan_APP.h"
#include "Arduino.h"
#include "HT_SSD1306Wire.h"

// ===== OLED Display Setup =====
#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#else
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
#endif

void VextON(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, LOW);
}

void VextOFF(void) {
  pinMode(Vext, OUTPUT);
  digitalWrite(Vext, HIGH);
}

// ===== LoRa Config =====
#define RF_FREQUENCY                915000000 // Must match transmitter
#define LORA_BANDWIDTH              0         // 125 kHz
#define LORA_SPREADING_FACTOR       7         // SF7
#define LORA_CODINGRATE             1         // 4/5
#define LORA_PREAMBLE_LENGTH        8
#define LORA_SYMBOL_TIMEOUT         0
#define LORA_FIX_LENGTH_PAYLOAD_ON  false
#define LORA_IQ_INVERSION_ON        false
#define BUFFER_SIZE                 256

char rxpacket[BUFFER_SIZE];
static RadioEvents_t RadioEvents;

bool lora_idle = true;
int16_t rssi;
int8_t snr;
uint16_t rxSize;

// ===== Function Prototypes =====
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr);

// ===== Display Received Data =====
void display_rx_status(const char *packet, int16_t rssi_val, int16_t size_val, int8_t snr_val) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);

  display.drawString(0, 0, "Packet Received!");
  display.drawString(0, 12, "RSSI: " + String(rssi_val) + " | SNR: " + String(snr_val));
  display.drawString(0, 24, "Size: " + String(size_val) + " bytes");

  String packetStr = String(packet);
  int maxLenPerLine = 30;
  display.drawString(0, 36, packetStr.substring(0, maxLenPerLine));
  if (packetStr.length() > maxLenPerLine)
    display.drawString(0, 48, packetStr.substring(maxLenPerLine));

  display.display();
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);
  Mcu.begin(HELTEC_BOARD, SLOW_CLK_TPYE);

  // OLED init
  VextON();
  delay(100);
  display.init();
  display.clear();
  display.setContrast(255);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth() / 2, display.getHeight() / 2 - 8, "LoRa Receiver");
  display.display();
  delay(1500);

  // LoRa Radio Init
  RadioEvents.RxDone = OnRxDone;
  Radio.Init(&RadioEvents);
  Radio.SetChannel(RF_FREQUENCY);
  Radio.SetRxConfig(
      MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
      LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
      LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
      0, true, 0, 0, LORA_IQ_INVERSION_ON, true);

  Serial.println("\n--- LoRa Receiver Started ---");

  display.clear();
  display.setFont(ArialMT_Plain_10);
  display.drawString(display.getWidth()/2, display.getHeight()/2 - 8, "Listening...");
  display.display();
  delay(500);

  // Start initial RX
  lora_idle = false;
  Radio.Rx(0);
}

// ===== LOOP =====
unsigned long lastPing = 0;
void loop() {
  // Always process LoRa events
  Radio.IrqProcess();

  // Re-trigger receive every 1 second (ping)
  if (millis() - lastPing >= 1000) {
    lastPing = millis();
    if (lora_idle) {
      lora_idle = false;
      Serial.println("Listening for packet...");
      Radio.Rx(0);  // Continuous receive
    }
  }
}

// ===== CALLBACK =====
void OnRxDone(uint8_t *payload, uint16_t size, int16_t rssi_val, int8_t snr_val) {
  memcpy(rxpacket, payload, size);
  rxpacket[size] = '\0';
  rssi = rssi_val;
  snr = snr_val;
  rxSize = size;

  Radio.Sleep();

  Serial.printf("\n Received packet: \"%s\" | RSSI: %d dBm | SNR: %d | Size: %d bytes\n",
                rxpacket, rssi, snr, rxSize);

  display_rx_status(rxpacket, rssi, rxSize, snr);

  // Ready for next RX
  lora_idle = true;
}