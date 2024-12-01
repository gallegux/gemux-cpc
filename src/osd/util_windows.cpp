/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Functions for windows version                                             |
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


#include "../target.h"

#ifdef TARGET_WINDOWS

#define WIN32_LEAN_AND_MEAN  // Excluye APIs menos utilizadas para evitar sobrecarga
#define NOGDI                // Excluye funciones de gráficos (GDI) que definen tipos como WORD

#include <windows.h>	// para obtener todas las unidades con 
#include <string>


std::string getLocalDrives() {
    char unidades[256]; // Buffer para almacenar las unidades de disco
    DWORD longitud = GetLogicalDriveStringsA(sizeof(unidades), unidades);

    if (longitud == 0) {
        //std::cerr << "Error al obtener las unidades de disco." << std::endl;
        return "";
    }

    // Recorrer el buffer para obtener todas las unidades de disco
	std::string localDrives = "";
    char* unidad = unidades;
    while (*unidad) {
		localDrives += *unidad;
        //std::cout << "Unidad: " << unidad << std::endl;
        unidad += strlen(unidad) + 1;  // Avanzar al siguiente string en el buffer
    }
	return localDrives;
}

#endif