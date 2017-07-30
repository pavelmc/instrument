/***************************************************
 * Multi-instrumento
 *
 * Author: M.Sc. Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/


//flash calcs, settings for some var at setup
void flashCalcs() {
    // how many data object per page
    flashDataPerPage = 256 / DATA_LEN;  // 42 for a data len of 6

    // how many pages we use for a scan (320 data object)
    flashPagesInAScan = TFT_WIDTH / flashDataPerPage;
    // check if we need a few bytes of the next page
    if ((TFT_WIDTH % flashDataPerPage) != 0) flashPagesInAScan += 1;

    // how many scans we have available with this chip
    flashMaxScans = (word)(flash.getMaxPage() / flashPagesInAScan);
}

// calc position
/*************************************************************************
 * SPI flash are not the same as a EEPROM
 * The mem is structured in data pages of 256 bytes long usually
 * we use a W25Q32 that has a lenght of 16384 pages
 *
 * So data mus be write /read in a way that data doesn't write over a data page
 * We use a trick
 ************************************************************************/
unsigned long getFlashPos(word index) {
    // working vars
    unsigned long pos;
    unsigned long local;
    long rest;

    // main position for this scan slice as direct address (bytes)
    pos = ((long)flashPosition * flashPagesInAScan) * 256;

    // how many pages of 256 bytes are covered by the index
    local = ((index / flashDataPerPage) * 256UL);

    // how many data object left in the next page (* DATA_LEN)
    rest = (long)(index % flashDataPerPage) * DATA_LEN;

    // joint the slice base and the local offset & return
    return pos + local + rest;
}


// store it
void flashWriteData(word index, unsigned long f) {
    // where to write, base of the 12 bytes
    unsigned long pos;
    // the base address
    pos = getFlashPos(index);

    // now we write
    flash.writeLong(pos, f, false);
    flash.writeWord(pos + 4, vl, false);
}


// retrieve it
unsigned long flashReadData(word index) {
    // where to write, base of the 12 bytes
    unsigned long pos;
    // the base address
    pos = getFlashPos(index);

    // now we read
    unsigned long fout = flash.readLong(pos);
    vl = flash.readWord(pos + 4);

    // return it
    return fout;
}


// init a new cycle of writes for the spiflash
void flashNext() {
    // at every scan init we increment this or fold back if at end
    flashPosition += 1;

    // limit fold back safe lock
    if (flashPosition > flashMaxScans) flashPosition = 0;

    // erase a few pages ahead
    // if not data can be mangled, BTDT
    unsigned long pos = flashPosition * flashPagesInAScan * 256L;
    flash.eraseSector(pos);

    // save eprom state
    saveEEPROM();
}
