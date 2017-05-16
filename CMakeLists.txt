cmake_minimum_required(VERSION 2.8.4)
project(HorseRace)

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -std=c++14 -Wall -pedantic")

#MY VARIABLES
set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(HEADER_DIR ${PROJECT_SOURCE_DIR}/headers)
set(FILES DBMS Log Properties Race Sockets Utils SHA)

STRING(REPLACE "/" "\\/" SRC_DIR ${SRC_DIR})
STRING(REPLACE "/" "\\/" HEADER_DIR ${HEADER_DIR})

execute_process(
        COMMAND echo ${FILES}
        COMMAND sed "s/[^ ][^ ]*/${SRC_DIR}\\/&.cpp/g"
        OUTPUT_VARIABLE SOURCE_FILES OUTPUT_STRIP_TRAILING_WHITESPACE
)

execute_process(
        COMMAND echo ${FILES}
        COMMAND sed "s/[^ ][^ ]*/${HEADER_DIR}\\/&.hpp/g"
        OUTPUT_VARIABLE HEADER_FILES OUTPUT_STRIP_TRAILING_WHITESPACE
)

separate_arguments(SOURCE_FILES)
separate_arguments(HEADER_FILES)

STRING(REPLACE "\\/" "/" SRC_DIR ${SRC_DIR})
STRING(REPLACE "\\/" "/" HEADER_DIR ${HEADER_DIR})

#link_directories(/usr/lib/x86_64-linux-gnu)

#COMPILE
add_executable(BOP2_proj ${SRC_DIR}/Main.cpp ${SOURCE_FILES} ${HEADER_FILES} src/Client/Clients.cpp src/Client/Privileges.cpp src/Client/ClientBases.cpp src/Client/ClientOthers.cpp src/Thread.cpp headers/Thread.hpp headers/GarbageCollector.hpp)
TARGET_LINK_LIBRARIES(BOP2_proj pq)
TARGET_LINK_LIBRARIES(BOP2_proj cryptopp)
#TARGET_LINK_LIBRARIES(BOP2_proj boost_regex)
