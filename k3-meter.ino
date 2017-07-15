
// make a read every 100 mse cand average it
void powerMeasureAndShow() {
    // check if we need to measure
    if (millis() > nextMeasure) {
        // then wee need to measure

        // voltage at the load
        takeADCSamples();

        // print it
        printLevelmV();
        printLevelmW();
        printLeveldBm();

        // reset the next measure
        nextMeasure = millis() + MEASURE_INTERVAL;
    }
}


// mostrar la interfaz
void showMeterMode() {
    // linea externa
    drawMainVFObox();

    // VFO Primario
    printLevelmV();

    // VFO primario pero como el sec
    subFreqPrint(false);

    // step print
    stepPrint();
}


// print level in mV at load
void printLevelmV() {
    // the value is expected in vl

    // clan print buffers
    cleanPrintbuffer();

    // load the value to the temp buffer
    ltoa(vl, t, DEC);

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
    word mW = mV2mW(vl);

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


// print level in dBm at load
void printLeveldBm() {
    // calc
    int dBm = mW2dBm(mV2mW(vl));

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
