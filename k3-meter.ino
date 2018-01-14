/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// make a read every 100 mse cand average it
void powerMeasureAndShow() {
    // check if we need to measure
    if (millis() > nextMeasure) {
        // then we need to measure

        // voltage at the load
        meterRead();

        // print it
        printLevelmV();
        printLevelmW();
        printLeveldBm();

        // reset the next measure
        nextMeasure = millis() + MEASURE_INTERVAL;
    }
}


// show the interface
void showMeterMode() {
    // outer box
    drawMainVFObox();

    // mV level as main
    printLevelmV();

    // main VFO in the pos of the sub
    subFreqPrint(false);

    // step print
    stepPrint();
}


// print level in mV at load
void printLevelmV() {
    // the value is expected in vm as milli Volts

    // clan print buffers
    cleanPrintbuffer();

    // load the value to the temp buffer
    ltoa(vm, t, DEC);

    // prep the print buffer
    byte l = strlen(t);
    prepValue4Print(l);

    // add the mv at the end
    if (l < 6) strcat(f, " mV");
    else       strcat(f, " V");

    // 2 empty chars at the end
    strncat(f, &empty[0], 2);

    // set and prepare
    tft.setCursor(20, 30);
    tft.setTextColor(ILI9340_YELLOW, ILI9340_BLACK);
    tft.setTextSize(4);

    // print it
    tft.print(f);
}


// print level in mW at load
void printLevelmW() {
    // calc
    word mW = tomW(vm);

    // reset the print buffers
    cleanPrintbuffer();

    // load the value to the temp buffer
    ltoa(mW, t, DEC);

    // prep the print buffer
    prepValue4Print(strlen(t));

    // add the unit, as we will measure tops 0.5W, we will
    // use just mW
    strcat(f, " mW");

    // 2 empty chars at the end
    strncat(f, &empty[0], 2);

    // set and prepare
    tft.setCursor(20, 100);
    tft.setTextColor(ILI9340_YELLOW, ILI9340_BLACK);
    tft.setTextSize(4);

    // print it
    tft.print(f);
}


// print level in dBm at load.
// WATCH OUT! it has limited resolution by the way it's calculated
void printLeveldBm() {
    // calc
    long dBm = todBm(tomW(vm));

    // reset the print buffers
    cleanPrintbuffer();

    // load the value to the temp buffer
    itoa(dBm, t, DEC);

    // prep the print buffer
    prepValue4Print(strlen(t));

    // add the unit, as we will measure tops 0.5W, we will
    // use just mW
    strcat(f, " dBm");
    // 2 empty chars at the end
    strncat(f, &empty[0], 2);

    // set and prepare
    tft.setCursor(20, 140);
    tft.setTextColor(ILI9340_YELLOW, ILI9340_BLACK);
    tft.setTextSize(4);

    // print it
    tft.print(f);
}
