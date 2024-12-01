/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  'Format Data' data structure implementation to format disquettes          |
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


FormatData:: FormatData(u8 track, u8 side, u8 sectors, 
                        BYTE sectorSize, BYTE gap, BYTE fillerByte) {
    this->track = track;
    this->side = side;
    this->sectors = sectors;
    this->sectorSize = sectorSize;
    this->contBytes = sectors << 2; // x4
    this->ptrData = &data[0][0];
    this->gap = gap;
    this->fillerByte = fillerByte;

    { // limpiar data
        BYTE* pd = &data[0][0];
        u8 cb = contBytes;

        while (cb) {
            *pd = 0;
            pd++;
            cb--;
        }
    }
}

u8   FormatData:: getTrack()        { return track; }
u8   FormatData:: getSide()         { return side; }
u8   FormatData:: getSectors()      { return sectors; }
BYTE FormatData:: getSectorSize()   { return sectorSize; }
BYTE FormatData:: getFillerByte()   { return fillerByte; }
BYTE FormatData:: getGap()          { return gap; }

void FormatData:: addByte(BYTE b) {
    *ptrData = b;
    ptrData++;
    contBytes--;
}

bool FormatData:: isFull() {
    return (contBytes == 0);
}

u8   FormatData:: getTrack(u8 sector)       { return data[sector][FD_TRACK]; }
u8   FormatData:: getSide(u8 sector)        { return data[sector][FD_SIDE]; }
BYTE FormatData:: getSectorId(u8 sector)    { return data[sector][FD_SECTOR_ID]; }
BYTE FormatData:: getSectorSize(u8 sector)  { return data[sector][FD_SECTOR_SIZE]; }

u16  FormatData:: getTrackSize() {
    u16 size = 0;

    for (u8 s = 0; s < sectors; s++) {
        size += 0x0080 << getSectorSize(s);
    }
    return size + TRACK_HEADER_LEN;
}


void FormatData:: print()  {
    debug_fdc("FDC:: formatData sectores=%d\n", sectors);
    for (u8 s = 0; s < sectors; s++) {
        debug_fdc("%02X %02X %02X %02X\n", data[s][0], data[s][1], data[s][2], data[s][3]);
    }
    debug_fdc("FDC:: ---\n");
}
