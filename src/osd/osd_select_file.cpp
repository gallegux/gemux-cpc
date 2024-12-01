/*___________________________________________________________________________
|                                                                            |
|  GEMUX-CPC - Amstrad CPC emulator                                          |
|  OSD window file selection                                                 |
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
#include <vector>
#include <algorithm>
#include "osd.h"
#include "osd_select_file.h"
#include "osd_input.h"
#include "osd_window.h"
#include "util.h"
#include "../tipos.h"
#include "../monitor.h"  // para OSD_COLS y OSD_ROWS
#include "../util.h"
#include "../file_util.h"
#include "../directories.h"
#include "../cdt.h"
#include "../dsk.h"
#include "../standard_disks.h"
#include "../target.h"
#include "../log.h"

#ifdef TARGET_WINDOWS
#include "util_windows.h"
#endif



OSD_FileItem:: OSD_FileItem(const std::string& desc, bool dir) {
	description = desc;
	isDir = dir;
}

std::string OSD_FileItem:: toString() {
	return description;
}




OSD_FileSelection:: OSD_FileSelection(const std::string& _title, OSD* _osd, std::filesystem::path _dir, std::string_view _ext) :
	OSD_BigSelect(_title, _osd), dir(std::filesystem::canonical(_dir)), ext(_ext)  {
	setHeight(true, 25, true);
	setWidth(54);
}


std::filesystem::path OSD_FileSelection:: getCurrentDir() { return dir; }


std::filesystem::path OSD_FileSelection:: getSelectedFile() { 	
	OSD_FileItem* ret = reinterpret_cast<OSD_FileItem*>(itemSelected);
	itemSelected = nullptr;

	if (ret != nullptr) {
		debug_osd("OSD:: selected [%s]\n", ret->toString().c_str());
		std::filesystem::path abspath = dir / ret->toString();
		return abspath;
	}
	else return "";
}



void OSD_FileSelection:: printHelp() {
	std::string text;
	if (createOption)  text.append("F2 New | ");
	text.append("F3 Search | F8 Delete");

	#ifdef TARGET_WINDOWS
 	text.append(" | F9 Drives");
	#endif

	printStatus(text);
}


void OSD_FileSelection:: printWindow() {
	OSD_TitledWindow::printWindow();
	printPathDir();
	printHelp();
	getFiles();
	printItemsPage();
	clearRestPage();
	printCursorPosition();
}


void OSD_FileSelection:: k_return() {
	OSD_SelectionItem* wi = items.at(cursor);
	OSD_FileItem* fi = reinterpret_cast<OSD_FileItem*>( items.at(cursor) );
	if (fi->isDir) {
		dir = dir / fi->toString(); // dir.parent_path();
		dir = std::filesystem::canonical(dir);
		debug_osd("OSD:: -> %s\n", dir.string().c_str());
		cursor = 0;
		firstInWindow = 0;
		printPathDir();
		getFiles();
		printItemsPage();
		clearRestPage();
		printCursorPosition();
		osd->updateUI();
	}
	else {
		itemSelected = items.at(cursor);
		debug_osd("OSD:: return=%d\n", cursor);
		finish();
	}
}


void OSD_FileSelection:: k_F2() {  // create
	if (createOption) {
		// preguntar el nombre
		osd->setColors(WINDOW_STATUS_BG, WINDOW_STATUS_FG);
		osd->writeTextC(winCol+1, winRow+statusRow, "Name: ");
		OSD_Input input {osd, winCol+7, winRow+statusRow, INPUT_NEW_FILE_LEN, "", WINDOW_INPUT_BG, WINDOW_INPUT_FG};

		std::string filename;
		std::filesystem::path path;
		if (input.getAnswer(filename)) {
			debug_osd("OSD:: new_file [%s]\n", filename.c_str());
			bool crear = false;
			trim(filename);
			
			if (filename != "") {
				std::string filenameExt {filename};
				toLowerCase(filenameExt);
				// si no termina con la extension se la ponemos
				debug_osd("OSD:: %s %s\n", filenameExt.c_str(), ext.c_str());
				if (!endsWith(filenameExt, ext))  filename.append(ext);
				path = dir / filename;
				debug_osd("OSD:: new_file [%s]\n", path.string().c_str());

				if (file_exists(path.string())) {
					debug_osd("OSD:: existe!\n");
					// preguntar si reemplazar
					OSD_PlainWindow w {osd, WINDOW_ALERT_BG, WINDOW_ALERT_FG};
					w.appendTextLine("A file with that name already exists");
					w.appendTextLine("");
					w.appendTextLine("Overwrite?  (Y/N)");
					osd->push(w);
					w.show();
					crear = w.getExitOk();
					osd->pop();
				}
				else crear = true;
			}
			if (crear) {
				bool creado = createFile(path);
				debug_osd("OSD:: creado %d\n", creado);
			}
		}
		printHelp();
		osd->updateUI();
	}
}


void OSD_FileSelection:: k_F3() {  // search
	osd->setColors(WINDOW_STATUS_BG, WINDOW_STATUS_FG);
	osd->writeTextC(winCol+1, winRow+statusRow, "Search: ");
	
	debug_osd("OSD:: lastSearch [%s]\n", filter.c_str());
	OSD_Input input {osd, winCol+9, winRow+statusRow, SEARCH_LEN, filter, WINDOW_INPUT_BG, WINDOW_INPUT_FG};
	std::string newFilter;
	bool respOk = input.getAnswer(newFilter);
	debug_osd("OSD:: search [%s]\n", newFilter.c_str());

	if (respOk  &&  newFilter != filter) {
		filter = newFilter;
		getFiles();
		cursor = 0;
		firstInWindow = 0;
		printItemsPage();
		clearRestPage();
	}
	printHelp();
	printCursorPosition();
	osd->updateUI();
}


void OSD_FileSelection:: k_F8() {  // delete
	bool deleted = false;
	OSD_SelectionItem* wi = items.at(cursor);
	OSD_FileItem* fi = reinterpret_cast<OSD_FileItem*>( items.at(cursor) );

	if (!fi->isDir) {
		std::filesystem::path p = dir / fi->toString(); // dir.parent_path();
		p = std::filesystem::canonical(p);
		debug_osd("OSD:: borrar %s\n", p.string().c_str());

		try {
			if (std::filesystem::remove(p)) {
				items.erase(items.begin() + cursor);
				if (items.size() == cursor) cursor--;
				if (cursor > items.size()-itemsRows) {
					if (--firstInWindow < 0) firstInWindow = 0;
				}
				printItemsPage();
				clearRestPage();
				printCursorPosition();
				osd->updateUI();
				deleted = true;
				debug_osd("OSD:: borrado!\n");
			} 
			else {
				debug_osd("OSD:: El archivo no existe o no se pudo eliminar.\n");
			}
		} 
		catch (const std::filesystem::filesystem_error& e) {
			debug_osd("Error al eliminar el archivo: %s\n", e.what());
		}
	}

	if (!deleted) {
		OSD_AlertWindow w {osd};
		w.appendTextLine("The file cannot be deleted");
		osd->push(w);
		w.show();
		osd->pop();
	}
}



#ifdef TARGET_WINDOWS
void OSD_FileSelection:: k_F9() {
	std::string localDrives = getLocalDrives();
	debug_osd("OSD:: Local drives = [%s]\n", localDrives.c_str());
								
	clearItems();
	items.reserve(localDrives.size());

	OSD_FileItem* fileItem;
	std::string xDdrive = " :\\";
	for (u8 i = 0; i < localDrives.size(); i++) {
		xDdrive[0] = localDrives[i];
		fileItem = new OSD_FileItem( xDdrive, true);
		addItem(fileItem);
	}
	cursor = 0;
	firstInWindow = 0;

	printSubtitle("Local drives");
	printItemsPage();
	clearRestPage();
	printHelp();
	printCursorPosition();

	osd->updateUI();
}
#endif



void OSD_FileSelection:: writeItem(bool selected) {
	if (selected)
		osd->setColors(WINDOW_SELECTED_ITEM_BG, WINDOW_SELECTED_ITEM_FG);
	else
		osd->setColors(WINDOW_ITEM_BG, WINDOW_ITEM_FG);

	OSD_SelectionItem* wi = items.at(cursor);
	OSD_FileItem* fi = reinterpret_cast<OSD_FileItem*>(wi);
	
	u8 fila = cursor - firstInWindow; // cursorRow + item1row;
	clearItemRow(fila);
	fila += winRow + item1row;

	if (fi->isDir) {
		if (fi->toString().size() > winWidth-6) {	
			std::string r { fi->toString().substr(0, winWidth-6) };
			osd->writeFastTextC(winCol+1, fila, r);
		}
		else osd->writeFastTextC(winCol+1, fila, fi->toString());
		std::string dir {"<DIR>"};
		osd->writeFastTextC(winCol+winWidth-6, fila, dir);
	}
	else if (fi->toString().size() > winWidth-2) {
		std::string r { fi->toString().substr(0, winWidth-2) };
		osd->writeFastTextC(winCol+1, fila, r);
	}
	else osd->writeFastTextC(winCol+1, fila, fi->toString());
}


void OSD_FileSelection:: setCreateOption(bool c) { createOption = c; }


void OSD_FileSelection:: printPathDir() {
	std::string s {dir.string()};

	if (s.size() > winWidth-2) {
		s = s.substr(s.size(), winWidth-2);
	}
	osd->setColors(WINDOW_SUBTITLE_BG, WINDOW_SUBTITLE_FG);
	osd->drawFilledRectangleC(winCol, winRow + subtitleRow, winWidth, 1);
	osd->writeFastTextC(winCol+1, winRow + subtitleRow, s);
}


void OSD_FileSelection:: getFiles() { 
	OSD_FileItem* fileItem;
	std::vector<std::string> vaux;
	bool isfilter = filter != "";
	dir = std::filesystem::canonical(dir);
	debug_osd("OSD:: DIR=%s\n", dir.string().c_str());
	clearItems();

    try {
		if (dir.parent_path() != dir) {
			items.reserve(1 + vaux.size());
			std::string parent {".."};
			fileItem = new OSD_FileItem(parent, true);
			addItem(fileItem);
		}

		// obtener los directorios hijos
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_directory()) {
                vaux.push_back( entry.path().filename().string() );
				//debug_osd("OSD:: getFiles: %s\n", entry.path().filename().string().c_str());
            }
        }
        std::sort(vaux.begin(), vaux.end()); // los ordenamos
		debug_osd("OSD:: subdirectorios=%d\n", vaux.size());

		for (const std::string& e: vaux) {  // y los anadimos
			fileItem = new OSD_FileItem(e, true);
			addItem(fileItem);
			//debug_osd("OSD:: getFiles()2 %s\n", fileItem->toString().c_str());
		}

		items.reserve(1 + vaux.size());  // (reservamos memoria)
		
		vaux.clear(); // limpiamos el vector
		std::string extension;
		std::string FiLeNaMe;  // nombre de fichero con mayusculas y minusculas
		std::string filename;  // nombre de fichero con minusculas

		// obtenemos los ficheros
        for (const auto& entry : std::filesystem::directory_iterator(dir)) {
            if (entry.is_regular_file()) {
				extension = entry.path().extension().string();
				FiLeNaMe = entry.path().filename().string();
				toLowerCase(extension);

				if (ext == extension) {
					if (isfilter) {
						filename = FiLeNaMe;
						toLowerCase(filename);
						if (filename.find(filter) != std::string::npos)  vaux.push_back(FiLeNaMe);
					}
					else vaux.push_back(FiLeNaMe);
				}
            }
        }
        std::sort(vaux.begin(), vaux.end()); // los ordenamos

		items.reserve(items.size() + vaux.size()); // (reservamos memoria)

		for (const std::string& e: vaux) {	// y los anadimos
			fileItem = new OSD_FileItem(e, false);
			addItem(fileItem);
			//debug_osd("OSD:: getFiles() %s\n", fileItem->toString().c_str());
		}
		debug_osd("OSD:: ficheros=%d\n", vaux.size());
		debug_osd("OSD:: items=%d\n", items.size());
    }
    catch (const std::filesystem::filesystem_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}



//void OSD_FileSelection:: setFunctionCreate( bool (*_functionCreate)(const std::string& f) ) {
//	functionCreate = _functionCreate;
//}
bool compareFiles(OSD_SelectionItem* a, OSD_SelectionItem* b) {
	OSD_FileItem* fa = reinterpret_cast<OSD_FileItem*>(a);
	OSD_FileItem* fb = reinterpret_cast<OSD_FileItem*>(b);

	if (fa->isDir != fb->isDir) return true;
	else return fa->description < fb->description;
}


u16 OSD_FileSelection:: insertNewFile(const std::filesystem::path& newPath) {
	std::string name { newPath.filename().string() };
	debug_osd("OSD:: name [%s]\n", name.c_str());
	OSD_FileItem* fi = new OSD_FileItem (name, false);
	auto pos = std::lower_bound(items.begin(), items.end(), fi, compareFiles);
	int index = std::distance(items.begin(), pos);
	debug_osd("OSD:: index=%d\n", index);
	items.insert(pos, fi);
	return index;
}


void OSD_FileSelection:: goToCursor() {
	firstInWindow = cursor - (itemsRows>>1);
	if (firstInWindow + itemsRows > items.size()) firstInWindow = items.size() - itemsRows;
	if (firstInWindow < 0) firstInWindow = 0;
	printItemsPage();
	clearRestPage();
	printCursorPosition();
}



bool OSD_FileSelection:: createFile(const std::filesystem::path& path) {
	bool created;

	if (ext == TAPE_EXT) {
		created = CDT::create(path.string());
	}
	else if (ext == DISK_EXT) {
		OSD_SimpleSelect w {"Select a disk format", osd};

		for (u8 i = 0; i < NUMBER_DSK_FORMATS; i++) {
			w.addOption(i, DSK_FORMATS[i].description);
		}
		osd->push(w);
		w.show();
		
		if (w.getExitOk()) {
			u8 sel = w.getValueSelected();
			debug_osd("OSD:: sel=%d\n", sel);
			DSK_FORMAT d = DSK_FORMATS[sel];
			created = DSK::create(path.string(), d.tracks, d.sides, d.sectors, d.size, d.fillerByte, d.gap, d.firstSector);
			debug_osd("OSD:: creado\n");
		}
		osd->pop();
	}
	if (created) {
		cursor = insertNewFile(path);
		goToCursor();
	}
	printHelp();

	return false;
}



//========================================================================================



OSD_SaveFile:: OSD_SaveFile(const std::string& title, OSD* osd, std::filesystem::path dir, std::string_view ext) :
	OSD_FileSelection(title, osd, dir, ext) {}


bool OSD_SaveFile:: getExitOk() { return newFile != ""; }


std::filesystem::path OSD_SaveFile:: getNewFile() { return newFile; }


void OSD_SaveFile:: printHelp() {
	std::string text = "F2 New | F3 Search | F4 Autoname | F8 Delete";

	#ifdef TARGET_WINDOWS
 	text.append(" | F9 Drives");
	#endif

	//clearStatusRow();
	//osd->writeFastTextC(winCol+1, statusRow, text);
	printStatus(text);
}


void OSD_SaveFile:: k_F2() {  // new file
	// preguntar el nombre
	osd->setColors(WINDOW_STATUS_BG, WINDOW_STATUS_FG);
	osd->writeTextC(winCol+1, winRow+statusRow, "Name: ");
	OSD_Input input {osd, winCol+7, winRow+statusRow, INPUT_NEW_FILE_LEN, "", WINDOW_INPUT_BG, WINDOW_INPUT_FG};

	std::string filename;
	if (input.getAnswer(filename)) {
		debug_osd("OSD:: new_file [%s]\n", filename.c_str());
		bool crear = false;
		trim(filename);
		
		if (filename != "") {
			std::string filenameExt {filename};
			toLowerCase(filenameExt);
			// si no termina con la extension se la ponemos
			debug_osd("OSD:: %s %s\n", filenameExt.c_str(), ext.c_str());
			if (!endsWith(filenameExt, ext))  filename.append(ext);
			newFile = dir / filename;
			debug_osd("OSD:: new_file [%s]\n", newFile.string().c_str());

			if (file_exists(newFile.string())) {
				debug_osd("OSD:: existe!\n");
				// preguntar si reemplazar
				OSD_YesNoQuestionWindow w {osd}; //, WINDOW_ALERT_BG, WINDOW_ALERT_FG};
				w.appendTextLine("A file with that name already exists");
				w.appendTextLine("");
				w.appendTextLine("Overwrite?  (Y/N)");
				osd->push(w);
				w.show();
				crear = w.getExitOk();
				if (!crear) newFile = "";
				osd->pop();
			}
			else crear = true;

			if (crear) {
				finish();
				return;
			}
		}
	}
	printHelp();
	osd->updateUI();
}


void OSD_SaveFile:: k_F4() {
	newFile = dir / (strDate() + ext);
	finish();
}
