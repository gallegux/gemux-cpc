/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  FDC Read track implementation                                             |
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


#include <vector>
#include "tipos.h"
#include "dsk.h"



ReadTrackData:: ReadTrackData(T_DskTrackInfo* trackInfo, std::vector<T_SectorInfo> _sectors, 
		u8 _sectorSize, u8 _numSectoresEscanear) {
	track = trackInfo->track;
	side = trackInfo->side;
	sectors = _sectors;
	currentSector = 0xFF;
	sectorSize = 0x0080 << _sectorSize;
	endTrackCount = -1;
	numSectoresEscanear = _numSectoresEscanear;
}


u8   ReadTrackData:: getTrack() { return track; }

u8   ReadTrackData:: getSide() { return side; }

u16  ReadTrackData:: getSectorSize() { return sectorSize; }

u8   ReadTrackData:: getCurrentSector() { return currentSector; }

void ReadTrackData:: nextSector()  {
	if (++currentSector >= sectors.size()) {
		endTrackCount++;
		currentSector = 0;
		//numSectoresEscaneados = 0;
	}
}

T_SectorInfo& ReadTrackData:: getSector() { return sectors[currentSector]; }

void ReadTrackData:: goFirstSector() { endTrackCount++; currentSector = 0; }

u8   ReadTrackData:: getNumSectoresEscanear() { return numSectoresEscanear; }

u8   ReadTrackData:: getNumSectoresEscaneados() { return currentSector; }

u8   ReadTrackData:: getEndTrackCounter() { return endTrackCount; }
/*
void ReadTrackData:: getSectorByID(BYTE sectorId, T_SectorInfo* sectorInfo) {
	for (u8 s = 0; s < sectors.size(); s++) {
		if (sectors[s].sectorId == sectorId) {
			*sectorInfo = sectors[s];
			return;
		}
	}
	sectorInfo = nullptr;
}
*/


//-----------------------------------------------------------------------


T_DskRawTrack:: T_DskRawTrack() {
	rawData = new BYTE[TRACK_BYTES];
	ptrRawData = &rawData[0];
	finRawData = ptrRawData + TRACK_BYTES;

	addBlockBytes(TRACK_BYTES, 0x4E); // rellenamos todo, y luego lo rellenamos con los valores adecuados

	ptrRawData = &rawData[0];

	addBlockBytes(80, 0x4E); // gap4a
	addBlockBytes(12, 0); // track sync 1
	addBlockBytes(3, 0); // iam
	*ptrRawData++ = 0xFC; 
	addBlockBytes(50, 0x4E); // gap1
	addBlockBytes(12, 0); // track sync 2
	addBlockBytes(3, 0); // id am
	*ptrRawData++ = 0xFC;
}

T_DskRawTrack:: ~T_DskRawTrack() {
	delete rawData;
	ptrRawData = nullptr;
	finRawData = nullptr;
}

void T_DskRawTrack:: addBlockBytes(u16 numBytes, BYTE value) {
	for (u8 cont = 0; cont < numBytes; numBytes++) *ptrRawData++ = value;
}

void T_DskRawTrack:: addSector(u8 pista, u8 cara, u8 sector, u8 longSector, BYTE* buffer) {
	iniciosDatosSectores.push_back(ptrRawData);

	*ptrRawData++ = pista;
	*ptrRawData++ = cara;
	*ptrRawData++ = sector;
	*ptrRawData++ = longSector;

	addBlockBytes(2, 0); // crc
	addBlockBytes(22, 0x4E); // gap2
	addBlockBytes(12, 0); // sync
	addBlockBytes(3, 0); // data am
	*ptrRawData++ = 0xFE;

	// copiar datos
	u16 numBytes = longSector << 8;
	for ( ; numBytes > 0; numBytes--) {
		*ptrRawData++ = *buffer++;
	}

	addBlockBytes(2, 0); // crc
	addBlockBytes(82, 0x4E);
}

bool T_DskRawTrack:: searchSector(u8 pista, u8 cara, u8 sector) {
	u8 sectores = iniciosDatosSectores.size();
	u8 p, c, s, l; // pista, cara, sector, long

	for (u8 cont = 0; cont < sectores; cont++) {
		ptrRawData = iniciosDatosSectores[cont];
		p = *ptrRawData++;
		c = *ptrRawData++;
		s = *ptrRawData;

		if (p == pista  &&  c == cara  && s == sector) {
			ptrRawData += -2 /*restamos los dos incrementos del puntero*/ + 44;
			return true;
		}
	}
	return false;
}

BYTE T_DskRawTrack:: getByte() {
	BYTE byte = *ptrRawData++;

	if (ptrRawData == finRawData) ptrRawData = &rawData[0];

	return byte;
}
