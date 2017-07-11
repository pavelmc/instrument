

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
    // a that uses float/double math
    //
    // we create our own pow() function using longs
    long ret = 1;
    for (byte i=0; i < (step +1); i++, ret *= 10);
    return ret/10;
}


// change the step
void changeStep() {
    // calculating the next step
    if (step < 7) {
        // simply increment
        step += 1;
    } else {
        // reset
        step = 0;
    }

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
  tft.drawRect(0, 0, 320, 20, ILI9340_WHITE);
  tft.fillRect(1, 1, 318, 18, ILI9340_BLUE);
  tft.setCursor(60, 3);
  tft.setTextColor(ILI9340_WHITE);
  tft.setTextSize(2);
  tft.println(modeLabels[mode]);
}


// prepare the freq for printing
void prepFreq4Print(long fr, boolean doHz) {
    // temp vars
    byte l = 0;

    // clear the buffers
    memset(f, 0, sizeof(f));
    memset(t, 0, sizeof(t));

    // load the freq to the temp buffer
    ltoa(fr, t, DEC);

    // check the size to know where to cut it down
    l = strlen(t);

    switch (l) {
        case 5: // "90.000"
            strncat(f, &empty[0], 5);   // "     "
            strncat(f, &t[0], 2);       //       "90"
            strcat(f, ".");             //          "."
            strncat(f, &t[2], 3);       //            "000"
            break;

        case 6: // "455.000"
            strncat(f, &empty[0], 4);   // "    "
            strncat(f, &t[0], 3);       //      "455"
            strcat(f, ".");             //          "."
            strncat(f, &t[3], 3);       //            "000"
            break;

        case 7: // "1.840.000"
            strncat(f, &empty[0], 2);   // "  "
            strncat(f, &t[0], 1);       //    "1"
            strcat(f, ".");             //      "."
            strncat(f, &t[1], 3);       //        "840"
            strcat(f, ".");             //            "."
            strncat(f, &t[4], 3);       //              "000"
            break;

        case 8: // "14.230.000"
            strncat(f, &empty[0], 1);   // " "
            strncat(f, &t[0], 2);       //   "14"
            strcat(f, ".");             //      "."
            strncat(f, &t[2], 3);       //        "230"
            strcat(f, ".");             //            "."
            strncat(f, &t[5], 3);       //              "000"
            break;

        case 9: // "145.170.000"
            strncat(f, &t[0], 3);       // "145"
            strcat(f, ".");             //     "."
            strncat(f, &t[3], 3);       //       "170"
            strcat(f, ".");             //           "."
            strncat(f, &t[6], 3);       //             "000"
            break;
    }

    // done; the result is in f, but without "Hz"

    // check if Hz
    if (doHz == true) strcat(f, "Hz");
}


//move a var between limits, usefull for options movement
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
/***************************************************************************
 * We will move in two main steps, that must be coordinates
 *
 * 100.00 to 200.0 Mhz (VHF) => This will be direct trough a filter
 * 0.0 to 100.000 Mhz   (HF) => This will be a mix from the following
 *      Fix 200 Mhz reference + range from 100 to 200 Mhz (inverting)
 *
 * We will use D5 to signal the switch as this
 *  D5 low is VHF
 *  D5 high is HF
 *
 * We use the Si5351mcu lib
 *  CLK2 will be the VHF
 *  CLK0 will be the 220Mhz reference.
 ***************************************************************************/
void setFreq(unsigned long f) {
    // internal freq, if the freq change more than 10 Khz must reset
    static unsigned long lastResetFreq = f;

    Si.setFreq(2, f);

    //~ // we will use the mainFreq pointer as the freq to output.
    //~ if (f > SW_FREQ) {
        //~ // above 100

        //~ // check if we are already there or need to change
        //~ if (band == HF) {
            //~ // no need to use the mixin
            //~ digitalWrite(HFVHF, 0);

            //~ // shut down the 220 Mhz reference
            //~ Si.disable(0);

            //~ // reset the band flag
            //~ band = VHF;
        //~ }

        //~ // set the freq on the correct out

    //~ } else {
        //~ // below 100 Mhz

        //~ // check if we are already there or need to change
        //~ if (band == VHF) {
            //~ // need to use the mixin
            //~ digitalWrite(HFVHF, 1);

            //~ // start the 220 Mhz reference
            //~ Si.enable(0);

            //~ // reset the band flag
            //~ band = HF;
        //~ }

        //~ // set the freq on the correct out
        //~ long nf = XFO_FREQ - f;
        //~ Si.setFreq(2, nf);
    //~ }

    // reset if bigger than 10k the steps
    if (abs(f - lastResetFreq) > 10000) {
        // reset
        Si.reset();

        // store the new pattern
        lastResetFreq = f;
    }
}


void drawMainVFObox() {
    // just draw the main VFO box
    tft.drawRect(0, 22, 320, 64, ILI9340_WHITE);
}


// track min/max
void trackMinMax(word val, unsigned long f) {
    // minimum
    if (val < minfv) {
        // new minimum
        minf = f;
        minfv = val;
    }

    // maximum
    if (val > maxfv) {
        // new max
        maxf = f;
        maxfv = val;
    }
}
