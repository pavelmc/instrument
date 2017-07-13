
//~ // update the fpos data from the environment and data passed
//~ void flashPosUpdate(byte offset) {
    //~ // absolute position
    //~ fpos.addr =
    //~ fpos.page = (word)(fpos.addr / 256);
    //~ fpos.offset = (byte)(fpos.addr % 256);
//~ }

// get the actual flash position
unsigned long getFlashPos(word index) {
    unsigned long pos = (sizeof(adat) * 320UL * flashPosition) + ((long)index * sizeof(adat));
    return pos;
}

// store the
void flashWriteData(word index) {
    // now we write
    flash.writeAnything(getFlashPos(index), adat);
}


// retrieve
void flashReadData(word index) {
    // now we write
    flash.readAnything(getFlashPos(index), adat);
}


// init a new cycle of writes for the spiflash
void flashNext() {
    // at every scan init we increment this or fold back if at end
    flashPosition += 1;

    // limit fold back safe lock
    if (flashPosition > flashMaxData) flashPosition = 0;

    // save eprom state
    saveEEPROM();
}
