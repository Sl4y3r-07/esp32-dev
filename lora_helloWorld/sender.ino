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
#define TX_OUTPUT_POWER               5    // dBm
#define LORA_BANDWIDTH               0     // [0: 125 kHz]
#define LORA_SPREADING_FACTOR            7     // [SF7..SF12]
#define LORA_CODINGRATE               1     // [1: 4/5]
#define LORA_PREAMBLE_LENGTH            8     // Same for Tx and Rx
#define LORA_FIX_LENGTH_PAYLOAD_ON         false
#define LORA_IQ_INVERSION_ON            false

#define BUFFER_SIZE                 30 // Define the payload size here

char txpacket[BUFFER_SIZE];
char rxpacket[BUFFER_SIZE];

double txNumber;

bool lora_idle=true;

static RadioEvents_t RadioEvents;
void OnTxDone( void );
void OnTxTimeout( void );

// --- Display Update Function ---
// Modified to use a small font (ArialMT_Plain_10) and split the packet over two lines
void display_tx_status(const char* packet, bool success) {
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10); // Use small font for visibility
  
  // Line 1: Status (Y=0)
  display.drawString(0, 0, success ? "Status: TX Done" : "Status: TX Timeout");
  
  // Line 2: Packet Label (Y=14, tightened spacing)
  display.drawString(0, 14, "Packet:");
  
  // --- Text Splitting Logic ---
  String packetStr = String(packet);
  // 20 characters is a safe limit for ArialMT_Plain_10 on a 128px screen
  int maxLenPerLine = 30; 

  // Line 3: First part of the packet (Y=28)
  display.drawString(0, 28, packetStr.substring(0, maxLenPerLine));

  // Line 4: Second part of the packet (Y=42)
  if (packetStr.length() > maxLenPerLine) {
    display.drawString(0, 42, packetStr.substring(maxLenPerLine));
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
  display.drawString(display.getWidth()/2, display.getHeight()/2 - 8, "LoRa Sender");
  display.display();
  delay(1500); // Show initial message
  // --- End OLED Initialization ---

  txNumber=0;

  RadioEvents.TxDone = OnTxDone;
  RadioEvents.TxTimeout = OnTxTimeout;
  
  Radio.Init( &RadioEvents );
  Radio.SetChannel( RF_FREQUENCY );
  Radio.SetTxConfig( MODEM_LORA, TX_OUTPUT_POWER, 0, LORA_BANDWIDTH,
                 LORA_SPREADING_FACTOR, LORA_CODINGRATE,
                 LORA_PREAMBLE_LENGTH, LORA_FIX_LENGTH_PAYLOAD_ON,
                 true, 0, 0, LORA_IQ_INVERSION_ON, 3000 ); 
 }



void loop()
{
 if(lora_idle == true)
 {
  delay(1000);
  txNumber += 0.01;
  // The payload length is up to 30 chars, so two lines are necessary for the display
  sprintf(txpacket,"Hello world number %0.2f",txNumber); //start a package
 
  Serial.printf("\r\nsending packet \"%s\" , length %d\r\n",txpacket, strlen(txpacket));
    
    // --- Display the full packet before sending ---
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.setFont(ArialMT_Plain_10);

    display.drawString(0, 0, "Status: Sending...");
    display.drawString(0, 14, "Packet:");

    String packetStr = String(txpacket);
    int maxLenPerLine = 30;

    // Line 3: First part of the packet (Y=28)
    display.drawString(0, 28, packetStr.substring(0, maxLenPerLine));

    // Line 4: Second part of the packet (Y=42)
    if (packetStr.length() > maxLenPerLine) {
      display.drawString(0, 42, packetStr.substring(maxLenPerLine));
    }

    display.display();
    // --- End Display Update ---

  Radio.Send( (uint8_t *)txpacket, strlen(txpacket) ); //send the package out 
  lora_idle = false;
 }
 Radio.IrqProcess( );
}

void OnTxDone( void )
{
 Serial.println("TX done......");
 display_tx_status(txpacket, true); // Update display on success
 lora_idle = true;
}

void OnTxTimeout( void )
{
  Radio.Sleep( );
  Serial.println("TX Timeout......");
  display_tx_status(txpacket, false); // Update display on timeout
  lora_idle = true;
}