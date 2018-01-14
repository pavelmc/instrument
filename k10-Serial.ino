/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// SERIAL comms with the PC.
/******************************************************************************
 * This is the main interaction with the PC.
 * -> Send a freq + "\n" and we set the VFO to that (ex: "7110000\n")
 * -> Send just a "\n" and we answer with the reading of the level
 *  in raw ADC units
 *
 * All answers are terminated with an "\n"
 *
 * As simple as that.
 *
 * BEWARE!
 * there is a delay between the freq command and the data reading.
 *
 ******************************************************************************/
void serialComms() {
    // local vars
    static char sb[11];     // serial buffer
    static byte sbc = 0;    // serial buffer counter

    // wait for chars to come on the serial line
    if (Serial.available() > 0) {
        // temp vars
        char ch;

        // read a char
        ch = (char)Serial.read();

        // add the char only if a number
        if ((ch >= '0') && (ch <= '9')) sb[sbc++] = ch;

        // check if its a carrier on
        if (ch == 'e') coff = 0;

        // check if its a carrier off
        if (ch == 'd') coff = 1;

        // fail safe if sbc goes high on a random stream
        if (sbc > 11) sbc = 0;

        // parse command if its a newline [end of string]
        if (ch == '\n') {
            // terminate the string
            sb[sbc] = 0;

            // extract the value
            unsigned long temp = atol(sb);

            if (temp > LIMI_LOW & temp < LIMI_HIGH) {
                // put the sketch on that freq
                setFreq(temp);

                // delay to cope with spurs
                delay(SCAN_PAUSE);

                // Read ad values
                takeADCSamples(1);

                // send raw values via serial
                Serial.println(dB);
            }

            // reset counter and blank string
            sbc = 0;
            sb[sbc] = 0;
        }
    }
}


// show pc mode
void showPCMode() {
    // ADVICE PC control
    tft.setCursor(30, 40);
    tft.setTextColor(ILI9340_YELLOW);
    tft.setTextSize(4);
    tft.print("PC CONTROL!");

    // unplug mesg
    tft.setTextColor(ILI9340_CYAN);
    tft.setTextSize(2);
    tft.setCursor(10, 80);
    tft.print("Unplug the USB and");
    tft.setCursor(10, 100);
    tft.print("click MENU to leave");

}
