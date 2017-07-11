
// make a read every 100 mse cand average it
void powerMeasureAndShow() {
    // check if we need to measure
    if (millis() > nextMeasure) {
        // then wee need to measure

        // voltage at the load
        vrl = takeSample(ADC_L);

        // print it
        printLevelmV();

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
    // calc
    long p = tomV(vrl, ADC_L);

    // reset the print buffers
    memset(t, 0, sizeof(t));
    memset(f, 0, sizeof(f));

    // load the value to the temp buffer
    ltoa(p, t, DEC);

    byte l = strlen(t);

    switch (l) {
        case 5:
            // "3.450.6 mv
            strncat(f, &t[0], 1);
            strcat(f, ".");
            strncat(f, &t[1], 3);
            strcat(f, ".");
            strncat(f, &t[4], 1);
            break;

        case 4:
            // "  450.6 mv
            strncat(f, &empty[0], 2);
            strncat(f, &t[0], 3);
            strcat(f, ".");
            strncat(f, &t[3], 1);
            break;

        case 3:
            // "   50.6 mv
            strncat(f, &empty[0], 3);
            strncat(f, &t[0], 2);
            strcat(f, ".");
            strncat(f, &t[2], 1);
            break;

        case 2:
            // "    8.5 mv
            strncat(f, &empty[0], 4);
            strncat(f, &t[0], 1);
            strcat(f, ".");
            strncat(f, &t[1], 1);
            break;

        case 1:
            strncat(f, &empty[0], 4);
            strncat(f, &empty[0], 2);
            strcat(f, t);
            break;
    }

    // add the mv at the end
    strcat(f, " mV");
    strncat(f, &empty[0], 2);

    // set and prepare
    tft.setCursor(10, 30);
    tft.setTextColor(ILI9340_YELLOW, ILI9340_BLACK);
    tft.setTextSize(4);

    // print it
    tft.print(f);
}
