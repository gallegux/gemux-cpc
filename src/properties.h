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


#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include "tipos.h"


class Properties
{
private:
    std::unordered_map<std::string, std::string> properties;

    void trim(std::string& cadena);

public:
    Properties();

    bool load(const std::string& fichero);
    bool load(const std::string_view& fichero);

    bool save(const std::string& fichero);
    //bool save(const std::string_view fichero);

    bool getString(const std::string& propiedad, std::string& valor) const;
    bool getString(const std::string_view& propiedad, std::string& valor) const;

    bool getNumber(const std::string& propiedad, int* valor) const;
    bool getNumber(const std::string_view& propiedad, int* valor) const;

	void setString(const std::string& propiedad, std::string& valor);
	void setString(const std::string_view& propiedad, std::string& valor);

	void setNumber(const std::string& propiedad, int valor);
	void setNumber(const std::string_view& propiedad, int valor);

    void print();
};