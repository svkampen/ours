set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${netmine_SOURCE_DIR}/bin)
set(netmine_SRC_FILES main.cpp Board.cpp Chunk.cpp ChunkGenerator.cpp)

INCLUDE_DIRECTORIES(${netmine_SOURCE_DIR}/include ${netmine_SOURCE_DIR}/simmopp/include)
LINK_DIRECTORIES(${netmine_SOURCE_DIR}/bin)

ADD_DEFINITIONS(-ggdb -g3 -Wall -std=gnu++1z -O2)
ADD_EXECUTABLE(netmine ${netmine_SRC_FILES})
TARGET_LINK_LIBRARIES(netmine ncursesw)
