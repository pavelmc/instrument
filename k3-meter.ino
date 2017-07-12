
// make a read every 100 mse cand average it
void powerMeasureAndShow() {
    // check if we need to measure
    if (millis() > nextMeasure) {
        // then wee need to measure

        // voltage at the load
        vrl = takeSample(ADC_L);
        vdl = tomV(vrl, ADC_L);

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


// prep the f print buffer
void prepMeterBuffer(byte l) {
    switch (l) {
        case 6:
            // "65.536" u
            strncat(f, &t[0], 2);
            strcat(f, ".");
            strncat(f, &t[2], 3);
            break;

        case 5:
            // "3.450.6" mu
            strncat(f, &t[0], 1);
            strcat(f, ".");
            strncat(f, &t[1], 3);
            strcat(f, ".");
            strncat(f, &t[4], 1);
            break;

        case 4:
            // "  450.6" mu
            strncat(f, &empty[0], 2);
            strncat(f, &t[0], 3);
            strcat(f, ".");
            strncat(f, &t[3], 1);
            break;

        case 3:
            // "   50.6" mu
            strncat(f, &empty[0], 3);
            strncat(f, &t[0], 2);
            strcat(f, ".");
            strncat(f, &t[2], 1);
            break;

        case 2:
            // "    8.5" mu
            strncat(f, &empty[0], 4);
            strncat(f, &t[0], 1);
            strcat(f, ".");
            strncat(f, &t[1], 1);
            break;

        case 1:
            // "    0.1" mu
            strncat(f, &empty[0], 4);
            strcat(f, "0.");
            strcat(f, t);
            break;
    }
}


// print level in mV at load
void printLevelmV() {
    // the value is expected in vrl

    // reset the print buffers
    memset(t, 0, sizeof(t));
    memset(f, 0, sizeof(f));

    // load the value to the temp buffer
    ltoa(vdl, t, DEC);

    // prep the print buffer up to 65
    byte l = strlen(t);
    prepMeterBuffer(l);

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
    unsigned long p = mV2mW(vdl);

    // reset the print buffers
    memset(t, 0, sizeof(t));
    memset(f, 0, sizeof(f));

    // load the value to the temp buffer
    ltoa(p, t, DEC);

    // prep the print buffer up to 65
    byte l = strlen(t);
    prepMeterBuffer(l);

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
    int dBm = mW2dBm(mV2mW(vdl));

    // reset the print buffers
    memset(t, 0, sizeof(t));
    memset(f, 0, sizeof(f));

    // load the value to the temp buffer
    itoa(dBm, t, DEC);

    // prep the print buffer up to 65
    byte l = strlen(t);
    prepMeterBuffer(l);

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
