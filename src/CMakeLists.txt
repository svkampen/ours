set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${netmine_SOURCE_DIR}/bin)
set(SAVED_LIB_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")

set(CMAKE_EXE_LINKER_FLAGS "-fPIC")
set(netmine_SRC_FILES main.cpp Board.cpp BoardView.cpp Chunk.cpp ChunkGenerator.cpp ChunkView.cpp Server.cpp CursesGui.cpp Game.cpp
	ConnectionManager.cpp netmine.pb.cc NetworkGame.cpp Client.cpp Argparse.cpp Loader.cpp Saver.cpp Config.cpp FreeType.cpp
	CursesStream.cpp CursesStreamBuf.cpp Window.cpp Utils.cpp ImageSaver.cpp GuileInterpreter.cpp PairHash.cpp Backtrace.cpp)

FIND_PACKAGE(PkgConfig)
pkg_check_modules(PC_GUILE guile-2.0)
pkg_check_modules(PC_FT freetype2)

INCLUDE_DIRECTORIES(${netmine_SOURCE_DIR}/include ${PC_GUILE_INCLUDE_DIRS} ${PC_FT_INCLUDE_DIRS})
LINK_DIRECTORIES(${netmine_SOURCE_DIR}/bin)

ADD_CUSTOM_COMMAND(
	COMMAND cd ${netmine_SOURCE_DIR}/src && ./make-proto.sh
	OUTPUT ${netmine_SOURCE_DIR}/src/netmine.pb.cc ${netmine_SOURCE_DIR}/include/netmine.pb.h
)


FIND_PACKAGE(Boost COMPONENTS REQUIRED system thread log)

FIND_LIBRARY(PROTOBUF NAMES protobuf)
FIND_LIBRARY(GC NAMES gc)
FIND_LIBRARY(NCURSES NAMES ncursesw)
FIND_LIBRARY(THREADS NAMES pthread)
FIND_LIBRARY(DYN NAMES dl)
FIND_LIBRARY(UNISTRING NAMES unistring)
FIND_LIBRARY(ZLIB NAMES z)

set(CMAKE_FIND_LIBRARY_SUFFIXES ${SAVED_LIB_SUFFIXES})

ADD_DEFINITIONS(-DNCURSES_NOMACROS -std=gnu++17 -Ofast -ffast-math -march=native -fdiagnostics-color=always -ggdb -g3 -fno-omit-frame-pointer -fPIC)
ADD_EXECUTABLE(netmine ${netmine_SRC_FILES})
TARGET_LINK_LIBRARIES(netmine ${PROTOBUF} bfd unwind unwind-x86_64 ${Boost_LIBRARIES} gmp crypt ${PC_GUILE_LIBRARIES} ${PC_FT_LIBRARIES} ${GC} ${UNISTRING} ffi pthread ${NCURSES} dl readline z png)
