/***************************************************
 * Multi-instrumento
 ****************************************************/

#include "SPI.h"
#include <EEPROM.h>
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9340.h"

// These are the pins used for the UNO
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

#define VERSION 5

// structured data: Main Configuration Parameters
struct mConf {
    byte ver = VERSION;
    long vfoa;
    long vfob;
    byte step;
    byte mode;
    int ppm;
};


// declaring the main configuration variable for mem storage
struct mConf conf;

// vars


/****** Frequency control related ******************************************/
long vfoA = 100000000L;        // VFO A
long vfoB =   7110000L;        // VFO B
long *mainFreq;              // main freq, the one it's used now
long *subFreq;               // the one in reserve
word pep[10];
char f[15];            // this is the frequency box like "145.170.670"
int ppm = 3670;        // this is the correction value for the si5351

// define the mixing xtal and jumping
#define XFO_FREQ    200000000   // 200 Mhz
#define SW_FREQ     100000000   // 100 Mhz

// limits
#define LIMI_LOW       100000   // 100 Khz
#define LIMI_HIGH   224000000   // 100 Khz


/****** SWEEP related defines and vars **************************************/
byte sspan = 2;

//~ unsigned long sweep_spans[] = { 320, 1280, 5120, 20480, 81920, 327680, 1310720,
    //~ 10485760, 83886080};
//~ char *sweep_spans_labels[] = { "  320Hz", "1.28kHz", "5.12kHz", "20.5kHz",
    //~ "81.9kHz", " 328kHz", "1.31MHz", "10.5MHz", "83.8MHz"};

unsigned long sweep_spans[] = {
    400,            // 400
    2000,           // 2k
    5000,           // 5k
    20000,          // 20k
    100000,         // 100k
    500000,         // 500k
    5000000,        // 5Mhz
    20000000,       // 20Mhz
    100000000       // 100MHz
};
char *sweep_spans_labels[] = {
    "  400Hz",
    "2.00kHz",
    "5.00kHz",
    "20.0kHz",
    " 100kHz",
    " 500kHz",
    " 5.0MHz",
    "20.0MHz",
    " 100MHz"
};



// scan limits
unsigned long scan_low, scan_high, sstep;

// measure limits
long minf, maxf;    // min/max feq values
word minfv = 65530;
word maxfv = 0;


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
char empty[] = "     ";      // "empty" string to copy from


/****** ADC related vars ******************************************************
 *  A0 = Vrs    voltage raw (adc) source
 *  A1 = Vr50   voltage raw (adc) over 50 ohms resistor
 *  A2 = Vro    voltage raw (adc) output
 *  A3 = Vrl    voltage raw (adc) load
 *
 ******************************************************************************/
#define ADC_S   0
#define ADC_50  1
#define ADC_O   2
#define ADC_L   3

// raw vars in ADC units (0-1023)
unsigned long vrs = 0;
unsigned long vr50 = 0;
unsigned long vro = 0;
unsigned long vrl = 0;

// final values in mv * 10
unsigned long vds = 0;
unsigned long vd50 = 0;
unsigned long vdo = 0;
unsigned long vdl = 0;

// ADC samples for uversampling, the real value is ADC_SAMPLES / ADC_DIVIDER (4)
#define ADC_SAMPLES     60
#define ADC_DIVIDER     10

#define DIODES  true // we use diodes instead of AD3807 for now

#ifndef DIODES
    // var related to AD3807
    // FLOAT values that holds the result of the calculations
    float dBm = 0;      // will hold the dBm result of the adc units
    float watt = 0;     // will hold the watt result of the dBm in the environment
    float volts = 0;    // will hold the volts result of the dBm in the environment
    float dBuV = 0;     // will hold the dBuV result of the dBm in the environment

    // ADc conversion vars
    float AD_a=0.09;
    float AD_b=-83.49;
#else
    // vars related to diode measurements
    word vsrs = 0;
    word vsr50 = 0;
    word vsro = 0;
    word vsrl = 0;
#endif


/***** moeter mode vars and defines ******************************************/
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


// check push button
void checkPushButton() {
    // do what you need to do in any case
    switch (mode) {
        case MODE_MENU:
            // apply the new mode
            mode = smode;
            changeMode();
            break;

        case MODE_SIGEN:
            // just change the step
            changeStep();
            break;

        case MODE_SWEEP:
            // start the scan
            makeScan();
            break;

        case MODE_METER:
            // do what?
            changeStep();

            break;

        case MODE_CONFIG:
            // show config menu
            confSelected = !confSelected;

            // show subsettings if 0, if 1 is return
            if (config == 0) {
                subSettings();
            } else {
                // reset selection
                confSelected = false;

                // set mode - Main Menu
                mode = 0;

                // update eeprom for the mode
                saveEEPROM();

                // draw menu
                menu();
            }

            break;
    }

}


// check hold button
void checkHoldButton() {
    // do what you need to do in any case
    switch (mode) {
        case MODE_SIGEN:
            // menu
            back2menu();
            break;

        case MODE_SWEEP:
            // menu
            back2menu();
            break;

        case MODE_METER:
            // menu
            back2menu();
            break;

        case MODE_CONFIG:
            // menu
            back2menu();
            break;
    }
}


// check button
void checkButton() {
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
