/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  User options with functions keys implementation                           |
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


//https://wiki.libsdl.org/SDL2/SDL_Scancode
// https://www.cpcwiki.eu/index.php/Programming:Keyboard_scanning
// https://www.cpcwiki.eu/index.php/File:Spanish_6128_Keyboard_Robcfg.jpg
// https://github.com/libsdl-org/SDL/blob/SDL2/include/SDL_keycode.h


#include <string>
#include <string_view>
#include <fstream>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <SDL2/SDL_keycode.h>
#include "cpc.h"
#include "memory.h"
#include "cpc_keyboard.h"
#include "util.h"
#include "monitor.h"
#include "target.h"
#include "directories.h"
#include "osd/osd.h"
#include "osd/osd_window.h"
#include "osd/osd_select_file.h"
#include "osd/osd_upper_roms.h"
#include "osd/osd_pokes.h"


const std::string DISK_LETTER[2] = {"A", "B"};


std::unordered_map<u16, u16> HASH_KEYBOARD = {
	// fila 1
	{SDL_SCANCODE_GRAVE, CPC_ESC},
	{SDL_SCANCODE_1, CPC_1},
	{SDL_SCANCODE_2, CPC_2},
	{SDL_SCANCODE_3, CPC_3},
	{SDL_SCANCODE_4, CPC_4},
	{SDL_SCANCODE_5, CPC_5},
	{SDL_SCANCODE_6, CPC_6},
	{SDL_SCANCODE_7, CPC_7},
	{SDL_SCANCODE_8, CPC_8},
	{SDL_SCANCODE_9, CPC_9},
	{SDL_SCANCODE_0, CPC_0},
	{SDL_SCANCODE_MINUS, CPC_MENOS},
	{SDL_SCANCODE_EQUALS, CPC_CLR},
	{SDL_SCANCODE_BACKSPACE, CPC_BORR},
	// fila 2
	{SDL_SCANCODE_TAB, CPC_TAB},
	{SDL_SCANCODE_Q, CPC_Q},
	{SDL_SCANCODE_W, CPC_W},
	{SDL_SCANCODE_E, CPC_E},
	{SDL_SCANCODE_R, CPC_R},
	{SDL_SCANCODE_T, CPC_T},
	{SDL_SCANCODE_Y, CPC_Y},
	{SDL_SCANCODE_U, CPC_U},
	{SDL_SCANCODE_I, CPC_I},
	{SDL_SCANCODE_O, CPC_O},
	{SDL_SCANCODE_P, CPC_P},
	{SDL_SCANCODE_LEFTBRACKET, CPC_ARROBA},
	{SDL_SCANCODE_RIGHTBRACKET, CPC_ASTERISCO},
	{SDL_SCANCODE_RETURN, CPC_RETURN},
	{SDL_SCANCODE_DELETE, CPC_PESETAS},
	// fila 3
	{SDL_SCANCODE_CAPSLOCK, CPC_FIJA_MAYS},
	{SDL_SCANCODE_A, CPC_A},
	{SDL_SCANCODE_S, CPC_S},
	{SDL_SCANCODE_D, CPC_D},
	{SDL_SCANCODE_F, CPC_F},
	{SDL_SCANCODE_G, CPC_G},
	{SDL_SCANCODE_H, CPC_H},
	{SDL_SCANCODE_J, CPC_J},
	{SDL_SCANCODE_K, CPC_K},
	{SDL_SCANCODE_L, CPC_L},
	{SDL_SCANCODE_SEMICOLON, CPC_ENYE},
	{SDL_SCANCODE_APOSTROPHE, CPC_DOS_PUNTOS},
	{SDL_SCANCODE_BACKSLASH, CPC_MAS},
	// fila 4
	{SDL_SCANCODE_LSHIFT, CPC_MAYUSCULAS},
	{SDL_SCANCODE_APPLICATION, CPC_BARRA_INV},  // en el cpc esta tecla esta al lado del shift derecha
	{SDL_SCANCODE_Z, CPC_Z},
	{SDL_SCANCODE_X, CPC_X},
	{SDL_SCANCODE_C, CPC_C},
	{SDL_SCANCODE_V, CPC_V},
	{SDL_SCANCODE_B, CPC_B},
	{SDL_SCANCODE_N, CPC_N},
	{SDL_SCANCODE_M, CPC_M},
	{SDL_SCANCODE_COMMA, CPC_COMA},
	{SDL_SCANCODE_PERIOD, CPC_PUNTO},
	{SDL_SCANCODE_SLASH, CPC_BARRA},
	{SDL_SCANCODE_RSHIFT, CPC_MAYUSCULAS},
	// fila 5
	{SDL_SCANCODE_LCTRL, CPC_CONTROL},
	{SDL_SCANCODE_LALT, CPC_COPIA},
	{SDL_SCANCODE_SPACE, CPC_ESPACIO},
	{SDL_SCANCODE_RCTRL, CPC_INTRO},
	// cursor
	{SDL_SCANCODE_UP, CPC_CURSOR_ARRIBA},
	{SDL_SCANCODE_DOWN, CPC_CURSOR_ABAJO},
	{SDL_SCANCODE_LEFT, CPC_CURSOR_IZQ},
	{SDL_SCANCODE_RIGHT, CPC_CURSOR_DERECHA},
	// teclado numerico
	{SDL_SCANCODE_KP_0, CPC_F0},
	{SDL_SCANCODE_KP_1, CPC_F1},
	{SDL_SCANCODE_KP_2, CPC_F2},
	{SDL_SCANCODE_KP_3, CPC_F3},
	{SDL_SCANCODE_KP_4, CPC_F4},
	{SDL_SCANCODE_KP_5, CPC_F5},
	{SDL_SCANCODE_KP_6, CPC_F6},
	{SDL_SCANCODE_KP_7, CPC_F7},
	{SDL_SCANCODE_KP_8, CPC_F8},
	{SDL_SCANCODE_KP_9, CPC_F9},
	{SDL_SCANCODE_KP_PERIOD, CPC_TN_PUNTO}
};


constexpr u8 JOYSTICK_QAOP = 0;
constexpr u8 JOYSTICK_CURSORS = 1;
constexpr u8 JOYSTICK_KEYPAD = 2;
constexpr u8 NO_JOYSTICK = 3;


constexpr u16 JOYSTICK_SET_QAOP[6] =  {
	SDL_SCANCODE_Q, SDL_SCANCODE_A, SDL_SCANCODE_O, SDL_SCANCODE_P, 
	SDL_SCANCODE_SPACE, SDL_SCANCODE_LALT
};
constexpr u16 JOYSTICK_SET_CURSORS[6] = {
	SDL_SCANCODE_UP, SDL_SCANCODE_DOWN, SDL_SCANCODE_LEFT, SDL_SCANCODE_RIGHT, 
	SDL_SCANCODE_SPACE, SDL_SCANCODE_LALT
};
constexpr u16 JOYSTICK_SET_KEYPAD[6] = {
	SDL_SCANCODE_KP_8, SDL_SCANCODE_KP_2, SDL_SCANCODE_KP_4, SDL_SCANCODE_KP_6, 
	SDL_SCANCODE_KP_0, SDL_SCANCODE_KP_PERIOD
};

const u16* JOYSTICK_SETS[3] = { JOYSTICK_SET_QAOP, JOYSTICK_SET_CURSORS, JOYSTICK_SET_KEYPAD };

const std::string_view JOYSTICK_SET_NAMES[] = {
	"Joystick: QAOP + SPACE + COPY", 
	"Joystick: Cursor keys + SPACE + COPY", 
	"Joystick: Keypad + 0 + .",
	"No joystick"
};



#define KEY_CASES(X) case SDL_SCANCODE_##X: \
	if (altgr) { X##_altgr(); releaseLeftControlKey(); } \
	else X##_normal(); \
	break;


/* los menus si se ponen con altgr+Fx deja el efecto de tener la tecla control del cpc pulsada 
   por eso utilizo releaseLeftControlKey
*/

#ifdef TARGET_PC
// asociacion tecla con funcion a ejecutar
#define F1_normal	showHelp
#define F1_altgr 	showCopyright
#define F2_normal 	insertEjectDiskA
#define F2_altgr 	flipAdiskProtection
#define F3_normal 	insertEjectDiskB
#define F3_altgr 	flipBdiskProtection
#define F4_normal 	insertPokes
#define F4_altgr 	quitEmulator
#define F5_normal 	insertEjectCassette
#define F5_altgr 	flipCassetteProtection
#define F6_normal	playCassette
#define F6_altgr	recordCassette
#define F7_normal	flipPauseCassette
#define F7_altgr	stopCassette
#define F8_normal	loadSnapshot
#define F8_altgr	saveSnapshot
#define F9_normal	loadCPCsettings
#define F9_altgr	saveCPCsettings
#define F10_normal	saveScreen
#define F10_altgr	flipScanlines
#define F11_normal	changePalette
#define F11_altgr	changePaletteVariant
#define F12_normal	resetCPC
#define F12_altgr	select_cpc_ram_roms
#define BLOQ_normal	showLastPrinterJob
#define BLOQ_altgr	showPrinterJobs
#define PAUSE_normal	playPauseEmulation
#define PAUSE_altgr		changeEmulationSpeed
#define HOME_normal	rewindCassette
#define HOME_altgr		rewindCassetteOneTrack
#define END_normal	windCassette
#define END_altgr	windCassetteOneTrack
#define PAGEUP_normal	upVolume
#define PAGEUP_altgr	flipFastLoads
#define PAGEDOWN_normal	downVolume
#define PAGEDOWN_altgr	changeJoystickKeys
#endif

#ifdef TARGET_LILYGO
// asociacion tecla con funcion a ejecutar
#define F1_normal	mainMenu
#define F1_altgr 	showHelp
#define F2_normal 	insertEjectDiskA
#define F2_altgr 	flipAdiskProtection
#define F3_normal 	insertEjectDiskB
#define F3_altgr 	flipBdiskProtection
#define F4_normal 	insertEjectCassette
#define F4_altgr 	flipCassetteProtection
#define F5_normal	playCassette
#define F5_altgr	recordCassette
#define F6_normal	flipPauseCassette
#define F6_altgr	stopCassette
#define F7_normal	loadSnapshot
#define F7_altgr	saveSnapshot
#define F8_normal	insertPokes
#define F8_altgr	doNothing
#define F9_normal	changeCPC
#define F9_altgr	saveScreen
#define F10_normal	doNothing
#define F10_altgr	doNothing
#define F11_normal	changeColourBalance
#define F11_altgr	flipScanlines
#define F12_normal	resetCPC
#define F12_altgr	resetESP32
#define PAUSE_normal	playPauseEmulation
#define PAUSE_altgr		changeEmulationSpeed
#define BEGIN_normal	rewindCassette
#define BEGIN_altgr		rewindCassetteOneTrack
#define END_normal	windCassette
#define END_altgr	windCassetteOneTrack
#define REPAG_normal	upVolume
#define REPAG_altgr		doNothing
#define AVPAG_altgr		downVolume
#define AVPAG_altgr		doNothing
#endif


extern bool bp_pintar;   // para activar o desactivar trazas

SDL_Event event;
SDL_Keycode keycode;
SDL_Scancode scancode;
SDL_Keymod keymodStates;

bool altgr = false;
bool GLOBAL_PAUSE = false;
bool GLOBAL_TURBO = false;

#ifdef TARGET_PC
bool GLOBAL_QUIT = false;
#endif

std::filesystem::path disksDir       = std::string {DISKS_DIR};
std::filesystem::path tapesDir       = std::string {TAPES_DIR};
std::filesystem::path snapshotsDir   = std::string {SNAPSHOTS_DIR};
std::filesystem::path machinesDir    = std::string {MACHINES_DIR};
std::filesystem::path screenshotsDir = std::string {SCREENSHOTS_DIR};

CPC* eu_cpc;
OSD* eu_osd;
u8 joystick = NO_JOYSTICK;



void releaseLeftControlKey() {
    SDL_Event event;
    event.type = SDL_KEYDOWN;
    event.key.keysym.sym = SDLK_LCTRL; // Simular la tecla Left Control
    //event.key.state = SDL_PRESSED;
    event.key.repeat = 0;
    event.key.keysym.mod = KMOD_LCTRL; // Indicar que el modificador es Left Control

    // Enviar el evento
    //SDL_PushEvent(&event);
    // Simular un breve retraso para ver el resultado
    //SDL_Delay(50);
    // Crear un evento de liberación de Left Control
    event.type = SDL_KEYUP;
    event.key.state = SDL_RELEASED;
    // Enviar el evento
    SDL_PushEvent(&event);
}




bool eu_getPause() { return GLOBAL_PAUSE; }
bool eu_getTurboSpeed() { return GLOBAL_TURBO; }

#ifdef TARGET_PC
bool eu_getQuit() { return GLOBAL_QUIT; }
#endif

void flipPause() { GLOBAL_PAUSE = !GLOBAL_PAUSE; }
void flipTurbo() { GLOBAL_TURBO = !GLOBAL_TURBO; }


void eu_setObjects(CPC* cpc, OSD* osd) {
	eu_cpc = cpc;
	eu_osd = osd;
}


void setEstadoTecla(u32 tecla, bool estado, Keyboard* teclado) {
    //printf("EstadoTecla.  [%s,%d] %d \n", SDL_GetKeyName(tec24la), tecla, estado);
    auto iteratorTeclas = HASH_KEYBOARD.find(tecla);
    if (iteratorTeclas != HASH_KEYBOARD.end()) {
		//printf("USER:: tecla=%ld\n", tecla);
        teclado->setKeyStatus(iteratorTeclas->second, estado);
    }
}



void doNothing() {}

//*****************************************************************************************

void showHelp() {
	//eu_cpc->getMonitor()->printHelp();
	eu_osd->showHelp();
	eu_osd->waitESC();
}

void showCopyright() {
	eu_osd->showCredits();
	eu_osd->waitESC();
}


void insertEjectDisk(u8 nDisk) {
	if (eu_cpc->hasFDC()) {
		DiscDrive* ud = eu_cpc->getDiscDrive(nDisk);
		if (ud->hasDSK()) {
			ud->eject();
			eu_osd->setPopupMessage("Disk " + DISK_LETTER[nDisk] + " ejected");
		}
		else {
			OSD_FileSelection fs {"Select a disk image", eu_osd, disksDir, DISK_EXT};
			fs.setCreateOption(true);
			fs.show();
			
			if (fs.getExitOk()) {
				std::filesystem::path file = fs.getSelectedFile();
				std::string f {file.string()};
				bool p = ud->insert(f);
				if (p) {
					eu_osd->setPopupMessage("Disk inserted in " + DISK_LETTER[nDisk]);
					disksDir = file.parent_path();
				}
				else eu_osd->setPopupMessage("Not valid file");
			}
		}
	}
}

void flipDiskProtection(u8 nDisk) {
	if (eu_cpc->hasFDC()) {
		bool p = eu_cpc->getDiscDrive(nDisk)->flipWriteProtection();
		if (p) eu_osd->setPopupMessage("Disk " + DISK_LETTER[nDisk] + " protected");
		else   eu_osd->setPopupMessage("Disk " + DISK_LETTER[nDisk] + " unprotected");
	}
	else eu_osd->setPopupMessage("No disk on drive " + DISK_LETTER[nDisk]);
}

void insertEjectDiskA() { insertEjectDisk(0); }

void insertEjectDiskB() { insertEjectDisk(1); }

void flipAdiskProtection() {	flipDiskProtection(0); }

void flipBdiskProtection() {	flipDiskProtection(1);	}

void insertEjectCassette() {
	if (eu_cpc->getTape()->hasCDT()) {
		eu_cpc->getTape()->eject();
		eu_osd->setPopupMessage("Cassette ejected");
	}
	else {
		OSD_FileSelection fs {"Select a tape image", eu_osd, tapesDir, TAPE_EXT};
		fs.setCreateOption(true); //w.setCreateOption(false);
		fs.show();
		
		if (fs.getExitOk()) {
			std::filesystem::path file = fs.getSelectedFile();
			std::string f { file.string() };
			bool p = eu_cpc->getTape()->insert(f);
			if (p) {
				eu_osd->setPopupMessage("Cassette inserted");
				tapesDir = file.parent_path();
			}
			else eu_osd->setPopupMessage("Not valid file");
		}
	}
}

void noCassetteMsg() {
	eu_osd->setPopupMessage("No cassette inserted");
}

void flipCassetteProtection() {
	if (eu_cpc->getTape()->hasCDT()) {
		bool p = eu_cpc->getTape()->flipWriteProtection();
		if (p) eu_osd->setPopupMessage("Cassette protected");
		else   eu_osd->setPopupMessage("Cassette unprotected");
	}
	else noCassetteMsg();
}

void playCassette() {
	if (eu_cpc->getTape()->hasCDT()) {
		eu_cpc->getTape()->play();
		eu_osd->setPopupMessage("Play pressed");
	}
	else noCassetteMsg();
}

void recordCassette() {
	Tape* tape = eu_cpc->getTape();
	if (tape->hasCDT()) {
		if (tape->getWriteProtection()) {
			eu_osd->setPopupMessage("Cassette protected");
		}
		else {
			eu_cpc->getTape()->record();
			eu_osd->setPopupMessage("Record pressed");
		}
	}
	else noCassetteMsg();
}

void flipPauseCassette() {
	if (eu_cpc->getTape()->hasCDT()) {
		bool p = eu_cpc->getTape()->pause();
		if (p) eu_osd->setPopupMessage("Pause pressed");
		else   eu_osd->setPopupMessage("Pause not pressed");
	}
	else noCassetteMsg();
}

void stopCassette() {
	if (eu_cpc->getTape()->hasCDT()) {
		eu_cpc->getTape()->stop();
		eu_osd->setPopupMessage("Cassette stopped");
	} 
	else noCassetteMsg();
}

void loadSnapshot() {
	OSD_FileSelection fs {"Select a snapshot file", eu_osd, snapshotsDir, SNAPSHOT_EXT};
	fs.show();

	if (fs.getExitOk()) {
		std::filesystem::path file = fs.getSelectedFile();
		bool p = eu_cpc->loadSna(file.string());
		if (p) {
			eu_osd->setPopupMessage("Snapshot loaded");
			snapshotsDir = file.parent_path();
		}
		else eu_osd->setPopupMessage("Not valid file");
	}
}

void saveSnapshot() {
	//std::string ext { SNAPSHOT_EXT };
	//std::string fichero {screenshotsDir.string() + strDate() + ext };
	//eu_cpc->saveSna(fichero);
	//eu_osd->setPopupMessage("Snapshot saved");

	OSD_SaveFile w {"Select or create a file", eu_osd, snapshotsDir, SNAPSHOT_EXT};
	w.show();

	if (w.getExitOk()) {
		std::filesystem::path file = w.getNewFile();
		debug_osd("OSD:: new_file [%s]\n", file.string().c_str());
		bool p = eu_cpc->saveSna(file.string());
		if (p) {
			eu_osd->setPopupMessage("Snapshot saved");
			machinesDir = file.parent_path();
		}
		else eu_osd->setPopupMessage("Snapshot not saved");
	}
}

void saveScreen() {
	//std::string ext { SCREENSHOT_EXT };
	//std::string file {screenshotsDir.string() + strDate() + ext};
	//std::filesystem::path path {SCREENSHOTS_DIR};
	//path = path / file;
	//file = path.string();
	//
	//debug("[%s] \n", file.c_str());
	//if (eu_cpc->getMonitor()->saveScreen(file)) {
	//	eu_osd->setPopupMessage("Screenshot saved");
	//}
	//else {
	//	eu_osd->setPopupMessage("Error saving screenshot");
	//}

	OSD_SaveFile w {"Select or create a file", eu_osd, screenshotsDir, SCREENSHOT_EXT};
	eu_osd->push(w);
	w.show();
	eu_osd->pop();
	if (w.getExitOk()) {
		std::filesystem::path file = w.getNewFile();
		debug_osd("OSD:: new_file [%s]\n", file.string().c_str());
		bool p = eu_cpc->getMonitor()->saveScreen(file.string());
		if (p) {
			eu_osd->setPopupMessage("Screenshot saved");
			machinesDir = file.parent_path();
		}
		else eu_osd->setPopupMessage("Screenshot not saved");
	}
}

void changePalette() {
	Monitor* mon = eu_cpc->getMonitor();
	mon->changePalette();
	u8 palette = mon->getPalette();
	u8 variant =  mon->getPaletteVariant();
	eu_osd->setPopupMessage("Palette " + mon->getPaletteName() + "-" + std::to_string(mon->getPaletteVariant()+1));
}

void changePaletteVariant() {
	Monitor* mon = eu_cpc->getMonitor();
	mon->changePaletteVariant();
	u8 palette = mon->getPalette();
	u8 variant =  mon->getPaletteVariant();
	eu_osd->setPopupMessage("Palette " + mon->getPaletteName() + "-" + std::to_string(mon->getPaletteVariant()+1));
}

void flipScanlines() {
	eu_cpc->getMonitor()->flipScanlines();
}

void resetCPC() {
	eu_cpc->reset(); 
	eu_osd->setPopupMessage("Reset");
}

#ifdef TARGET_LILYGO
void resetESP32() {
	eu_osd->setPopupMessage("Reset ESP32");
}
#endif

#ifdef TARGET_PC
void quitEmulator() {
	printf("!!! SALIR !!!\n");
	GLOBAL_QUIT = true;
}
#endif


void changeCPC() {
	OSD_SimpleSelect w {"Select a CPC model", eu_osd};
	
	for (u8 i = 0; i < CPC_MODELS_COUNT; i++) {
		w.addOption(i, CPC::STANDARD_MODELS[i].description);
	}
	std::string currentModel = "Current: " + CPC::CPC_BASE[eu_cpc->getType()];
	{
		std::string lowerRom = eu_cpc->getMemory()->getLowerRomFile();
		toUpperCase(lowerRom);

		if      (lowerRom.find("_UK.ROM")) currentModel.append(" English");
		else if (lowerRom.find("_ES.ROM")) currentModel.append(" Spanish");
		else if (lowerRom.find("_DK.ROM")) currentModel.append(" Danish");
		else if (lowerRom.find("_FR.ROM")) currentModel.append(" French");
	}
	w.setStatus(currentModel);
	eu_osd->push(w);
	w.show();

	if (w.getExitOk()) {
		eu_cpc->setStandardModel( w.getValueSelected() );
		eu_cpc->reset();
	}
	eu_osd->pop();
}


void changeRAMsize() {
	OSD_SimpleSelect w {"Select RAM size", eu_osd};

	if (eu_cpc->getType() != 2)  w.addOption(1, " 128 KB");	
	w.addOption( 3, " 256 KB");
	w.addOption( 5, " 384 KB");
	w.addOption( 7, " 512 KB");
	w.addOption(11, " 768 KB");
	w.addOption(15, "1024 KB");
	w.addOption(23, "1536 KB");
	w.addOption(31, "2048 KB");

	Memory* mem = eu_cpc->getMemory();
	w.setStatus("Current: " + std::to_string(mem->getRamSize()) + " KB");
	eu_osd->push(w);
	w.show();

	if (w.getExitOk()) {
		u8 paginasNuevas = w.getValueSelected();
		u8 paginasAhora = mem->getExtendedRamPages();
		
		if (paginasNuevas > paginasAhora) { 
			// anadir
			debug_osd("OSD:: anadir mem %d\n", paginasNuevas-paginasAhora);
			mem->addExtendedRamPages(paginasNuevas-paginasAhora);
			eu_cpc->reset();
		}
		else if (paginasAhora > paginasNuevas) {
			// quitar
			debug_osd("OSD:: quitar mem %d\n", paginasAhora-paginasNuevas);
			mem->removeExtendedRamPages(paginasAhora-paginasNuevas);
			eu_cpc->reset();
		}
	}
	eu_osd->pop();
}


void changeLowerRom() {
	OSD_FileSelection w {"Select a Lower ROM image", eu_osd, ROMS_DIR, ROMS_EXT};
	w.setHeight(true, 20, true);
	w.setWidth(44);
	eu_osd->push(w);
	w.show();

	if (w.getExitOk()) {
		bool cambio = eu_cpc->getMemory()->setLowerRom(w.getSelectedFile().string());
		if (cambio) eu_cpc->reset();
		else        eu_osd->setPopupMessage("Not valid file");
	}
	eu_osd->pop();
}


void changeUpperRoms() {
	Memory* mem = eu_cpc->getMemory();
	OSD_UpperRomSelection rs {"Change Upper ROMs", eu_osd};

	std::string romFile;
	bool loaded;

	for (u8 i = 0; i < Memory::MAX_ROMS; i++) {
		mem->getUpperRomFile(i, romFile, &loaded);
		rs.setUpperRom(i, romFile, loaded);
	}

	rs.setHeight(false, 16, true);
	rs.setWidth(50);
	eu_osd->push(rs);
	rs.show();

	if (rs.getExitOk()) {
		for (u8 i = 0; i < Memory::MAX_ROMS; i++)  mem->setUpperRom(i, rs.getUpperRom(i));
		eu_cpc->reset();
	}
	eu_osd->pop();
}


void select_cpc_ram_roms() {
	OSD_SimpleSelect w {"Select to change", eu_osd};
	w.addOption(0, "CPC version");
	w.addOption(1, "RAM size");
	w.addOption(2, "Lower ROM");
	w.addOption(3, "Upper ROMs");
	//w.pre_show();
	eu_osd->push(w); // pantalla limpia

	bool finish = false;
	while (!finish) {
		w.show();
		if (w.getExitOk()) {
			u8 n = w.getValueSelected();
			switch (n) {
				case 0: eu_osd->backStack(); changeCPC();       eu_osd->updateUI(); break; // pantalla limpia
				case 1: eu_osd->backStack(); changeRAMsize();   eu_osd->updateUI(); break;
				case 2: eu_osd->backStack(); changeLowerRom();  eu_osd->updateUI(); break;
				case 3: eu_osd->backStack(); changeUpperRoms(); eu_osd->updateUI(); break;
			}
		}
		else finish = true;
	}
	eu_osd->pop(); // pantalla limpia
}

void loadCPCsettings() {
	OSD_FileSelection fs {"Select a CPC settings file", eu_osd, machinesDir, MACHINES_EXT};
	fs.show();

	if (fs.getExitOk()) {
		std::filesystem::path file = fs.getSelectedFile();
		bool p = eu_cpc->loadConfig(file.string());
		if (p) {
			eu_osd->setPopupMessage("Settings loaded");
			machinesDir = file.parent_path();
		}
		else eu_osd->setPopupMessage("Not valid file");
	}
}

void saveCPCsettings() {
	OSD_SaveFile w {"Select or create a file", eu_osd, machinesDir, MACHINES_EXT};
	w.show();

	if (w.getExitOk()) {
		std::filesystem::path file = w.getNewFile();
		debug_osd("OSD:: new_file [%s]\n", file.string().c_str());
		bool p = eu_cpc->saveConfig(file.string());
		if (p) {
			eu_osd->setPopupMessage("Settings saved");
			machinesDir = file.parent_path();
		}
		else eu_osd->setPopupMessage("Settings not saved");
	}
}


void playPauseEmulation() {
	GLOBAL_PAUSE = !GLOBAL_PAUSE; 
	if (GLOBAL_PAUSE) eu_osd->setPopupMessage("Emulation paused");
	else       eu_osd->setPopupMessage("Emulation running");
}

void changeEmulationSpeed() {
	GLOBAL_TURBO = !GLOBAL_TURBO; 
	if (GLOBAL_TURBO) eu_osd->setPopupMessage("Turbo speed");
	else       eu_osd->setPopupMessage("Normal speed");

}

void insertPokes() {
	OSD_PokeSelection w {"Pokes", eu_osd};
	w.show();

	if (w.getExitOk()) {
		Memory* mem = eu_cpc->getMemory();
		DIR address;
		BYTE value;

		for (u8 i = 0; i < NUM_POKES; i++) {
			w.getPoke(i, &address, &value);
			debug_osd("POKE:: %04X %02X\n", address, value);
			if (address != 0)  mem->writeByte(address, value);
		}
	}
}

void popupTapeTrack(Tape* tape) {
	std::string track = std::to_string( tape->getCurrentTrack()+1 );
	std::string tracks = std::to_string( tape->getNumTracks() );
	std::string msg = "Cassette track " + track + "/" + tracks;
	eu_osd->setPopupMessage(msg);
}

void rewindCassette() { 
	Tape* tape = eu_cpc->getTape();
	
	if (tape->hasCDT()) {
		tape->rewind();
		popupTapeTrack(tape);
	}
	else noCassetteMsg();
}

void rewindCassetteOneTrack() { 
	Tape* tape = eu_cpc->getTape();
	
	if (tape->hasCDT()) {
		tape->rewind1();
		popupTapeTrack(tape);
	}
	else noCassetteMsg();
}

void windCassette() { 
	Tape* tape = eu_cpc->getTape();
	
	if (tape->hasCDT()) {
		tape->wind();
		popupTapeTrack(tape);
	}
	else noCassetteMsg();
}

void windCassetteOneTrack() { 
	Tape* tape = eu_cpc->getTape();
	
	if (tape->hasCDT()) {
		tape->wind1();
		popupTapeTrack(tape);
	}
	else noCassetteMsg();
}

void flipFastLoads() {
	FDC::flipFastMode();
	eu_osd->setPopupMessage( CDT::flipSkipPauses() ? "Faster loads on" : "Normal loads");
}

void showLastPrinterJob() { eu_osd->setPopupMessage("showLastPrinterJobs"); }

void showPrinterJobs() { eu_osd->setPopupMessage("showPrinterJobs"); }

void upVolume() { eu_osd->setPopupMessage("Volume +"); }

void downVolume() { eu_osd->setPopupMessage("Volume -"); }

void changeColourBalance() { eu_osd->setPopupMessage("Change colour balance"); }


void changeJoystickKeys() {
	debug("EU: changeJoystickKeys\n");
	// restauramos
	HASH_KEYBOARD[SDL_SCANCODE_Q]         = CPC_Q;
	HASH_KEYBOARD[SDL_SCANCODE_A]         = CPC_A;
	HASH_KEYBOARD[SDL_SCANCODE_O]         = CPC_O;
	HASH_KEYBOARD[SDL_SCANCODE_P]         = CPC_P;
	HASH_KEYBOARD[SDL_SCANCODE_SPACE]     = CPC_ESPACIO;
	HASH_KEYBOARD[SDL_SCANCODE_LALT]      = CPC_COPIA;
	HASH_KEYBOARD[SDL_SCANCODE_UP]        = CPC_CURSOR_ARRIBA;
	HASH_KEYBOARD[SDL_SCANCODE_DOWN]      = CPC_CURSOR_ABAJO;
	HASH_KEYBOARD[SDL_SCANCODE_LEFT]      = CPC_CURSOR_IZQ;
	HASH_KEYBOARD[SDL_SCANCODE_RIGHT]     = CPC_CURSOR_DERECHA;
	HASH_KEYBOARD[SDL_SCANCODE_KP_8]      = CPC_F8;
	HASH_KEYBOARD[SDL_SCANCODE_KP_2]      = CPC_F2;
	HASH_KEYBOARD[SDL_SCANCODE_KP_4]      = CPC_F4;
	HASH_KEYBOARD[SDL_SCANCODE_KP_6]      = CPC_F6;
	HASH_KEYBOARD[SDL_SCANCODE_KP_0]      = CPC_F0;
	HASH_KEYBOARD[SDL_SCANCODE_KP_PERIOD] = CPC_TN_PUNTO;
	//
	if (++joystick > NO_JOYSTICK)  joystick = 0;
	
	eu_osd->setPopupMessage( std::string{JOYSTICK_SET_NAMES[joystick]} );

	if (joystick != NO_JOYSTICK) {
		const u16* set = JOYSTICK_SETS[joystick];

		HASH_KEYBOARD[set[0]] = CPC_JOY1_ARRIBA;
		HASH_KEYBOARD[set[1]] = CPC_JOY1_ABAJO;
		HASH_KEYBOARD[set[2]] = CPC_JOY1_IZQ;
		HASH_KEYBOARD[set[3]] = CPC_JOY1_DER;
		HASH_KEYBOARD[set[4]] = CPC_JOY1_FIRE1;
		HASH_KEYBOARD[set[5]] = CPC_JOY1_FIRE2;
	}
}

//*****************************************************************************************


void eu_leerTeclado() {
	SDL_Keymod ms;
    
    while (SDL_PollEvent(&event) != 0) {
        keycode = event.key.keysym.sym;
		scancode = SDL_GetScancodeFromKey(keycode);
		keymodStates = SDL_GetModState(); // Obtener el estado de los modificadores
		altgr = (keymodStates & KMOD_RALT) != 0;
		//debug("<%d,%d> ", keymodStates, scancode);

        switch (event.type) {

            case SDL_KEYDOWN:
                switch (scancode) {
					KEY_CASES(F1)
					KEY_CASES(F2)
					KEY_CASES(F3)
					KEY_CASES(F4)
					KEY_CASES(F5)
					KEY_CASES(F6)
					KEY_CASES(F7)
					KEY_CASES(F8)
					KEY_CASES(F9)
					KEY_CASES(F10)
					KEY_CASES(F11)
					KEY_CASES(F12)
					KEY_CASES(PAUSE)
					KEY_CASES(PAGEUP)
					KEY_CASES(PAGEDOWN)
					KEY_CASES(HOME)
					KEY_CASES(END)
                }
                if (!GLOBAL_PAUSE) setEstadoTecla(scancode, true, eu_cpc->getKeyboard());
                break;

            case SDL_KEYUP:
                if (!GLOBAL_PAUSE) setEstadoTecla(scancode, false, eu_cpc->getKeyboard());
                break;

			#ifdef TARGET_PC
            case SDL_QUIT:  GLOBAL_QUIT = true; break; // X de la ventana
			#endif
        }
    }
}
