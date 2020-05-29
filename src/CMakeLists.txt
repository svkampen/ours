set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${ours_SOURCE_DIR}/bin)
set(SAVED_LIB_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})

set(CMAKE_EXE_LINKER_FLAGS "-fPIC")
set(ours_SRC_FILES main.cpp Board.cpp BoardView.cpp Chunk.cpp ChunkGenerator.cpp ChunkView.cpp Server.cpp CursesGui.cpp Game.cpp
	ConnectionManager.cpp ours.pb.cc NetworkGame.cpp Client.cpp Argparse.cpp Loader.cpp Saver.cpp Config.cpp FreeType.cpp
	CursesStream.cpp CursesStreamBuf.cpp Window.cpp Utils.cpp ImageSaver.cpp PairHash.cpp)

FIND_PACKAGE(PkgConfig)
pkg_check_modules(PC_FT freetype2)

INCLUDE_DIRECTORIES(${ours_SOURCE_DIR}/include ${PC_FT_INCLUDE_DIRS})
LINK_DIRECTORIES(${ours_SOURCE_DIR}/bin)

ADD_CUSTOM_COMMAND(
	COMMAND cd ${ours_SOURCE_DIR}/src && ./make-proto.sh
	OUTPUT ${ours_SOURCE_DIR}/src/ours.pb.cc ${ours_SOURCE_DIR}/include/ours.pb.h
)


FIND_PACKAGE(Boost COMPONENTS REQUIRED system thread log)

FIND_LIBRARY(PROTOBUF NAMES protobuf)
FIND_LIBRARY(NCURSES NAMES ncursesw)
FIND_LIBRARY(THREADS NAMES pthread)
FIND_LIBRARY(DYN NAMES dl)
FIND_LIBRARY(ZLIB NAMES z)

set(CMAKE_FIND_LIBRARY_SUFFIXES ${SAVED_LIB_SUFFIXES})

ADD_DEFINITIONS(-DNCURSES_NOMACROS -std=gnu++17 -Ofast -march=native -fdiagnostics-color=always -ggdb -g3 -fno-omit-frame-pointer -fPIC -DBOOST_LOG_DYN_LINK -I/usr/include/ncursesw -I/usr/include/freetype2)
ADD_EXECUTABLE(ours ${ours_SRC_FILES})
TARGET_LINK_LIBRARIES(ours ${PROTOBUF} ${Boost_LIBRARIES} gmp crypt ${PC_FT_LIBRARIES} ffi pthread ${NCURSES} dl readline z png)
