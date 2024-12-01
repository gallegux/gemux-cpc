/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Gemux-CPC launch implementation                                           |
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
#include <stdio.h>
#include <fstream>
#include <chrono>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "memory.h"
#include "z80.h"
#include "cpc.h"
#include "monitor.h"
#include "breakpoint.h"
#include "disc_drive.h"
#include "util.h"
#include "entrada_usuario.h"
#include "osd/osd.h"
#include "directories.h"
#include "properties.h"
#include "target.h"
#include "compilation_options.h"

/**
ARGUMENTOS:
[-464 | -664 | -6128 | -c <config> | -v <version> ] [-s <color|bw|green|orange>]
[-da <discoA>] [-db <discoB>] [-t <cinta>] 
[-p <programa>]
**/

#define NANOSEG(x) std::chrono::duration_cast<std::chrono::nanoseconds>(x)
/**
https://retrowiki.es/viewtopic.php?f=115&t=200038368

http://www.z80.info/z80info.htm
https://floooh.github.io/2021/12/06/z80-instruction-timing.html
**/

constexpr std::string_view WINDOW_X = "window.x";
constexpr std::string_view WINDOW_Y = "window.y";



bool crearVentana(SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** texture, int x, int y);

#ifdef TARGET_PC
std::string getArg(const std::string& flag, int argc, char* argv[]);
bool tratarArgumentosMaquina(int argc, char* argv[], CPC* cpc);
void tratarArgumentosPerifericos(int argc, char* argv[], CPC* cpc);
bool loadLastConfig(CPC* cpc);
void saveWindowConfig(SDL_Window* window);
void loadWindowConfig(int* x, int* y);
#endif

extern bool bp_pintar;
extern bool print_io;
extern bool log_instAlcanzada;
extern bool log_teclaPulsada;



int main(int argc, char* argv[])
{

    SDL_Window*   window;
    SDL_Renderer* renderer;
    SDL_Texture*  texture;
	uint32_t*     pixels = new uint32_t[MONITOR_WIDTH * MONITOR_HEIGHT];


	#ifdef TARGET_PC
    printf("GEMUX-CPC\n[-464 | -664 | -6128* | -v <version> | -c <config>] "
		"[-m <paginas64k>] "
        "[-s <color|bw|green|orange>] "
        "[-da <discoA>] [-db <discoB>] [-t <cinta>] "
        "[-p <programa>]\n");
	CPC::print_models();
	
	{
		int x = -1, y = -1;
		loadWindowConfig(&x, &y);
    	if (!crearVentana(&window, &renderer, &texture, x, y)) return 1;
		// anadir icono
		//SDL_Surface* icon = IMG_Load("icono.png");
		//SDL_SetWindowIcon(window, icon);
		//// Liberar el icono después de establecerlo
		//SDL_FreeSurface(icon);
	}
	#endif

	OSD osd {renderer, texture, pixels};
    CPC cpc;
    Keyboard* teclado = cpc.getKeyboard();
    Memory* mem = cpc.getMemory();
    Monitor* monitor = cpc.getMonitor();
    Z80* cpu = cpc.getCpu();
    CRTC* crtc = cpc.getCrtc();

	monitor->setSDLObjects(renderer, texture, pixels);
	monitor->setOSD(&osd);

    eu_setObjects(&cpc, &osd);



	#ifdef TARGET_PC
	{
		bool b = tratarArgumentosMaquina(argc, argv, &cpc); // 1 si los arg tienen configuracion de maquina
		if (!b) {
			b = loadLastConfig(&cpc); // 1 si se ha encontrado el fichero
			if (!b) cpc.set6128();
		}
		tratarArgumentosPerifericos(argc, argv, &cpc);
	}
	#endif

	#ifdef TARGET_LILYGO
	{
		bool b = loadLastConfig(&cpc);
		if (!b) cpc.set6128();
	}
	#endif


	// afijar
    SDL_Event event;
    u32 contadorCiclosTeclado = 0;
    std::chrono::nanoseconds tiempo {0};
    std::chrono::nanoseconds tiempoInstrucciones[8] = {
        std::chrono::nanoseconds {   0},
        std::chrono::nanoseconds { 800},
        std::chrono::nanoseconds {1800},
        std::chrono::nanoseconds {2800},
        std::chrono::nanoseconds {3800},
        std::chrono::nanoseconds {4800},
        std::chrono::nanoseconds {5800},
        std::chrono::nanoseconds {6800}
    };


	osd.setPopupMessage("Press F1 for HELP", 4000);
   
    while (!eu_getQuit()) {
        if (eu_getPause()) {
            eu_leerTeclado();
            continue;
        }
        else if (contadorCiclosTeclado > 3300) {
            eu_leerTeclado();
            contadorCiclosTeclado = 0;
        }

		//if (bp_pintar) {
		//	cpc.getCpu()->printInstructionASM();
		//	cpc.getCpu()->printEstado();
		//	cpc.getCrtc()->print();
		//	printf("\n");
		//}

        cpc.execute();

        contadorCiclosTeclado += cpu->getInstructionCycles();
        osd.update(cpu->getInstructionCycles());

        if (!eu_getTurboSpeed()) {
            auto now_high_res = std::chrono::high_resolution_clock::now();
            std::chrono::nanoseconds tpoLimite = tiempo + tiempoInstrucciones[cpu->getInstructionCycles()];
            while (std::chrono::duration_cast<std::chrono::nanoseconds>(now_high_res.time_since_epoch()) < tpoLimite) {
                now_high_res = std::chrono::high_resolution_clock::now();
            }
            tiempo = std::chrono::duration_cast<std::chrono::nanoseconds>(now_high_res.time_since_epoch());
        }
    }

	#ifdef TARGET_PC
	saveWindowConfig(window);
	#endif

	std::string lastCfgFile {LAST_CONFIGURATION_FILE};
	cpc.saveConfig(lastCfgFile);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
	debug("FIN\n");
     
    return 0;
}


//--------------------------------------------------------------------------------------------------´
//--------------------------------------------------------------------------------------------------´


bool crearVentana(SDL_Window** window, SDL_Renderer** renderer, SDL_Texture** texture, int x, int y) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        debug("Error : SDL failed to initialize. SDL Error: '%s'", SDL_GetError());
        return false;
    }
	if (x < 0 || y < 0)  x = y = SDL_WINDOWPOS_CENTERED;

    *window = SDL_CreateWindow("GEMUX CPC", x, y, MONITOR_WIDTH, MONITOR_HEIGHT, 0); //740,80
    if (!window) {
        debug("Error: failed to open window. SDL Error: '%s'", SDL_GetError());
        return false;
    }
    *renderer = SDL_CreateRenderer(*window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        debug("Error: failed to create renderer. SDL Error: '%s'", SDL_GetError());
        return false;
    }
    *texture = SDL_CreateTexture(*renderer, SDL_PIXELFORMAT_ARGB8888, 
                            SDL_TEXTUREACCESS_STATIC, MONITOR_WIDTH, MONITOR_HEIGHT);
	
	debug("Screen %dx%d\n", MONITOR_WIDTH, MONITOR_HEIGHT);
    return true;
}


#ifdef TARGET_PC

std::string getArg(const std::string& flag, int argc, char* argv[]) {
    std::string strValorArg;
    for (int i = 0; i < argc-1; i++) {
        strValorArg = argv[i];
        if (strValorArg == flag) {
            return std::string(argv[i+1]);
        }
    }
    return "";
}


bool tratarArgumentosMaquina(int argc, char* argv[], CPC* cpc) {
    std::string a;

    bool cpcSeleccionado = false;

    for (u8 na = 0; na < argc; ) {
        a = argv[na++];

        if (a == "-c" && !cpcSeleccionado) {	// fichero de configuracion
            a = argv[na++];
            printf("main:: fichero=%s\n", a.c_str());
            cpc->loadConfig(a);
            cpcSeleccionado = true;
        }
		else if (a == "-v" && !cpcSeleccionado) {	// version standard de cpc
			a = argv[na++];
			int v = DEFAULT_6128;
			if (toNumber(a, &v) && v >= 0 && v < CPC_MODELS_COUNT) {
				cpc->setStandardModel(v);
				cpcSeleccionado = true;
			}
		}
        else if (a == "-464" && !cpcSeleccionado) {	// 464 uk
            cpc->set464();
            cpcSeleccionado = true;
        }
        else if (a == "-664" && !cpcSeleccionado) {	// 664 uk
            cpc->set664();
            cpcSeleccionado = true;
        }
        else if (a == "-6128" && !cpcSeleccionado) {	// 6138 uk
            cpc->set6128();
            cpcSeleccionado = true;
            //cpc6128 = true;
        }
    }
    return cpcSeleccionado;
}


void tratarArgumentosPerifericos(int argc, char* argv[], CPC* cpc) {
    std::string a;

    for (u8 na = 0; na < argc; ) {
        a = argv[na++];

		if (a == "-sna") {
			a = argv[na++];
			cpc->loadSna(a);
		}
        else if (a == "-m") {	// memoria
            int n = 0;
            a = argv[na++];
            if (toNumber(a, &n)) cpc->getMemory()->addExtendedRamPages(n);
        }
        else if (a == "-da") {	// disco A
            a = argv[na++];
            printf("disco.0=%s\n", a.c_str());
            printf("has fdc %d    unidad a %d\n", cpc->hasFDC(), cpc->getDiscDrive(0) != nullptr);
            if (cpc->hasFDC()) cpc->getDiscDrive(0)->insert(a);
        }
        else if (a == "-db") {	// disco B
            a = argv[na++];
            printf("disco.1=%s\n", a.c_str());
            if (cpc->hasFDC()) cpc->getDiscDrive(1)->insert(a);
        }
        else if (a == "-t") {	// cinta
            a = argv[na++];
            printf("cinta=%s\n", a.c_str());
            cpc->getTape()->insert(a);
        }
        else if (a == "-s") {	// screen
            a = argv[na++];
            printf("monitor=%s\n", a.c_str());
            if (a == "bw") {
                cpc->getMonitor()->setPalette(GRAY_PALETTE);
            }
            else if (a == "green") {
                cpc->getMonitor()->setPalette(GREEN_PALETTE);
            }
            else if (a == "orange") {
                cpc->getMonitor()->setPalette(ORANGE_PALETE);
            }
            else if (a == "color") {
                cpc->getMonitor()->setPalette(COLOR_PALETTE);
            }
        }
        //else if (a == "-r") {
        //    a = argv[na++];
        //    printf("programa=%s\n", a.c_str());
        //    std::ifstream file(a, std::ios::binary);
        //    BYTE b;
        //    u16 pos = 0x4000;
        //    Memory* mem = cpc->getMemory();
        //    while (file.read(reinterpret_cast<char*>(&b), sizeof(b)))  mem->writeByte(pos++, b);
        //    file.close();
        //    printf("Tam=%d\n", pos-0x4000);
        //    cpc->getCpu()->set_PC_SP(0x4000, 0xFFFE);
        //}
        //else if (a == "-p") {
        //    a = argv[na++];
        //    printf("programa=%s\n", a.c_str());
        //    cargarPrograma(cpc->getMemory(), cpc->getCpu(), a);
        //}
    }
}

void loadWindowConfig(int* x, int *y) {
	try {
		Properties pp;
		pp.load( std::string {WINDOW_POSITION_FILE} );
		pp.getNumber(WINDOW_X, x);
		pp.getNumber(WINDOW_Y, y);
	}
	catch (...) {
		std::cout << "Window config not loaded\n";
	}
}

void saveWindowConfig(SDL_Window* window) {
	try {
		int windowX, windowY;
		SDL_GetWindowPosition(window, &windowX, &windowY);
		Properties pp;
		pp.setNumber(WINDOW_X, windowX);
		pp.setNumber(WINDOW_Y, windowY);
		pp.save( std::string {WINDOW_POSITION_FILE} );
	}
	catch (...) {
		std::cout << "Window config not saved\n";
	}
}

#endif


// return 1 si se ha cargado el fichero
bool loadLastConfig(CPC* cpc) {
    std::filesystem::path path = LAST_CONFIGURATION_FILE;
    //path = path / LAST_CONFIGURATION_FILE;
    //debug("lastConfig: %s\n", path.string().c_str());
	return cpc->loadConfig(path.string());
}


