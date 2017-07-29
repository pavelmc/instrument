

// SERIAL comms with the PC.

/******************************************************************************
 * This is the main interaction with the PC.
 * -> Send a freq + "\n" and we set the VFO to that (ex: "7110000\n")
 * -> Send just a "\n" and we answer with the reading of the level.
 *
 * All answers are terminated with an "\n"
 *
 * As simple as that.
 ******************************************************************************/
void serialComms() {
    // local vars
    char ch;
    static char sb[11];     // serial buffer
    static byte sbc = 0;    // serial buffer counter

    // wait for chars to come on the serial line
    while (Serial.available()) {
        // read a char
        ch = (char)Serial.read();

        // add the char only if a number
        if ((ch >= '0') && (ch <= '9')) sb[sbc++] = ch;

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

                // allow a time to settle
                delay(SCAN_PAUSE);
            } else {
                // send value via serial
                sendVal();
            }

            // reset counter and blank string
            sbc = 0;
            sb[sbc] = 0;
        }
    }
}



// send val of the reading for the actual VFO
void sendVal() {
    // Read ad values and calc mvolts (again)
    takeADCSamples();

    // send values via serial
    Serial.print(vl);
    Serial.println("");
}


// show pc mode
void showPCMode() {
    mainFreqPrint();
}
