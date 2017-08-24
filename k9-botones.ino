/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// check button
void checkButton() {
    // To check values when analog button are pressed
    abm.check();

    // step button check
    if (dbBtnPush.update() == 1) {
        // button changed

        // btn down
        if (dbBtnPush.fell()) {
            // start counting
            btnDownTime = millis();
        }

        // bton up
        if (dbBtnPush.rose()) {
            long t = millis() - btnDownTime;

            // check if hold
            if (t > 1000 and t < 3000) {
                // call action for buttons HOLD
                checkHoldButton();
            }

            // check on click
            if (t < 500 and t > 100) {
                checkPushButton();
            }

            // reset timer
            btnDownTime = millis();
        }
    }
}


// buttons definition
/******************************************************************************
 *   MENU       MAGIC         <(LEFT)         >(RIGHT)
 *
 * MENU
 *  Click
 *      MENU = go to selected mode
 *      OTHER = go to menu mode
 *  Hold = Cambia vfoA <> vfoB
 * MAGIC
 *  Click = usual mod by mode
 *  Hold = SWEEP = DB limits
 * < min of left
 * > max or rigth
 *
 * encoder push is OK
 *
 *****************************************************************************/


// check push button
void checkPushButton() {
    // do what you need to do in any case

    // MENU
    if (mode == MODE_MENU) {
        // apply the new mode
        mode = smode;
        changeMode();
    }

    // SIGEN
    if (mode == MODE_SIGEN) {
        // just change the step
        changeStep(1);
    }

    // SWEEP
    if (mode == MODE_SWEEP || mode == MODE_SA) {
        // start the scan
        makeScan();
    }

    // LC
    if (mode == MODE_LC) {
        // make calcs
        makeCCalcs();
    }

    // CONFIG
    if (mode == MODE_CONFIG) {
        // show config menu
        confSelected = !confSelected;

        // show subsettings
        subSettings();
    }
}


// check hold button
void checkHoldButton() {
    // do what you need to do in any case

    //~ // MENU
    //~ if (mode == MODE_MENU) {
        //~ // NOOP
    //~ }

    //~ // SIGEN
    //~ if (mode == MODE_SIGEN) {
        //~ // NOOP
    //~ }

    //~ // SWEEP
    //~ if (mode == MODE_SWEEP) {
        //~ // NOOP
    //~ }

    //~ // METER
    //~ if (mode == MODE_METER) {
        //~ // NOOP
    //~ }

    //~ // CONFIG
    //~ if (mode == MODE_CONFIG) {
        //~ // NOOP
    //~ }
}


// MENU button click
void bMenuClick() {
    // if not in menu return to it
    // if in menu just like OK
    if (mode != MODE_MENU) {
        // return to menu
        mode = MODE_MENU;
        changeMode();
    } else {
        // we are in menu, just like OK
        mode = smode;
        changeMode();
    }

    // save settings on exit
    saveEEPROM();
}


// MENU button hold
void bMenuHold() {
    // switch VFOs
    long temp = vfoB;
    vfoB = vfoA;
    vfoA = temp;

    // set freq
    setFreq(*mainFreq);

    // update interface
    changeMode();

    // save settings on exit
    saveEEPROM();
}


// MAGIC button click
void bMagicClick() {
    // Go to most common options in each mode

    //~ // MENU
    //~ if (mode == MODE_MENU) {
        //~ // NOOP
    //~ }

    // SIGEN
    if (mode == MODE_SIGEN) {
        // the most useful option is return to sweep mode
        mode = MODE_SWEEP;
        changeMode();
    }

    // SWEEP
    if (mode == MODE_SWEEP || mode == MODE_SA) {
        // most useful option is go to signal generator
        // to retune the center freq
        mode = MODE_SIGEN;
        changeMode();
    }

    // LC
    if (mode == MODE_LC) {
        // change to sweep mode
        mode = MODE_SWEEP;
        changeMode();
        // Set min level freq as center of the scan and
        // re do the scan
        *mainFreq = minf;
        makeScan();
    }

    //~ // CONFIG
    //~ if (mode == MODE_CONFIG) {
        //~ // NOOP
    //~ }
}


// MAGIC button click
void bMagicHold() {
    // Go to most common options in each mode

    //~ // MENU
    //~ if (mode == MODE_MENU) {
        //~ // NOOP
    //~ }

    //~ // SIGEN
    //~ if (mode == MODE_SIGEN) {
        //~ // NOOP
    //~ }

    // SWEEP
    if (mode == MODE_SWEEP) {
        // show the dB bandwidth
        //~ showDB();
    }

    //~ // METER
    //~ if (mode == MODE_METER) {
        //~ // NOOP
    //~ }

    //~ // CONFIG
    //~ if (mode == MODE_CONFIG) {
        //~ // NOOP
    //~ }
}


// LEFT button click
void bLeftClick() {
    // Go left or select next left option

    //~ // MENU
    //~ if (mode == MODE_MENU) {
        //~ // NOOP
    //~ }

    // SIGEN
    if (mode == MODE_SIGEN) {
        // mover el paso detras
        changeStep(-1);
    }

    // SWEEP & SA
    if (mode == MODE_SWEEP || mode == MODE_SA) {
        // Set min level freq as center of the scan and
        // re do the scan
        *mainFreq = minf;
        makeScan();
    }

    //~ // METER
    //~ if (mode == MODE_METER) {
        //~ // NOOP
    //~ }

    // LC
    if (mode == MODE_LC) {
        // move the cmult
        cmult = moveWithLimits(cmult, -1, 0, 3);

        // update display
        lcdUpdateC();
    }

    // CONFIG
    if (mode == MODE_CONFIG) {
        // mover el paso detras
        changeStep(-1);
    }
}


// RIGHT button click
void bRightClick() {
    // Go right or select next right option

    //~ // MENU
    //~ if (mode == MODE_MENU) {
        //~ // NOOP
    //~ }

    // SIGEN
    if (mode == MODE_SIGEN) {
        // mover el paso adelante
        changeStep(1);
    }

    // SWEEP & SA
    if (mode == MODE_SWEEP || mode == MODE_SA) {
        // sef vfo to max and make a scan there
        *mainFreq = maxf;
        makeScan();
    }

    //~ // METER
    //~ if (mode == MODE_METER) {
        //~ // NOOP
    //~ }

    // LC
    if (mode == MODE_LC) {
        // move the cmult
        cmult = moveWithLimits(cmult, +1, 0, 3);

        // update display
        lcdUpdateC();
    }

    // CONFIG
    if (mode == MODE_CONFIG) {
        // mover el paso adelante
        changeStep(1);
    }
}


/******************************************************************************
 * create the buttons, it has the next part in z-end file, check there
 *****************************************************************************/

// menu
Button bMENU = Button(695, &bMenuClick, &bMenuHold);

// magic
Button bMAGIC = Button(509, &bMagicClick, &bMagicHold);

// Less "<"
Button bLEFT = Button(367, &bLeftClick);

// More ">"
Button bRIGHT = Button(177, &bRightClick);
