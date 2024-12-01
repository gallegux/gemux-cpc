/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  CDT block write implementation                                            |
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


#include <string>
#include <filesystem>
#include "cdt.h"
#include "tipos.h"
#include "file_util.h"
#include "directories.h"
#include "log.h"


void CDT:: setLevel(bool b, u32 ciclos) {
	if (rleOut == nullptr) {
		debug_cdt("CDT:: crear outputstream\n");
		writeCsw.writeCswAtEnd = (listBlockPositions.size() == 0) || (currentBlock == listBlockPositions.size());

		if (writeCsw.writeCswAtEnd) {
			debug_cdt("CDT:: write csw at end\n");
			writeCsw.ptrFile = &cdtFile;
			writeCsw.blockPos = cdtFile.tellg();
			cdtFile.seekp(writeCsw.blockPos, std::ios::beg);
			debug_cdt("CDT:: block_pos = %lld\n", writeCsw.blockPos);
			rleOut = new ZRLEOutputStream(cdtFile);
		}
		else {
			debug_cdt("CDT:: write csw at track #%d\n", currentBlock);
			writeCsw.tempCswFile = std::fstream( std::string{CSW_AUX_FILE}, 
				std::ios::binary | std::ios::out | std::ios::in | std::ios::trunc);
			writeCsw.ptrFile = &writeCsw.tempCswFile;
			writeCsw.blockPos = 0;
			rleOut = new ZRLEOutputStream(writeCsw.tempCswFile);
		}
		
		BYTE blockType = 0x18;
		u32 blockLen_storedPulses = 0xEEEEEEEE;
		u16 pause = 0;
		u32 samplingRate = 44100;
		BYTE compressionType = ZRLE;

		writeCsw.ptrFile->write(reinterpret_cast<char*>(&blockType), 1);
		writeCsw.ptrFile->write(reinterpret_cast<char*>(&blockLen_storedPulses), 4); // blockLen
		writeCsw.ptrFile->write(reinterpret_cast<char*>(&pause), 2);
		writeCsw.ptrFile->write(reinterpret_cast<char*>(&samplingRate), 3);
		writeCsw.ptrFile->write(reinterpret_cast<char*>(&compressionType), 1);
		writeCsw.ptrFile->write(reinterpret_cast<char*>(&blockLen_storedPulses), 4); // storedPulses

		debug_cdt("CDT:: start_write  blockPos=%05X\n", writeCsw.blockPos);
	}
	rleOut->addPulseLevel(b, ciclos);
}


void CDT:: endWrite() {
	rleOut->close();

	writeCsw.totalBlockLen = 1 + 0x0E + rleOut->getBytesOutput();
	u32 pulses = rleOut->getPulses();
	u32 blockLen = writeCsw.totalBlockLen - 1 - 4; // menos el x18 y menos los 4 bytes "Block length (without these four bytes)"
		
	debug_cdt("CDT:: endWrite()  pulses=%d  fin=%llX  total_block_len=%04X\n",  \
		pulses, writeCsw.blockPos + writeCsw.totalBlockLen, writeCsw.totalBlockLen );

	// nos situamos en el campo de block_len
	writeCsw.ptrFile->seekp(writeCsw.blockPos + 1, std::ios::beg); // posicion de Block_length	
	writeCsw.ptrFile->write(reinterpret_cast<char*>(&blockLen), 4);  // escribmos el tamanio
	writeCsw.ptrFile->seekp(writeCsw.blockPos + 1 + 0x0A, std::ios::beg); // el numero de pulsos se encuentra 6 bytes despues del tamanio
	writeCsw.ptrFile->write(reinterpret_cast<char*>(&pulses), 4);
	writeCsw.ptrFile->flush();
	//
	delete rleOut;
	rleOut = nullptr;
	writeCsw.ptrFile = nullptr;

	if (writeCsw.writeCswAtEnd) {
		cdtFilesize += writeCsw.totalBlockLen;
		listBlockPositions.push_back(writeCsw.blockPos);
		wind();  // nos situamos al final de la cinta
	}
	else {
		u8 ct = currentBlock;
		recomponer();
		goTrack(ct+1);
	}
}


// recomponer indice de pistas
void CDT:: recomponer() {
	std::fstream dst {cdtFilename + ".new", std::ios::binary | std::ios::out}; // el fichero nuevo

	cdtFile.seekg(0, std::ios::beg);
	writeCsw.tempCswFile.seekg(0, std::ios::beg);

	debug_cdt("CDT:: copiar inicio cinta, hasta pista #%d (%5X)\n", currentBlock, listBlockPositions[currentBlock]);
	file_copyPiece(cdtFile, listBlockPositions[currentBlock], dst);
	debug_cdt("CDT:: copiar pista nueva CSW (tam=%ld)\n", writeCsw.totalBlockLen);
	file_copyPiece(writeCsw.tempCswFile, writeCsw.totalBlockLen, dst);
	debug_cdt("CDT:: copiar pistas posteriores (bytes=%ld)\n", cdtFilesize - listBlockPositions[currentBlock]);
	file_copyPiece(cdtFile, cdtFilesize - listBlockPositions[currentBlock], dst);
	
	cdtFile.close();
	writeCsw.tempCswFile.close();
	dst.close();

	if (dst.fail())  debug_cdt("CDT:: Error al cerrar dst\n");
	if (cdtFile.fail())  debug_cdt("CDT:: Error al cerrar org\n");
	if (writeCsw.tempCswFile.fail())  debug_cdt("CDT:: Error al cerrar csw\n");

	debug_cdt("CDT:: borrar CSW\n");
	std::filesystem::remove(CSW_AUX_FILE);
	debug_cdt("CDT:: borrar original\n"); 
	std::filesystem::remove(cdtFilename);

	debug_cdt("CDT:: renombrar .new\n");
	std::rename( (cdtFilename + ".new").c_str(), cdtFilename.c_str() );

	debug_cdt("CDT:: abrir fichero CDT nuevo\n");
	open(cdtFilename);
}
