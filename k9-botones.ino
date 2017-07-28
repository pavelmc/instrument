

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


// declaración de las rutinas de botones
/******************************************************************************
 *   MENU       MAGIC         <(LEFT)         >(RIGHT)
 *
 * MENU es para dejar lo que estas haciendo y volver al menu principal
 * MAGIC opciones muy usuales según el modo
 * < minimo o izquierda
 * > maximo o derecha
 *
 * Además esta el botón del push es es "OK" o select
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
    if (mode == MODE_SWEEP) {
        // start the scan
        makeScan();
    }

    // METER
    if (mode == MODE_METER) {
        // do what?
        changeStep(1);
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

    // MENU
    if (mode == MODE_MENU) {
        // NOOP
    }

    // SIGEN
    if (mode == MODE_SIGEN) {
        // NOOP
    }

    // SWEEP
    if (mode == MODE_SWEEP) {
        // NOOP
    }

    // METER
    if (mode == MODE_METER) {
        // NOOP
    }

    // CONFIG
    if (mode == MODE_CONFIG) {
        // NOOP
    }
}


// MENU button click
void bMenuClick() {
    // no importa donde, simplemente retornar al menu
    mode = MODE_MENU;
    changeMode();

    // save settings on exit
    saveEEPROM();
}


// MAGIC button click
void bMagicClick() {
    // Go to most common options in each mode

    // MENU
    if (mode == MODE_MENU) {
        // NOOP
    }

    // SIGEN
    if (mode == MODE_SIGEN) {
        // NOOP
    }

    // SWEEP
    if (mode == MODE_SWEEP) {
        // most useful option is go to signal generator
        // to retune the center freq
        mode = MODE_SIGEN;
        changeMode();
    }

    // METER
    if (mode == MODE_METER) {
        // NOOP
    }

    // CONFIG
    if (mode == MODE_CONFIG) {
        // NOOP
    }
}


// MAGIC button click
void bMagicHold() {
    // Go to most common options in each mode

    // MENU
    if (mode == MODE_MENU) {
        // NOOP
    }

    // SIGEN
    if (mode == MODE_SIGEN) {
        // NOOP
    }

    // SWEEP
    if (mode == MODE_SWEEP) {
        // show the dB bandwidth
        showDB();
    }

    // METER
    if (mode == MODE_METER) {
        // NOOP
    }

    // CONFIG
    if (mode == MODE_CONFIG) {
        // NOOP
    }
}


// LEFT button click
void bLeftClick() {
    // Go left or select next left option

    // MENU
    if (mode == MODE_MENU) {
        // NOOP
    }

    // SIGEN
    if (mode == MODE_SIGEN) {
        // mover el paso detras
        changeStep(-1);
    }

    // SWEEP
    if (mode == MODE_SWEEP) {
        // Set min level freq as center of the scan and
        // re do the scan
        makeScan2Min();
    }

    // METER
    if (mode == MODE_METER) {
        // NOOP
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

    // MENU
    if (mode == MODE_MENU) {
        // NOOP
    }

    // SIGEN
    if (mode == MODE_SIGEN) {
        // mover el paso adelante
        changeStep(1);
    }

    // SWEEP
    if (mode == MODE_SWEEP) {
        // sef vfo to max and make a scan there
        makeScan2Max();
    }

    // METER
    if (mode == MODE_METER) {
        // NOOP
    }

    // CONFIG
    if (mode == MODE_CONFIG) {
        // mover el paso adelante
        changeStep(1);
    }
}


/******************************************************************************
 *  Creando botones, tiene su contrapartida en setup, ver
 *****************************************************************************/

// menu
Button bMENU = Button(695, &bMenuClick);

// magic
Button bMAGIC = Button(509, &bMagicClick, &bMagicHold);

// Less "<"
Button bLEFT = Button(367, &bLeftClick);

// More ">"
Button bRIGHT = Button(177, &bRightClick);
