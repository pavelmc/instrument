/***************************************************
 * Multi-instrumento
 *
 * Author: Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// take ADC samples and put the values on the environment
void takeADCSamples(bool hres) {
    // get mV level
    mVr = tomV(takeSample(ADC_R));
    // convert it to dB
    if (hres == 1) {
        // high res for the PC
        dB = todB(mVr, 1);
    } else {
        // low res fot the TFT
        dB = todB(mVr, 0);
    }


    // DEBUG
    #ifdef DEBUG
        Serial.print("dB: ");
        Serial.println(dB);
    #endif
}


// measure at the meter
void meterRead() {
    // temp var
    long tmp = 0;
    byte count = 5;

    // take sample
    for (byte i = 0; i < count; i++) {
        tmp += takeSample(ADC_M);
    }
    vm = (word)(tmp / count);

    // zero or mv...
    if (vm > adcrM) {
        // take the difference and compute it as mV
        vm = tomV(vm - adcrM);
    } else {
        // no go, set to zero
        vm = 0;
    }
}


// take samples of one ADC at a time, using average of ADC_SAMPLES
word takeSample(byte adc) {
    // take the adc samples
    if (ADC_OS > 0) {
        // temp var
        unsigned long total = 0;

        // oversampling (sample * 4^n)
        for (byte i = 0; i < pow(4, ADC_OS); i++)
            total += analogRead(adc);

        total = total >> ADC_OS;

        return (word)total;
    } else {
        return analogRead(adc);
    }

}
