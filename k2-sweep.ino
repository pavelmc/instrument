/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// draw the VFO box
void sweep_box() {
    // main box
    tft.drawRect(0, 22, TFT_WIDTH, 44, ILI9340_WHITE);

    // main vfo
    mainFreqPrint();

    // lower box
    tft.drawRect(0, 68, TFT_WIDTH, 70, ILI9340_WHITE);

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
    tft.print((char *)"SPAN: ");

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
 *      sstep = scan step (sweep_spans[sspan] / TFT_WIDTH) in hz
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
    sstep = sweep_spans[sspan] / TFT_WIDTH;

    // scan limts
    scan_low = *mainFreq - hs;
    scan_high = *mainFreq + hs;

    // limits check
    if (scan_low < LIMI_LOW) {
        // limit to low range
        scan_low = LIMI_LOW;
        // recalc step
        sstep = (scan_high - LIMI_LOW) / TFT_WIDTH;
    }

    if (scan_high > LIMI_HIGH) {
        // limit to low range
        scan_high = LIMI_HIGH;
        // recalc step
        sstep = (LIMI_HIGH - scan_low) / TFT_WIDTH;
    }

    // the var that hold the masurement
    word measure;
    word count = 0;

    //define last x
    word lx = 0;

    // cal the scan process
    makeScan2Flash(140, true);

    // calculate the range for the display
    word span = maxv - minv;

    // 15% increase either side
    word rangeEdges = (span * 15L) / 100;

    // calc min/max
    word tftmin;
    if (rangeEdges > minv)    tftmin = 0;     // can't be less than zero
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
        tftmax = TFT_HEIGHT - minv;
    }

    // determine -6dB, -3dB & -1dB; 89.12
    dB05l = (word)(maxv * 8912L / 10000);        // -0.5dB aka max * 0.8912
    dB1l  = (word)(maxv * 794L  / 1000);         // -1dB   aka max * 0.794
    dB3l  = maxv / 2;                            // -3dB   aka max * 0.5
    dB6l  = maxv / 4;                            // -6dB   aka max * 0.25
    dB9l  = maxv / 8;                            // -9dB   aka max * 0.125

    //  clean screen and draw graphic
    drawbars(tftmin, tftmax);

    #ifdef DEBUG
        // print serial headers
        Serial.println((char *)"freq;load");
    #endif

    // draw and spit via serial if debug
    for (word i = 0; i < TFT_WIDTH; i++) {
        // read the value from FLASH
        hs = flashReadData(i);

        // scale the masurement against min/max plus edges
        measure = map(vl, tftmin, tftmax, 0, TFT_HEIGHT);

        // draw the lines
        if (i > 0) {
            // draw the line just in the second step, as the first will be down
            tft.drawLine(i - 1 , (TFT_HEIGHT - lx), i, (TFT_HEIGHT - measure), ILI9340_CYAN);
        }

        // prepare for next cycle
        lx = measure;

        #ifdef DEBUG
            // "freq;load"
            Serial.print(hs);
            Serial.print(";");
            Serial.println(vl);
        #endif
    }

    // print min max
    // print labels, default size and color
    tft.setTextColor(ILI9340_YELLOW);
    tft.setTextSize(1);

    // min value
    tft.setCursor(5, 215);
    tft.print((char *)"MIN:");
    minmaxSweepValue(minv);
    tft.setCursor(28, 215);
    tft.print(f);
    prepFreq4Print(minf, true);
    tft.setCursor(5, 225);
    tft.print(f);

    // max value
    tft.setCursor(230, 215);
    tft.print((char *)"MAX:");
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
        // multiple of 3 = 3 bars (2 on the center)
        bars = 3;
    else
        // not mutiple of 3 = 4 bars (3 on the center)
        bars = 4;

    // erase the screen
    tft.fillScreen(ILI9340_BLACK);

    // vertical divisions
    for (word y = 0; y < TFT_WIDTH; y += (TFT_WIDTH / bars))
        tft.drawLine(y, 0, y, TFT_HEIGHT, ILI9340_WHITE);

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
    dB05 = map(dB05l, tftmin, tftmax, 0, TFT_HEIGHT);
    dB1  = map(dB1l, tftmin, tftmax, 0, TFT_HEIGHT);
    dB3  = map(dB3l, tftmin, tftmax, 0, TFT_HEIGHT);
    dB6  = map(dB6l, tftmin, tftmax, 0, TFT_HEIGHT);
    dB9  = map(dB9l, tftmin, tftmax, 0, TFT_HEIGHT);

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

    // print span on top + center freq
    tft.setCursor(140, 3);
    tft.setTextSize(1);
    tft.setTextColor(ILI9340_GREEN);
    tft.print(sweep_spans_labels[sspan]);
    prepFreq4Print((scan_high-scan_low) / 2 + scan_low, true);
    tft.setCursor(128, 13);
    tft.print(f);
}


// print db lines
// it has a ward mechanism: if it will be positioned beyond screen limits
// it will not be printed, nice
void printdBlines(int val, char *text) {
    if (val > 8 and val < (TFT_HEIGHT - 8)) {
        tft.drawLine(0, TFT_HEIGHT - val, TFT_WIDTH, TFT_HEIGHT - val, ILI9340_WHITE);
        tft.setCursor((TFT_WIDTH - 38), (TFT_HEIGHT - 8) - val);
        tft.print(text);
    }
}


// show the DB measurements
void showDB() {
    // vars
    long fc;

    //back to main interfae
    changeMode();

    // reset values
    fdb3s = fdb3e = fdb6s = fdb6e = 0;
    bw3db = bw6db = 0;

    // update the data from the mem
    for (word i = 0; i < TFT_WIDTH; i++) {
        // read the value from FLASH
        fc = flashReadData(i);
        // now vl has the data

        // -3dB
        if (fdb3s == 0 and vl > dB3l) fdb3s = fc;
        if (fdb3s != 0 and fdb3e == 0 and vl < dB3l) fdb3e = fc;

        // -6dB
        if (fdb6s == 0 and vl > dB6l) fdb6s = fc;
        if (fdb6s != 0 and fdb6e == 0 and vl < dB6l) fdb6e = fc;

        // draw progress bar
        tft.fillRect(0, 140, i, 5, ILI9340_GREEN);
    }

    // erase bar
    tft.fillRect(0, 140, TFT_WIDTH, 5, ILI9340_BLACK);

    // calc the bandwidth of each one
    bw3db = fdb3e - fdb3s;
    bw6db = fdb6e - fdb6s;

    #ifdef DEBUG
        Serial.println((char *)"3dB");
        Serial.print(bw3db);
        Serial.print(";");
        Serial.print(fdb3s);
        Serial.print(";");
        Serial.println(fdb3e);
    #endif

    // print

    // rectangle
    tft.drawRect(0, 148, TFT_WIDTH, 92, ILI9340_WHITE);

    // set font & color
    tft.setTextSize(2);
    tft.setTextColor(ILI9340_YELLOW);

    // -3dB label ------------------
    tft.setCursor(6, 152);
    tft.print((char *)"-3dB BW: ");

    // -3db val
    tft.setCursor(130, 152);
    prepFreq4Print(bw3db, true);
    tft.print(f);

    // -3db freqs
    tft.setTextColor(ILI9340_WHITE);
    tft.setCursor(4, 172);
    prepFreq4Print(fdb3s, true);
    tft.print(f);
    tft.setCursor(160, 172);
    prepFreq4Print(fdb3e, true);
    tft.print(f);

    // -6dB label ------------------
    tft.setTextColor(ILI9340_YELLOW);
    tft.setCursor(6, 192);
    tft.print((char *)"-6dB BW: ");

    // -6db val
    tft.setCursor(130, 192);
    prepFreq4Print(bw6db, true);
    tft.print(f);

    // -6db freqs
    tft.setTextColor(ILI9340_WHITE);
    tft.setCursor(4, 212);
    prepFreq4Print(fdb6s, true);
    tft.print(f);
    tft.setCursor(160, 212);
    prepFreq4Print(fdb6e, true);
    tft.print(f);
}


// initial scan into SPIFLASH, the pos is for the bar
void makeScan2Flash(byte pos, bool write2flash) {
    // increment the position, if instructed
    if (write2flash == true) flashNext();

    word count = 0;
    unsigned long hs;

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

        // save to flash only if needen
        if (write2flash == true) flashWriteData(count, hs);

        // count increase
        count += 1;

        // we need to update the bar here, TODO
        tft.fillRect(0, pos, count, 5, ILI9340_GREEN);
    }
}
