
// take ADC samples and put the adc values on the environment
void takeADCSamples() {
    // cycle in the samplings

    // voltage at the source point
    vrs = takeSample(ADC_S);

    // voltage at the 50 ohms resistor
    vr50 = takeSample(ADC_50);

    // voltage at the output of the sensor
    vro = takeSample(ADC_O);

    // voltage at the load
    vrl = takeSample(ADC_L);
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

// related to diode measurements




// convert adc units to rel mVolts (mV * 10)
// 1 V = 10000 aka /10000 for V
word tomV(word value, byte adc) {
    // local vars
    unsigned long corrected;
    unsigned long temp;

    switch (adc) {
        case ADC_S:
            if (value >= vsrs)
                corrected = value - vsrs;
            else
                corrected = 0;

            break;

        case ADC_50:
            if (value >= vsr50)
                corrected = value - vsr50;
            else
                corrected = 0;

            break;

        case ADC_O:
            if (value >= vsro)
                corrected = value - vsro;
            else
                corrected = 0;

            break;

        case ADC_L:
            if (value >= vsrl)
                corrected = value - vsrl;
            else
                corrected = 0;

            break;
    }

    // now convert it to mv * 10
    temp = corrected * 50000L;
    temp /= ((ADC_SAMPLES / ADC_DIVIDER) * 1023);

    // return it
    return (word)temp;
}


// read and set the base diode offsets
void setDiodeOffset() {
    // read the data in the diodes
    takeADCSamples();

    //update the vars
    vsrs    = vrs;
    vsr50   = vr50;
    vsro    = vro;
    vsrl    = vrl;
}
