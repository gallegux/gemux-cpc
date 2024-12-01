/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  DSK (standard and extended) file implementation                           |
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
#include "file_util.h"

// seekg -> puntero de escritura (PE)
// seekp -> puntero de lectura   (PL)


void rellenar(std::fstream& f, u16 veces, u8 b) {
	for (u16 i = 0; i < veces; ++i) {
        f.write(reinterpret_cast<char*>(&b), sizeof(b));
    }
}


void limpiarBuffer(BYTE* ptrBuffer, u16 cont) {
	while (cont) {
		*ptrBuffer = 0;
		ptrBuffer++;
		cont--;
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


u32  DSK:: getFileSize()	{ return tamFichero; }
u16  DSK:: getTrackSideSize_standard() 	{ return dskHeader.getTrackSize(); }
bool DSK:: isFormatted() 	{ return dskHeader.tracks > 0; }
u8   DSK:: getTracks() 		{ return dskHeader.tracks; }
u8   DSK:: getSides()  		{ return dskHeader.sides; }

// devuelve si el disco esta protegido contra estcritura
bool DSK:: isProtected() { return protegido; }

// devuelve si el disco esta protegido contra estcritura
bool DSK:: flipProtected() { return (protegido = !protegido); }

void DSK:: setProtected(bool p) { protegido = p; }


// devuelve la suma del tamanio de los sectores
u16 DSK:: getTrackSideSize_extended() {
	T_DskTrackInfo trackInfo;
	trackInfo.load(fichero);
	u16 size = 0;
	T_SectorInfo sectorInfo;

	for (u8 s = 0; s < trackInfo.sectors; s++) {
		sectorInfo.load(fichero);
		//sectorInfo.print();
		size += sectorInfo.dataLength;
	}
	return size + TRACK_HEADER_LEN;
}


i32 DSK:: getTrackPos(u8 track, u8 side) {
	if (extended) {
		// este formato no almacena el tamanio de la pista
		// pero tiene una tabla con las posiciones de cada par pista-cara
		return tablaPosicionesPistas[track][side]; 
	}
	else { // standard
		// este formato no tiene tabla, pero guarda el tamanio de cada pista y con eso calculamos la posicion de cada pista
		i32 pos = track * dskHeader.sides * dskHeader.trackSize + DSK_HEADER_LEN;
		if (side) pos += dskHeader.trackSize;
		return pos;
	}
}


i32 DSK:: goTrackSide(u8 track, u8 side) {
	i32 fp = getTrackPos(track, side);
	//debug_dsk("DSK:: track,side pos  %04lX\n", fp);
	fichero.seekg(fp, std::ios::beg);
	fichero.seekp(fp, std::ios::beg);
	return fp;
}



void DSK:: createTrackSide(std::fstream& dskFile, u8 pista, u8 cara, u8 sectores, u8 sectorSize, 
	BYTE fillerByte, BYTE gap, BYTE primerSectorId)
{
	T_DskTrackInfo trackInfo {pista, cara, sectores, sectorSize};
	trackInfo.write(dskFile); // grabar cabecera
	u16 contBytes = sizeof(trackInfo); // contador de bytes usados en dskTrackInfo

	// info sectores de la pista
	for (u8 sector = 0; sector < sectores; sector++) {
		//debug_dsk("writin sector info %d\n", sector);
		T_SectorInfo sectorInfo {pista, cara, (u8) (primerSectorId+sector), sectorSize};

		contBytes += sizeof(sectorInfo);
		sectorInfo.write(dskFile);
	}
	// rellenamos hasta alcanzar el tamanio del track_header (#100)
	//debug_dsk("ini relleno 2  %d  %d\n", contBytes, TRACK_HEADER_LEN-contBytes);
	rellenar(dskFile, TRACK_HEADER_LEN-contBytes, 0);
	//debug_dsk("fin relleno 2\n");

	// escribir los sectores vacios
	for (u8 sector = 0; sector < sectores; sector++) {
		//debug_dsk("writing empty sector... %d  %d\n", sector, sectorSize);
		//rellenar(dskFile, sectorSize << 8, fillerByte);
		rellenar(dskFile, BYTES_SECTOR(sectorSize), fillerByte);
		// en winape, cuando se formatea un sector con sectorSize=4 el tamanio es 8192bytes
	}
}


void DSK:: createTrack(std::fstream& dskFile, u8 pista, u8 caras, u8 sectores, u8 sectorSize, 
	BYTE fillerByte, BYTE gap, BYTE primerSectorId)
{
	for (u8 cara = 0; cara < caras; cara++) {
		createTrackSide(dskFile, pista, cara, sectores, sectorSize, fillerByte, gap, primerSectorId);
	}
}



// STATIC | crea un fichero dsk
bool DSK:: create(const std::string& fichero, u8 pistas, u8 caras, u8 sectores, u8 sectorSize, 
		BYTE fillerByte, BYTE gap, BYTE primerSectorId)
{
	std::fstream dskFile(fichero, std::ios::out | std::ios::binary);

	T_DskHeader dskHeader {pistas, caras};
	dskHeader.trackSize = sectores * BYTES_SECTOR(sectorSize);
	//dskHeader.calcTrackSize(sectores, sectorSize);
	dskHeader.write(dskFile);
	
	u16 contBytes = sizeof(dskHeader) + pistas*caras; // num bytes para rellenar
	u8 trackSizeH = (u8) (dskHeader.trackSize >> 8); // track size table
	
	rellenar(dskFile, pistas*caras, trackSizeH); // escribimos el byte alto del tamanio del sector 
	rellenar(dskFile, DSK_HEADER_LEN-contBytes, 0); // relleno cabecera

	// pistas
	for (u8 pista = 0; pista < pistas; pista++) {
		createTrack(dskFile, pista, caras, sectores, sectorSize, fillerByte, gap, primerSectorId);
	}
	dskFile.close();
	return true;
}


// solo se utiliza para los extended
void DSK:: crearTabla(u8 pistas, u8 caras) {
	debug_dsk("DSK:: crear tabla posiciones pistas. pistas=%d caras=%d\n", pistas, caras);
	tablaPosicionesPistas = new i32*[pistas];

	for (u8 p = 0; p < pistas; p++) {
		tablaPosicionesPistas[p] = new i32[caras];
	}
}


//---------------------------------------------------------------------------------------------------------

void DSK:: open_standard(std::string& f) {
	debug_dsk("DSK:: open() fichero=%s tam=%ld tracks=%d sides=%d trackSize=%04X extended=%d\n", 
		f.c_str(), tamFichero, dskHeader.tracks, dskHeader.sides, dskHeader.trackSize, extended);
	u32 pos = DSK_HEADER_LEN;
	for (u8 pista = 0; pista < dskHeader.tracks; pista++) {
		debug_dsk("DSK::  open() pista %d -> %05lX\n", pista, pos);
		pos += dskHeader.trackSize;
	}
	debug_dsk("DSK:: track_size = %04X\n", dskHeader.trackSize);
}


void DSK:: open_extended(std::string& f) {
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
			if (dskHeader.trackSize == 0 && trackSize != 0)
				dskHeader.trackSize = trackSize;
			//debug_dsk("DSK::  %d %d\n", b, trackSize);
			//trackSize = trackSize << 8;
			tablaPosicionesPistas[pista][cara] = (pos != 0) ? pos : -1;
			debug_dsk("DSK::  pista,cara %d,%d -> %05lX\n", pista, cara, pos);
			if (trackSize != 0) pos += trackSize;
		}
	}
}


bool DSK:: open(std::string& f) {
	fichero = std::fstream(f, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
	nombreFichero = f;
	// ate nos situal al final
    
    tamFichero = fichero.tellg(); // Obtener la posicion, que corresponde al tamanio del fichero
    fichero.seekg(0, std::ios::beg); // Mover el puntero al inicio del fichero

	dskHeader.load(fichero);
	extended = dskHeader.isExtended();

	if (extended)	open_extended(f);
	else 			open_standard(f);
	
	return true;
}


void DSK:: close() {
	if (fichero) fichero.close();
}

//---------------------------------------------------------------------------------------------------------

void DSK:: saveAs(std::string& f) {
	std::fstream ficheroNuevo = std::fstream(f, std::ios::in | std::ios::out | std::ios::binary);
	
	u32 tamFicheroOrigen = DSK_HEADER_LEN;

	for (u8 pista = 0; pista < dskHeader.tracks; pista++) {
		for (u8 cara = 0; cara < dskHeader.sides; cara++) {
			tamFicheroOrigen += tablaPosicionesPistas[pista][cara];
		}
	}
	#define TAM_BUFFER 512
	fichero.seekg(0, std::ios::beg);
	u32 numBloques = tamFicheroOrigen >> 7;  // bloques TAM_BUFFER
	unsigned char buffer[TAM_BUFFER];

	for (u32 bloque = 0; bloque < numBloques; bloque++) {
		fichero.read(reinterpret_cast<char*>(&buffer), TAM_BUFFER);
		ficheroNuevo.write(reinterpret_cast<char*>(buffer), TAM_BUFFER);
	}
	fichero.close();
	fichero = std::move(ficheroNuevo); // el fichero de trabajo es el creado
	nombreFichero = f;
	#undef TAM_BUFFER
}



// obtiene el track info
// deja PL en la posicion del 'Sector Information List'
void DSK:: getTrackInfo(u8 track, u8 side, T_DskTrackInfo* trackInfo) {
	//u32 pos = getTrackPos(track, side); // tablaPosicionesPistas[track][side];
	goTrackSide(track, side);
	//fichero.seekg(pos, std::ios::beg);
	trackInfo->load(fichero);
}


/*void DSK:: getTrackSectorsData(u8 track, u8 side, std::vector<T_SectorInfo>& sectores, u16* trackDataLen) {
	goTrackSide(track, side);
	T_DskTrackInfo trackInfo;
	trackInfo.load(fichero);
	trackInfo.print();

	debug_dsk("DSK:: getTrackDataLength() extended=%d sectores=%d\n", extended, trackInfo.sectors);

	*trackDataLen = 0;
	for (u8 s = 0; s < trackInfo.sectors; s++) {
		T_SectorInfo sectorInfo;
		sectorInfo.load(fichero);
		sectorInfo.print();
		sectores.push_back(sectorInfo);
		*trackDataLen += sectorInfo.getDataLength();
	}
*/
	/*
	if (extended) {
		*trackDataLength = trackInfo.sectors * trackInfo.getSectorDataLength();
		// ultimo sector
		fichero.seekg( (trackInfo.sectors-1) * trackInfo.getSectorDataLength() , std::ios::cur);
		sectorInfo.load(fichero);
	}
	else {
		u16 sum = 0;
		debug_dsk("DSK:: getTrackDataLength() sectores=%d\n", trackInfo.sectors);
		for (u8 s = 0; s < trackInfo.sectors; s++) {
			sectorInfo.load(fichero);
			sectorInfo.print();
			sectores.push_back(sectorInfo);
		}
	}
	*/
	// avanzar
//	fichero.seekg(TRACK_HEADER_LEN - sizeof(T_DskTrackInfo) - sizeof(T_SectorInfo) * trackInfo.sectors, 
//					std::ios::cur);
//}


void DSK:: getTrack(u8 track, u8 side, T_DskTrackInfo* trackInfo, std::vector<T_SectorInfo>& sectores) {
	getTrackInfo(track, side, trackInfo);

	for (u8 s = 0; s < trackInfo->sectors; s++) {
		T_SectorInfo sectorInfo;
		sectorInfo.load(fichero);
		debug_dsk("DSK:: getTrack() ");
		sectorInfo.print();
		sectores.push_back(sectorInfo);
	}
}


// la finalidad de este metodo es obtener el sectorId
// se invoca a traves de la funcion readId
void DSK:: getSectorInfo_N(u8 track, u8 side, u8 numSector, T_SectorInfo* sectorInfo) {
	debug_fdc("FDC:: getSectorInfo_N   track=%d side=%d sector=%d\n", track, side, numSector);

	T_DskTrackInfo trackInfo;
	getTrackInfo(track, side, &trackInfo);

	if (numSector > trackInfo.sectors) numSector = trackInfo.sectors;

	u8 sector = 0;
	do {
		sectorInfo->load(fichero);
	} while (++sector < numSector);
}

//------------------------------------------------------------------------------------------


// obtiene el sector con el ID indicado
// devuelve true si lo encuentra, false en el caso contrario
// deja PL y PE al inicio de los datos del sector
bool DSK:: getSectorInfo_ID_extended(u8 track, u8 side, u8 sectorId, T_SectorInfo* sectorInfo) {
	debug_dsk("DSK:: getSectorInfo_ID_extended()   track=%d side=%d sectorId=%02X\n", track, side, sectorId);

	u32 posSectorData = getTrackPos(track, side);  // inicio de la pista
	debug_dsk("DSK:: pos_pista = %05lX\n", posSectorData);

	fichero.seekg(posSectorData, std::ios::beg);  // inicio de la pista
	T_DskTrackInfo trackInfo;
	trackInfo.load(fichero);
	
	posSectorData += TRACK_HEADER_LEN; // apuntando a los datos del primer sector

	//debug_fdc("DSK:: getSectorInfo_ID_extended()   sectores=%d\n", trackInfo.sectors); 
	
	for (u8 ns = 0; ns < trackInfo.sectors; ns++) {
		sectorInfo->load(fichero);
		//debug_dsk("DSK:: sectorId=%02X  sectorSize=%02X\n", sectorInfo->sectorId, sectorInfo->sectorSize); FLUSH
		//sectorSize = 0x0080 << sectorInfo->sectorSize;
		//debug_dsk("DSK:: getSectorInfo_ID_extended()  bucle   sectorId=%02X  sectorSize=%02X  sectorPos=%05lX\n", 
		//			sectorInfo->sectorId, sectorInfo->sectorSize, posSectorData);

		if (sectorInfo->sectorId == sectorId  &&  sectorInfo->track == track  &&  sectorInfo->side == side) {
				// posiblemente las dos ultimas comparaciones sean redundantes
			fichero.seekg(posSectorData, std::ios::beg);  // dejamos PL en los datos
			fichero.seekp(posSectorData, std::ios::beg);  // dejamos PE en los datos
			//debug_fdc("FDC:: getSector_ID()   sector_data_pos=%05lX\n", posSectorData);
			return true; 
		}
		posSectorData += sectorInfo->getDataLength();
	}
	return false;
}


// obtiene el sector con el ID indicado
// devuelve true si lo encuentra, false en el caso contrario
// deja PL y PE al inicio de los datos del sector
bool DSK:: getSectorInfo_ID_standard(u8 track, u8 side, u8 sectorId, T_SectorInfo* sectorInfo) {
	debug_dsk("DSK:: getSectorInfo_ID_standard()   track=%d side=%d sectorId=%02X\n", track, side, sectorId);

	u32 posSectorInfo, posSectorData;
	posSectorInfo = posSectorData = getTrackPos(track, side);
	debug_dsk("DSK:: pos_pista = %05lX\n", posSectorInfo);

	fichero.seekg(posSectorInfo, std::ios::beg);  // posSectorInfo ahora tiene la pos de la pista
	T_DskTrackInfo trackInfo;
	trackInfo.load(fichero);
	u16 sectorSize = BYTES_SECTOR(trackInfo.sectorSize);
	
	posSectorInfo += sizeof(T_DskTrackInfo); // ahora apunta al primer sector
	posSectorData += TRACK_HEADER_LEN; // apuntando a los datos del primer sector

	//debug_fdc("DSK:: getSectorInfo_ID_extended()   sectores=%d\n", trackInfo.sectors); 
	
	for (u8 ns = 0; ns < trackInfo.sectors; ns++) {
		//fichero.read(reinterpret_cast<char*>(sectorInfo), sizeof(sectorInfo));
		sectorInfo->load(fichero);
		debug_dsk("DSK:: sectorId=%02X  sectorSize=%02X\n", sectorInfo->sectorId, sectorInfo->sectorSize); FLUSH
		debug_dsk("DSK:: getSectorInfo_ID_standard()  bucle   sectorId=%02X  sectorSize=%02X  sectorPos=%05lX\n", 
					sectorInfo->sectorId, sectorInfo->sectorSize, posSectorData);

		if (sectorInfo->sectorId == sectorId  &&  sectorInfo->track == track  &&  sectorInfo->side == side) {
				// posiblemente las dos ultimas comparaciones sean redundantes
			fichero.seekg(posSectorData, std::ios::beg);  // dejamos PL en los datos
			fichero.seekp(posSectorData, std::ios::beg);  // dejamos PE en los datos
			//debug_fdc("FDC:: getSector_ID()   sector_data_pos=%05lX\n", posSectorData);
			return true; 
		}
		posSectorData += sectorSize;
	}
	return false;
}


// obtiene el sector con el ID indicado
// devuelve true si lo encuentra, false en el caso contrario
// deja PL y PE al inicio de los datos del sector
bool DSK:: getSectorInfo_ID(u8 track, u8 side, u8 sectorId, T_SectorInfo* sectorInfo) {
	return (extended) ? getSectorInfo_ID_extended(track, side, sectorId, sectorInfo)
					  : getSectorInfo_ID_standard(track, side, sectorId, sectorInfo);
}


//--------------------------------------------------------------------------------------


bool DSK:: existsTrack(u8 track) {
	return track < dskHeader.tracks;
}

bool DSK:: existsTrackSide(u8 track, u8 side) {
	if (dskHeader.isExtended()) {
		return  track < dskHeader.tracks  &&  tablaPosicionesPistas[track][side] != -1;
	}
	else {
		u32 posTrackSide = track * dskHeader.trackSize * dskHeader.sides + dskHeader.trackSize * side;
		return (posTrackSide < tamFichero);
	}
}

// ?? numero de sector o sector id, solo usada por readTrack, REHACER
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


void DSK:: readByte(BYTE *dato) {
	fichero.read(reinterpret_cast<char*>(dato), 1);
}

void DSK:: writeByte(BYTE dato) {
	fichero.write(reinterpret_cast<char*>(dato), 1);
}


//bool DSK:: existsTrackInFile(u8 track, u8 side) {
//	return tablaPosicionesPistas[track][side] != -1;
//}


u16 copiarPista_standard(std::fstream& fin, std::fstream& fout) {
	u32 tamPista = 0;
	u32 posSector0 = fin.tellg();
	posSector0 += TRACK_HEADER_LEN;
	T_DskTrackInfo trackInfo;
	trackInfo.load(fin);
	u16 tamDatosSector = BYTES_SECTOR(trackInfo.sectorSize);

	// escribir el T_TrackInfo
	trackInfo.write(fout);

	u32 posSector[trackInfo.sectors];
	u32 tamSector[trackInfo.sectors];
	T_SectorInfo sectorInfo;
	// calcular el nº total de bytes de los sectores, la posicion de cada uno y su tamano
	for (u8 sector = 0; sector < trackInfo.sectors; sector++) {
		sectorInfo.load(fin);
		tamSector[sector] = sectorInfo.dataLength = BYTES_SECTOR(sectorInfo.sectorSize);
		sectorInfo.write(fout);
		posSector[sector] = posSector0 + sector * tamDatosSector;
		tamPista += sectorInfo.dataLength;
	}

	u32 numBytes = 0x100 - fin.tellg() & 0x00FF;
	rellenar(fout, numBytes, 0);
	
	// copiar los sectores
	for (u8 sector = 0; sector < trackInfo.sectors; sector++) {
		fin.seekg(posSector[sector]);
		//copiarBytes(fin, fout, tamSector[sector]);
		file_copyPiece(fin, tamSector[sector], fout);
	}

	return tamPista;
}


void formatearPista(std::fstream& fout, FormatData* formatData) {
	T_DskTrackInfo trackInfo;
	trackInfo.track = formatData->getTrack();
	trackInfo.side = formatData->getSide();
	trackInfo.sectors = formatData->getSectors();
	trackInfo.sectorSize = formatData->getSectorSize();
	trackInfo.filler = formatData->getFillerByte();
	trackInfo.gap = formatData->getGap();
	trackInfo.write(fout);
	//trackInfo.print();

	T_SectorInfo sectorInfo;

	for (u8 s = 0; s < formatData->getSectors(); s++) {
		sectorInfo.track = formatData->getTrack(s);
		sectorInfo.side = formatData->getSide(s);
		sectorInfo.sectorId = formatData->getSectorId(s);
		sectorInfo.sectorSize = formatData->getSectorSize(s);
		sectorInfo.dataLength = BYTES_SECTOR(formatData->getSectorSize(s));
		sectorInfo.write(fout);
		//sectorInfo.print();
	}
	rellenar(fout, TRACK_HEADER_LEN - sizeof(T_DskTrackInfo) - sizeof(T_SectorInfo) * formatData->getSectors(), 0);
	rellenar(fout, formatData->getTrackSize() - TRACK_HEADER_LEN, formatData->getFillerByte());
}


// reconstruye un disco standard
void DSK:: rebuild_standard(FormatData* formatData)
{
	debug_dsk("DSK:: rebuild_standard\n");

	close();
	std::string ficheroBak = nombreFichero + ".bak";
	std::rename(ficheroBak.c_str(), nombreFichero.c_str());

	DSK dskViejo;
	dskViejo.open(ficheroBak);
	std::fstream fout {nombreFichero, std::ios::out | std::ios::binary};

	// dsk_header
	T_DskHeader newDskHeader;
	newDskHeader.tracks = dskViejo.dskHeader.tracks;
	newDskHeader.sides = dskViejo.dskHeader.sides;
	newDskHeader.trackSize = 0xFFFF;
	newDskHeader.write(fout);
	
	//tamPistaAnt = dskViejo.dskHeader.getTrackSideSize_standard();
	rellenar(fout, DSK_HEADER_LEN-sizeof(newDskHeader), 0);	

	u32 tamanios[newDskHeader.tracks][newDskHeader.sides] = {0};

	for (u8 p = 0; p < newDskHeader.tracks; p++) {
		for (u8 c = 0; c < newDskHeader.sides; c++) {
			if (p == formatData->getTrack()  &&  c == formatData->getSide()) {
				formatearPista(fout, formatData);
			}
			else {
				goTrackSide(p, c);
				tamanios[p][c] = copiarPista_standard(fichero, fout);
			}
		}
	}

	// escribimos el tamanio de cada pista-cara
	fout.seekg(DSK_HEADER_LEN, std::ios::beg);
	BYTE tamPista;
	for (u8 p = 0; p < newDskHeader.tracks; p++) {
		for (u8 c = 0; c < newDskHeader.sides; c++) {
			tamPista = (BYTE) (tamanios[p][c] >> 8);
			fout.write(reinterpret_cast<char*>(&tamPista), 1);
		}
	}
	dskViejo.close();
	fout.close();

	open_extended(nombreFichero);
}


// reconstruye un disco extended
void DSK:: rebuild_extended(FormatData* formatData) {
	debug_dsk("DSK:: reconstruir extended, pista=%d cara=%d tamPista=%04X\n", 
		formatData->getTrack(), formatData->getSide(), formatData->getTrackSize());

	std::string nombreFicheroTmp = "rebuild.dsk";
	std::fstream fout {nombreFicheroTmp, std::ios::out | std::ios::binary | std::ios::ate };

	// escribir el header del disco
	T_DskHeader newDskHeader;
	newDskHeader.tracks = dskHeader.tracks;
	newDskHeader.sides = dskHeader.sides;
	newDskHeader.trackSize = 0;
	newDskHeader.write(fout);

	newDskHeader.print();
	
	// copiar el tamanio de las pistas
	fichero.seekg(sizeof(T_DskHeader), std::ios::beg);
	BYTE b;
	
	for (u8 p = 0; p < newDskHeader.tracks; p++) {
		for (u8 c = 0; c < newDskHeader.sides; c++) {
			if (p == formatData->getTrack()  &&  c == formatData->getSide()) {
				b = (BYTE) (formatData->getTrackSize() >> 8);
				fout.write(reinterpret_cast<char*>(&b), 1);
			}
			else {
				fichero.read(reinterpret_cast<char*>(&b), 1);
				fout.write(reinterpret_cast<char*>(&b), 1);
				//copiarBytes(fichero, fout, 1);
			}
		}
	}
	debug_dsk("DSK:: rellenar hasta 0x100  %ld\n", 
		DSK_HEADER_LEN - sizeof(T_DskHeader) - newDskHeader.tracks);

	// rellenar con 0s hasta 0x100
	rellenar(fout, DSK_HEADER_LEN - sizeof(T_DskHeader) - newDskHeader.tracks, 0);	
	fichero.seekg(DSK_HEADER_LEN, std::ios::beg);
	
	// copiar pistas
	debug_dsk("DSK:: copiar pistas\n");

	for (u8 p = 0; p < newDskHeader.tracks; p++) {
		for (u8 c = 0; c < newDskHeader.sides; c++) {
			if (p == formatData->getTrack()  &&  c == formatData->getSide()) {
				formatearPista(fout, formatData);
				debug_dsk("DSK:: formatear pista\n");
			}
			else {
				goTrackSide(p, c);
				u16 tp = getTrackSideSize_extended();
				goTrackSide(p, c);
				//copiarBytes(fichero, fout, tp);
				file_copyPiece(fichero, tp, fout);
			}
		}
	}

	// closes
	debug_dsk("DSK:: closes\n");
	fout.close();
	close();

	// renombrar ficheros
	debug_dsk("DSK:: renombrar ficheros\n");
	// de momento conservamos el fichero original, luego borrarlo
	std::string nombreFicheroOrg = nombreFichero + "-orig";
	std::rename(nombreFichero.c_str(), nombreFicheroOrg.c_str());
	debug_dsk("DSK:: ren [%s] -> [%s]\n", nombreFichero.c_str(), nombreFicheroOrg.c_str());

	std::rename(nombreFicheroTmp.c_str(), nombreFichero.c_str());
	debug_dsk("DSK:: ren [%s] -> [%s]\n", nombreFicheroTmp.c_str(), nombreFichero.c_str());

	// abrir el nuevo fichero
	open(nombreFichero);
}


void DSK:: formatTrack(FormatData* formatData) {
	debug_dsk("DSK:: formatTrack, exteded=%d track=%d side=%d\n", extended, formatData->getTrack(), formatData->getSide());

	u16 tamPistaActual;
	
	if (extended) {
		goTrackSide(formatData->getTrack(), formatData->getSide());
		tamPistaActual = getTrackSideSize_extended();
	}
	else {
		tamPistaActual = dskHeader.trackSize;
	}
	debug_dsk("DSK:: tam_pista_actual=%04X    extended=%d    tam_pista_nueva = %04X\n", tamPistaActual, extended, formatData->getTrackSize());

	if (tamPistaActual != formatData->getTrackSize()) {
		debug_dsk("DSK:: reconstruir\n");
		if (extended) rebuild_extended(formatData);
		else          rebuild_standard(formatData); 
		// probar el rebuild_standard!
		// posiblemente sea mejor pasarlo siempre a extended porque el tamanio de los sectores puede ser diferente
		// y asi ahorrarse la comprobacion de tamanio de sectores
	}
	else {
		debug_dsk("DSK:: NO reconstruir\n");
		goTrackSide(formatData->getTrack(), formatData->getSide());
		formatearPista(fichero, formatData);
	}
}

