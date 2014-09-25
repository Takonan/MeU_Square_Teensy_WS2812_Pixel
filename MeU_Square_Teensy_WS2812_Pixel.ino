//////////////////////////////////////////////////////////////////////////
//Filenames: MeU_Square_Teensy_WS2812_Pixel.ino
//Authors: Robert Tu
//Date Created: June 5, 2014
//Notes:
/*
  PLEASE NOTE: This version of MeU Square uses the Teensy 3.1 microcontroller which
  is Arduino compatible. You will need to download the Teensyduino software
  first before being able to program MeU Long.
  
  You can download it here:
  http://www.pjrc.com/teensy/teensyduino.html
  
  Once Teensyduino is installed, the Teensy board option will become available
  under the Tools Menu in the Arduino IDE
  
  Also the Adafruit NeoPixel Library had to be modified in order to work with
  this version of MeU. Please be sure to install the correct version of
  the NeoPixel library that came with this version of MeU.
  
  This is the Arduino file that controls the MeU panel. It utilizes Adafruit's
  NeoPixel and GFX libraries to control the WS2812 LEDs.
  
  In the main loop the program waits for a serial message sent from a mobile
  device via bluetooth. The message is then parsed for specific commands. 
  
  The message protocol is as follows:
  
  "RRGGBB..nn"
  
  where "RR" is Red colour data, "GG" is green colour data and "BB" is blue coloured 
  data in string hex format. 
  
  Since MeU Square has a resolution of 16x16, only 1536 characters can be sent over:
  (16x16 = 256 * 3 colours * 2 characters per colour = 1536 characters)
  
  Most image files can be broken down into RGB data and sent over to the panel
  Image files must be 16x16 pixels
  
*/

//////////////////////////////////////////////////////////////////////////

#include <avr/pgmspace.h> // to use PROGMEM
// Adafruit_NeoMatrix example for single NeoPixel Shield.
// Scrolls 'Howdy' across the matrix in a portrait (vertical) orientation.

#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <SimpleTimer.h>
#include <math.h>
#ifndef PSTR
 #define PSTR // Make Arduino Due happy
#endif

#define PIN 6
#define HEIGHT 16
#define WIDTH 16

// MATRIX DECLARATION:
// Parameter 1 = width of NeoPixel matrix
// Parameter 2 = height of matrix
// Parameter 3 = pin number (most are valid)
// Parameter 4 = matrix layout flags, add together as needed:
//   NEO_MATRIX_TOP, NEO_MATRIX_BOTTOM, NEO_MATRIX_LEFT, NEO_MATRIX_RIGHT:
//     Position of the FIRST LED in the matrix; pick two, e.g.
//     NEO_MATRIX_TOP + NEO_MATRIX_LEFT for the top-left corner.
//   NEO_MATRIX_ROWS, NEO_MATRIX_COLUMNS: LEDs are arranged in horizontal
//     rows or in vertical columns, respectively; pick one or the other.
//   NEO_MATRIX_PROGRESSIVE, NEO_MATRIX_ZIGZAG: all rows/columns proceed
//     in the same order, or alternate lines reverse direction; pick one.
//   See example below for these values in action.
// Parameter 5 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)


// Example for NeoPixel Shield.  In this application we'd like to use it
// as a 5x8 tall matrix, with the USB port positioned at the top of the
// Arduino.  When held that way, the first pixel is at the top right, and
// lines are arranged in columns, progressive order.  The shield uses
// 800 KHz (v2) pixels that expect GRB color data.
Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(HEIGHT, WIDTH, PIN,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
  NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG,
  NEO_GRB            + NEO_KHZ800);




const unsigned int MAX_INPUT = HEIGHT*WIDTH+3;
const unsigned int MAX_MSG = HEIGHT*WIDTH+3;
const unsigned int MATRIX_LENGTH = HEIGHT*WIDTH;

//Buffer tables to hold RGB data
byte RedTable[MATRIX_LENGTH];
byte BlueTable[MATRIX_LENGTH];
byte GreenTable[MATRIX_LENGTH];

int ByteCount;
int TableCount = 0;
boolean ReadyToDraw = false;
boolean ImageLoaded = false;
String input_line;

// The Bluetooth Mate Silver is connected to Pins 0 and 1 of Teensy which are accessed through the Hardware Serial Library
// In this case Uart is the Serial port for bluetooth

HardwareSerial Uart = HardwareSerial();

void setup()
{
  //Uart is connected to bluetooth
  Uart.begin(115200);
  Uart.setTimeout(1000);
  
  //Serial is connected to PC for monitoring of data
  Serial.begin(115200);
 
  //Initialize screen and reset to zero
  matrix.begin();
  
  //WARNING! DO NOT SET THIS VALUE PAST 40 OTHERWISE YOU WILL DAMAGE THE LEDS!!!
  //Brightness determines the amount of current drawn and at full brightness, these LEDS can draw
  //up to 15A!!! So be sure this value doesn't go past 40. 
  
  matrix.setBrightness(40);
  
  matrix.fillScreen(0);
  matrix.show();
  
}


void loop()
{
  
  
  if (Uart.available () > 0) 
  {
    //Read data as string
    //data comes in as 1536 (256*6) characters and is RGB in hex "RRGGBBRRGGBB......RRGGBB" where "RR" is Red colour data, "GG" is green colour data and "BB" is blue coloured data
    
    input_line = Uart.readString();
    parseData(input_line);
   
    
  } else {
   
    Uart.flush();
  }


}

//Parse the data and display image on matrix
void parseData (String data)
{
  
  //Serial debugger to make sure bluetooth is receiving all 1536 characters
  Serial.println(data.length());
  Serial.println(data);
  
  //reset the screen
  matrix.fillScreen(0);
  matrix.show();
  
  //temporary bytes to store converted RGB data
  byte Red;
  byte Green;
  byte Blue;

  //loop through matrices and extract RGB data from 1536 character string
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int loc = x + y*WIDTH;
  
      Red = SerialReadHexByte(data.charAt(0+loc*6), data.charAt(1+loc*6));
      Green = SerialReadHexByte(data.charAt(2+loc*6), data.charAt(3+loc*6));
      Blue = SerialReadHexByte(data.charAt(4+loc*6), data.charAt(5+loc*6));
      
      //after converting store into RGB tables     
      RedTable[loc] = Red;
      GreenTable[loc] = Green;
      BlueTable[loc] = Blue;
        
     
    }
  }
  
  //Once tables have stored values, load up into matrix buffer to display
  for (int y = 0; y < HEIGHT; y++) {
    for (int x = 0; x < WIDTH; x++) {
      int loc = x + y*HEIGHT; 
      matrix.drawPixel(x, y, drawRGB24toRGB565(RedTable[loc], GreenTable[loc], BlueTable[loc]));
    }
  }
 
  matrix.show();
    
  
}  // end of process_data

//Function to convert hex to integer
byte SerialReadHexDigit(byte c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    } else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    } else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    } 
    
}

//Function to convert hex to integer
byte SerialReadHexByte(byte d, byte e)
{
    byte a = SerialReadHexDigit(d);
    byte b = SerialReadHexDigit(e);
   
    return (a*16) + b;
}

/**************************************************************************/
/*!
    @brief  Converts a 24-bit RGB color to an equivalent 16-bit RGB565 value

    @param[in]  r
                8-bit red
    @param[in]  g
                8-bit green
    @param[in]  b
                8-bit blue

    @section Example

    @code 

    // Get 16-bit equivalent of 24-bit color
    uint16_t gray = drawRGB24toRGB565(0x33, 0x33, 0x33);

    @endcode
*/
/**************************************************************************/
uint16_t drawRGB24toRGB565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r / 8) << 11) | ((g / 4) << 5) | (b / 8);
}



