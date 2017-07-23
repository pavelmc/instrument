

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
 *      minfv = min value
 *      maxfv = max value
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
            minfv = maxfv = vl;
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
    word span = maxfv - minfv;

    // 15% increase either side
    word rangeEdges = (word)((span * 15L) / 100);

    // calc min/max
    word tftmin;
    if (rangeEdges > minfv)    tftmin = 0;     // no pude ser menor que cero
    else    tftmin = minfv - rangeEdges;

    // max limit
    word tftmax = (word)(maxfv + rangeEdges);

    // new span with +/-15%
    span = tftmax - tftmin;

    // check for low limit
    if (span < 180) {
        // reset tft limits to a more confortable view
        if (tftmin > 60) tftmin -= 60;
        // whatever tftmin is tftmax if 240 above it.
        tftmax = 240 - minfv;
    }

    //  clean screen and draw graphic
    drawbars();

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
    minmaxSweepValue(minfv);
    tft.setCursor(28, 215);
    tft.print(f);
    prepFreq4Print(minf, true);
    tft.setCursor(5, 225);
    tft.print(f);

    // max value

    tft.setCursor(230, 215);
    tft.print("MAX:");
    minmaxSweepValue(maxfv);
    tft.setCursor(253, 215);
    tft.print(f);
    prepFreq4Print(maxf, true);
    tft.setCursor(230, 225);
    tft.print(f);
}


// draw bars
void drawbars() {
    // calc how many bars
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
    for (byte x = 0; x < 240; x += 80)
        tft.drawLine(0, x, 320, x, ILI9340_WHITE);

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
