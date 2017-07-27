

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
    switch (mode) {
        case MODE_MENU:
            // apply the new mode
            mode = smode;
            changeMode();
            break;

        case MODE_SIGEN:
            // just change the step
            changeStep(1);
            break;

        case MODE_SWEEP:
            // start the scan
            makeScan();
            break;

        case MODE_METER:
            // do what?
            changeStep(1);

            break;

        case MODE_CONFIG:
            // show config menu
            confSelected = !confSelected;

            // show subsettings if 0, if 1 is return
            if (config == 0) {
                subSettings();
            } else {
                // reset selection
                confSelected = false;

                // set mode - Main Menu
                mode = 0;

                // update eeprom for the mode
                saveEEPROM();

                // draw menu
                menu();
            }
            break;
    }
}


// check hold button
void checkHoldButton() {
    // do what you need to do in any case
    switch (mode) {
        case MODE_SIGEN:
            // menu
            break;

        case MODE_SWEEP:
            // menu
            break;

        case MODE_METER:
            // menu
            break;

        case MODE_CONFIG:
            // menu
            break;
    }
}


// MENU button click
void bMenuClick() {
    // no importa donde, simplemente retornar al menu
    mode = MODE_MENU;
    changeMode();
}


// MAGIC button click
void bMagicClick() {
    // Go to most common options in each mode
    switch (mode) {
        case MODE_SIGEN:
            // menu
            break;

        case MODE_SWEEP:
            // most useful option is go to signal generator
            // to retune the center freq
            mode = MODE_SIGEN;
            changeMode();
            break;

        case MODE_METER:
            // menu
            break;

        case MODE_CONFIG:
            // menu
            break;
    }
}


// LEFT button click
void bLeftClick() {
    // Go left or select next left option
    switch (mode) {
        case MODE_SIGEN:
            // mover el paso detras
            changeStep(-1);
            break;

        case MODE_SWEEP:
            // Set min level freq as center of the scan and
            // re do the scan
            makeScan2Min();
            break;

        case MODE_METER:
            // change unit of the measurement
            break;

        case MODE_CONFIG:
            // menu
            break;
    }
}


// RIGHT button click
void bRightClick() {
    // Go right or select next right option
    switch (mode) {
        case MODE_SIGEN:
            // mover el paso adelante
            changeStep(1);
            break;

        case MODE_SWEEP:
            // sef vfo to max and make a scan there
            makeScan2Max();
            break;

        case MODE_METER:
            // menu
            break;

        case MODE_CONFIG:
            // menu
            break;
    }
}


/******************************************************************************
 *  Creando botones, tiene su contrapartida en setup, ver
 *****************************************************************************/

// menu
Button bMENU = Button(695, &bMenuClick);

// magic
Button bMAGIC = Button(509, &bMagicClick);

// Less "<"
Button bLEFT = Button(367, &bLeftClick);

// More ">"
Button bRIGHT = Button(177, &bRightClick);
