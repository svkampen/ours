set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${netmine_SOURCE_DIR}/bin)
set(SAVED_LIB_SUFFIXES ${CMAKE_FIND_LIBRARY_SUFFIXES})
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc -static-libstdc++ -Wl,--dynamic-linker /lib64/ld-linux-x86-64.so.2")
set(netmine_SRC_FILES main.cpp Board.cpp BoardView.cpp Chunk.cpp ChunkGenerator.cpp ChunkView.cpp Server.cpp Gui.cpp Game.cpp
	ConnectionManager.cpp netmine.pb.cc NetworkGame.cpp Client.cpp Argparse.cpp Loader.cpp Saver.cpp
	CursesStream.cpp CursesStreamBuf.cpp Window.cpp Utils.cpp ImageSaver.cpp)

INCLUDE_DIRECTORIES(${netmine_SOURCE_DIR}/include ${netmine_SOURCE_DIR}/simmopp/include)
LINK_DIRECTORIES(${netmine_SOURCE_DIR}/bin)

ADD_CUSTOM_TARGET(
	generate_protobuf
	COMMAND cd ${netmine_SOURCE_DIR}/src && ./make-proto.sh
)

FIND_PACKAGE(Boost COMPONENTS REQUIRED system thread log)
FIND_LIBRARY(PROTOBUF NAMES protobuf)
FIND_LIBRARY(NCURSES NAMES ncursesw)
FIND_LIBRARY(THREADS NAMES pthread)
FIND_LIBRARY(DYN NAMES dl)
FIND_LIBRARY(PNG NAMES png)
FIND_LIBRARY(ZLIB NAMES z)

set(CMAKE_FIND_LIBRARY_SUFFIXES ${SAVED_LIB_SUFFIXES})

ADD_DEFINITIONS(-D__STRICT_ANSI__ -DNCURSES_NOMACROS -Wall -std=gnu++1z -fcolor-diagnostics -Ofast -ggdb -g3)
ADD_EXECUTABLE(netmine ${netmine_SRC_FILES})
ADD_DEPENDENCIES(netmine generate_protobuf)
TARGET_LINK_LIBRARIES(netmine ${Boost_LIBRARIES} pthread ${PROTOBUF} ${NCURSES} dl z ${PNG})
