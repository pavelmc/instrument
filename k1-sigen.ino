/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// draw the VFO box
void vfobox() {
    // caja superior para el VFO y usada en otros ámbitos
    drawMainVFObox();

    // VFO Primario
    mainFreqPrint();

    // Step
    stepPrint();

    // VFO Secundario o de reserva
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

//~ // draw the scale
//~ /********************* SCALE AND BAR *****************************************
 //~ * We use a trick here, as the scale is full wide we pass just the X var from
 //~ * where the scale must be drown
 //~ *
 //~ * For reference
 //~ *      Sigen = 88
 //~ *      Meter = ??
 //~ *****************************************************************************/
//~ void scale(byte X) {
    //~ /******
     //~ * Scale span is (y, x): 12, 108 - 309, 108
     //~ *****/
    //~ // box first
    //~ tft.drawRect(0, X, 320, 32, ILI9340_WHITE);

    //~ // mark the ticks, zero + 11 ticks
    //~ word y;
    //~ for (byte i = 0; i < 12; i++) {
        //~ y = 12 + (i * 27);
        //~ tft.drawLine(y, (X + 16), y, (X + 20), ILI9340_WHITE);
    //~ }

    //~ // print the numbers scale
    //~ tft.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
    //~ tft.setTextSize(1);

    //~ // rest of the numbers
    //~ char *marks[] = {"-80", "-70", "-60", "-50", "-40", "-30", "-20", "-10", " 0", "+10", "+20", "+30"};
    //~ for (byte i = 0; i < 12; i++) {
        //~ // print the numbers
        //~ tft.setCursor(4 + (i * 27), (X + 6));
        //~ tft.print(marks[i]);
    //~ }

    //~ // ending line in two steps white and red
    //~ tft.drawLine(12, (X + 20), 309, (X + 20), ILI9340_WHITE);
//~ }


//~ // la barra en movimiento, ver comentario en la escala
//~ void movingBar(byte X, word val, bool peph = false) {
    //~ /* Print a bar according to the max scale,
    //~ * we use 65535 as maximun as that is what we will handle, we must scale it before pass it
    //~ *
    //~ * the bars are a bunch of retangles that change from green to red half the way, filling the extra space
    //~ * with a blank long retangle to the end to erase any previous ones
    //~ *
    //~ */

    //~ // max value on the bar scale
    //~ word maxval = 309;

    //~ // map the value within the scale
    //~ val = map(val, 0, 65535, 12, maxval);

    //~ // the real part
    //~ tft.fillRect(12, (X + 22), val, 5, ILI9340_GREEN);

    //~ // the erase part
    //~ tft.fillRect(val, (X + 22), maxval + 10 - val, 5, ILI9340_BLACK);

    //~ // PEP holding
    //~ if (peph == true) {
        //~ // show a tick on the average of the last n readdings
        //~ // rotate the array and look for the highest
        //~ word maxv = 0;

        //~ for (byte i = 0; i < 9; i++) {
            //~ pep[i] = pep[i+1];
            //~ maxv = max(maxv, pep[i]);
        //~ }

        //~ pep[4] = val;
        //~ maxv = max(maxv, val);

        //~ // paint the rectangle
        //~ tft.fillRect(maxv - 2, (X + 22), 2, 5, ILI9340_CYAN);
    //~ }
//~ }
