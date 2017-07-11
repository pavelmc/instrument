

// draw the VFO box
void sweep_box() {
    // linea gruesa, aka: dos rectangulos
    tft.drawRect(0, 22, 320, 44, ILI9340_WHITE);

    // VFO Primario
    mainFreqPrint();

    // lower box
    tft.drawRect(0, 68, 320, 70, ILI9340_WHITE);

    // print span value
    printSpan();

    // print limits
    scan_limits();

}


// print span value
void printSpan() {
    // print span label
    tft.setCursor(12, 74);
    tft.setTextSize(2);
    tft.print("SPAN: ");

    // span
    tft.setCursor(80, 74);
    tft.setTextSize(2);
    tft.setTextColor(ILI9340_YELLOW, ILI9340_BLACK);
    tft.print(sweep_spans_labels[sspan]);
}


// scan limits
void scan_limits() {
    long hs = (sweep_spans[sspan] / 2);

    // start
    tft.setTextColor(ILI9340_CYAN, ILI9340_BLACK);
    prepFreq4Print(*mainFreq - hs, true);
    tft.setCursor(18, 94);
    tft.setTextSize(2);
    tft.print(f);

    // end
    tft.setTextColor(ILI9340_CYAN, ILI9340_BLACK);
    prepFreq4Print(*mainFreq + hs, true);
    tft.setCursor(18, 118);
    tft.setTextSize(2);
    tft.print(f);
}


//
void moveSpanUpdate(char dir) {
    // update the value
    sspan = moveWithLimits(sspan, dir, 0, 8);
    // update the display
    printSpan();
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
    unsigned long hs = sweep_spans[sspan] / 2;
    // scan step
    sstep = sweep_spans[sspan] / 320;
    // scan limts
    scan_low = *mainFreq - hs;
    scan_high = *mainFreq + hs;
    // the var that hold the masurement
    word measure, point;
    word count = 0;
    //define last x
    word lx = 0;

    // set freq at the start point
    setFreq(scan_low);

    //  clean screen and draw bars
    drawbars();

    // print serial headers
    Serial.println("freq;mv");

    // reset the low and hig parameters
    minf = maxf = scan_low;
    minfv = 65535;
    maxfv = 0;

    // do the scan
    for (hs = scan_low; hs < scan_high; hs += sstep) {
        //set the frequency
        setFreq(hs);

        // allow a 10 msec to settle
        delay(5);

        // take sample
        measure = takeSample(ADC_L);

        // reuse point for the mv
        point = tomV(measure, ADC_L);

        //track min/max
        trackMinMax(point, hs);

        // print the graph
        measure = map(point, 0, 2500, 0, 240);
        /***** this is ---> x ; -^ is y top left is 0,0  ******/
        tft.drawLine(count -1 , (240 - lx), count, (240 - measure), ILI9340_CYAN);
        // prepare for next cycle
        lx = measure;

        // convert it to mV and spill it out by serial
        Serial.print(hs);
        Serial.print(";");
        Serial.println(point);

        // count increase
        count += 1;
    }

    // print min max
    // print labels, default size and color
    tft.setTextColor(ILI9340_YELLOW);
    tft.setTextSize(1);

    // min value
    // label
    tft.setCursor(5, 215);
    tft.print("MIN:");
    // value
    tft.setCursor(28, 215);
    memset(f, 0, sizeof(f));
    memset(t, 0, sizeof(t));
    strcat(f, "(");
    ltoa(minfv, t, DEC);
    strcat(f, t);
    strcat(f, ")");
    tft.print(f);
    // freq
    tft.setCursor(5, 225);
    prepFreq4Print(minf, true);
    tft.print(f);

    // max value
    // label
    tft.setCursor(250, 215);
    tft.print("MAX:");
    // value
    tft.setCursor(272, 215);
    memset(f, 0, sizeof(f));
    memset(t, 0, sizeof(t));
    strcat(f, "(");
    ltoa(maxfv, t, DEC);
    strcat(f, t);
    strcat(f, ")");
    tft.print(f);
    // freq
    tft.setCursor(240, 225);
    prepFreq4Print(maxf, true);
    tft.print(f);
}


// draw bars
void drawbars() {
    // erase the screen
    tft.fillScreen(ILI9340_BLACK);

    // vertical divisions
    for (word y = 0; y < 320; y += 80)
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
}
