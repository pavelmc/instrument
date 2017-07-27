

// draw the VFO box
void sweep_box() {
    // linea gruesa, aka: dos rectangulos
    tft.drawRect(0, 22, 320, 44, ILI9340_WHITE);

    // VFO Primario
    mainFreqPrint();

    // lower box
    tft.drawRect(0, 68, 320, 70, ILI9340_WHITE);

    // update the display
    // print span and label
    printSpan();

    // update the scan limits
    scan_limits();
}


// print span value
void printSpan() {
    // print span label
    tft.setTextColor(ILI9340_YELLOW, ILI9340_BLACK);
    tft.setCursor(12, 74);
    tft.setTextSize(2);
    tft.print("SPAN: ");

    // span
    tft.setCursor(80, 74);
    tft.setTextSize(2);
    tft.print(sweep_spans_labels[sspan]);
}


// print scan limits
void scan_limits() {
    long hs = (sweep_spans[sspan] / 2);
    long tmp;

    // start
    tmp = *mainFreq - hs;
    if (tmp < LIMI_LOW ) {
        tft.setTextColor(ILI9340_RED, ILI9340_BLACK);
        tmp = LIMI_LOW;
    } else {
        tft.setTextColor(ILI9340_CYAN, ILI9340_BLACK);
    }

    prepFreq4Print(tmp, true);
    tft.setCursor(18, 94);
    tft.setTextSize(2);
    tft.print(f);

    // end
    tmp = *mainFreq + hs;
    if (tmp > LIMI_HIGH) {
        tft.setTextColor(ILI9340_RED, ILI9340_BLACK);
        tmp = LIMI_HIGH;
    } else {
        tft.setTextColor(ILI9340_CYAN, ILI9340_BLACK);
    }

    prepFreq4Print(tmp, true);
    tft.setCursor(18, 118);
    tft.setTextSize(2);
    tft.print(f);
}


// move span and update display
void moveSpanUpdate(char dir) {
    // update the value
    sspan = moveWithLimits(sspan, dir, 0, SPAN_COUNT);

    // update the display
    // print span and label
    printSpan();

    // update the scan limits
    scan_limits();
}


// make the scan
/*********************** do scan *********************************************
 * we have:
 *      *mainFreq = central freq
 *      sweep_spans[sspan] = the full scan span
 *
 * we must calc
 *      scan_low = scan init
 *      scan_high = scan end
 *      sstep = scan step ( = sweep_spans[sspan] / 320) in hz
 *
 * We need to define and keep account of min an max
 *      minf = freq of the minimum value
 *      maxf = freq of the maximun value
 *      minv = min value
 *      maxv = max value
 *
 *****************************************************************************/
void makeScan() {
    // half scan span, reused below as freq seeping
    long hs = sweep_spans[sspan] / 2;

    // scan step
    sstep = sweep_spans[sspan] / 320;

    // scan limts
    scan_low = *mainFreq - hs;
    scan_high = *mainFreq + hs;

    // limits check
    if (scan_low < LIMI_LOW) {
        // limit to low range
        scan_low = LIMI_LOW;
        // recalc step
        sstep = (scan_high - LIMI_LOW) / 320;
    }

    if (scan_high > LIMI_HIGH) {
        // limit to low range
        scan_high = LIMI_HIGH;
        // recalc step
        sstep = (LIMI_HIGH - scan_low) / 320;
    }

    // the var that hold the masurement
    word measure;
    word count = 0;

    //define last x
    word lx = 0;

    // initial scan into SPIFLASH
    flashNext();    // increment the position

    // do the scan to flash
    for (hs = scan_low; hs < scan_high; hs += sstep) {
        //set the frequency
        setFreq(hs);

        // allow a time to settle
        delay(SCAN_PAUSE);

        // take samples and convert it to mV
        takeADCSamples();

        // start point settings at first time
        if (hs == scan_low) {
            // reset the low and hig parameters
            minf = maxf = scan_low;

            // set min/max to this parameter
            minv = maxv = vl;
        }

        //track min/max
        trackMinMax(vl, hs);

        // write to FLASH
        flashWriteData(count, hs);

        // count increase
        count += 1;

        // we need to update the bar here, TODO
        tft.fillRect(0, 140, count, 5, ILI9340_GREEN);
    }

    // calculate the range for the display
    word span = maxv - minv;

    // 15% increase either side
    word rangeEdges = (span * 15L) / 100;

    // calc min/max
    word tftmin;
    if (rangeEdges > minv)    tftmin = 0;     // no pude ser menor que cero
    else    tftmin = minv - rangeEdges;

    // max limit
    word tftmax = (word)(maxv + rangeEdges);

    // new span with +/-15%
    span = tftmax - tftmin;

    // check for low limit
    if (span < 180) {
        // reset tft limits to a more confortable view
        if (tftmin > 60) tftmin -= 60;
        // whatever tftmin is tftmax if 240 above it.
        tftmax = 240 - minv;
    }

    // determine -6dB, -3dB & -1dB; 89.12
    dB05l = (word)(maxv * 8912L / 10000);        // -0.5dB aka max * 0.8912
    dB1l  = (word)(maxv * 794L  / 1000);          // -1dB aka max * 0.794
    dB3l  = maxv / 2;                            // -3dB aka max * 0.5
    dB6l  = maxv / 4;                            // -6dB aka max * 0.25
    dB9l  = maxv / 8;                            // -9dB aka max * 0.125
    word color;

    //  clean screen and draw graphic
    drawbars(tftmin, tftmax);

    // print serial headers
    Serial.println("freq;load");

    // draw and spit via serial
    for (word i = 0; i < 320; i++) {
        // read the value from FLASH
        hs = flashReadData(i);

        // scale the masurement against min/max plus edges
        measure = map(vl, tftmin, tftmax, 0, 240);

        // draw the lines
        if (i > 0) {
            // draw the line just in the second step, as the first will be down
            tft.drawLine(i - 1 , (240 - lx), i, (240 - measure), ILI9340_CYAN);
        }

        // prepare for next cycle
        lx = measure;

        // "freq;load"
        Serial.print(hs);
        Serial.print(";");
        Serial.println(vl);
    }

    // print min max
    // print labels, default size and color
    tft.setTextColor(ILI9340_YELLOW);
    tft.setTextSize(1);

    // min value
    tft.setCursor(5, 215);
    tft.print("MIN:");
    minmaxSweepValue(minv);
    tft.setCursor(28, 215);
    tft.print(f);
    prepFreq4Print(minf, true);
    tft.setCursor(5, 225);
    tft.print(f);

    // max value

    tft.setCursor(230, 215);
    tft.print("MAX:");
    minmaxSweepValue(maxv);
    tft.setCursor(253, 215);
    tft.print(f);
    prepFreq4Print(maxf, true);
    tft.setCursor(230, 225);
    tft.print(f);
}


// draw bars
void drawbars(word tftmin, word tftmax) {
    // calc how many vertical lines
    byte bars = sweep_spans[sspan] % 3 ;
    if (bars == 0)
        // multiple of 3 = 4 bars (2 on the center)
        bars = 3;
    else
        // not mutiple of 3 = 5 bars (3 on the center)
        bars = 4;

    // erase the screen
    tft.fillScreen(ILI9340_BLACK);

    // vertical divisions
    for (word y = 0; y < 320; y += (320 / bars))
        tft.drawLine(y, 0, y, 240, ILI9340_WHITE);

    // horizontal lines
    /************************************************************************
     * We will draw lines for
     * -0.5dB
     * -1db
     * -3db
     * -6dB
     * -9dB
     ************************************************************************/
    int dB05, dB1, dB3, dB6, dB9;
    dB05 = map(dB05l, tftmin, tftmax, 0, 240);
    dB1  = map(dB1l, tftmin, tftmax, 0, 240);
    dB3  = map(dB3l, tftmin, tftmax, 0, 240);
    dB6  = map(dB6l, tftmin, tftmax, 0, 240);
    dB9  = map(dB9l, tftmin, tftmax, 0, 240);

    // put dB labels, if possible
    tft.setTextColor(ILI9340_WHITE);
    tft.setTextSize(1);

    // print horizontal lines
    printdBlines(dB05, "-0.5dB");
    printdBlines(dB1, "-1dB");
    printdBlines(dB3, "-3dB");
    printdBlines(dB6, "-6dB");
    printdBlines(dB9, "-9dB");

    // print labels, default size and color
    tft.setTextColor(ILI9340_YELLOW);
    tft.setTextSize(1);

    // start
    tft.setCursor(5, 5);
    prepFreq4Print(scan_low, true);
    tft.print(f);

    // end
    tft.setCursor(230, 5);
    prepFreq4Print(scan_high, true);
    tft.print(f);

    // print span on top
    tft.setCursor(120, 5);
    tft.setTextSize(2);
    tft.setTextColor(ILI9340_GREEN);
    tft.print(sweep_spans_labels[sspan]);
}


// print db lines
void printdBlines(int val, char *text) {
    if (val > 8 and val < 232) {
        tft.drawLine(0, 240 - val, 320, 240 - val, ILI9340_WHITE);
        tft.setCursor(282, 232 - val);
        tft.print(text);
    }
}

//
void makeScan2Min() {
    *mainFreq = minf;
    makeScan();
}

//
void makeScan2Max() {
    *mainFreq = maxf;
    makeScan();
}
