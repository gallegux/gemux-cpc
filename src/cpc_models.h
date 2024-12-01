#pragma once


#include <string>
#include "tipos.h"


constexpr u8 CPC_MODELS_COUNT = 9;
constexpr u8 CPC_BASE_COUNT = 3;
constexpr u8 DEFAULT_464 = 0;
constexpr u8 DEFAULT_664 = 4;
constexpr u8 DEFAULT_6128 = 5;


enum CPC_TYPE { CPC464 = 0, CPC664 = 1, CPC6128 = 2, UNKNOWN = 3 };



typedef struct {
	u8 cpcType;
	std::string description;
	std::string lowerRowFile;
	std::string upperRom0File;
	std::string upperRom7File;
	//bool fdc;
	//u8 extRamPages;
} CPC_MODEL;


