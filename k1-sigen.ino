/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// draw the VFO box
void vfobox() {
    // draw vfo box
    drawMainVFObox();

    // main vfo
    mainFreqPrint();

    // Step
    stepPrint();

    // sub vfo
    subFreqPrint(true);
}


// update the main freq
void mainFreqPrint() {
    // set and prepare
    tft.setCursor(6, 30);
    tft.setTextColor(ILI9340_YELLOW, ILI9340_BLACK);
    tft.setTextSize(4);

    // prep freq for print
    prepFreq4Print(*mainFreq, true);

    // print it
    tft.print(f);
}


// update the sub freq
void subFreqPrint(bool main) {
    // set and prepare
    tft.setCursor(142, 66);
    tft.setTextColor(ILI9340_CYAN, ILI9340_BLACK);
    tft.setTextSize(2);

    // prep freq for print
    if (main) prepFreq4Print(*subFreq, true);
    else      prepFreq4Print(*mainFreq, true);

    // print it
    tft.print(f);
}


// update the step
void stepPrint() {
    tft.setCursor(10, 66);
    tft.setTextColor(ILI9340_BLUE, ILI9340_BLACK);
    tft.setTextSize(2);
    tft.print(stepLabels[step]);
}


// move the VFO
void moveVFO(char dir, bool print) {
    // move
    *mainFreq += getStep() * dir;

    // limit check
    if (*mainFreq < LIMI_LOW) *mainFreq = LIMI_HIGH;
    if (*mainFreq > LIMI_HIGH) *mainFreq = getStep();

    // update freq
    setFreq(*mainFreq);

    // update the LCD
    if (print) mainFreqPrint();
}
