set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${netmine_SOURCE_DIR}/bin)
set(netmine_SRC_FILES main.cpp Board.cpp Chunk.cpp ChunkGenerator.cpp Server.cpp Gui.cpp Game.cpp
	ConnectionManager.cpp netmine.pb.cc)

INCLUDE_DIRECTORIES(${netmine_SOURCE_DIR}/include ${netmine_SOURCE_DIR}/simmopp/include)
LINK_DIRECTORIES(${netmine_SOURCE_DIR}/bin)

ADD_CUSTOM_TARGET(
	generate_protobuf
	COMMAND cd ${netmine_SOURCE_DIR}/src && ./make-proto.sh
)

ADD_DEFINITIONS(-ggdb -Wall -std=gnu++1z -g3 -DBOOST_LOG_DYN_LINK -fcolor-diagnostics)
ADD_EXECUTABLE(netmine ${netmine_SRC_FILES})
ADD_DEPENDENCIES(netmine generate_protobuf)
TARGET_LINK_LIBRARIES(netmine ncursesw boost_system boost_thread boost_log pthread protobuf)
