/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// take ADC samples and put the adc values on the environment
void takeADCSamples() {
    // take all ADC samples at once

    // so far just level
    vrl = takeSample(ADC_L);

    // convert it to mV
    vl  = tomV(vrl);
}


// measure at the meter
void meterRead() {
    // take sample
    vm = takeSample(ADC_M);

    // zero or mv...
    if (vm > vrm) {
        // take the difference and compute it as mV
        vm = tomV(vm - vrm);
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

    return (total / ADC_DIVIDER);
}
