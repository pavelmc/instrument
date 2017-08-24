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
    // temp var
    word total = 0;

    //cycle for ADC_SAMPLES
    for (byte i = 0; i < ADC_SAMPLES; i++)
        total += analogRead(adc);

    return (word)(total / ADC_DIVIDER);
}


// convert adc units to mV * 10
word tomV(word value) {
    // local vars
    long tmp;

    // now convert it to mv * 10
    tmp = value * 50000L;
    tmp /= ((ADC_SAMPLES / ADC_DIVIDER) * 1023);

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

    /*** dB = ((V * 26.0)/1000) -110.5 ***/

    // tmp has the volt value
    tmp *= 260;         // 26.0
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
