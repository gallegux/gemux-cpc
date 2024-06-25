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

#pragma once

#include <list>
#include <string>
#include <fstream>
#include <string.h>
#include "tipos.h"
#include "log.h"

// es.wikipedia.org/wiki/Controlador_de_disquete

#define MAX_TRACKS 80
#define MAX_SECTORS 36
#define DSK_HEADER_LEN 0x100
#define TRACK_HEADER_LEN 0x100
#define SECTOR_INFO_LEN 8
#define SECTOR_INFORMATION_LIST_POS 0x18
#define TRACK_TABLE_POS 0x34

// errores en lectura/escritura
#define DSK_PROTECTED -4
#define DSK_SECTOR_NOT_FOUND -3
#define DSK_TRACK_NOT_FOUND -2
#define DSK_SIDE_NOT_FOUND -1
#define DSK_NOT_VALID_FORMAT -10



#pragma pack(push, 1)
class T_DskHeader 
{
private:    u8 fileType[34] = {0};
private:    u8 creator[14] = {'G','e','m','u','x',' ',' ',' ',' ',' ',' ',' ',' ',' '};
public:     u8 tracks = 0;
public:     u8 sides = 0;
public:     u16 trackSize = 0; // unused
	        // a continuacion track size table para los extended
public:
	T_DskHeader();
	T_DskHeader(u8 _tracks, u8 _sides);

	bool isFormatted();
    
	void calcTrackSize(u8 sectors, u16 sectorSize);
	u16 getTrackSize();

	void write(std::fstream& f);
	void load(std::fstream& f);

    void setExtended(bool e);
    bool isExtended();
};
#pragma pack(pop)



#pragma pack(push, 1)
class T_DskTrackInfo
{
private:    u8 blockInfo[12] = {'T','r','a','c','k',' ','I','n','f','o','\r','\n'};
private:    u8 noused1[4] = {0,0,0,0};
public:     u8 track = 0;
public:     u8 side = 0;
private:    u8 noused2[2] = {0,0};
public:     u8 sectorSize = 0; // 1=256; 2=512; ... byte alto
public:     u8 sectors = 0; //9;
public:     u8 gap = 0; //0x4E;
public:     u8 filler = 0 ; //0xE5;
    // y luego el Track Information Block para los Extended
	// inicializo valores pq sino al leer la estructura no funciona
public:
	T_DskTrackInfo();
	T_DskTrackInfo(u8 _track, u8 _side, u8 _sectors, u8 _sectorSize);
	T_DskTrackInfo(u8 _track, u8 _side, u8 _sectors, u8 _sectorSize, u8 _gap, u8 _filler);

	bool isFormatted();

	void write(std::fstream& f);
	void load(std::fstream& f);
};
#pragma pack(pop)


#pragma pack(push, 1)
class T_SectorInfo 
{
public:     u8 track = 0; // C
public:     u8 side = 0; // H
public:     u8 sectorId = 0; // R
public:     u8 sectorSize = 0; // N  , byte alto
private:    u8 str1 = 0;
private:    u8 str2 = 0;
public:     u16 actualDataLen = 0; // el tamanio en bytes, solo se usa en los extended

public:
	T_SectorInfo();
	T_SectorInfo(u8 _track, u8 _side, u8 _sectorId, u8 _sectorSize);
	T_SectorInfo(u8 _track, u8 _side, u8 _sectorId, u8 _sectorSize, u16 _actualDataLen);

	void write(std::fstream& f);
	void load(std::fstream& f);
};
#pragma pack(pop)




class DSK
{
	T_DskHeader dskHeader;	// informacion minima
	std::fstream fichero;
	u32 tamFichero = 0;
	u32** tablaPosicionesPistas = nullptr;  // posicion en el fichero para cada pista,cara
	// la tabla guarda el tamaño total de cada pista (incluida la cabecera), y
	// nos sirve para situarnos en la pista sin recorrer el fichero
	bool extended = true;
	bool protegido = false;  // disco protegido?
	//u8 pistaActual = 0;

	void crearTabla(u8 pistas, u8 caras);	// crea las tablas

public:

	DSK();
	~DSK();

	bool open(std::string& f);
	void close();	// cierra el fichero actual
	void saveAs(std::string& f);	// graba el disco con otro nombre

	bool isFormatted();
	u8 getTracks();
	u8 getSides();
	u32 getTrackPos(u8 track);

	void getTrackInfo(u8 track, u8 side, T_DskTrackInfo* trackInfo);
	void getSectorInfo_N(u8 track, u8 side, u8 sector, T_SectorInfo* sectorInfo);
	bool getSectorInfo_ID(u8 track, u8 side, u8 sectorId, T_SectorInfo* sectorInfo);
	bool existsTrack(u8 track, u8 side);
	bool existsSector(u8 track, u8 side, u8 sector); // true=encontrado, false=no encontrado
	//void readSector(u8 track, u8 side, u8 sector, u16 sectorSize, BYTE* buffer);
	void readSectorData(u16 sectorSize, BYTE* buffer);
	void writeSectorData(u16 sectorSize, BYTE* buffer);
	//void writeSector(u8 track, u8 side, u8 sector, BYTE *buffer);
	
	// formatea una pista, de momento solo para discos standard
	void formatTrack(u8 track, u8 side, u8 sectors, u8 sectorSize, u8 firstSectorId, BYTE fillerByte);

	bool isProtected();
	void setProtected(bool p);
	bool flipProtected();

	static void create(std::string& f, u8 pistas, u8 sides, u8 sectores, u8 sectorSize, BYTE fillerByte, BYTE gap, BYTE primerSectorId);
	static void createStandardData(std::string& f);
	static void createStandardSystem(std::string& f);
	
};