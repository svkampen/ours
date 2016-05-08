set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${netmine_SOURCE_DIR}/bin)
set(netmine_SRC_FILES main.cpp)

INCLUDE_DIRECTORIES(${netmine_SOURCE_DIR}/include)
LINK_DIRECTORIES(${netmine_SOURCE_DIR}/bin)

ADD_DEFINITIONS(-ggdb -g3 -Wall -Werror -std=gnu++14 -O3)
ADD_EXECUTABLE(netmine ${netmine_SRC_FILES})
TARGET_LINK_LIBRARIES(netmine ncursesw)
