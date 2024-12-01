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

#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <fstream>
#include "tipos.h"
#include "csw_rle.h"
//#include "directories.h"


constexpr char CDT_MAJOR_VERSION = 1;
constexpr char CDT_MINOR_VERSION = 0;

constexpr bool LEVEL_LOW  = false;
constexpr bool LEVEL_HIGH = true;
constexpr u8 TAPE_HEADER_LEN = 10;

constexpr u16 MIN_PAUSE = 100;



/**
orden de etapas para cada bloque:

0x10  pilot -> sync -> data -> pause -> end
0x11  pilot -> sync -> data -> pause -> end
0x12  pilot -> end
0x13  sync -> end
0x14  data -> pause -> end
0x15  sample_data -> pause -> end
0x18  rle -> pause -> end
0x20  pause -> end

el resto consumen bytes
**/


// para guardar informacion sobre los tamaños de los bloques y calcular la posicion de cada uno
typedef struct {
	u8 headerSize;  // numero de bytes de la cabecera del bloque (excluido el tipo de bloque)
	u8 dataLenOffset;   // donde se encuentra la longitud de los datos en el bloque, 255 si no tiene
	u8 bytesUsed;   // numero de bytes usados para guardar el tamano, 255 si no tiene
	u8 mult;     // multiplicador de (dataLenOffset), ya que pueden ser varios bytes (bytesUsed)
} T_BlockSize;  // informacion acerca del tamanio del bloque


// agrupar las variables donde se tienen los datos del bloque
typedef struct {
	char type;
	u32 totalPulses;
	u32 dataLen, samplingRate;
	u16 pilot_pulse, pilotTone, sync1Pulse, sync2Pulse, pause, 
		oneBitPulse, zeroBitPulse, pulsesLen, statesPerSample;
	u8 usedBits, pulses;
} T_BlockData;


// agrupar las variables para grabar bloque csw
typedef struct {
	std::streamoff blockPos; // posicion de inicio del bloque, donde se escribe x18
	//std::streamoff blockLenPos; // posicion donse se guarda la posicion del tamaño del bloque
	u32 totalBlockLen; // tamaño total del bloque (cabecera+rle)
	bool writeCswAtEnd; // indica si el csw que se escribe es al final de la cinta
	std::fstream tempCswFile; // fichero csw
	std::fstream* ptrFile; // puntero al fichero donde se escribe (fichero o tempCsw)
} T_WriteBlock;



class CDT
{

	static bool SKIP_PAUSES; // minimizar las pausas para que carguen mas rapido

	// stages
	enum STAGE {PILOT = 0, SYNC, DATA, SAMPLE_DATA, PAUSE, END, CSW, PULSE_SECUENCE};
	enum COMPRESSION_TYPE { RLE = 1, ZRLE = 2};


	std::string cdtFilename;
	std::fstream cdtFile;
	bool cdtProtected = false; // proteccion de escritura
	u32 cdtFilesize;
	bool cdtEndReached = false; // fin de cinta alcanzado
	
	std::vector<std::streamoff> listBlockPositions; 
	u16 currentBlock;

	T_BlockData block; // bloque actual
	u8 numCurrentStage = 0; // stage actual
	void (CDT::*stageGetLevel)() = &CDT::endGetLevel; // funcion actual de updateLevel
	STAGE blockStages[5] = {END, END, END, END, END}; // array de stages del bloque actual

	i32 cyclesCounter; // contador de ciclos del pulso actual
	u32 pulseCycles; // ultimo valor de ciclos del pulso obtenido o calculado
	u32 zeroPulseCycles, onePulseCycles; // ciclos para el cero y el uno
	i32 pulsesCounter; // contador de pulsos
	i32 dataByteCounter; // contador de bytes en los bloques DATA
	BYTE dataByte; // byte leido en los bloques DATA
	bool bit; // bit leido en los bloques DATA
	u8 bitsToShift; // bits a rotar del byte obtenido en un bloque DATA
	bool level; // level del pulso actual

	u16* pulsesList = nullptr; // lista de pulsos, para las stages sync y pure_pulse
	u16* ptrPulse = nullptr; // puntero a un elemento de la lista de pulsos (ira creciendo hasta pulsesListEnd)
	u16* pulsesListEnd = nullptr; // fin de la lista de pulsos

	// para grabar bloques
	I_RLEOutputStream* rleOut = nullptr;
	I_RLEInputStream*  rleIn  = nullptr;
	T_WriteBlock writeCsw;


	void nextBlock();
	void flipLevel();

	void getCyclePulsesFromList();
	bool readBit();

	void nothing() {}
	void releasePointers();

	void startPilotStage();
	void startSyncStage();
	void startPulseSequenceStage();
	void startDataStage();
	void startSampleDataStage();
	void startCswStage();
	void startPauseStage();
	void startEndStage();
	
	void setEndStage();

	void pilotGetLevel();
	void syncGetLevel();
	void dataGetLevel();
	void sampleDataGetLevel();
	void cswGetLevel();
	void pauseGetLevel();
	void endGetLevel();
	
	void setBlockStageFunctions(STAGE s1, STAGE s2);
	void setBlockStageFunctions(STAGE s1, STAGE s2, STAGE s3);
	void setBlockStageFunctions(STAGE s1, STAGE s2, STAGE s3, STAGE s4);
	void setBlockStageFunctions(STAGE s1, STAGE s2, STAGE s3, STAGE s4, STAGE s5);
	void nextStage();

	// estas funciones principalmente obtienen los datos de la cabecera
	void startBlock_x10();
	void startBlock_x11();
	void startBlock_x12();
	void startBlock_x13();
	void startBlock_x14();
	void startBlock_x15();
	void startBlock_x18(); // csw
	void startBlock_x20();
	void startBlock_x21();
	void startBlock_x23();
	void startBlock_x24();
	void startBlock_x26();
	void startBlock_x28();
	void startBlock_x2a();
	void startBlock_x2b();
	void startBlock_x30();
	void startBlock_x31();
	void startBlock_x32();
	void startBlock_x33();
	void startBlock_x35();
	void startBlock_x5a();

	void getBlockPositions();

public:
	static void setSkipPauses(bool skipPauses);
	static bool getSkipPauses();
	static bool flipSkipPauses();

	CDT();
	~CDT();

	bool open(std::string& fichero);
	void close();

	void getLevel(u8 ciclos); // obtener el level al que esta la cinta tras X ciclos
	bool getLevel(); // devuelve el ultimo level obtenido
	bool isFinished();
	
	u8 getNumBlocks();
	u8 getCurrentBlock();
	void goTrack(u8 track);
	void rewind();  // al principio del todo
	void wind();   // al final del todo
	void rewind1();  // retroceder una pista
	void wind1();  // avanzar uns pista

	void setLevel(bool bit, u32 ciclos); // fijar el level al escribir
	void endWrite();
	void recomponer();

	void setProtected(bool p);
	bool isProtected();
	bool flipProtected();

	static bool create(const std::string& file);

private:
	// funciones de inicio de los stages
	static constexpr void (CDT::*START_STAGE_FUNCTIONS[8])() = {
		&CDT::startPilotStage, &CDT::startSyncStage, &CDT::startDataStage,
		&CDT::startSampleDataStage, &CDT::startPauseStage, &CDT::startEndStage, 
		&CDT::startCswStage, &CDT::startPulseSequenceStage
	};
	// funciones de actualizacion del nivel (alto/bajo) de la cinta en lectura
	static constexpr void (CDT::*GET_LEVEL_FUNCTIONS[8])() = {
		&CDT::pilotGetLevel, &CDT::syncGetLevel, &CDT::dataGetLevel,
		&CDT::sampleDataGetLevel, &CDT::pauseGetLevel, &CDT::endGetLevel, 
		&CDT::cswGetLevel, &CDT::syncGetLevel
	};

};
