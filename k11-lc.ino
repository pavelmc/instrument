/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/

/******************************************************************************
 * Calc either L or C
 *
 * In parameter f (Mhz) and l (nH) or C (pF)
 * Out the other L or C in nH or pF
 ******************************************************************************/
unsigned long calcLorC(unsigned long f, long lorc) {
    // vars
    unsigned long ret;
    unsigned long f2;

    // scale and ^2
    f2 = f / 10000;
    f2 *= f2;

    // calc the dividend
    f2 *= lorc;
    f2 /= 100;

    // calc
    ret = 2533000000 / f2;

    // ret
    return ret;
}


// draw interface
void lcInterface() {
    // advice
    tft.setTextColor(ILI9340_CYAN);
    tft.setTextSize(2);
    tft.setCursor(2, 30);
    tft.print((char *)"Make a parallel LC circuit");
    tft.setCursor(2, 50);
    tft.print((char *)"put a good known cap on it");
    tft.setCursor(2, 70);
    tft.print((char *)"choose its value here, hit");
    tft.setCursor(2, 90);
    tft.print((char *)"OK button to know results.");

    // ask for cap
    tft.setTextColor(ILI9340_YELLOW);
    tft.setCursor(2, 120);
    tft.print((char *)"Cap value:");
    lcdUpdateC();

    // results
    tft.setCursor(2, 145);
    tft.print((char *)"Res Freq:");
    tft.setCursor(2, 165);
    tft.print((char *)"Inductance:");
}


// update the data
void lcdUpdateC() {
    // print selected cap value
    tft.setCursor(135, 120);
    tft.setTextColor(ILI9340_WHITE, ILI9340_BLACK);

    // reset the print buffers
    cleanPrintbuffer();

    // load the value to the temp buffer
    ltoa(getcap() * 10, t, DEC);

    // prep the print buffer
    prepValue4Print(strlen(t));

    // add ending fp and print
    strcat(f, " pF");
    tft.print(f);
}


// get the actual cap value
long getcap() {
    long resmult = 1;
    byte mult = cmult;
    while (mult--) {resmult *= 10;}

    // calc the value
    resmult *=  kcaps[cindex];

    // return
    return resmult;
}


// calc the inductor value
/****************************************************************************
 * The algorithm is like this
 *
 * - Make a scan from 0.1 MHz to 220.0 MHz
 *      detect the minimum
 * - Make a second scan with a span of 3 Mhz and centered on the min
 *      detect the minimum
 * - check freq, if less than 1Mhz bad result, else calc the value and show.
 *****************************************************************************/
void makeCCalcs() {
    // set scan parameters for the first round
    scan_low  =   1000000;                   //   1.0 Mhz
    scan_high = 220000000;                   // 220.0 MHz
    sstep = (scan_high - scan_low) / TFT_WIDTH;   // ~640 khz

    // make first scan
    makeScan2Flash(136, false);

    // erase bar
    tft.fillRect(0, 136, TFT_WIDTH, 5, ILI9340_BLACK);

    // now minf/minv has the aprox resonant freq

    // set it to center and +/- 1.5 Mhz
    scan_low  = minf - 1500000;               // -1.5 MHz
    scan_high = minf + 1500000;               // +1.5 MHz
    sstep = 9375;                            // ~10 khz

    // make second scan
    makeScan2Flash(136, false);

    // erase bar
    tft.fillRect(0, 136, TFT_WIDTH, 5, ILI9340_BLACK);

    // print freq
    tft.setTextColor(ILI9340_YELLOW, ILI9340_BLACK);
    tft.setCursor(112, 145);
    prepFreq4Print(minf, true);
    tft.print(f);

    // now minf has the value, calc
    long ind = calcLorC(minf, getcap);

    // reset the print buffers
    cleanPrintbuffer();

    // load the value to the temp buffer
    ltoa(ind * 10, t, DEC);

    // prep the print buffer
    prepValue4Print(strlen(t));

    // add ending fp and print
    strcat(f, " nH");
    tft.setCursor(140, 165);
    tft.print(f);
}
