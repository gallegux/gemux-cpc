# en windows:
# C:\msys64\usr\bin\make

OS := $(shell uname)
.DEFAULT_GOAL := dev
GOAL := $(MAKECMDGOALS)

ifeq ($(GOAL),)
	GOAL = dev
endif


ifeq ($(OS), Linux)
	SRCDIR     := src
	PRJ_DIR    := /home/fran/gemux
	OBJDIR     := $(PRJ_DIR)/build/obj
	BINDIR     := $(PRJ_DIR)/build/
	RELEASEDIR := $(PRJ_DIR)/dist

	TARGET         := $(BINDIR)/gemux
	TARGET_RELEASE := $(RELEASEDIR)/gemux

	LIBS := -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer
endif

ifeq ($(findstring MSYS,$(OS)),MSYS)
	OS = Windows
	PRJ_DIR            := F:/gemux/gemuxcpc
	SDL2_INCLUDE       := $(PRJ_DIR)/SDL2/x86_64-w64-mingw32/include
	SDL2_LIB           := $(PRJ_DIR)/SDL2/x86_64-w64-mingw32/lib
	SDL2_IMAGE_INCLUDE := $(PRJ_DIR)/SDL2_image/x86_64-w64-mingw32/include
	SDL2_IMAGE_LIB     := $(PRJ_DIR)/SDL2_image/x86_64-w64-mingw32/lib
	SDL2_MIXER_INCLUDE := $(PRJ_DIR)/SDL2_mixer/x86_64-w64-mingw32/include
	SDL2_MIXER_LIB     := $(PRJ_DIR)/SDL2_mixer/x86_64-w64-mingw32/lib

	OBJDIR     := D:/gemux/build/obj
	SRCDIR     := src
	BINDIR     := D:/gemux/build
	RELEASEDIR := D:/gemux/dist

	TARGET         := $(BINDIR)/gemux.exe
    TARGET_RELEASE := $(RELEASEDIR)/gemux.exe

	LIBS := -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer
endif



OFILES = $(wildcard $(OBJDIR)/*.o) \
         $(wildcard $(OBJDIR)/miniz/*.o) \
         $(wildcard $(OBJDIR)/osd/*.o)
SOURCES = $(wildcard $(SRCDIR)/*.cpp) \
          $(wildcard $(SRCDIR)/miniz/*.cpp) \
		  $(wildcard $(SRCDIR)/osd/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

OPTIMIZACIONES_COMP = -Ofast -funroll-loops -finline-functions
OPTIMIZACIONES_LINK = -s


# directorios donde estan los .h
DIR_INCLUDES = -I$(SDL2_INCLUDE) -I$(SDL2_IMAGE_INCLUDE) -I$(SDL2_MIXER_INCLUDE)
# directorios lib
DIR_LIBS     = -L$(SDL2_LIB) -L$(SDL2_IMAGE_LIB) -L$(SDL2_MIXER_LIB)
# librerias sin el prefijo "lib" ni la extension
#LIBS         = -lmingw32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer


dev: crear_directorios set_target set_compilation_options $(OBJECTS)
ifeq ($(OS), Linux)
	@g++ $(OBJECTS) -o $(TARGET) $(LIBS)
else
	@g++ $(OBJECTS) -o $(TARGET)  $(DIR_INCLUDES) $(DIR_LIBS) $(LIBS)
endif

	@echo $(TARGET)



# version de distribucionfind $(OBJDIR) -type f -name "*.o" -delete
release: crear_directorios clean set_target set_compilation_options $(OBJECTS)
ifeq ($(OS), Linux)
	@g++ $(OBJECTS) $(OPTIMIZACIONES_LINK) -o $(TARGET_RELEASE) $(LIBS)
else
	# compilar icono
	windres var\iconos_barras_45.rc -o $(OBJDIR)\iconos.o
	# solo ventana
	@g++ $(OBJECTS) $(OBJDIR)/iconos.o $(OPTIMIZACIONES_LINK) -o $(TARGET_RELEASE) \
		$(DIR_INCLUDES) $(DIR_LIBS) $(LIBS) -mwindows
endif

	@echo $(TARGET_RELEASE)


# cambia el fichero compilation_options.h
set_target:
	cp "src/target.h-$(OS)" "src/target.h"


set_compilation_options:
	cp "src/compilation_options.h-$(GOAL)" "src/compilation_options.h"


# Regla para los archivos objeto
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
#	g++ -c $< -o $@  $(DIR_INCLUDES) $(DIR_LIBS) $(LIBS)
# En función del target, usar las opciones correspondientes
	@echo $@
	@if [ "$(OS)" = "Linux" ]; then \
	    if [ "$(GOAL)" = "dev" ]; then \
	        g++ -c -std=c++17 $< -o $@ $(LIBS); \
	    else \
	        g++ -c -std=c++17 $< -o $@ $(RELEASE_OPTIMIZACIONES) $(LIBS); \
	    fi; \
	else \
	    if [ "$(GOAL)" = "dev" ]; then \
	        g++ -c $< -o $@ $(DIR_INCLUDES) $(DIR_LIBS) $(LIBS); \
	    else \
	        g++ -c $< -o $@ $(DIR_INCLUDES) $(DIR_LIBS) $(RELEASE_OPTIMIZACIONES) $(LIBS); \
	    fi; \
	fi


clean:
ifeq ($(OS), Linux)
	find $(OBJDIR) -type f -name "*.o" -delete
else
	@PowerShell -Command "$(foreach file, $(OFILES) $(TARGET), Remove-Item -Path '$(file)' -ErrorAction SilentlyContinue;); exit 0;"
endif


crear_directorios:
	mkdir -p "$(OBJDIR)/osd"
	mkdir -p "$(OBJDIR)/miniz"
	mkdir -p "$(RELEASEDIR)"
