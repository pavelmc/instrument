
// setup
void setup() {
    // serial int
    Serial.begin(115200);

    // frequency pointer settings
    mainFreq = &vfoA;     // main freq, the one it's used now
    subFreq = &vfoB;      // the one in reserve

    // encoder push button setup
    pinMode(btnPush, INPUT_PULLUP);
    dbBtnPush.attach(btnPush);
    dbBtnPush.interval(debounceInterval);

    // set the analog as inputs
    pinMode(ADC_S, INPUT);
    pinMode(ADC_50, INPUT);
    pinMode(ADC_O, INPUT);
    pinMode(ADC_L, INPUT);

    // HVHF as output signaling hf or vhf operation
    pinMode(HFVHF, OUTPUT);

    // TFT settings
    tft.begin();

    // poner horizontal tipo wide screen
    tft.setRotation(3);

    // clear the screen
    tft.fillScreen(ILI9340_BLACK);

    // check the eeprom contents and load it
    checkInitEEPROM();

    // adc setups, while the Si oscillators are off
    setDiodeOffset();

    //~ // Si5351 start up
    //~ Si.init();

    //~ // set & apply my calculated correction factor
    //~ Si.correction(ppm);

    //~ // pre-load some sweet spot freqs
    //~ Si.setFreq(0, 150000000);
    //~ Si.setFreq(2, *mainFreq);

    //~ // explicit shutdown
    //~ Si.off();

    //~ // reset the PLLs
    //~ Si.reset();

    //~ // enable the principal output
    //~ Si.setPower(2, 3);
    //~ Si.enable(2);

    //initialize the SI5351
    si5351.init(SI5351_CRYSTAL_LOAD_8PF, 27000000L, 0);
    si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLA);
    si5351.set_pll(SI5351_PLL_FIXED, SI5351_PLLB);
    si5351.output_enable(SI5351_CLK0, 0);
    si5351.output_enable(SI5351_CLK1, 0);
    si5351.output_enable(SI5351_CLK2, 1);
    si5351.set_freq(500000000L , SI5351_CLK2);

    // draw the interface
    changeMode();

    // initialize the spi flash
    flash.begin();

    // do flash set for a few needed vars
    flashCalcs();
}


// loop
void loop(void) {
    // check for button routines
    checkButton();

    // encoder check
    checkEncoder();

    // in power meter mode we need to update the screen info
    if (mode == MODE_METER) powerMeasureAndShow();
}
