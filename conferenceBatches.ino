#include <Adafruit_NeoPixel.h>
#include <VirtualWire.h>

#ifdef __AVR__
#include <avr/power.h>
#endif


#define ID 200
#define PIN 0 //NeoPixel
#define NLEDS 1

//raw tempsensor ranges to scale the color of the NeoPixel. Note that those are different when powered through 9V versus USB (5V)
#define TMIN 145
#define TMAX 160

// Data wire is plugged into pin 2 on the digiSpark
#define ONE_WIRE_BUS 2

/* Digital IO pin that will be used for sending data to the transmitter */
const int TX_DIO_Pin = 1;

/* The transmit buffer that will hold the data to be
     transmitted. */
byte TxBuffer[3];

byte data[12];

long filtReg;
int bitShift = 4;

int waitTimePixel = 50;

int waitThreshold = 20;
int waitCounter = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
//Adafruit_NeoPixel strip = Adafruit_NeoPixel(NLEDS, PIN, NEO_RGBW + NEO_KHZ800);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NLEDS, PIN, NEO_RGB + NEO_KHZ800);



void setup() {

  // This is for Trinket 5V 16MHz, you can remove these three lines if you are not using a Trinket
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  // End of trinket special code

  /* Initialises the DIO pin used to send data to the Tx module */
  vw_set_tx_pin(TX_DIO_Pin);
  /* Set the transmit logic level (LOW = transmit for this
     version of module)*/
  vw_set_ptt_inverted(true);

  /* Transmit at 2000 bits per second */
  vw_setup(2000);    // Bits per sec

  //always send the ID as wirst part of the buffer
  TxBuffer[0] = ID;


  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

}

void loop() {

  
  filtReg = filtReg - (filtReg >> bitShift) + analogRead(1);
  int filtOut = filtReg >> bitShift;

  int tempMap = map(min(max(filtOut,TMIN),TMAX), TMIN, TMAX, 0, 120);
  
  strip.setPixelColor(0, strip.Color(0,tempMap,120-tempMap,0));
  strip.show();
  delay(waitTimePixel);


  //once every waitCounter measurements, send it over the radio
  if (waitCounter++ > waitThreshold) {
    waitCounter = 0;
    TxBuffer[1] = filtOut;
    

    /* add a check-byte to the buffer */
    TxBuffer[2] = (TxBuffer[0] + TxBuffer[1]) % 255;
    /* Send the data (3 bytes) */
    vw_send((byte *)TxBuffer, 3);
    /* Wait until the data has been sent */
    vw_wait_tx();
    delay(100);
  }
}



