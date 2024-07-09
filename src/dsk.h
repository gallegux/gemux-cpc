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

// https://www.cpcwiki.eu/index.php/Format:DSK_disk_image_file_format
// es.wikipedia.org/wiki/Controlador_de_disquete

#pragma once

#include <list>
#include <string>
#include <fstream>
#include <string.h>
#include "tipos.h"
#include "log.h"

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


#define FD_TRACK 0
#define FD_SIDE 1
#define FD_SECTOR_ID 2
#define FD_SECTOR_SIZE 3



class FormatData
{
	u8 track, side, sectors;
	BYTE sectorSize, gap, fillerByte;
	u8 contBytes;
	BYTE data[MAX_SECTORS][4];
	BYTE* ptrData;

public:
    FormatData(u8 track, u8 side, u8 sectors, BYTE sectorSize, BYTE gap, BYTE fillerByte);

	u8 getTrack();
	u8 getSide();
	u8 getSectors();
	BYTE getSectorSize();
	BYTE getFillerByte();
	BYTE getGap();
	void addByte(BYTE b);
	bool isFull();
	void print();

	u8   getTrack(u8 sector);
	u8   getSide(u8 sector);
	BYTE getSectorId(u8 sector);
	BYTE getSectorSize(u8 sector);
	u16  getTrackSize();			// el tamanio es calculado
};



#pragma pack(push, 1)
class T_DskHeader 
{
private:    u8 fileType[34] = {0};
private:    u8 creator[14] = {'G','e','m','u','x','-','C','P','C',' ',' ',' ',' ',' '};
public:     u8 tracks = 0;
public:     u8 sides = 0;
public:     u16 trackSize = 0; // solo aplica para los discos standard
	        // a continuacion track size table para los extended
public:
	T_DskHeader();
	T_DskHeader(u8 _tracks, u8 _sides);

	bool isFormatted();
    
	u16 getTrackSize(); // solo aplica para los discos standard

	void write(std::fstream& f);
	void load(std::fstream& f);

    void setExtended(bool e);
    bool isExtended();

	void print();
};
#pragma pack(pop)



#pragma pack(push, 1)
class T_DskTrackInfo
{
private:    u8 blockInfo[12] = {'T','r','a','c','k','-','I','n','f','o','\r','\n'};
private:    u8 noused1[4] = {0,0,0,0};
public:     u8 track = 0;
public:     u8 side = 0;
private:    u8 noused2[2] = {0,0};
public:     u8 sectorSize = 0; // 1=256; 2=512; 4=1024... byte alto
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

	u16 getSectorDataLength(); // solo para los discos standar

	void print();
};
#pragma pack(pop)


#pragma pack(push, 1)
class T_SectorInfo 
{
public:     u8 track = 0; // C
public:     u8 side = 0; // H
public:     u8 sectorId = 0; // R
public:     u8 sectorSize = 0; // N  , byte alto
public:     BYTE fdcSt1 = 0;
public:     BYTE fdcSt2 = 0;
public:     u16 dataLength = 0; // el tamanio en bytes, solo se usa en los extended

public:
	T_SectorInfo();
	T_SectorInfo(u8 _track, u8 _side, u8 _sectorId, u8 _sectorSize);
	T_SectorInfo(u8 _track, u8 _side, u8 _sectorId, u8 _sectorSize, u16 _actualDataLen);

	void write(std::fstream& f);
	void load(std::fstream& f);

	u16 getDataLength();

	void print();
};
#pragma pack(pop)




class DSK
{
protected:
	std::fstream fichero;
	std::string nombreFichero;

	u32 tamFichero = 0;
	bool extended = true;
	bool protegido = false;  // disco protegido?

	T_DskHeader dskHeader;	// informacion minima
	i32** tablaPosicionesPistas = nullptr;  // posicion en el fichero para cada pista,cara

	void crearTabla(u8 pistas, u8 caras);	// crea la tablaPosicionesPistas

	bool getSectorInfo_ID_standard(u8 track, u8 side, u8 sectorId, T_SectorInfo* sectorInfo);
	bool getSectorInfo_ID_extended(u8 track, u8 side, u8 sectorId, T_SectorInfo* sectorInfo);

	void open_standard(std::string& f);
	void open_extended(std::string& f);

	void rebuild_standard(FormatData* formatData);
	/* en los discos standard todas las pistas tienen el mismo tamanio, 
	   y los sectores en las pistas tambien tienen el mismo tamanio
	   eso es asi porque la localizacion de cada pista y sector se calcula
	*/
	void rebuild_extended(FormatData* formatData);
	/* en los discos extended cada pista y sector puede tener un tamanio distinto
	   para las pistas tenemos una tabla y los sectores habra que calcularlos sumando 
	   el tamanio de los sectores previos
	*/

	u16  getTrackSideSize_standard(); // devuelve el tamanio de la pista en el fichero dsk
	u16  getTrackSideSize_extended();

public:

	DSK();
	~DSK();

	bool open(std::string& f);
	void close();	// cierra el fichero actual
	void saveAs(std::string& f);	// graba el disco con otro nombre (siempre como extended)

	bool isFormatted(); // se considera formateado si el nº_pistas>0
	u8   getTracks();
	u8   getSides();

	bool isProtected();
	void setProtected(bool p);
	bool flipProtected();

	u32 getTamFichero();

	//bool existsTrackInFile(u8 track); // comprueba si hay datos de la pista en el fichero

	i32  getTrackPos(u8 track, u8 side); // consulta la tabla y devuelve la posicion de una pista
	i32  goTrackSide(u8 track, u8 side); // pone PE y PL en el inicio de la pista
	void getTrackInfo(u8 track, u8 side, T_DskTrackInfo* trackInfo);

	void getSectorInfo_N (u8 track, u8 side, u8 sector,   T_SectorInfo* sectorInfo);

	// obtiene el sector con el ID indicado
	// devuelve true si lo encuentra, false en el caso contrario
	// deja PL y PE al inicio de los datos del sector
	bool getSectorInfo_ID(u8 track, u8 side, u8 sectorId, T_SectorInfo* sectorInfo);

	bool existsTrack(u8 track);
	bool existsTrackSide(u8 track, u8 side);
	bool existsSector(u8 track, u8 side, u8 sector); // true=encontrado, false=no encontrado
	//bool existsTrackInFile(u8 track, u8 side);

	void readSectorData(u16 sectorSize, BYTE* buffer);	// lee un sector entero
	void writeSectorData(u16 sectorSize, BYTE* buffer);  // graba un sector entero
	void readByte(BYTE *dato);
	void writeByte(BYTE dato);
	
	// formatea una pista, puede reconstruir el disco
	void formatTrack(FormatData *formatData);

	// metodos estaticos

	static void createTrackSide(std::fstream& dskFile, u8 pista, u8 cara, u8 sectores, u8 sectorSize, BYTE fillerByte, BYTE gap, BYTE primerSectorId);
	static void createTrack(std::fstream& dskFile, u8 pista, u8 caras, u8 sectores, u8 sectorSize, BYTE fillerByte, BYTE gap, BYTE primerSector);
	static void create(std::string& f, u8 pistas, u8 sides, u8 sectores, u8 sectorSize, BYTE fillerByte, BYTE gap, BYTE primerSectorId);
	
	static void createStandardData(std::string& f); // 40 pistas, 9 sectores, 1 cara
	static void createStandardSystem(std::string& f); // 40 pistas, 9 sectores, 1 cara

	static void create35Data(std::string& f); // 80 pistas, 9 sectores, 2 caras
	static void create35System(std::string& f); // 80 pistas, 9 sectores, 2 caras
	
	// TODO anadir mas tipos de discos
};


