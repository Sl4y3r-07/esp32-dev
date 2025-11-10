#include "LoRaWan_APP.h"
#include "Arduino.h"
#include <Wire.h> 
#include "HT_SSD1306Wire.h"

// --- OLED Display Definitions and Initialization ---
#ifdef WIRELESS_STICK_V3
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_64_32, RST_OLED);
#else
// Assuming a standard Heltec LoRa board with 128x64 display
static SSD1306Wire display(0x3c, 500000, SDA_OLED, SCL_OLED, GEOMETRY_128_64, RST_OLED);
#endif

// Functions to control Vext power for the display
void VextON(void)
{
pinMode(Vext,OUTPUT);
digitalWrite(Vext, LOW);
}

void VextOFF(void) //Vext default OFF
{
pinMode(Vext,OUTPUT);
digitalWrite(Vext, HIGH);
}


// --- LoRa Definitions ---
#define RF_FREQUENCY                915000000 // Hz
// TX_OUTPUT_POWER definition removed as this is a receiver
#define LORA_BANDWIDTH               0     // 125 kHz
#define LORA_SPREADING_FACTOR            7     // SF7
#define LORA_CODINGRATE               1     // 4/5
#define LORA_PREAMBLE_LENGTH            8     
#define LORA_SYMBOL_TIMEOUT             0     
#define LORA_FIX_LENGTH_PAYLOAD_ON         false
#define LORA_IQ_INVERSION_ON            false

#define BUFFER_SIZE                 30 // Define the payload size here

char rxpacket[BUFFER_SIZE];

static RadioEvents_t RadioEvents;

int16_t txNumber;

int16_t rssi,rxSize;
int8_t snr; // To store Signal-to-Noise Ratio

bool lora_idle = true;

// Function Prototypes
void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi, int8_t snr );

// --- Display Update Function ---
// Updated to use small font (ArialMT_Plain_10) throughout and split the received packet
void display_rx_status(const char* packet, int16_t rssi_val, int16_t size_val, int8_t snr_val) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
  display.setFont(ArialMT_Plain_10); // Use small font for maximum text space (10pt is about 10-12px high)
  
  // Line 1: Status (Y=0)
  display.drawString(0, 0, "LoRa Receiver: PACKET RECEIVED!");

  // Line 2: RSSI and SNR (Y=12) - Tighter spacing
  display.drawString(0, 12, "RSSI: " + String(rssi_val) + " dBm | SNR: " + String(snr_val));
  
  // Line 3: Packet Size (Y=24) - Tighter spacing
  display.drawString(0, 24, "Size: " + String(size_val) + " bytes");
  
  // Line 4: Packet Label (Y=36) - Tighter spacing
  display.drawString(0, 36, "Packet Content:");
  
  // --- Text Splitting Logic for Full Packet Display ---
  String packetStr = String(packet);
  // 20 characters is a safe limit for ArialMT_Plain_10 on a 128px screen
  int maxLenPerLine = 30; 

  // Line 5: First part of the packet (Y=48)
  display.drawString(0, 48, packetStr.substring(0, maxLenPerLine));

  // Line 6: Second part of the packet (Y=60)
  if (packetStr.length() > maxLenPerLine) {
    display.drawString(0, 60, packetStr.substring(maxLenPerLine));
  }
  // --- End Text Splitting Logic ---
  
  display.display();
}


void setup() {
 Serial.begin(115200);
 Mcu.begin(HELTEC_BOARD,SLOW_CLK_TPYE);
 
 // --- OLED Initialization ---
  VextON(); // Power up the OLED
  delay(100);
  display.init();
  display.clear();
  display.display();
  display.setContrast(255);
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_16);
  display.drawString(display.getWidth()/2, display.getHeight()/2 - 8, "LoRa Receiver");
  display.display();
  delay(1500); // Show initial message
  // --- End OLED Initialization ---
  
 txNumber=0;
 rssi=0;

 RadioEvents.RxDone = OnRxDone;
 Radio.Init( &RadioEvents );
 Radio.SetChannel( RF_FREQUENCY );
 Radio.SetRxConfig( MODEM_LORA, LORA_BANDWIDTH, LORA_SPREADING_FACTOR,
       LORA_CODINGRATE, 0, LORA_PREAMBLE_LENGTH,
       LORA_SYMBOL_TIMEOUT, LORA_FIX_LENGTH_PAYLOAD_ON,
       0, true, 0, 0, LORA_IQ_INVERSION_ON, true );
  
  // Initial display status
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_CENTER);
  display.setFont(ArialMT_Plain_10);
  display.drawString(display.getWidth()/2, display.getHeight()/2 - 8, "Listening...");
  display.display();
}



void loop()
{
if(lora_idle)
{
 lora_idle = false;
 Serial.println("into RX mode");
 Radio.Rx(0); // Set radio to receive indefinitely
}
Radio.IrqProcess( ); // Handle LoRa events
}

void OnRxDone( uint8_t *payload, uint16_t size, int16_t rssi_val, int8_t snr_val )
{
 // Update global variables with received values
 rssi = rssi_val;
 snr = snr_val;
 rxSize = size;
  
  // Copy the payload and ensure null termination
 memcpy(rxpacket, payload, size );
 rxpacket[size]='\0';
  
 Radio.Sleep( );
  
 Serial.printf("\r\nreceived packet \"%s\" with rssi %d , snr %d, length %d\r\n",rxpacket,rssi,snr,rxSize);
  
  // --- Display the received packet on OLED ---
  display_rx_status(rxpacket, rssi, rxSize, snr);
  // --- End OLED Update ---
  
 lora_idle = true;
}