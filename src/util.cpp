/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Helper functions                                                          |
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
#include <cctype>
#include <cstdint>
#include <fstream>
#include <string>
#include <stdio.h>
#include <sstream>
#include <ctime>
#include "tipos.h"
//#include "font.h"
// para obtener la fecha en formato string <iostream> <sstream> <ctime>






void toUpperCase(std::string& str) {
    for (char& c : str) {
        c = std::toupper(static_cast<unsigned char>(c));
    }
}

void toLowerCase(std::string& str) {
    for (char& c : str) {
        c = std::tolower(static_cast<unsigned char>(c));
    }
}

//------------------------------------------------------

#include <random>

// Crear un dispositivo aleatorio para obtener una semilla.
std::random_device rd;
// Usar Mersenne Twister como generador de números pseudoaleatorios.
std::mt19937 gen(rd());

u32 random(u32 max) {
    std::uniform_int_distribution<> distrib(0, max);
    return distrib(gen);
}

//------------------------------------------------------


#include <iostream>
#include <string>
#include <cstring>  // Para std::strlen
#include <vector> 
#include <filesystem>
#include <algorithm>


// Sobrecarga para comparar con std::string prefix
bool startsWith(const std::string& str, const std::string& prefix) {
    // Verificar que la longitud de la cadena es mayor o igual que la del prefijo
    if (str.length() < prefix.length()) {
        return false;
    }

    // Comparar el prefijo con el comienzo de la cadena
    return str.compare(0, prefix.length(), prefix) == 0;
}

// Sobrecarga para comparar con const char* prefix
bool startsWith(const std::string& str, const char* prefix) {
    size_t prefix_length = std::strlen(prefix);

    // Verificar que la longitud de la cadena es mayor o igual que la del prefijo
    if (str.length() < prefix_length) {
        return false;
    }

    // Comparar el prefijo con el comienzo de la cadena
    return str.compare(0, prefix_length, prefix) == 0;
}


bool toNumber(const std::string& str, int* valor) {
    try {
        *valor = std::stoi(str);
        return true;
    } 
    catch (const std::invalid_argument& e) {
        //std::cerr << "Argumento inválido: " << e.what() << std::endl;
        return false;
    } 
    catch (const std::out_of_range& e) {
        //std::cerr << "Fuera de rango: " << e.what() << std::endl;
        return false;
    }
}



std::string strDate() {
    // Obtener el tiempo actual
    std::time_t tiempo_actual = std::time(nullptr);
    // Convertir el tiempo actual a una estructura tm
    std::tm* tiempo_local = std::localtime(&tiempo_actual);
    // Usar un stringstream para formatear la salida
    std::ostringstream salida;
    salida << std::put_time(tiempo_local, "%Y-%m-%d %H-%M-%S");
    
    return salida.str();

//    std::time_t now = std::time(nullptr);
//    std::tm* tm_ptr = std::localtime(&now);
//
//    // Usamos std::ostringstream para formatear la fecha y hora
//    std::ostringstream oss;
//    oss << (tm_ptr->tm_year + 1900)    // Año
//        << (tm_ptr->tm_mon + 1)       // Mes (tm_mon es de 0 a 11)
//        << tm_ptr->tm_mday            // Día
//        << tm_ptr->tm_hour            // Hora
//        << tm_ptr->tm_min             // Minuto
//        << tm_ptr->tm_sec;            // Segundo
//
//    return oss.str();
}



uint32_t colorValue(TColor c) {
	return  c.r << 16  |  c.g << 8  |  c.b ;
}


void concat(std::string& dest, std::string_view s1, std::string s2) {
	dest.append(s1);
	dest.append(s2);
}


std::string concat(std::string_view s1, std::string s2) {
	std::string r;

	r.append(s1);
	r.append(s2);

	return r;
}


std::string concat(std::string_view s1, std::string s2, std::string s3) {
	std::string r;

	r.append(s1);
	r.append(s2);
	r.append(s3);

	return r;
}



void centerText(std::string& texto, u8 ancho) {
    if (texto.size() >= ancho) return;

    u8 espacios_total = ancho - texto.size();  // Espacio total a distribuir
    u8 espacios_izquierda = espacios_total / 2;  // Mitad del espacio a la izquierda
    u8 espacios_derecha = espacios_total - espacios_izquierda;  // Resto del espacio a la derecha

    texto.insert(0, espacios_izquierda, ' ');
    texto.append(espacios_derecha, ' ');
}


std::string toHex(u32 number, u8 width) {
	std::stringstream ss;
    ss << std::setw(width) << std::setfill('0') << std::hex << number;
	return ss.str();
}



std::string trim(const std::string& str) {
    // Encontrar el primer carácter no blanco
    auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
    // Encontrar el último carácter no blanco
    auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
    
    // Retornar la subcadena que contiene solo los caracteres no blancos
    return (start < end) ? std::string(start, end) : std::string();
}

void trim_string(std::string& str) {
    // Eliminar espacios en blanco al principio
    str.erase(str.begin(), std::find_if(str.begin(), str.end(),
        [](unsigned char ch) { return !std::isspace(ch); }));

    // Eliminar espacios en blanco al final
    str.erase(std::find_if(str.rbegin(), str.rend(),
        [](unsigned char ch) { return !std::isspace(ch); }).base(), str.end());
}

bool endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.length() > str.length()) {
        return false; // La subcadena es más larga que la cadena
    }
    return str.compare(str.length() - suffix.length(), suffix.length(), suffix) == 0;
}


