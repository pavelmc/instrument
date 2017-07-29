/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// setup
void setup() {
    // serial int
    Serial.begin(57600);   // 1/4 Mbps

    // frequency pointer settings
    mainFreq = &vfoA;     // main freq, the one it's used now
    subFreq = &vfoB;      // the one in reserve

    // encoder push button setup
    pinMode(btnPush, INPUT_PULLUP);
    dbBtnPush.attach(btnPush);
    dbBtnPush.interval(debounceInterval);

    // set the analog as inputs
    pinMode(ADC_M, INPUT);
    pinMode(ADC_L, INPUT);

    // TFT settings
    tft.begin();

    // poner horizontal tipo wide screen
    tft.setRotation(3);

    // clear the screen
    tft.fillScreen(ILI9340_BLACK);

    // check the eeprom contents and load it
    checkInitEEPROM();

    // Si5351 start up
    Si.init();
    Si.off();

    // now we have the Si5351 off, we take the base readings for the meter
    vbm = takeSample(ADC_M);

    // set & apply my calculated correction factor
    Si.correction(ppm);

    // set power
    Si.setPower(0, 0);
    Si.setPower(2, 0);

    // enable outputs
    Si.enable(0);
    Si.enable(2);

    // pre-load the output freq
    setFreq(vfoA);

    // draw the interface
    changeMode();

    // initialize the spi flash
    flash.begin();

    // do flash set for a few needed vars
    flashCalcs();

    // analog buttons settings
    abm.init(ANALOG_PIN, 5, 20);

    // add buttons
    abm.add(bMENU);
    abm.add(bMAGIC);
    abm.add(bLEFT);
    abm.add(bRIGHT);
}


// loop
void loop(void) {
    // check for button routines
    checkButton();

    // encoder check
    checkEncoder();

    // in power meter mode we need to update the screen info
    if (mode == MODE_METER) powerMeasureAndShow();

    // serial coms
    if (mode == MODE_PC) serialComms();
}
