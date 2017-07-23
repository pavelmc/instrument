/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/

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
#define debounceInterval  10    // in milliseconds
Bounce dbBtnPush = Bounce();
unsigned long btnDownTime = 0;

// lib instantiation as "Si"
#include "si5351mcu.h"
Si5351mcu Si;
//~ #include <si5351light.h> // https://github.com/etherkit/Si5351Arduino/releases/tag/v2.0.1
//~ Si5351light Si;

// analog buttons
#define BUTTONS_COUNT 8     // just 4 buttons
#include <BMux.h>
#define ANALOG_PIN A0

// Creating the AnalogButtons2 instance;
// 5 msec of debounce and 20 units of tolerance
BMux abm;


/****** VERSION INFORMATION ***********************/
#define VERSION 5

// vars

// this is the IF at 500kc with a polosa filter
#define VFO_OFFSET      455000  //  455kHz

/****** Frequency control related ******************************************/
long vfoA = 100000000L;        // VFO A
long vfoB =   7110000L;        // VFO B
long *mainFreq;              // main freq, the one it's used now
long *subFreq;               // the one in reserve
word pep[10];
char f[15];            // this is the frequency box like "145.170.670"
int ppm = 3670;        // this is the correction value for the si5351

// define the mixing xtal and jumping
// limits
#define LIMI_LOW      1000000   // 1 Mhz
#define LIMI_HIGH   220000000   // 100 Khz


/****** SWEEP related defines and vars **************************************/
byte sspan = 2;

unsigned long sweep_spans[] = {
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
    200000000       // 200MHz

};
char *sweep_spans_labels[] = {
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
    " 200MHz"
};

#define SPAN_COUNT  12

// scan limits
long scan_low, scan_high, sstep;

// measure limits
long minf, maxf;    // min/max feq values
word minfv = 65530;
word maxfv = 0;

// the delay pause, in milli seconds after each pause
#define SCAN_PAUSE  2
#define DATA_LEN    12 // bytes

//declare some vars related to the spi flash
byte flashDataPerPage;  // how many data object we have per page of 256 bytes
byte flashPagesInAScan; // cuantas páginas se lleva un scan.
word flashMaxScans;     // the amount of scans we dispose of
word flashPosition;     // the object space in the flash at we are now


/****** MODE related vars and defines ***************************************/
byte mode = 0;
byte smode = mode;      // selected mode in the menu selection
#define MODE_MENU       0
#define MODE_SIGEN      1
#define MODE_SWEEP      2
#define MODE_METER      3
#define MODE_CONFIG     4
#define MODE_COUNT      5

char *modeLabels[] = {
    "MODE SELECTOR",
    "SIGNAL GENERATOR",
    "SWEEP ANALYZER",
    "POWER METER",
    "SETTINGS"
};

/***** Step related vars en defines *****************************************/
byte step = 2;  // 100Hz
char *stepLabels[] = {
    "   1Hz",
    "  10Hz",
    " 100Hz",
    "  1kHz",
    " 10kHz",
    "100kHz",
    "  1MHz",
    " 10MHz"
};

/****** BAND data ***********************************************************/
#define HFVHF   5      // D5 by now
#define VHF     0
#define HF      1
boolean band =  0;


/****** config / settings vars and define ***********************************/
byte config = 0;
char *configLabels[] = {
    "Si5351 PPM",
    "Return to Menu"
};
boolean confSelected = false;
#define CONF_COUNT      2


/****** LCD print and formatting related ************************************/
char t[15];                 // this is a temp buffer
char empty[] = "     ";    // "empty" string to copy from


/****** ADC related vars ******************************************************
 *  A0 = Vrs    voltage raw (adc) source
 *  A1 = Vr50   voltage raw (adc) over 50 ohms resistor
 *  A2 = Vro    voltage raw (adc) output
 *  A3 = Vrl    voltage raw (adc) load
 *
 ******************************************************************************/
#define ADC_S   (A0)
#define ADC_50  (A1)
#define ADC_O   (A2)
#define ADC_L   (A3)

// raw vars in ADC units (0-1023)
word vrg = 0;
word vr50 = 0;
word vro = 0;
word vrl = 0;

// final values in mv * 10
word vg = 0;
word v50 = 0;
word vo = 0;
word vl = 0;

// special case, the load has an offset
word vlo = 0;

// ADC samples for uversampling, the real value is ADC_SAMPLES / ADC_DIVIDER (4)
#define ADC_SAMPLES     60
#define ADC_DIVIDER     10


/***** meter mode vars and defines ******************************************/
#define MEASURE_INTERVAL    250     // msecs
unsigned long nextMeasure = millis() + MEASURE_INTERVAL;


// the encoder need to move
void encoderMoved(char dir) {
    // do what you need to do in any case (Mode)
    switch (mode) {
        case MODE_MENU:
            // move in the menu selection
            moveMenu(dir);
            break;

        case MODE_SIGEN:
            // move the VFO but in which dir
            moveVFO(dir, true);
            break;

        case MODE_SWEEP:
            // move the scan span (sspan)
            moveSpanUpdate(dir);
            break;

        case MODE_METER:
            // select between measures for the main bar
            moveVFO(dir, false);
            // print main as sub
            subFreqPrint(false);
            break;

        case MODE_CONFIG:
            // showing or modifiying
            moveConfig(dir);
            break;
    }
}


// check button
void checkButton() {
    // To check values when analog button are pressed
    abm.check();

    // step button check
    if (dbBtnPush.update() == 1) {
        // button changed

        // btn down
        if (dbBtnPush.fell()) {
            // start counting
            btnDownTime = millis();
        }

        // bton up
        if (dbBtnPush.rose()) {
            long t = millis() - btnDownTime;

            // check if hold
            if (t > 1000 and t < 3000) {
                // call action for buttons HOLD
                checkHoldButton();
            }

            // check on click
            if (t < 500 and t > 100) {
                checkPushButton();
            }

            // reset timer
            btnDownTime = millis();
        }
    }
}

/*** Other function in files and z-end ***/
