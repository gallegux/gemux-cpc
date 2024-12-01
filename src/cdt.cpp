/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  CDT implementation                                                        |
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

// https://www.alessandrogrussu.it/tapir/tzxform120.html
// https://www.cpcwiki.eu/index.php/Format:CDT_tape_image_file_format#Tape-Image_.28.CDT.29_file_format_.28Amstrad_specific.29

#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <algorithm>
#include <string.h>
#include "tipos.h"
#include "log.h"
#include "cdt.h"
#include "compilation_options.h"
#include "csw_rle.h"
#include "directories.h"
#include "util.h"
#include "file_util.h"




// static
bool CDT::SKIP_PAUSES = false;


// cuanto ocupa la cabecera (sin datos) de cada bloque
const std::unordered_map<BYTE, T_BlockSize> HM_BLOCK_DATALEN = {
	{0x10, {  4,   2,   2, 1} },
	{0x11, {0x12,0xF,   3, 1} },
	{0x12, {  4, 255, 255, 1} },
	{0x13, {  1,   0,   1, 2} },
	{0x14, {0xA,   7,   3, 1} },
	{0x15, {  8,   5,   3, 1} },
	{0x18, {  4,   0,   4, 1} },
	{0x19, {  4,   0,   4, 1} },
	{0x20, {  2, 255, 255, 1} },
	{0x21, {  1,   0,   1, 1} },
	{0x22, {  0, 255, 255, 1} },
	{0x23, {  2, 255, 255, 1} },
	{0x24, {  2, 255, 255, 1} },
	{0x25, {  0, 255, 255, 1} },
	{0x26, {  2,   0,   2, 2} }, // *2
	{0x27, {  0, 255, 255, 1} },
	{0x28, {  2,   0,   2, 1} },
	{0x2A, {  4,   0,   4, 1} },
	{0x2B, {  1,   0,   4, 1} },
	{0x30, {  1,   0,   1, 1} },
	{0x31, {  2,   1,   1, 1} },
	{0x32, {  2,   0,   2, 1} },
	{0x33, {  1,   0,   1, 3} },   // *3
	{0x35, {0x14,0x10,  4, 1} },
	{0x5A, {  9, 255, 255, 1} }
};



CDT:: CDT() {
	pulsesList = nullptr;
}


CDT:: ~CDT() {
	releasePointers();
}


void CDT:: setSkipPauses(bool skipPauses) { SKIP_PAUSES = skipPauses; }

bool CDT:: getSkipPauses() { return SKIP_PAUSES; }

bool CDT:: flipSkipPauses() {
	SKIP_PAUSES = !SKIP_PAUSES;
	return SKIP_PAUSES;
}



// estatico
bool CDT:: create(const std::string& filename) {
	try {
		std::ofstream f (filename, std::ios::binary);

		char CDT_HEADER[10] = {'Z','X','T','a','p','e','!', '\x1A', CDT_MAJOR_VERSION, CDT_MINOR_VERSION};
		f.write(CDT_HEADER, sizeof(CDT_HEADER));
		f.close();
		return true;
	}
	catch (...) {
		return false;
	}
}



void CDT:: getBlockPositions() {
	BYTE headerId;
	u32 dataBlockLen;
	std::streamoff blockPos;
	
	listBlockPositions.clear();

	while (cdtFile.tellg() < cdtFilesize  &&  !cdtFile.eof()) {
		blockPos = cdtFile.tellg();
		listBlockPositions.push_back(blockPos);
		debug_cdt("CDT:: block_pos %05X", blockPos);

		file_nextByte(cdtFile, &headerId);
		auto it = HM_BLOCK_DATALEN.find(headerId);
		if (it != HM_BLOCK_DATALEN.end()) {
			const T_BlockSize& block = it->second;
			//u8 header[16];
			//fichero.read(reinterpret_cast<char*>(&header), b.blockHeaderSize);
			debug_cdt("   type %02X   header_size %02d  len_offset %03d  bytes %03d  mult %d", 
						headerId, block.headerSize, block.dataLenOffset, block.bytesUsed, block.mult);
			std::streamoff so = cdtFile.tellg();
			//debug_cdt("CDT:: file_pos = %llx\n", static_cast<std::streamoff>(fichero.tellg()));

			if (block.dataLenOffset != 255) {
				cdtFile.seekg(block.dataLenOffset, std::ios::cur);
				switch (block.bytesUsed) {
					case 1: dataBlockLen = file_nextByte(cdtFile);   break;
					case 2: dataBlockLen = file_nextWord(cdtFile);   break;
					case 3: dataBlockLen = file_next3Bytes(cdtFile); break;
					case 4: dataBlockLen = file_nextDWord(cdtFile);  break;
				}
				//debug_cdt("CDT:: blockLen = %d", dataBlockLen);
				dataBlockLen *= block.mult;
				//debug_cdt("   blockLen*mult+sum = %d\n", dataBlockLen);
				debug_cdt("  DATA=%d\n", dataBlockLen);

				cdtFile.seekp(blockPos + 1 + block.headerSize + dataBlockLen, std::ios::beg);
			}
			else {
				cdtFile.seekp(blockPos + 1 + block.headerSize, std::ios::beg);
				debug_cdt("\n");
			}
		}
		else {
			debug_cdt("CDT!!  no encontrado  %02X\n", headerId);
		}
	}
	debug_cdt("CDT:: tracks = %d\n", listBlockPositions.size());
}



bool CDT:: open(std::string& f) {
	cdtFilename = f;
	debug_cdt("CDT:: open()  fichero = %s", f.c_str());
	cdtFile = std::fstream(f, std::ios::in | std::ios::out | std::ios::binary | std::ios::ate);
	// ate nos situal al final
	// Obtener la posición, que corresponde al tamaño del fichero
	cdtFilesize = cdtFile.tellg();
	debug_cdt("   tamFichero = %ld\n", cdtFilesize);
	// Mover el puntero al inicio del fichero
	cdtFile.seekg(0, std::ios::beg);

	// leer cabecera y comprobar que es buena
	char file_header[7];
	cdtFile.read(reinterpret_cast<char*>(file_header), 7);

	char CDT_HEADER[8] = {'Z','X','T','a','p','e','!'};
	if (memcmp(file_header, CDT_HEADER, 7) != 0) {
		cdtFile.close();
		debug_cdt("CDT:: open() fichero no valido\n");
		return false;
	}

	char b;
	cdtFile.read(&b, 1);

	cdtFile.read(&b, 1);
	debug_cdt("CDT:: open() version = %d", b);
	cdtFile.read(&b, 1);
	debug_cdt(".%d\n", b);   

	getBlockPositions();
	rewind();
	return true;
}


void CDT:: close() { cdtFile.close(); }


u8 CDT:: getNumBlocks() { return listBlockPositions.size(); }

u8 CDT:: getCurrentBlock() { return currentBlock; }


void CDT:: rewind() { 
	debug_cdt("CDT:: rewind()\n") ;
	cdtFile.clear();
	cdtFile.seekg(10, std::ios::beg);
	cdtFile.seekp(10, std::ios::beg);
	std::streamoff fpos = cdtFile.tellg(); 
	debug_cdt("CDT:: rewind() %lld\n", fpos);
	level = LEVEL_LOW;
	cyclesCounter = 0;
	cdtEndReached = false; 
	setEndStage();
	currentBlock = 0;
}


void CDT:: wind() {
	level = LEVEL_LOW;
	cyclesCounter = 0;
	cdtEndReached = false; 
	//etapa = END;
	setEndStage();
	cdtFile.clear();
	cdtFile.seekg(0, std::ios::end);
	cdtFile.seekp(0, std::ios::end);
	debug_cdt("CDT:: wind() eof=%d\n", cdtFile.eof());
	currentBlock = listBlockPositions.size();
}


void CDT:: rewind1() {
	if (currentBlock > 0)  --currentBlock;
	cdtFile.clear();
	cdtFile.seekg( listBlockPositions[currentBlock], std::ios::beg);
	cdtFile.seekp( listBlockPositions[currentBlock], std::ios::beg);
	setEndStage();
	level = LEVEL_LOW;
	cyclesCounter = 0;
}


void CDT:: wind1() {
	if (currentBlock < listBlockPositions.size())  ++currentBlock;
	else  cdtEndReached = true;
	cdtFile.clear();
	cdtFile.seekg( listBlockPositions[currentBlock], std::ios::beg);
	//fichero.seekp( v_posicion_bloques[currentTrack], std::ios::beg);
	debug_cdt("CDT:: wind() eof=%d\n", cdtFile.eof());
	setEndStage();
	level = LEVEL_LOW;
	cyclesCounter = 0;
}


void CDT:: goTrack(u8 track) {
	if (track <= listBlockPositions.size()) {
		currentBlock = track;
		cdtFile.clear();
		cdtFile.seekg( listBlockPositions[track], std::ios::beg);
		setEndStage();
		level = LEVEL_LOW;
		cyclesCounter = 0;

		cdtEndReached = track == listBlockPositions.size();
	}
}


void CDT:: setProtected(bool p) { cdtProtected = p; }

bool CDT:: isProtected() { return cdtProtected; }

bool CDT:: flipProtected() { return (cdtProtected = !cdtProtected); }


bool CDT:: isFinished() { return cdtEndReached; };

bool CDT:: getLevel() { return level; }
