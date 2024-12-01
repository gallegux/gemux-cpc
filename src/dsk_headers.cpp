/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  DSK headers implementation                                                |
|                                                                            |
|  Copyright (c) 2024 Gallegux (gallegux@gmail.com)                          |
|                                                                            |
|  This program is free software: you can redistribute it and/or modify      |
|  it under the terms of the GNU General Public License as published by      |
|  the Free Software Foundation, either version 3 of the License, or         |
|  any later version.                                                        |
|                                                                            |
|  This program is distributed in the hope that it will be useful, but       |
|  WITHOUT ANY WARRANTY; without even the implied warranty of                |
|  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              |
|  GNU General Public License for more details.                              |
|                                                                            |
|  You should have received a copy of the GNU General Public License         |
|  along with this program; if not, see <http://www.gnu.org/licenses/>.      |
|                                                                            |
|  If you use this code, please attribute the original source by mentioning  |
|  the author and providing a link to the original repository.               |
|___________________________________________________________________________*/

#include "tipos.h"
#include "dsk.h"
#include "log.h"


constexpr char EXTENDED_HEADER[34] = {'E','X','T','E','N','D','E','D',' ','C','P','C',' ','D','S','K',' ','F','i','l','e','\r','\n','D','i','s','k','-','I','n','f','o','\r','\n'};
constexpr char STANDARD_HEADER[34] = {'M','V',' ','-',' ','C','P','C','E','M','U',' ','D','i','s','k','-','F','i','l','e','\r','\n','D','i','s','k','-','I','n','f','o','\r','\n'};
constexpr u8 EXTENDED_INIT_LEN = 8;


bool T_DskHeader:: isFormatted() { return tracks > 0; }

T_DskHeader:: T_DskHeader() {
    memcpy(fileType, EXTENDED_HEADER, 34);
}

T_DskHeader:: T_DskHeader(u8 _tracks, u8 _sides) {
    T_DskHeader();

    tracks = _tracks;
    sides = _sides;
}

/*void T_DskHeader:: calcTrackSize(u8 sectors, u16 sectorSize) { 
    trackSize = sides * sectors * sectorSize + TRACK_HEADER_LEN; 
}*/

u16 T_DskHeader:: getTrackSize() {
    return trackSize;
}


void T_DskHeader:: setExtended(bool e) {
    memcpy(fileType, e ? EXTENDED_HEADER : STANDARD_HEADER, 34);
    /*
    if (e) {
        memcpy(fileType, EXTENDED_HEADER, 34);
    }
    else {
        memcpy(fileType, STANDARD_HEADER, 34);
    }
    */
}

bool T_DskHeader:: isExtended() {
    return memcmp(fileType, EXTENDED_HEADER, 8) == 0;
}


void T_DskHeader:: write(std::fstream& f) {
    f.write(reinterpret_cast<char*>(this), sizeof(*this));
}


void T_DskHeader:: load(std::fstream& f) {
    f.read(reinterpret_cast<char*>(this), sizeof(*this));
}


void T_DskHeader:: print() {
    debug_dsk("DSK:: dsk-header  tracks=%d sides=%d\n", tracks, sides);
}

//---------------------------------------------------------------------

T_DskTrackInfo:: T_DskTrackInfo() {}

T_DskTrackInfo:: T_DskTrackInfo(u8 _track, u8 _side, u8 _sectors, u8 _sectorSize) {
    track = _track;
    side = _side;
    sectors = _sectors;
    sectorSize = _sectorSize;
    gap = 0x4E;
    filler = 0xE5;
}

T_DskTrackInfo:: T_DskTrackInfo(u8 _track, u8 _side, u8 _sectors, u8 _sectorSize, u8 _gap, u8 _filler) {
    T_DskTrackInfo(_track, _side, _sectors, _sectorSize);
    gap = _gap;
    filler = _filler;
}	


bool T_DskTrackInfo:: isFormatted() { 
    return sectors > 0; 
}

void T_DskTrackInfo:: write(std::fstream& f) {
    f.write(reinterpret_cast<char*>(this), sizeof(*this));
}

void T_DskTrackInfo:: load(std::fstream& f) {
    f.read(reinterpret_cast<char*>(this), sizeof(*this));
}

u16 T_DskTrackInfo:: getSectorDataLength() {
    return 0x0080 << sectorSize;
}

void T_DskTrackInfo:: print() {
    debug_dsk("DSK:: track-header  track=%d side=%d sectors=%d\n", track, side, sectors);
}

//--------------------------------------------------------------------


T_SectorInfo:: T_SectorInfo() {}

T_SectorInfo:: T_SectorInfo(u8 _track, u8 _side, u8 _sectorId, u8 _sectorSize) {
    track = _track;
    side = _side;
    sectorId = _sectorId;
    sectorSize = _sectorSize;
    dataLength = 0x0080 << _sectorSize;
}
T_SectorInfo:: T_SectorInfo(u8 _track, u8 _side, u8 _sectorId, u8 _sectorSize, u16 _actualDataLen) {
    T_SectorInfo(_track, _side, _sectorId, _sectorSize);
    dataLength = _actualDataLen;
}

void T_SectorInfo:: write(std::fstream& f) {
    f.write(reinterpret_cast<char*>(this), sizeof(*this));
}
void T_SectorInfo:: load(std::fstream& f) {
    f.read(reinterpret_cast<char*>(this), sizeof(*this));
}

u16 T_SectorInfo:: getDataLength() {
    return  (dataLength == 0)  ?  0x0080 << sectorSize
                                  :  dataLength;
}

void T_SectorInfo:: print() {
    debug_dsk("DSK:: sector-info  track=%d side=%d sectorID=%02X size1=%02X size2=%04X\n", 
                track, side, sectorId, sectorSize, dataLength);
}
