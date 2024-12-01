/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Properties file implementation                                            |
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


#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include "tipos.h"
#include "properties.h"
#include "log.h"


Properties:: Properties() {}


bool Properties:: load(const std::string& nombreArchivo) {
    std::ifstream archivo(nombreArchivo);

    if (!archivo.is_open()) {
        return false;
    }

    std::string linea, propiedad, valor;
    size_t separador;

    while (std::getline(archivo, linea)) {   // la linea se guarda en la variable linea
        if (linea.length() > 0  &&  linea[0] != '#') {
            separador = linea.find('=');
            if (separador != std::string::npos) {
                propiedad = linea.substr(0, separador);
                valor = linea.substr(separador + 1);
                trim(propiedad);
                trim(valor);

                properties[propiedad] = valor;
            }
        }
    }
    archivo.close();
    return true;
}



bool Properties:: save(const std::string& fichero) {
	std::ofstream f {fichero};

	for (const auto& par : properties) {
		f << par.first << "=" << par.second << std::endl;
	}

	f.close();

	return true;
}



bool Properties:: getNumber(const std::string& nombrePropiedad, int* valor) const {
    auto it = properties.find(nombrePropiedad);

    if (it != properties.end()) {
        *valor = std::stoi( it->second );
        return true;
    } 
    else {
        return false;
    }
}

bool Properties:: getNumber(const std::string_view& nombrePropiedad, int* valor) const {
	std::string x {nombrePropiedad};
	return getNumber(x, valor);
}


bool Properties:: getString(const std::string& nombrePropiedad, std::string& valor) const {
    auto it = properties.find(nombrePropiedad);

    if (it != properties.end()) {
        valor = it->second;
        return true;
    } 
    else {
        return false;
    }
}

bool Properties:: getString(const std::string_view& nombrePropiedad, std::string& valor) const {
	std::string x {nombrePropiedad};
	return getString(x, valor);
}


void Properties:: setString(const std::string& propiedad, std::string& valor) {
	properties[ std::string{propiedad} ] = valor;
}

void Properties:: setString(const std::string_view& propiedad, std::string& valor) {
	properties[ std::string{propiedad} ] = valor;
}


void Properties:: setNumber(const std::string& propiedad, int valor) {
	properties[ std::string{propiedad} ] = std::to_string(valor);
}

void Properties:: setNumber(const std::string_view& propiedad, int valor) {
	properties[ std::string{propiedad} ] = std::to_string(valor);
}


void Properties:: print() {
    for (const auto& par : properties) {
        debug("%s=%s\n", par.first.c_str(), par.second.c_str());
    }
}


void Properties:: trim(std::string& cadena) {
    cadena.erase(cadena.find_last_not_of(" \t\f\v\n\r")+1);
    cadena.erase(0,cadena.find_first_not_of(" \t\f\v\n\r"));
}
