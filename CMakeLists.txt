cmake_minimum_required(VERSION 3.0)

if (WIN32)
    add_definitions(-DWIN32)
endif ()

#if (CMAKE_COMPILER_IS_GNUCXX)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++1z")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O1")
#endif ()

project(zengine)
include_directories(. src external)


# boost
find_package(Boost 1.62  REQUIRED)
set(EXTERNAL_FILES ${EXTERNAL_FILES} external/boost.cpp)
include_directories(${Boost_INCLUDE_DIR})
#link_libraries(${Boost_LIBRARIES})

# chaiscript
add_definitions(-DNO_SCRIPTING)
#add_definitions(-DCHAISCRIPT_NO_THREADS -DCHAISCRIPT_NO_THREADS_WARNING -DCHAISCRIPT_NO_DYNLOAD)
#include_directories(ChaiScript/include)

# glfw
include_directories(external/glfw/include)
#
if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_GLFW=3 -s DISABLE_EXCEPTION_CATCHING=0 --preload-file ../resources")
else()
    set(EXTERNAL_FILES ${EXTERNAL_FILES} external/rglfw.c)
endif()

if (MATH_LIBRARY)
    link_libraries("${MATH_LIBRARY}")
endif()


if(WIN32)
    link_libraries(ws2_32 mswsock dsound)
    #set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
    #set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static -static-libgcc -static-libstdc++")
else()
    link_libraries(pthread)
endif ()


#add_definitions(-DBOOST_ASIO_SEPARATE_COMPILATION)
#add_definitions(-DBOOST_ASIO_DISABLE_THREADS)

#add_definitions(-DNETWORK_TRACE)
add_definitions(-DNETLOG_ENABLE)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")



#set(GAME_PATH game)
set(GAME_PATH examples/snake)
include_directories(${GAME_PATH})
add_subdirectory(${GAME_PATH})


macro(srcDir var dir)
    file(GLOB ${var} RELATIVE ${PROJECT_SOURCE_DIR}
            ${dir}/*.hpp ${dir}/*.cpp ${dir}/*.c)
    #foreach(f ${FILES})
    #    list(APPEND ${var} "src/${f}")
    #endforeach()

endmacro()

srcDir(COMPONENTS_FILES src/components)
srcDir(OPENGL_FILES src/opengl)
srcDir(UTIL_FILES src/util)
srcDir(ENGINE_FILES src)
#srcDir(EXTERNAL_FILES external)

set(ALL_FILES ${UTIL_FILES} ${OPENGL_FILES} ${COMPONENTS_FILES} ${ENGINE_FILES} ${GAME_FILES} ${EXTERNAL_FILES})
#message(${ALL_FILES})

#include_directories(${SDL2PP_INCLUDE_DIRS})
add_executable(zengine ${ALL_FILES})
target_link_libraries(zengine ${SDL2PP_LIBRARIES} ${Boost_LIBRARIES})


find_package(Perl)

if(PERL_FOUND)
    add_custom_command(OUTPUT build/.timestamp
            COMMAND ${PERL_EXECUTABLE} build/pp.pl build/inc.pl ${ALL_FILES}
            COMMAND ${CMAKE_COMMAND} -E touch build/.timestamp
            DEPENDS build/pp.pl build/inc.pl ${ALL_FILES}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Preprocess files"
            )
    add_custom_target(RunGenerator DEPENDS build/.timestamp)
    add_dependencies(zengine RunGenerator)
endif()

add_executable(nettest  network_test.cpp src/components/network.cpp external/boost.cpp)

#target_compile_definitions(nettest PUBLIC -DBOOST_ASIO_ENABLE_HANDLER_TRACKING)
target_compile_definitions(nettest PUBLIC -DLOG_HEADER_ONLY)
#target_link_libraries(nettest ${Boost_LIBRARIES})


set_source_files_properties(src/util/audio.cpp PROPERTIES COMPILE_FLAGS "-fpermissive -w")


#set_property(TARGET zengine PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
