/*
  Diego Aguilera
  11-Jan-2015
  Test code for the mrf49xa board
 */

#include <SPI.h>

//SPI commands
const long    GENCREG 		= 0x8038;		// Cload=12.5pF; TX registers & FIFO are disabled
const long    PMCREG 		= 0x8200;		// Everything off, uC clk enabled
const long    RXCREG 		= 0x94A1;		// BW=135kHz, DRCSI=-97dBm, pin8=VDI, fast VDI
const long    TXBREG 		= 0xB800;
const long    FIFORSTREG	= 0xCA81;		// Sync. latch cleared, limit=8bits, disable sensitive reset
const long    BBFCREG 		= 0xC22C;		// Digital LPF (default)
const long    AFCCREG		= 0xC4D7;		// Auto AFC (default)
const long    CFSREG 		= 0xA7D0;		// Fo=915.000MHz (default)
const long    TXCREG		= 0x9830;		// df=60kHz, Pmax, normal modulation polarity 
const long    DRSREG 		= 0xC623;		// 9579Baud (default)

int LED = 13;
int CS = 17;

uint16_t STSREAD = 0x0000;

void setup(void) {
  //Set LED
  pinMode(LED, OUTPUT);
  
  //Set CS
  pinMode(CS, OUTPUT);
  digitalWrite(CS, HIGH);
  
  //Begin Serial
  Serial.begin(57600);
  
  delay(500);
}

void loop(void) {
  //Start SPI
  spiBegin();
  
  //Wait for reset to finish
  delay(200);
  
  //Init MRF49XA
  initMRF49XA();
  
  //Attempt to read status register
  spiCommand( STSREAD );
  uint16_t statusReg = spiRead();
  uint8_t statusReg1 = (uint8_t)((statusReg & 0xFF00) >> 8);
  uint8_t statusReg2 = (uint8_t)(statusReg & 0x00FF);
  
  //Print
  Serial.println(statusReg1, HEX);
  Serial.println(statusReg2, HEX);
  
  spiEnd();
  delay(700);
}

void initMRF49XA(void) {
  spiCommand( FIFORSTREG );
  spiCommand( FIFORSTREG | 0x0002);
  spiCommand( GENCREG);
  spiCommand( CFSREG);
  spiCommand( PMCREG);
  spiCommand( RXCREG);
  spiCommand( TXCREG);	
//---- antenna tunning
  spiCommand( PMCREG | 0x0020);		// turn on tx
  delay(5);
//---- end of antenna tunning
  spiCommand( PMCREG | 0x0080);		// turn off Tx, turn on receiver
  spiCommand( GENCREG | 0x0040);		// enable the FIFO
  spiCommand( FIFORSTREG);
  spiCommand( FIFORSTREG | 0x0002);	// enable syncron latch	 
}

void spiBegin(void) {
  digitalWrite(CS, HIGH);
  SPI.begin();
  //Lower speed
  SPI.setClockDivider(SPI_CLOCK_DIV16);
}

void spiEnd(void) {
  SPI.end();
  digitalWrite(CS, LOW);
}

void spiCommand(uint16_t spiCmd) {
  spiWrite((spiCmd & 0xFF00) >> 8);
  spiWrite((spiCmd & 0x00FF));
  delay(1);
}

uint16_t spiRead(void) {
  digitalWrite(CS, LOW);
  
  uint8_t spiData0 = SPI.transfer(0x00);
  uint8_t spiData1 = SPI.transfer(0x00);
  
  digitalWrite(CS, HIGH);
 
  uint16_t spiData = (uint16_t)(spiData0 << 8 | spiData1);
  return spiData;
}

void spiWrite(uint8_t spiData) {
  digitalWrite(CS, LOW);
  SPI.transfer(spiData);
  digitalWrite(CS, HIGH);
}


 
 
  

