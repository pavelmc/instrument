/***************************************************
 * Multi-instrumento
 *
 * Author: Pavel Milanes Costa
 * Email: pavelmc@gmail.com
 ****************************************************/

/*************************************************************************
 * A SPI flash is not the same as an EEPROM, data must be write /read in
 * a way that the data doesn't write beyond a data page or troubles will
 * came, then we must use a trick to not do that.
 *
 * The flash mem is structured in data pages of 256 bytes long (usually)
 * we use a W25Q32 that has a length of 16384 pages of 256 bytes: 4 MBytes
 *
 * You can use any SPI Flash supported or compatible with the SPIFlash lib
 * as this mechanism will compute the required values and will keep up
 * no matter what size of the flash it's if they user a 256 bytes page.
 *
 * SPI flash are finite on writes and are not meant to be used as RAM
 * as in here, so I made a trick, I write each scan data in a new slice
 * of the mem, so avoiding wear out of the SPI, so far so good.
 ************************************************************************/


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
    // where to write, base of the data
    unsigned long pos = getFlashPos(index);

    // now we write
    flash.writeLong(pos, f, false);         // 4 bytes
    flash.writeShort(pos + 4, dB, false);   // 2 bytes signed
    //~ flash.writeWord(pos + 4, mVr, false);   // 2 bytes unsigned
}


// retrieve it
unsigned long flashReadData(word index) {
    // where to write, base of the data
    unsigned long pos = getFlashPos(index);

    // now we read
    unsigned long fout = flash.readLong(pos);
    dB = flash.readShort(pos + 4);  // 2 bytes signed
    //~ mVr = flash.readWord(pos + 4);  // 2 bytes unsigned

    // return it
    return fout;
}


// Init a new cycle of writes for the spiflash.
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
