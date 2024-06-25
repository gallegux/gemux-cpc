/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  DSK file implementation                                                   |
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

#include <list>
#include <string>
#include <fstream>
#include "tipos.h"
#include "log.h"
#include "dsk.h"

// seekg -> puntero de escritura (PE)
// seekp -> puntero de lectura   (PL)


void rellenar(std::fstream& f, u16 veces, u8 b) {
	for (u16 i = 0; i < veces; ++i) {
        f.write(reinterpret_cast<char*>(&b), sizeof(b));
    }
}


DSK:: DSK() {}

DSK:: ~DSK() {
	if (fichero) fichero.close();

	if (tablaPosicionesPistas != nullptr) {
		for (u8 p = 0; p < dskHeader.tracks; p++) {
			delete[] tablaPosicionesPistas[p];
		}
	}
	delete tablaPosicionesPistas;
}

bool DSK:: isFormatted() { return dskHeader.tracks > 0; }
u8 DSK:: getTracks() { return dskHeader.tracks; }
u8 DSK:: getSides()  { return dskHeader.sides; }

// devuelve si el disco esta protegido contra estcritura
bool DSK:: isProtected() { return protegido; }

// devuelve si el disco esta protegido contra estcritura
bool DSK:: flipProtected() { return (protegido = !protegido); }

void DSK:: setProtected(bool p) { protegido = p; }


u32 DSK:: getTrackPos(u8 track) {
	if (extended) {
		// este formato no almacena el tamanio de la pista
		// pero tiene una tabla con las posiciones de cada par pista-cara
		return tablaPosicionesPistas[track][0]; 
	}
	else { // standard
		// este formato no tiene tabla, pero guarda el tamanio de cada pista y con eso calculamos la posicion de cada pista
		return track * dskHeader.trackSize + DSK_HEADER_LEN;
	}
}


// crea un fichero dsk
void DSK:: create(std::string& fichero, u8 pistas, u8 caras, u8 sectores, u8 sectorSize, 
		BYTE fillerByte, BYTE gap, BYTE primerSectorId) {
	std::fstream dskFile(fichero, std::ios::out | std::ios::binary);

	T_DskHeader dskHeader {pistas, caras};
	dskHeader.calcTrackSize(sectores, sectorSize);
	dskHeader.write(dskFile);
	
	u16 contBytes = sizeof(dskHeader) + pistas*caras; // num bytes para rellenar
	u8 trackSizeH = (u8) (dskHeader.trackSize >> 8); // track size table
	
	rellenar(dskFile, pistas*caras, trackSizeH); // escribimos el byte alto del tamanio del sector 
	rellenar(dskFile, DSK_HEADER_LEN-contBytes, 0); // relleno cabecera

	// pistas
	for (u8 pista = 0; pista < pistas; pista++) {
		for (u8 cara = 0; cara < caras; cara++) {
			T_DskTrackInfo trackInfo {pista, cara, sectores, sectorSize};

			trackInfo.write(dskFile); // grabar cabecera

			contBytes = sizeof(trackInfo); // contador de bytes usados en dskTrackInfo

			// info sectores de la pista
			for (u8 sector = 0; sector < sectores; sector++) {
				//debug_dsk("writin sector info %d\n", sector);
				T_SectorInfo sectorInfo {pista, cara, (u8) (primerSectorId+sector), sectorSize};

				contBytes += sizeof(sectorInfo);
				sectorInfo.write(dskFile);
			}

			// rellenamos hasta #100
			//debug_dsk("ini relleno 2  %d  %d\n", contBytes, TRACK_HEADER_LEN-contBytes);
			rellenar(dskFile, TRACK_HEADER_LEN-contBytes, 0);
			//debug_dsk("fin relleno 2\n");

			// escribir los sectores vacios
			for (u8 sector = 0; sector < sectores; sector++) {
				//debug_dsk("writing empty sector... %d  %d\n", sector, sectorSize);
				rellenar(dskFile, sectorSize, fillerByte);
			}
		}
	}
	dskFile.close();
}


// solo se utiliza para los extended
void DSK:: crearTabla(u8 pistas, u8 caras) {
	tablaPosicionesPistas = new u32*[pistas];

	for (u8 p = 0; p < pistas; p++) {
		tablaPosicionesPistas[p] = new u32[caras];
	}
}


bool DSK:: open(std::string& f) {
	fichero = std::fstream(f, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
	// ate nos situal al final
    
    tamFichero = fichero.tellg(); // Obtener la posicion, que corresponde al tamanio del fichero
    fichero.seekg(0, std::ios::beg); // Mover el puntero al inicio del fichero

	dskHeader.load(fichero);
	extended = dskHeader.isExtended();

	if (extended) {
		debug_dsk("DSK:: open() fichero=%s tam=%ld tracks=%d sides=%d extended=%d\n", 
			f.c_str(), tamFichero, dskHeader.tracks, dskHeader.sides, extended);
		crearTabla(dskHeader.tracks, dskHeader.sides) ;

		u32 pos = DSK_HEADER_LEN;
		u32 trackSize = 0;
		char b; // para leer bytes

		for (u8 pista = 0; pista < dskHeader.tracks; pista++) {
			for (u8 cara = 0; cara < dskHeader.sides; cara++) {
				fichero.read(&b, 1);
				trackSize = b << 8;
				//debug_dsk("DSK::  %d %d\n", b, trackSize);
				//trackSize = trackSize << 8;
				tablaPosicionesPistas[pista][cara] = pos;
				debug_dsk("DSK::  pista,cara %d,%d -> %05lX\n", pista, cara, pos);
				pos += trackSize;
			}
		}
	}
	else {
		debug_dsk("DSK:: open() fichero=%s tam=%ld tracks=%d sides=%d trackSize=%04X extended=%d\n", 
			f.c_str(), tamFichero, dskHeader.tracks, dskHeader.sides, dskHeader.trackSize, extended);
		u32 pos = DSK_HEADER_LEN;
		for (u8 pista = 0; pista < dskHeader.tracks; pista++) {
			debug_dsk("DSK::  open() pista %d -> %05lX\n", pista, pos);
			pos += dskHeader.trackSize;
		}
	}
	return true;
}


void DSK:: close() {
	if (fichero) fichero.close();
}



void DSK:: saveAs(std::string& f) {
	std::fstream ficheroNuevo = std::fstream(f, std::ios::in | std::ios::out | std::ios::binary);
	
	u32 tamFicheroOrigen = DSK_HEADER_LEN;

	for (u8 pista = 0; pista < dskHeader.tracks; pista++) {
		for (u8 cara = 0; cara < dskHeader.sides; cara++) {
			tamFicheroOrigen += tablaPosicionesPistas[pista][cara];
		}
	}

	fichero.seekg(0, std::ios::beg);
	u32 numBloques = tamFicheroOrigen >> 7;  // bloques 512
	unsigned char buffer[512];

	for (u32 bloque = 0; bloque < numBloques; bloque++) {
		fichero.read(reinterpret_cast<char*>(&buffer), sizeof(512));
		ficheroNuevo.write(reinterpret_cast<char*>(buffer), 512);
	}
	fichero.close();
	fichero = std::move(ficheroNuevo); // el fichero de trabajo es el creado
}


// todos los sectores de la pista tienen el mismo tamanio
// devuelve el NUMERO de sector de la pista, o -1 si no se ha encontrado
i8 getNumSector(u8 numSectors, u8 sector, u16 sectorSize, std::fstream& fichero, T_SectorInfo* sectorInfo) {
	u8 contador = 0;
	for (u8 ns = 0; ns < numSectors; ns++) {
		sectorInfo->load(fichero);
		//fichero->read(reinterpret_cast<char*>(&sectorInfo), sizeof(T_SectorInfo));
		if (sectorInfo->sectorId == sector) {
			// comprobacion
			debug_dsk("DSK:: getNumSector() track=%d sector=%02X\n", sectorInfo->track, sectorInfo->sectorId);
			return contador;
		}
		contador++;
	}
	return -1;
}

// obtiene el track info
// deja PL en la posicion del 'Sector Information List'
void DSK:: getTrackInfo(u8 track, u8 side, T_DskTrackInfo* trackInfo) {
	u32 pos = getTrackPos(track); // tablaPosicionesPistas[track][side];
	//debug_dsk("DSK:: getTrackInfo() pos_track=%05lX\n", pos);
	fichero.seekg(pos, std::ios::beg);
	fichero.read(reinterpret_cast<char*>(trackInfo), sizeof(*trackInfo));
	//debug_dsk("DSK:: block_info=%s track=%d side=%d sec_size=%02X sectors=%02X filler=%02X gap=%02X\n", 
	//	trackInfo->blockInfo, trackInfo->track, trackInfo->side, trackInfo->sectorSize, trackInfo->sectors,
	//	trackInfo->filler, trackInfo->gap);
}

// obtiene el sector con el Nº indicado
// esta funcion se invoca cuando no se conoce el formato del disco
// cuando se conoce el formato se invoca a getSectorInfo_ID
// PL queda en el inicio de los datos del sector
void DSK:: getSectorInfo_N(u8 track, u8 side, u8 numSector /*num_sector*/, T_SectorInfo* sectorInfo) {
	debug_fdc("FDC:: getSector_N   track=%d side=%d sector=%02X\n", track, side, numSector);
	u32 pos = getTrackPos(track) + sizeof(T_DskTrackInfo) + numSector * sizeof(sectorInfo);
		//tablaPosicionesPistas[track][side] + sizeof(T_DskTrackInfo) + sector * sizeof(sectorInfo);
	debug_dsk("DSK:: getSectorInfo_N() pos_sector=%05lX\n", pos);
	fichero.seekg(pos, std::ios::beg);
	fichero.read(reinterpret_cast<char*>(sectorInfo), sizeof(sectorInfo));
	debug_fdc("DSK:: track=%d side=%d sector=%d -> sectorId=%02X, sectorSize=%d\n",
		track, side, numSector, sectorInfo->sectorId, sectorInfo->sectorSize);
	// dejamos el puntero en los datos del sector
}

// obtiene el sector con el ID indicado
// devuelve true si lo encuentra, false en el caso contrario
// deja PL y PE al inicio de los datos del sector
bool DSK:: getSectorInfo_ID(u8 track, u8 side, u8 sectorId, T_SectorInfo* sectorInfo) {
	u32 posSectorInfo, posSectorData;
	u16 sectorSize = 0;
	posSectorInfo = posSectorData = getTrackPos(track);
	posSectorInfo += sizeof(T_DskTrackInfo);
	posSectorData += 0x100; // apuntando al primer sector

	//debug_fdc("DSK:: getSector_ID()   track=%d side=%d sector=%02X pos_info=%06X\n", track, side, sectorId, posSectorInfo);
	fichero.seekg(posSectorInfo, std::ios::beg);
	
	for (u8 ns = 0; ns < MAX_SECTORS; ns++) {
		fichero.read(reinterpret_cast<char*>(sectorInfo), sizeof(sectorInfo));
		sectorSize = sectorInfo->sectorSize;
		sectorSize = sectorSize << 8;
		//debug_dsk("DSK:: getSector_ID()   sector_bucle=%02X\n", sectorInfo->sectorId);
		if (sectorInfo->sectorId == sectorId && sectorInfo->track == track && sectorInfo->side == side) {
			//debug_fdc("FDC:: getSector_ID()   sector_size=%d sector_pos=%05lX\n", sectorSize, posSectorData);
				//p = tablaPosicionesPistas[track][side] + 256 + sectorSize * ns;
				//debug_dsk("DSK:: getSector_ID()   sector encontrado  n=%d  sector_pos=%06lX\n", ns, p);
				//fichero.seekg(p, std::ios::beg);
			fichero.seekg(posSectorData, std::ios::beg);  // dejamos PL en los datos
			fichero.seekp(posSectorData, std::ios::beg);  // dejamos PE en los datos
			//debug_fdc("FDC:: getSector_ID()   sector_data_pos=%05lX\n", posSectorData);
			return true; // numero secuencial del sector
		}
		posSectorData += sectorSize;
	}
	return false;
}

bool DSK:: existsTrack(u8 track, u8 side) {
	return track < dskHeader.tracks;
}

// ?? numero de sector o sector id
bool DSK:: existsSector(u8 track, u8 side, u8 sector) {
	if (track > dskHeader.tracks) return false;
	if (side > dskHeader.sides) return false;
	return true;
}


void DSK:: readSectorData(u16 sectorSize, BYTE* buffer) {
	fichero.read(reinterpret_cast<char*>(buffer), sectorSize);
}

void DSK:: writeSectorData(u16 sectorSize, BYTE* buffer) {
	fichero.write(reinterpret_cast<char*>(buffer), sectorSize);
}


// TODO
void DSK:: formatTrack(u8 track, u8 side, u8 sectors, u8 sectorSize, u8 firstSectorId, BYTE fillerByte) {
	if (protegido) return;

	u32 pos = tablaPosicionesPistas[track][side];
	fichero.seekg(pos, std::ios::beg);

	T_DskTrackInfo trackInfo {track, side, sectors, sectorSize};

	for (u8 s = 0; s < sectors; s++) {
		T_SectorInfo sectorInfo;
	}
	/*
	fichero.read(reinterpret_cast<char*>(&trackInfo), sizeof(T_DskTrackInfo));
	debug_dsk("DSK:: pistas=%d,%d sides=%d,%d\n", track, side, trackInfo.track, trackInfo.side);

	i8 numSector = getNumSector(trackInfo.sectors, sector, trackInfo.sectorSize, &fichero);
	if (numSector == -1) return -3;
	pos += TRACK_HEADER_MIN_LEN + trackInfo.sectorSize * numSector;
	fichero.seekp(pos, std::ios::beg);
	fichero.write(reinterpret_cast<char*>(buffer), trackInfo.sectorSize);
*/


	//return 1;
}


void DSK:: createStandardData(std::string& fichero) {
	create(fichero, 40, 1, 9, 2, 0xE5, 0x4E, 0xC1);
}


void DSK:: createStandardSystem(std::string& fichero) {
	create(fichero, 40, 1, 9, 2, 0xE5, 0x4E, 0x41);
}

