/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// function to get out of the specif option and back to menu
void back2menu() {
    // get back to menu
    mode = 0;
    changeMode();
}


// return the actual step
long getStep() {
    // we get the step from the global step var
    // pow(10,step) is the ideal function here but that eats 6% of the firmware
    // as that uses float/double math
    //
    // we create our own pow() function using longs
    long ret = 1;
    for (byte i=0; i < (step +1); i++, ret *= 10);
    return ret/10;
}


// change the step
void changeStep(char dir) {
    char nstep = (char)step + dir;

    // limit check
    if (nstep > STEP_COUNT) nstep = 0;
    if (nstep < 0) nstep = STEP_COUNT;

    // update real value
    step = (byte)nstep;

    // update the LCD
    stepPrint();
}


// interrupt for rotary
void checkEncoder() {
    unsigned char result = encoder.process();
    if (result) {
        if (result == DIR_CW) encoderMoved(+1);
        if (result == DIR_CCW) encoderMoved(-1);
    }
}


// draw the top box, the label depends on the mode
void drawtopbox() {
  // rectangle is (y, x, +y, +x, color)
  tft.drawRect(0, 0, TFT_WIDTH, 20, ILI9340_WHITE);
  tft.fillRect(1, 1, 318, 18, ILI9340_BLUE);
  tft.setCursor(60, 3);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(2);
  tft.println(modeLabels[mode]);
}


// clean the print buffer
void cleanPrintbuffer() {
    // reset the print buffers
    memset(t, 0, sizeof(t));
    memset(f, 0, sizeof(f));
}


// prepare the freq for printing
void prepFreq4Print(long fr, boolean doHz) {
    // clear the buffers
    cleanPrintbuffer();

    // load the freq to the temp buffer
    ltoa(fr, t, DEC);
    byte l = strlen(t);     // 26630

    // check size
    /*** by using if instead of switch/case we save a few bytes ***/

    if (l == 2) { // "90"
        strncat(f, &empty[0], 5);   // "     "
        strncat(f, &empty[0], 4);   //       "    "
        strcat(f, t);               //            "90"
    }

    if (l == 3) { // "900"
        strncat(f, &empty[0], 5);   // "     "
        strncat(f, &empty[0], 3);   //       "   "
        strcat(f, t);               //           "900"
    }

    if (l == 4) { // "9.000"
        strncat(f, &empty[0], 5);   // "     "
        strncat(f, &empty[0], 1);   //       " "
        strncat(f, &t[0], 1);       //        "9"
        strcat(f, ".");             //          "."
        strncat(f, &t[1], 3);       //            "000"
    }

    if (l == 5) { // "90.000"
        strncat(f, &empty[0], 5);   // "     "
        strncat(f, &t[0], 2);       //       "90"
        strcat(f, ".");             //          "."
        strncat(f, &t[2], 3);       //            "000"
    }

    if (l == 6) { // "455.000"
        strncat(f, &empty[0], 4);   // "    "
        strncat(f, &t[0], 3);       //      "455"
        strcat(f, ".");             //          "."
        strncat(f, &t[3], 3);       //            "000"
    }

    if (l == 7) { // "1.840.000"
        strncat(f, &empty[0], 2);   // "  "
        strncat(f, &t[0], 1);       //    "1"
        strcat(f, ".");             //      "."
        strncat(f, &t[1], 3);       //        "840"
        strcat(f, ".");             //            "."
        strncat(f, &t[4], 3);       //              "000"
    }

    if (l == 8) { // "14.230.000"
        strncat(f, &empty[0], 1);   // " "
        strncat(f, &t[0], 2);       //   "14"
        strcat(f, ".");             //      "."
        strncat(f, &t[2], 3);       //        "230"
        strcat(f, ".");             //            "."
        strncat(f, &t[5], 3);       //              "000"
    }

    if (l == 9) { // "145.170.000"
        strncat(f, &t[0], 3);       // "145"
        strcat(f, ".");             //     "."
        strncat(f, &t[3], 3);       //       "170"
        strcat(f, ".");             //           "."
        strncat(f, &t[6], 3);       //             "000"
    }

    // done; the result is in f, but without "Hz"

    // check if Hz
    if (doHz == true) strcat(f, "Hz");
}


/******************************************************************************
 * Prep value for print, for unit not freq
 *
 * This one assume that:
 *      - The value is loaded at t[]
 *      - The result will be placed at f[]
 *****************************************************************************/
void prepValue4Print(byte l) {
    switch (l) {
        case 7:
            // "965.536" u
            strncat(f, &t[0], 3);
            strcat(f, ".");
            strncat(f, &t[3], 3);
            break;

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


// move a var between limits, useful for options movement
byte moveWithLimits(byte var, char dir, byte low, byte high) {
    // temp value
    int t = (int)var + dir;

    // limits low
    if (t < low) t = high;

    // limits high
    if (t > high) t = low;

    // return it
    return (byte)t;
}


// set freq to the Si5351
void setFreq(unsigned long f) {
    // set main RF
    // but desctivate it if SA mode
    if (mode == MODE_SA) {
        // disabled
        Si.disable(2);
    } else {
        // set freq
         Si.setFreq(2, f);
    }

    // set VFO freq to obtain a VFO_OFFSET
    Si.setFreq(0, f + vfoOffset);
}


void drawMainVFObox() {
    // just draw the main VFO box
    tft.drawRect(0, 22, TFT_WIDTH, 64, ILI9340_WHITE);
}


// track min/max
void trackMinMax(long val, unsigned long f) {
    // minimum
    if (val <= minv) {
        // new minimum
        minf = f;
        minv = val;
    }

    // maximum
    if (val >= maxv) {
        // new max
        maxf = f;
        maxv = val;
    }
}


// convert adc units to mVolts (mV * 10)
// 1 V = 10000 aka /10000 for V
word tomV(word value) {
    // local vars
    unsigned long tmp;

    // now convert it to mv * 10
    tmp = value * 50000L;
    tmp /= ((ADC_SAMPLES / ADC_DIVIDER) * 1023);

    // return it
    return (word)tmp;
}


// convert mV * 10 to mW *10
unsigned long mV2mW(word mv) {
    /********************************************
     * power is  P = V^2 / 50
     *
     * but as we are working in mv * 10 the formula changes to
     *
     * p = (v^2 / (50 * 10000))
     *
     * to return mW * 10
     ********************************************/
    // rise volts to square
    unsigned long p = mv;
    p = mv * p;
    // divide bt 50 ohms plus correction
    p /= 500000;
    // now it has the power in mW * 10
    return p;
}


// convert mV *10 to dBm * 10
int mW2dBm(word mw) {
    /*********************************************************************
     * dBm is 10 * log10(mw)
     *
     * but we areworking in mW * 10, so the formula remains as is
     *
     * just you need to substract 10 to the result
     *
     * As we need dBm expressed in * 10
     * we multiply it by 10 and convert it to int
     ********************************************************************/
    // calc it
    double dbm = log10(mw) * 10;

    // it has a 10 shift, move
    dbm -= 10.0;

    // move the decimal point into integer area
    dbm *= 10.0;

    // return it
    return (int)dbm;
}


// process the values to show in the min/max in the sweep
void minmaxSweepValue(word value) {
    // reset the print buffers
    cleanPrintbuffer();

    // load the value to the temp buffer and prepare the final buffer
    ltoa(value, t, DEC);
    prepValue4Print(strlen(t));
    //strcat(f, " mV");

    // ready to print in the f[] buffer
}
