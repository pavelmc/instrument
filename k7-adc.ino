/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// take ADC samples and put the values on the environment
void takeADCSamples() {
    // get mV level
    mVdB = tomV(takeSample(ADC_R));
    // convert it to dB
    dB = todB(mVdB);
}


// measure at the meter
void meterRead() {
    // take sample
    vm = takeSample(ADC_M);

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


// convert adc units to mV * 10
word tomV(word value) {
    // now convert it to mv * 10
    long tmp = (value * 50000L)/max_samples;

    // return it
    return (word)tmp;
}


// convert mV to dB * 10; 1 dB = 10
/*****************************************************************************
 * We are using a MC3372 Chip with the RSSI utility
 * (~60dB linear detector, ~90 dB non very linear in the lower end)
 *
 * As per the datasheet the dB is something like this
 *
 * dB = Volt * Multiplier - Base_dB
 *
 * A initial values are this:
 * - Multiplier: 26.0
 * - Base_dB: 110.5
 *
 * Base_dB is defined GLOBALLY as its used in the TFT procedure as well
 *
 * That values must be tunned in the adjust stage
 *****************************************************************************/
int todB(word value) {
    // local vars
    long tmp = value;

    /*** The formula:
     *
     * dB = ((V * 26.0)/1000) -110.5
     *
     * But we have a OpAmp following the RSSI output
     * with a gain of 2, so we split the multiplier by half
     * so not 26: 13
     *
     * ***/

    // tmp has the volt value
    tmp *= 130;         // 26.0
    tmp /= 10000;       // one more zero because the decimal above
    tmp -= Base_dB ;    // base dB

    #ifdef DEBUG
        Serial.print("mv * 10: ");
        Serial.print(value);
        Serial.print(" dB: ");
        Serial.println(dB);
    #endif

    // return it
    return (int)tmp;
}
