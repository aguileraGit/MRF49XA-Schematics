#include <SPI.h>
#include <MRF49XA.h>
/*
Diego Aguilera
 mrfxa49 demo code port attempt
 25-Jan-2015
 */

//Need to adjust pin number
//Chip Select
int nCS = 11;

//Fifo select - Active low output
int nFSEL = 9; //Needs to be updaed

int FINT = 7; //Needs to be updaed

//LED
int TLED = 13;

void intMrf49xa() {
  Serial.println("Int triggered");
  
}

void setup() {
  
  //Board init
  pinMode(nCS, OUTPUT);
  pinMode(nFSEL, OUTPUT);
  pinMode(FINT, OUTPUT);
  
  //Led
  pinMode(TLED, OUTPUT);

  //Init SPI CS pin high
  digitalWrite(nCS, HIGH);

  //IRQ is connected to int2 - nIRQ
  attachInterrupt(0, intMrf49xa, FALLING);

  Serial.begin(115200);
  
  //Blink LED
  digitalWrite(TLED, HIGH);
  delay(50);
  digitalWrite(TLED, LOW);
  delay(50);
  digitalWrite(TLED, HIGH);
  delay(50);
  digitalWrite(TLED, LOW);
  delay(50);
  
  Serial.println("Starting...");
  delay(2000);
}

void SPIWrite(byte data) {
  //Using Arduino's SPI Library instead of sending each bit
  SPI.transfer(data);  
}

byte SPIRead(void) {
  byte returnData = SPI.transfer(0x00);
}

void RegisterSet(word setting) {
  digitalWrite(nCS, LOW);
  SPIWrite(setting >> 8);
  SPIWrite(setting);
  digitalWrite(nCS, HIGH);
}

void MRF49XAInit(void) {
  
  detachInterrupt(0);
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
  delay(5);                            // wait for oscillator to stablize
  // end of antenna tuning
  RegisterSet(PMCREG | 0x0080);           // turn off transmitter, turn on receiver
  RegisterSet(GENCREG | 0x0040);          // enable the FIFO
  RegisterSet(FIFORSTREG);
  RegisterSet(FIFORSTREG | 0x0002);       // enable synchron latch
  RegisterSet(0x0000);				    // read status byte (read ITs)
  attachInterrupt(0, intMrf49xa, FALLING);
}




