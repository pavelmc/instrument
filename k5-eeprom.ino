/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


// structured data: Main Configuration Parameters
struct mConf {
    byte ver = VERSION;
    long vfoa;
    long vfob;
    byte step;
    byte mode;
    int ppm;
    word flashPosition;
    byte sspan;
    long vfoOffset;
};

// declaring the main configuration variable for mem storage
struct mConf conf;


// check if the EEPROM is initialized
void checkInitEEPROM() {
    // read the eeprom config data
    byte ver;
    EEPROM.get(0, ver);

    // check for the initializer and version
    if (ver == VERSION) {
        // same firmware version, load the values
        loadEEPROM();
    } else {
        // different firmware version, save the default values.
        saveEEPROM();
    }
}


// initialize the EEPROM mem, also used to store the values in the setup mode
// this procedure has a protection for the EEPROM life using update semantics
// it actually only write a cell if it has changed
void saveEEPROM() {
    // load the parameters in the environment
    // conf.ver = VERSIOON;     // no need to, this is the default
    conf.vfoa           = vfoA;
    conf.vfob           = vfoB;
    conf.step           = step;
    conf.mode           = mode;
    conf.ppm            = ppm;
    conf.flashPosition  = flashPosition;
    conf.sspan          = sspan;
    conf.vfoOffset      = vfoOffset;

    // write it
    EEPROM.put(0, conf);
}


// load the eprom contents
void loadEEPROM() {
    // write it
    EEPROM.get(0, conf);

    // load the parameters to the environment
    vfoA            = conf.vfoa;
    vfoB            = conf.vfob;
    step            = conf.step;
    mode            = conf.mode;
    ppm             = conf.ppm;
    flashPosition   = conf.flashPosition;
    sspan           = conf.sspan;
    vfoOffset       = conf.vfoOffset;
}
