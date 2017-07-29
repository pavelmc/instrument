
// menu functions
void menu() {
    //fail safe
    if (smode == 0) smode = 1;
    // draw the main box
    tft.drawRect(0, 22, 320, 218, ILI9340_WHITE);

    // set text size
    tft.setTextSize(2);
    tft.setTextColor(ILI9340_CYAN, ILI9340_BLACK);

    for (byte i = 1; i < MODE_COUNT; i++) {

        // print the mode number, activated or not.
        tft.setCursor(12, 26 + 26 * i);
        // mark if selected
        if (smode == i) tft.setTextColor(ILI9340_BLACK, ILI9340_YELLOW);

        // print the number
        tft.print(i);

        // label of the mode
        tft.setCursor(30, 26 + 26 * i);

        // print the mode label, always on the same color
        tft.setTextColor(ILI9340_CYAN, ILI9340_BLACK);
        tft.print(modeLabels[i]);
    }
}


// mode change
void changeMode() {
    // change the mode between the modes

    // erase the screen
    tft.fillScreen(ILI9340_BLACK);

    // redraw the top box
    drawtopbox();

    switch (mode) {
        case MODE_MENU:
            // draw the main menu
            menu();
            break;

        case MODE_SIGEN:
            // draw the signal generator interface
            vfobox();
            scale(88);
            break;

        case MODE_SWEEP:
            // draw the sweeper interface
            sweep_box();
            break;

        case MODE_METER:
            // draw the meter interface
            showMeterMode();
            break;

        case MODE_PC:
            // draw the PC interface
            showPCMode();
            break;

        case MODE_CONFIG:
            // draw the Configurations menu
            settings();
            break;
    }

    // save settings
    saveEEPROM();
}


// move between the menu entries
void moveMenu(char dir) {
    // increment smenu and pass it away to the menu...
    smode = moveWithLimits(smode, dir, 1, MODE_COUNT - 1);

    // call it
    menu();
}
