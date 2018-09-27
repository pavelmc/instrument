/***************************************************
 * Multi-instrumento
 *
 * Author: Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


/********** DEBUG MODE *************************/
// uncomment this to get a few hits via serial
//~ #define DEBUG true


#include "SPI.h"
#include <EEPROM.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"

// These are the pins used for SPI comms in the UNO
#define _sclk 13
#define _miso 12
#define _mosi 11
#define _dc 10
#define _rst 9
#define _cs 8

// Use hardware SPI
Adafruit_ILI9340 tft = Adafruit_ILI9340(_cs, _dc, _rst);

// TFT dimensions
#define TFT_WIDTH   320
#define TFT_HEIGHT  240
// some needed for scale
#define TFTH_13      TFT_HEIGHT / 3
#define TFTH_12      TFT_HEIGHT / 2
#define TFTH_23      TFTH_13 * 2


/*******************************************************************************
 *                        LCD coordinate model
 *
 *                               Y , X
 *
 *      0,0 **************************************************** 320,0
 *      *      y y y y y y y y y y                                   *
 *      *                                                            *
 *      * x                                                          *
 *      * x                                                          *
 *      * x                                                          *
 *      * x                                                          *
 *      * x                                                          *
 *      * x                                                          *
 *      * x                                                          *
 *      * x                                                          *
 *      * x                                                          *
 *      *                                                            *
 *      0,240 ************************************************ 324,240
 *
 *      - Coordinate system is always (y, x, +y, +x)
 *      - Text cursor is always referenced to the top left corner
 *
 * ****************************************************************************/

// Enable weak pullups in the rotary lib before inclusion
#define ENABLE_PULLUPS

// Flash memory
#include<SPIFlash.h>        // http://github.com/???/spiflash
SPIFlash flash(7);          // this is the PIN for the CHIP SELECT

// include the libs
#include <Rotary.h>         // https://github.com/mathertel/RotaryEncoder/
#include <Bounce2.h>        // https://github.com/thomasfredericks/Bounce2/

// define encoder pins
#define ENC_A    3              // Encoder pin A
#define ENC_B    2              // Encoder pin B
#define btnPush  4              // Encoder Button

// rotary encoder library setup
Rotary encoder = Rotary(ENC_A, ENC_B);

// the debounce instances
#define debounceInterval  30    // in milliseconds
Bounce dbBtnPush = Bounce();
//~ unsigned long btnDownTime = 0;

//#define SI_OVERCLOCK 950000000
// lib instantiation as "Si"
#include "si5351mcu.h"
Si5351mcu Si;

// analog buttons
#define BUTTONS_COUNT 4     // just 4 buttons
#include <BMux.h>
#define ANALOG_PIN A0

// Creating the AnalogButtons instance;
// defauult 5 msec of debounce and 20 units of tolerance
BMux abm;


/****** VERSION INFORMATION ***********************/
#define VERSION 6

// vars

// this is the IF with 27.000 Mhz XTALs
#define VFO_OFFSET      26994600       // default value

/****** Frequency control related ******************************************/
long vfoA = 100000000L;     // VFO A
long vfoB =   7110000L;     // VFO B
long *mainFreq;             // main freq, the one it's used now
long *subFreq;              // the one in reserve
int ppm = 3650;             // this is the correction value for the si5351
long  vfoOffset = VFO_OFFSET;
bool coff = false;          // whether the carrier must be on/off in PC mode

// define the mixing xtal and jumping
// limits
#define LIMI_LOW       100000  // 100 kHz
#define LIMI_HIGH   225000000 - VFO_OFFSET //


/****** SWEEP related defines and vars **************************************/
byte sspan = 2;

const unsigned long sweep_spans[] = {
    320,            // 320
    1000,           // 1k
    3000,           // 3k
    10000,          // 10k
    30000,          // 30k
    100000,         // 100k
    300000,         // 300k
    1000000,        // 1M
    3000000,        // 3M
    10000000,       // 10MHz
    30000000,       // 30MHz
    100000000,      // 100MHz
    300000000       // 300MHz

};

const char *sweep_spans_labels[] = {
    "  320Hz",
    "1.00kHz",
    "3.00kHz",
    "10.0kHz",
    "30.0kHz",
    " 100kHz",
    " 300kHz",
    "1.00MHz",
    "3.00MHz",
    "10.0MHz",
    "30.0MHz",
    " 100MHz",
    " 300MHz"
};

#define SPAN_COUNT  12

/********************* SWEEP related vars *********************************/
// scan limits
long scan_low, scan_high, sstep;

// measure limits
long minf, maxf;    // min/max feq values
long minv, maxv;

// the delay pause, in milli seconds after each freq set
// to avoid the click on the Si5351 and allow the voltage to settle
#define SCAN_PAUSE  1


/****************** FLASH related vars ***********************************/

// declare some vars related to the spi flash
byte flashDataPerPage;  // how many data object we have per page of 256 bytes
byte flashPagesInAScan; // How many pages a scan covers.
word flashMaxScans;     // the amount of scans we dispose of
word flashPosition;     // the object space in the flash at we are now

// length of the data to write to the SPI flash
#define DATA_LEN    6 // bytes


/****** MODE related vars and defines ***************************************/
byte mode = 0;
byte smode = mode;      // selected mode in the menu selection
#define MODE_MENU       0
#define MODE_SIGEN      1
#define MODE_SWEEP      2
#define MODE_SA         3
#define MODE_METER      4
#define MODE_LC         5
#define MODE_PC         6
#define MODE_CONFIG     7
#define MODE_COUNT      8

const char *modeLabels[] = {
    "MODE SELECTOR",
    "SIGNAL GENERATOR",
    "SWEEP ANALYZER",
    "SPECTRUM ANALYZER",
    "POWER METER",
    "INDUCTANCE METER",
    "COMPUTER MODE",
    "SETTINGS"
};

/***** Step related vars en defines *****************************************/
byte step = 2;  // 100Hz
const char *stepLabels[] = {
    "   1Hz",
    "  10Hz",
    " 100Hz",
    "  1kHz",
    " 10kHz",
    "100kHz",
    "  1MHz",
    " 10MHz"
};

#define STEP_COUNT  7

/****** config / settings vars and define ***********************************/
byte config = 0;
const char *configLabels[] = {
    "Si5351 PPM",
    "VFO Offset {IF}"
};

boolean confSelected = false;
#define CONF_COUNT      2


/****** LCD print and formatting related ************************************/
char f[15];                 // this is the frequency box like "145.170.670"
char t[15];                 // this is a temp buffer
char empty[] = "     ";     // "empty" string to copy spaces from


/****** ADC related vars ******************************************************
 *  A0 = {Reserverd for the analog buttons}
 *  A1 = adcrR, dB: dB in the receiver side in dB *100
 *  A2 = adcrM, vm: voltages in the meter in raw adc unites / mv *10
 *
 ******************************************************************************/
#define ADC_M   (A1)
#define ADC_R   (A3)

// raw vars in ADC units (0-1023)
word adcrR = 0;   // Receptor
word adcrM = 0;   // Meter

// final values in mv * 10
word mVr = 0;   // mV read
word vm = 0;    // Meter
long dB = 0;    // dB in reference to the maximum of the scale
                // in dB * 100; 1 = 0.01; 10 = 0.10, 100 = 1.00
                // in PC mode it will default to * 100 instead of * 10

// ADC samples for oversampling
// how many EXTRA bits we want to get over the default 10 bits
#define ADC_OS      3   // max bits (13)
word max_samples = pow(2, (10 + ADC_OS)) - 1;

// This is the base level, if we are dealing with mV it's 0
// but if we handle it as dB it will be a real value
#define Base_level     -1090  //

// define the 5 volts
#define V5V 49000    // 4.9 volts in the arduino

/***** meter mode vars and defines ******************************************/

#define MEASURE_INTERVAL    250     // msecs
unsigned long nextMeasure = millis() + MEASURE_INTERVAL;


/********************* LC vars **********************************************/
// feel free to add your values here if you need more, please increment the
// var below if you do so
const byte kcaps[19] = {
     3,         //  3p  30p 300p
     5,         //  5p  50p 500p
     8,         //  8p  80p 800p
    10,         // 10p 100p 1n0
    12,         // 12p 120p 1n2
    15,         // 15p 150p 1n5
    18,         // 18p 180p 1n8
    20,         // 20p 200p 2n0
    22,         // 22p 220p 2n2
    24,         // 24p 240p 2n4
    33,         // 33p 330p 3n3
    39,         // 39p 390p 3n9
    47,         // 47p 470p 4n7
    52,         // 52p 520p 5n2
    56,         // 56p 560p 5n6
    68,         // 68p 680p 6n8
    75,         // 75p 750p 7n5
    82,         // 82p 820p 8n2
    91,         // 91p 910p 9n2
};

#define CAP_MAX 18

// cap index
byte cindex = 0;

// cap multiplier
// final var is kcap[cindex] * 10^cmult
byte cmult = 0;


// the encoder need to move
void encoderMoved(char dir) {
    // do what you need to do in any case (Mode)

    // MENU
    if (mode == MODE_MENU) {
        // move in the menu selection
        moveMenu(dir);
    }

    // SIGEN
    if (mode == MODE_SIGEN) {
        // move the VFO but in which dir
        moveVFO(dir, true);
    }

    // SWEEP
    if (mode == MODE_SWEEP || mode == MODE_SA) {
        // move the scan span (sspan)
        moveSpanUpdate(dir);
    }

    // METER
    if (mode == MODE_METER) {
        // select between measures for the main bar
        moveVFO(dir, false);
        // print main as sub
        subFreqPrint(false);
    }

    // CONFIG
    if (mode == MODE_CONFIG) {
        // showing or modifiying
        moveConfig(dir);
    }

    // LC
    if (mode == MODE_LC) {
        // move the cindex
        cindex = moveWithLimits(cindex, dir, 0, CAP_MAX);

        // update display
        lcdUpdateC();
    }
}

/*** Other function in k#-files and z-end ***/
