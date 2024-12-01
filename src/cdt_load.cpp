/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  CDT block load implementation                                             |
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


#include "cdt.h"
#include "file_util.h"
#include "log.h"


// The timings are given in Z80 clock ticks (T states) unless otherwise stated. 1 T state = (1/3500000)s
inline u32 getCycles_t(u32 t) { return static_cast<u32>((t * 40) / 35); }

inline u32 getCycles_time(u32 tiempo) { return tiempo * 1000; }

inline void CDT:: flipLevel() { level = !level; }



void CDT:: getLevel(u8 ciclos) { // ciclos=nops
	cyclesCounter -= (ciclos << 2);  // x4 (y son t-states)
	if (cyclesCounter <= 0) { // si se han terminado los ciclos del pulso actual
		(this->*stageGetLevel)(); // pasamos al siguiente stage
	}
}



void CDT:: nextBlock() {
	currentBlock++;
	debug_cdt("\nCDT:: nextBlock()  %d/%d  pos=%05X/%05X\n", 
		currentBlock, getNumBlocks(),
		(u32) cdtFile.tellg(), cdtFilesize);

	if (cdtFile.tellg() < cdtFilesize  &&  !cdtFile.eof()) {
		cdtFile.read(&block.type, 1);
		debug_cdt("CDT:: nextBlock()  type=0x%02X\n", block.type);
		switch (block.type) {
			case 0x10:  startBlock_x10(); break;
			case 0x11:  startBlock_x11(); break;
			case 0x12:  startBlock_x12(); break;
			case 0x13:  startBlock_x13(); break;
			case 0x14:  startBlock_x14(); break;
			case 0x15:  startBlock_x15(); break;
			case 0x18:  startBlock_x18(); break;
			case 0x20:  startBlock_x20(); break;
			case 0x21:  startBlock_x21(); break;
			case 0x23:  startBlock_x23(); break;
			case 0x24:  startBlock_x24(); break;
			case 0x26:  startBlock_x26(); break;
			case 0x30:  startBlock_x30(); break;
			case 0x31:  startBlock_x31(); break;
			case 0x32:  startBlock_x32(); break;
			case 0x33:  startBlock_x33(); break;
			case 0x35:  startBlock_x35(); break;
			case 0x5A:  startBlock_x5a(); break;
		}
		return;
	}
	else {
		currentBlock = listBlockPositions.size();
		setEndStage();
		cdtEndReached = true;
		debug_cdt("CDT:: finDeCintaAlcanzado=1\n");
	}
}



// modifica los atributos pulseCycles y ptrPulse
void CDT:: getCyclePulsesFromList() {
	pulseCycles = getCycles_t(*ptrPulse);
	debug_cdt("CDT:: pulse_len = %d\n", pulseCycles);
	if (++ptrPulse == pulsesListEnd) ptrPulse = pulsesList;
}


// return true=obtenido false=no_obtenido
bool CDT:: readBit() {
	if (dataByteCounter > 0) {
		if (!bitsToShift) {
			file_nextByte(cdtFile, &dataByte);
			bitsToShift = 8;
		}
		bit = dataByte & 0x80;
		dataByte <<= 1;
		bitsToShift--;
		dataByteCounter--;
		return true;
	}
	else return false;
}


//-------------------------------------------------------------------------------------------

void CDT:: startPilotStage() {
	debug_cdt("CDT:: etapa = pilot    pulses=%d  pulse_cycles=%d\n", block.pilotTone, block.pilot_pulse);

	pulseCycles = getCycles_t(block.pilot_pulse);
	cyclesCounter += pulseCycles;
	pulsesCounter = block.pilotTone;
}

void CDT:: pilotGetLevel() {
	flipLevel();
	if (--pulsesCounter > 0)  cyclesCounter += pulseCycles;
	else  nextStage();
}


void CDT:: startSyncStage() {
	debug_cdt("CDT:: etapa = sync   pulsos = %d,%d\n", block.sync1Pulse, block.sync2Pulse);

	ptrPulse = pulsesList = new u16[2]; // uso la lista de pulsos en vez de variables propias
	pulsesList[0] = block.sync1Pulse;
	pulsesList[1] = block.sync2Pulse;

	getCyclePulsesFromList();
	cyclesCounter += static_cast<i32>(pulseCycles); // set cycle count for current level
	pulsesCounter = 2;
}

void CDT:: startPulseSequenceStage() {
	// la lista ya esta creada y con valores
	#ifdef debug_cdt
	for (u8 i = 0; i < block.pulses; i++) debug_cdt("%d ", pulsesList[i]);
	debug_cdt("\n");
	#endif
	ptrPulse = pulsesList;
	getCyclePulsesFromList(); // <<pulseCycles
	cyclesCounter += static_cast<i32>(pulseCycles); // set cycle count for current level
	pulsesCounter = block.pulses;
}

void CDT:: syncGetLevel() {
	flipLevel();
	if (--pulsesCounter > 0) {
		getCyclePulsesFromList(); // <<pulseCycles
		cyclesCounter += pulseCycles;
	}
	else  nextStage();
}


void CDT:: startDataStage() {
	block.zeroBitPulse = getCycles_t(block.zeroBitPulse);
	block.oneBitPulse =  getCycles_t(block.oneBitPulse);
	dataByteCounter = --block.dataLen * 8 + block.usedBits;
	bitsToShift = 0;
	debug_cdt("CDT:: stage = data   numero_bits=%d\n", dataByteCounter);

	flipLevel();
	dataGetLevel();
}

void CDT:: dataGetLevel() {
	flipLevel();
	if (--pulsesCounter > 0)  cyclesCounter += pulseCycles;
	else {
		if (readBit()) {  // modifica bit
			pulseCycles = (bit) ? block.oneBitPulse : block.zeroBitPulse;
			cyclesCounter += static_cast<u32>(pulseCycles);
			pulsesCounter = 2;
		}
		else  nextStage();
	}
}


void CDT:: startSampleDataStage() {
	dataByteCounter = --block.dataLen * 8 + block.usedBits;
	debug_cdt("CDT:: etapa = sample-data   numero_bits=%d\n", dataByteCounter);
	bitsToShift = 0;
	pulseCycles = getCycles_t(block.statesPerSample);
	
	sampleDataGetLevel();
}

void CDT:: sampleDataGetLevel() {
	if (readBit()) {  // modifica bit
		cyclesCounter += static_cast<u32>(pulseCycles);
		level = bit;
	}
	else  nextStage();
}


void CDT:: startCswStage() {
	//pulseCycles = getCycles_t(rleIn->getPulseCycles());
	pulseCycles = rleIn->getPulseCycles();
	debug_cdt("CDT:: stage = csw   pulse_cycles=%d\n", pulseCycles);
	cyclesCounter += (pulseCycles << 2); 
}

void CDT:: cswGetLevel() {
	//debug_cdt("%d\t|  ", cont_ciclos);
	if (--cyclesCounter <= 0) {
		if (rleIn->hasMore()) {
			flipLevel();
			pulseCycles = rleIn->getPulseCycles();
			cyclesCounter += (pulseCycles << 2);
			//debug_cdt("CDT:: +ciclos %d  ciclos %d   cont_ciclos %d\n", pc<<2, pc, cont_ciclos);
		}
		else  nextStage();
	}
}


void CDT:: startPauseStage() {
	debug_cdt("CDT:: stage = pause   ms=%d\n", block.pause);

	if (block.pause > 0) { 
		pulseCycles = getCycles_time(1); // pausa de 1ms para forzar el cambio de nivel
		cyclesCounter += static_cast<u32>(pulseCycles);
		
		if (SKIP_PAUSES) {
			debug_cdt("CDT:: pausa reducida a 100ms\n");
			pulseCycles = getCycles_time( 
				(MIN_PAUSE < block.pause) ? MIN_PAUSE : block.pause-1); // pausa en ms
		}
		else {
			pulseCycles = getCycles_time(block.pause-1); // pausa en ms
		}
		cyclesCounter += static_cast<u32>(pulseCycles);
		pulsesCounter = 2; // just one pulse
	}
	else nextStage();
}

void CDT:: pauseGetLevel() {
	//level = LEVEL_LOW;
	if (--pulsesCounter > 0)  cyclesCounter += pulseCycles; // anadir los ciclos de la pausa en si
	else  nextStage();
}


void CDT:: startEndStage() { 
	debug_cdt("CDT:: stage = end\n");
	nextBlock();
}

void CDT:: endGetLevel() {
	debug_cdt("CDT:: stage=end\n");
	nextBlock();
}


void CDT:: releasePointers() {
	if (pulsesList != nullptr) { // eliminamos la lista de pulsos si existe
		delete[] pulsesList;
		pulsesList = nullptr;
	}
	if (rleIn != nullptr) { // eliminamos el objeto de entrada csw si existe
		delete rleIn;
		rleIn = nullptr;
	}
}

inline void CDT:: setBlockStageFunctions(STAGE s1, STAGE s2, STAGE s3, STAGE s4, STAGE s5) {
	blockStages[0] = s1;
	blockStages[1] = s2;
	blockStages[2] = s3;
	blockStages[3] = s4;
	blockStages[4] = s5;
	numCurrentStage = 0;
}
inline void CDT:: setBlockStageFunctions(STAGE s1, STAGE s2, STAGE s3, STAGE s4) {
	blockStages[0] = s1;
	blockStages[1] = s2;
	blockStages[2] = s3;
	blockStages[3] = s4;
	numCurrentStage = 0;
}
inline void CDT:: setBlockStageFunctions(STAGE s1, STAGE s2, STAGE s3) {
	blockStages[0] = s1;
	blockStages[1] = s2;
	blockStages[2] = s3;
	numCurrentStage = 0;
}
inline void CDT:: setBlockStageFunctions(STAGE s1, STAGE s2) {
	blockStages[0] = s1;
	blockStages[1] = s2;
	numCurrentStage = 0;
}

inline void CDT:: nextStage() {
	releasePointers();
	//debug_cdt("CDT:: nextStage()=%d\n", numEtapa);
	stageGetLevel = GET_LEVEL_FUNCTIONS[blockStages[numCurrentStage]];
	(this->*START_STAGE_FUNCTIONS[blockStages[numCurrentStage++]])();
	//numEtapa++;  <-- asi no vale
}


void CDT:: setEndStage() {
	stageGetLevel = &CDT::endGetLevel;
}

//-------------------------------------------------------------------------------------------

void CDT:: startBlock_x10() { // standard speed data block
	debug_cdt("CDT:: startBlock_x10\n");
	file_nextWord(cdtFile, &block.pause);
	block.dataLen = file_nextWord(cdtFile);
	block.pilot_pulse = 2168;
	block.pilotTone = 3220;
	block.usedBits = 8;

	setBlockStageFunctions(PILOT, DATA, PAUSE, END);
	nextStage();
}


void CDT:: startBlock_x11() { // turbo speed data block
	debug_cdt("CDT:: startBlock_x11\n");
	file_nextWord(cdtFile, &block.pilot_pulse);
	file_nextWord(cdtFile, &block.sync1Pulse);
	file_nextWord(cdtFile, &block.sync2Pulse);
	file_nextWord(cdtFile, &block.zeroBitPulse);
	file_nextWord(cdtFile, &block.oneBitPulse);
	file_nextWord(cdtFile, &block.pilotTone);
	file_nextByte(cdtFile, &block.usedBits);
	file_nextWord(cdtFile, &block.pause);
	file_next3Bytes(cdtFile, &block.dataLen);

	debug_cdt("CDT:: B_11   pilot_p=%d  sync1_p=%d  sync2_p=%d  zero_p=%d  one_p=%d  pilot_t=%d  usedBits=%d  pause=%d  data=%ld\n",
		block.pilot_pulse, block.sync1Pulse, block.sync2Pulse, block.zeroBitPulse, block.oneBitPulse, block.pilotTone, block.usedBits, block.pause, block.dataLen);
	debug_cdt("CDT:: numero_bits=%d\n", (block.dataLen-1)*8 + block.usedBits );

	setBlockStageFunctions(PILOT, SYNC, DATA, PAUSE, END);
	nextStage();
}


void CDT:: startBlock_x12() { // pure tone
	file_nextWord(cdtFile, &block.pilot_pulse);
	file_nextWord(cdtFile, &block.pilotTone);
	debug_cdt("CDT:: startBlock_x12   pulse_length=%d  mum_pulses=%d\n", block.pilot_pulse, block.pilotTone);
	
	setBlockStageFunctions(PILOT, END);
	nextStage();
}


void CDT:: startBlock_x13() { // pulse sequence
	// como si fuera una etapa sync pero con una lista de pulsos mas grande
	file_nextByte(cdtFile, &block.pulses);
	debug_cdt("CDT:: startBlock_x13   pulses=%d\n", block.pulses);
	pulsesList = new u16[block.pulses];
	
	cdtFile.read(reinterpret_cast<char*>(pulsesList), block.pulses << 1); // leer toda la lista de golpe

	//pulsesListEnd = pulsesList;
	//for (u8 i = 0; i < block.pulses; i++)  file_nextWord(fichero, pulsesListEnd++); 

	setBlockStageFunctions(PULSE_SECUENCE, END); 
	nextStage();
}


void CDT:: startBlock_x14() { // pure data block
	debug_cdt("CDT:: startBlock_x14\n");
	file_nextWord(cdtFile, &block.zeroBitPulse);
	file_nextWord(cdtFile, &block.oneBitPulse);
	file_nextByte(cdtFile, &block.usedBits);
	file_nextWord(cdtFile, &block.pause);
	file_next3Bytes(cdtFile, &block.dataLen);

	debug_cdt("CDT:: B_14   zero_p=%d  one_p=%d  usedBits=%d  pause=%d  data=%ld\n",
		block.zeroBitPulse, block.oneBitPulse, block.usedBits, block.pause, block.dataLen);
	
	setBlockStageFunctions(DATA, PAUSE, END);
	nextStage();
}


void CDT:: startBlock_x15() { // direct recording
	debug_cdt("CDT:: startBlock_x15\n");
	file_nextWord(cdtFile, &block.statesPerSample);
	file_nextWord(cdtFile, &block.pause);
	file_nextByte(cdtFile, &block.usedBits);
	file_next3Bytes(cdtFile, &block.dataLen);

	setBlockStageFunctions(SAMPLE_DATA, PAUSE, END);
	nextStage();
}


void CDT:: startBlock_x18() {
	debug_cdt("CDT:: startBlock_x18\n");
	file_nextDWord(cdtFile, &block.dataLen);
	file_nextWord(cdtFile, &block.pause);
	file_next3Bytes(cdtFile, &block.samplingRate);
	BYTE compressionType = file_nextByte(cdtFile);
	file_nextDWord(cdtFile, &block.totalPulses);  // ignoro de momento el numero de pulsos

	debug_cdt("CDT:: B_18   block_data_len=%ld  block_pause=%ld  sampling_rate=%ld  comp=%d  num_ciclos=%ld\n", 
			block.dataLen, block.pause, block.samplingRate, compressionType, block.totalPulses);

	flipLevel();
	debug_cdt("CDT:: file_pos=%05X\n", (u32) cdtFile.tellg());

	if (compressionType == RLE) {
		rleIn = new RLEInputStream(cdtFile, block.dataLen - 10); //, level);
	}
	else if (compressionType == ZRLE) {
		rleIn = new ZRLEInputStream(cdtFile, block.dataLen - 10);
	}
	else {
		debug_cdt("CDT:: B_18   compression desconocida = %02X\n", compressionType);
		file_consumeBytes(cdtFile, block.dataLen-10);	
	}

	setBlockStageFunctions(CSW, PAUSE, END);
	nextStage();
}


void CDT:: startBlock_x20() { // pause or stop
	debug_cdt("CDT:: startBlock_x20\n");
	file_nextWord(cdtFile, &block.pause);

	setBlockStageFunctions(PAUSE, END, END, END, END);
	nextStage();
}

void CDT:: startBlock_x21() { // group start
	BYTE len;
	file_nextByte(cdtFile, &len);
	file_consumeBytes(cdtFile, len);
	nextBlock() ;
}

void CDT:: startBlock_x23() { file_nextByte(cdtFile); nextBlock() ;}

void CDT:: startBlock_x24() { file_nextByte(cdtFile); nextBlock() ;}

void CDT:: startBlock_x26() { // group start
	u16 len;
	file_nextWord(cdtFile, &len);
	file_consumeBytes(cdtFile, len);
	nextBlock();
}

void CDT:: startBlock_x28() {
	u16 len;
	file_nextWord(cdtFile, &len);
	file_consumeBytes(cdtFile, len-2);
	nextBlock();
}

void CDT:: startBlock_x2a() {
	file_nextWord(cdtFile);
	nextBlock();
}

void CDT:: startBlock_x2b() {
	file_consumeBytes(cdtFile, 5);
	nextBlock();
}

void CDT:: startBlock_x30() {
	debug_cdt("CDT:: startBlock_x30\n");
	BYTE len;
	file_nextByte(cdtFile, &len);
	file_consumeBytes(cdtFile, len);
	nextBlock();
}

void CDT:: startBlock_x31() {
	BYTE len;
	file_nextByte(cdtFile, &len);
	file_nextByte(cdtFile, &len);
	file_consumeBytes(cdtFile, len);
	nextBlock();
}

void CDT:: startBlock_x32() {
	u16 len;
	file_nextWord(cdtFile, &len);
	file_consumeBytes(cdtFile, len-2);
	nextBlock();
}

void CDT:: startBlock_x33() {
	BYTE len;
	file_nextByte(cdtFile, &len);
	//debug_cdt("CDT:: B_13 len=%d\n", len);
	file_consumeBytes(cdtFile, len*3);
	nextBlock();
}

void CDT:: startBlock_x35() {
	file_consumeBytes(cdtFile, 10);
	u16 len;
	file_nextWord(cdtFile, &len);
	file_consumeBytes(cdtFile, len);
	nextBlock();
}

void CDT:: startBlock_x5a() {
	file_consumeBytes(cdtFile, 1);
	nextBlock();
}

