
// settings menu
void settings() {
    // draw the main box
    tft.drawRect(0, 22, 320, 218, ILI9340_WHITE);

    // set text size
    tft.setTextSize(2);
    tft.setTextColor(ILI9340_CYAN, ILI9340_BLACK);

    for (byte i = 0; i < CONF_COUNT; i++) {

        // print the mode number, activated or not.
        tft.setCursor(12, 52 + 26 * i);
        // mark if selected
        if (config == i) tft.setTextColor(ILI9340_BLACK, ILI9340_YELLOW);

        // print the number
        tft.print(i);

        // label of the mode
        tft.setCursor(30, 52 + 26 * i);

        // print the mode label, always on the same color
        tft.setTextColor(ILI9340_CYAN, ILI9340_BLACK);
        tft.print(configLabels[i]);
    }

    if (confSelected) {
        // draw sub settings
        subSettings();
    }
}


// specific setting sub menu
void subSettings() {
    // erase the box
    tft.fillRect(20, 160, 280, 60, ILI9340_BLACK);

    // draw box or not
    if (confSelected) {
        // draw border
        tft.drawRect(20, 160, 280, 60, ILI9340_WHITE);

        // select the case
        switch (config) {
            case 0:
                // ppm
                //print the label
                tft.setTextSize(2);
                tft.setTextColor(ILI9340_YELLOW);
                tft.setCursor(30, 164);
                tft.print(configLabels[config]);
                // set note
                prepFreq4Print(*mainFreq, false);
                tft.setTextColor(ILI9340_WHITE);
                tft.setCursor(160, 164);
                // print it
                tft.print(f);

                showPPM();
                break;

            case 1:
                //
                break;
        }
    } else {
        // getting out, save the configs
        saveEEPROM();
    }
}


// show the ppm value
void showPPM() {
    // reset the print buffers
    memset(t, 0, sizeof(t));
    memset(f, 0, sizeof(f));

    // load the value to the temp buffer
    itoa(ppm, t, DEC);

    // copy and add
    strcat(f, t);
    strcat(f, empty);

    // place it on the place
    tft.setTextSize(2);
    tft.setTextColor(ILI9340_WHITE, ILI9340_BLACK);
    tft.setCursor(30, 190);

    // print it
    tft.print(f);
}


// move config
void moveConfig(char dir) {
    // config has moved

    // selcting or modifying ?
    if (confSelected) {
        // config selected, modifiying
        switch (config) {
            case 0: // ppm
                // mod the ppm
                ppm += 10L * dir;

                // apply the change
                Si.correction(ppm);
                setFreq(*mainFreq);

                // show it
                showPPM();
                break;

            case 1: // ?
                //
                break;
        }
    } else {
        // move selection
        config = moveWithLimits(config, dir, 0, CONF_COUNT - 1);

        // update
        settings();
    }
}
