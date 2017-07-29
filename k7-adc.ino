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
    if (vm > vbm) {
        // take the difference and compute it as mV
        vm = tomV(vm -vbm);
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


//~ // functions related to AD3805

//~ // conversion to dBm from the output of a Ad3807
//~ void dBmconv(word value){
    //~ // dBm = (ADOUT * 0.09) - 80.91;
    //~ dBm = (value * AD_a) + AD_b;
//~ }


//~ // conversion to watts from dBm
//~ word Wconv(word value){
    //~ // W...
    //~ watt = pow(10, dBm / 10);
//~ }


//~ // conversion to dBuV from dBm
//~ word dBuVconv(word value){
    //~ //
    //~ dBuV = dBm + 106.99;
//~ }
