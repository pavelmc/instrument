
// take ADC samples and put the adc values on the environment
void takeADCSamples() {
    // cycle in the samplings

    // voltage at the source point
    vrg = takeSample(ADC_S);

    // voltage at the 50 ohms resistor
    vr50 = takeSample(ADC_50);

    // voltage at the output of the sensor
    vro = takeSample(ADC_O);

    // voltage at the load
    vrl = takeSample(ADC_L);

    // convert it to mV
    vg  = tomV(vrg, false);
    v50 = tomV(vr50, false);
    vo  = tomV(vro, false);
    vl  = tomV(vrl, true);
}


// take samples of one ADC at a time, using average of ADC_SAMPLES
word takeSample(byte adc) {
    // temp var
    word total = 0;

    //cycle for ADC_SAMPLES
    for (byte i = 0; i < ADC_SAMPLES; i++) total += analogRead(adc);

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

// read and set the base diode offsets
void setDiodeOffset() {
    // read the data in the diodes
    takeADCSamples();

    //update the vars
    vlo = vrl;
}
