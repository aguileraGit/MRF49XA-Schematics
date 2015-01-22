/*
 Diego Aguilera
11-Jan-2015
Test code for the mrf49xa board
*/

#include <SPI.h>

//SPI commands
const long    GENCREG           = 0x8038;               // Cload=12.5pF; TX registers & FIFO are disabled
const long    PMCREG            = 0x8200;               // Everything off, uC clk enabled
const long    RXCREG            = 0x94A1;               // BW=135kHz, DRCSI=-97dBm, pin8=VDI, fast VDI
const long    TXBREG            = 0xB800;
const long    FIFORSTREG        = 0xCA81;               // Sync. latch cleared, limit=8bits, disable sensitive reset
const long    BBFCREG           = 0xC22C;               // Digital LPF (default)
const long    AFCCREG           = 0xC4D7;               // Auto AFC (default)
const long    CFSREG            = 0xA7D0;               // Fo=915.000MHz (default)
const long    TXCREG            = 0x9830;               // df=60kHz, Pmax, normal modulation polarity
const long    DRSREG            = 0xC623;               // 9579Baud (default)

int LED = 13;
int CS = 17;
int IRQ = 0;

void setup(void) {

 pinMode(LED, OUTPUT);

 //Set CS
 pinMode(CS, OUTPUT);
 digitalWrite(CS, HIGH);

 //Set IRQ pin - Needs to become an interrupt at some point
 pinMode(IRQ, INPUT);

 //Begin Serial
 Serial.begin(57600);
 delay(2000);
 Serial.println("Start");

 //Start SPI
  spiBegin();
}

void loop(void) {
  //Wait for reset to finish
  delay(200);

  //Reset to get POR bit
  Serial.println("Reset");
  spiCommand(FIFORSTREG | 0x0002);
  delay(200);

  Serial.println("Before init");
  readStatus();

  //Init MRF49XA
  initMRF49XA();
  delay(300);

  Serial.println("After init");
  readStatus();

  //spiEnd();
  delay(1000);
}

void MRF49XA_Send_Packet(unsigned char *data, unsigned char length){
        int a;
        //---- turn off receiver , enable Tx register
        spiCommand(PMCREG);                             // turn off the transmitter and receiver
        spiCommand(GENCREG | 0x0080);                   // Enable the Tx register
        //---- Packet transmission
        // Reset value of the Tx regs are [AA AA], we can start transmission
        //---- Enable Tx
        spiCommand(PMCREG |0x0020);                     // turn on tx
                digitalWrite(CS, LOW);                  // chip select low
                while(!digitalRead(IRQ));
                        spiWrite16(TXBREG | 0xAA);      // preamble
                while(!digitalRead(IRQ));
                        spiWrite16(TXBREG | 0x2D);      // sync pattern 1st byte
                while(!digitalRead(IRQ));
                        spiWrite16(TXBREG | 0xD4);      // sync pattern 2nd byte
                while(!digitalRead(IRQ));
                        spiWrite16(TXBREG | length);
                #ifdef SPECTRUM_TEST
                        for (a=0;1;){                   // endless loop
                #else
                        for (a=0;a<length;a++){         // send payload
                #endif
                        while(!digitalRead(IRQ));       // wait for SDO, when buffer data <=8 bits
                #ifdef SPECTRUM_TEST
                           spiWrite16(0xAA);            // spectrum test
                #else
                           spiWrite16(TXBREG | data[a]); // write a byte to tx register
                #endif
                }
                while(!digitalRead(IRQ)){}
                        spiWrite16(TXBREG |0x00);       // write a dummy byte since the previous byte is still in buffer
                while (!digitalRead(IRQ)){}             // wait for the last byte transmission end

                digitalWrite(CS, HIGH);                 // chip select high, end transmission
        //---- Turn off Tx disable the Tx register
        spiCommand(PMCREG | 0x0080);                    // turn off Tx, turn on the receiver
        spiCommand(GENCREG | 0x0040);                   // disable the Tx register, Enable the FIFO
}

void initMRF49XA(void) {
  Serial.println("Init");
 spiCommand( FIFORSTREG );
 spiCommand( FIFORSTREG | 0x0002);
 spiCommand( GENCREG);
 spiCommand( CFSREG);
 spiCommand( PMCREG);
 spiCommand( RXCREG);
 spiCommand( TXCREG);
 //---- antenna tunning
 spiCommand( PMCREG | 0x0020);          // turn on tx
 delay(50);
 //---- end of antenna tunning
 //spiCommand( PMCREG | 0x0080);                // turn off Tx, turn on receiver <----Error!
 delay(50);
 spiCommand( GENCREG | 0x0040);         // enable the FIFO
 spiCommand( FIFORSTREG);
 spiCommand( FIFORSTREG | 0x0002);      // enable syncron latch
}

void spiBegin(void) {
 digitalWrite(CS, HIGH);
 SPI.begin();
 //Lower speed
 SPI.setClockDivider(SPI_CLOCK_DIV16);
 SPI.setDataMode(SPI_MODE0);
 SPI.setBitOrder(MSBFIRST);
}

void spiEnd(void) {
 SPI.end();
 digitalWrite(CS, LOW);
}

void spiCommand(uint16_t spiCmd) {
 digitalWrite(CS, LOW);

 uint8_t toWrite = (spiCmd & 0xFF00) >> 8;
 //Serial.print("0x");
 //Serial.print(toWrite, HEX);
 uint8_t toPrint = SPI.transfer(toWrite);
 //Serial.println(toPrint, HEX);

 toWrite = (spiCmd & 0x00FF);
 //Serial.print(toWrite, HEX);
 //Serial.println(" ");
 toPrint = SPI.transfer(toWrite);
 //Serial.print(toPrint, HEX);

 delay(10);
 digitalWrite(CS, HIGH);

}

uint16_t readStatus() {
 digitalWrite(CS, LOW);

 uint8_t MSB = SPI.transfer(0x00);
 uint8_t LSB = SPI.transfer(0x00);

 digitalWrite(CS, HIGH);

 //Debug
 Serial.print("MSB: ");
 Serial.print(MSB, HEX);
 Serial.print(" LSB: ");
 Serial.print(LSB, HEX);
 Serial.println(" ");
 Serial.println(" ");

 //uint16_t spiData = (uint16_t)(MSB << 8 | LSB);
 //return spiData;
 return 0;
}


void spiWrite16(uint16_t spiCmd) {
 //digitalWrite(CS, LOW);
 SPI.transfer((spiCmd & 0xFF00) >> 8);
 SPI.transfer((spiCmd & 0x00FF));
 //digitalWrite(CS, HIGH);
 delay(0);
}

uint16_t spiRead(void) {
 digitalWrite(CS, LOW);

 uint8_t spiData0 = SPI.transfer(0x00);
 uint8_t spiData1 = SPI.transfer(0x00);

 digitalWrite(CS, HIGH);

 uint16_t spiData = (uint16_t)(spiData0 << 8 | spiData1);
 return spiData;
}
