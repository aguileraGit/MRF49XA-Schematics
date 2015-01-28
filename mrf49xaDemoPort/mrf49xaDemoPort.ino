#include <SPI.h>

#define XTAL_LD_CAP             0x0003              // crystal load 10pF

#define FREQ_Band               0x0030              //915MHz

#define CFSREG                  0xA7D0              // Center Frequency: 915.000MHz    
#define GENCREG                 (0x8000 | XTAL_LD_CAP | FREQ_Band)
#define AFCCREG                 0xC4F7
#define TXCREG                  0x9850              // Deviation: 75kHz
#define TXBREG                  0xB800
#define RXCREG                  0x9481              // BW 200KHz, LNA gain 0dB, RSSI -97dBm
#define FIFORSTREG              0xCA81
#define DRVSREG                 0xC623              // Data Rate:9579Baud
#define PMCREG                  0x8201

/*
Diego Aguilera
 mrfxa49 demo code port attempt
 25-Jan-2015
 */

//Interrupt Pin
int intPin = 2;

//Chip Select
int nCS = 17;

//Fifo select - Active low output
int nFSEL = 11; //Not yet implemented

int FINT = 10; //Not yet implemented

//LED
int TLED = 13;

volatile int intTriggered = 0;

void intMrf49xa() {
  intTriggered = 1;
}

void setup() {
  //Board init
  pinMode(nCS, OUTPUT);
  pinMode(nFSEL, OUTPUT);
  pinMode(FINT, INPUT);
  
  //Led
  pinMode(TLED, OUTPUT);

  //Init SPI CS pin high
  digitalWrite(nCS, HIGH);
  
  //nFSEL set high
  digitalWrite(nFSEL, HIGH);

  //IRQ is connected to int2 - nIRQ
  attachInterrupt(intPin, intMrf49xa, CHANGE);
  
  //Start serial and SPI
  Serial.begin(115200);
  SPI.begin();
  SPI.setClockDivider(SPI_CLOCK_DIV16);
  SPI.setDataMode(SPI_MODE0);
  SPI.setBitOrder(MSBFIRST);
  
  //Starting
  blinkLED();
  delay(2000);
  Serial.println("Starting.....");
  delay(500);
  
  //Init status register - should be 0x4000 for POR
  digitalWrite(nCS, LOW);
  SPIRead();
  SPIRead();
  digitalWrite(nCS, HIGH);
  Serial.println(" ");
}

void SPIWrite(byte data) {
  //Using Arduino's SPI Library
  SPI.transfer(data);  
}

byte SPIRead(void) {
  byte returnData = SPI.transfer(0x00);
  Serial.println(returnData, HEX);
}

void RegisterSet(word setting) {
  digitalWrite(nCS, LOW);
  SPIWrite((setting & 0xFF00) >> 8);
  SPIWrite(setting & 0x00FF);
  digitalWrite(nCS, HIGH);
}

void MRF49XAInit(void) {
  detachInterrupt(intPin);
  RegisterSet(FIFORSTREG);
  RegisterSet(FIFORSTREG | 0x0002);       // enable synchron latch
  RegisterSet(GENCREG);
  RegisterSet(AFCCREG);
  RegisterSet(CFSREG);
  RegisterSet(DRVSREG);
  RegisterSet(PMCREG);
  RegisterSet(RXCREG);
  RegisterSet(TXCREG);
  // antenna tuning on startup
  RegisterSet(PMCREG | 0x0020);           // turn on the transmitter
  delay(10);                              // wait for oscillator to stablize
  // end of antenna tuning
  RegisterSet(PMCREG | 0x0080);           // turn off transmitter, turn on receiver
  RegisterSet(GENCREG | 0x0040);          // enable the FIFO
  RegisterSet(FIFORSTREG);
  RegisterSet(FIFORSTREG | 0x0002);       // enable synchron latch
  RegisterSet(0x0000);                    // read status byte (read ITs)
  
  //Set low voltage
  RegisterSet(0xC001); //Sets voltage too low
  RegisterSet(0x8285); //Enables LBDEN bit
  				    
  attachInterrupt(intPin, intMrf49xa, CHANGE);
}

void blinkLED(void) {
  int blinkTimes = 10;
  for(int i = 0; i < blinkTimes; i++){
    digitalWrite(TLED, HIGH);
    delay(60);
    digitalWrite(TLED, LOW);
    delay(60);
  }
}


void loop() {
  //Init
  MRF49XAInit();
  
  //Reset command - Reset Atmel. Not surprised since both resets are tied together
  //RegisterSet(0xFE00);
  //delay(150);
  
  digitalWrite(nCS, LOW);
  SPIRead();
  SPIRead();
  digitalWrite(nCS, HIGH);
  Serial.println(" ");
  
  delay(1000);

  Serial.print("Triggered: ");
  Serial.println(intTriggered);
  intTriggered = 0;
  RegisterSet(0x8281); //Disables LBDEN bit
}



