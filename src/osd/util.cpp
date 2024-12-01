/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  Strings OSD utils                                                         |
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
#include "util.h"
#include "font.h"
#include "../tipos.h"


std::string string_compose(std::string& s1, std::string& s2) {
	std::string result;

	u8 len = s1.size();
	char c;

	for (u8 i = 0; i < len; i++) {
		c = s1[i];
		result += c;
		if (c == ESCAPE_CHAR) result += s2[i];
	}
	return result;
}

void string_decompose(const std::string& org, std::string& s1, std::string& s2) {
	char c;
	u8 len = org.size();
	s1.clear();
	s2.clear();

	for (u8 i = 0; i < len; i++) {
		c = org[i];
		if (c != ESCAPE_CHAR) {
			s1.push_back(c);
			s2.push_back(' ');
		}
		else {
			s1.push_back(c);
			s2.push_back(org[++i]);
		}
	}
}
