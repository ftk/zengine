cmake_minimum_required(VERSION 3.0)


if (WIN32)
    add_definitions(-DWIN32)
endif ()

#if (CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-Wall -std=c++1z)
#endif ()

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ")


project(zengine)

include_directories(. src)

# sld2pp

set(SDL2PP_CXXSTD "c++1z")

set(SDL2PP_WITH_EXPERIMENTAL_OPTIONAL "Y")
set(SDL2PP_WITH_TTF "Y")
set(SDL2PP_WITH_IMAGE "Y")
set(SDL2PP_WITH_MIXER "Y")

add_subdirectory(libSDL2pp)

# boost
set(Boost_USE_STATIC_LIBS   ON)

find_package(Boost 1.62 COMPONENTS serialization system REQUIRED )

include_directories(${Boost_INCLUDE_DIR})
link_libraries(${Boost_LIBRARIES})

include_directories(ChaiScript/include)

if(WIN32)
    link_libraries(ws2_32 mswsock)
    #set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mwindows")
    #set (CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -static -static-libgcc -static-libstdc++")
else()
    link_libraries(pthread dl)
endif ()


add_definitions(-DBOOST_ASIO_SEPARATE_COMPILATION)
#add_definitions(-DBOOST_ASIO_DISABLE_THREADS)

#add_definitions(-DNETWORK_TRACE)
add_definitions(-DNETLOG_ENABLE)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG")




#set(GAME_PATH game)
set(GAME_PATH examples/pong)
include_directories(${GAME_PATH})
add_subdirectory(${GAME_PATH})


macro(srcDir var dir)
    file(GLOB ${var} RELATIVE ${PROJECT_SOURCE_DIR}
            ${dir}/*.hpp ${dir}/*.cpp)
    #foreach(f ${FILES})
    #    list(APPEND ${var} "src/${f}")
    #endforeach()

endmacro()

srcDir(COMPONENTS_FILES src/components)
srcDir(MODULES_FILES src/modules)
srcDir(OPENGL_FILES src/opengl)
srcDir(UTIL_FILES src/util)
srcDir(ENGINE_FILES src)

set(ALL_FILES ${UTIL_FILES} ${OPENGL_FILES} ${COMPONENTS_FILES} ${ENGINE_FILES} ${MODULES_FILES} ${GAME_FILES})
#message(${ALL_FILES})

include_directories(${SDL2PP_INCLUDE_DIRS})
add_executable(zengine ${ALL_FILES})
target_link_libraries(zengine ${SDL2PP_LIBRARIES})

find_package(Perl)

if(PERL_FOUND)
    add_custom_command(OUTPUT build/.timestamp
            COMMAND ${PERL_EXECUTABLE} build/pp.pl build/inc.pl ${ALL_FILES}
            #COMMAND ${CMAKE_COMMAND} -E echo ok
            COMMAND ${CMAKE_COMMAND} -E touch build/.timestamp
            DEPENDS build/pp.pl build/inc.pl ${ALL_FILES}
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            COMMENT "Preprocess files"
            )
    add_custom_target(RunGenerator DEPENDS build/.timestamp)
    add_dependencies(zengine RunGenerator)
endif()


add_executable(nettest  network_test.cpp src/components/network.cpp src/asio_impl.cpp)


#target_compile_definitions(nettest PUBLIC -DBOOST_ASIO_ENABLE_HANDLER_TRACKING)
target_compile_definitions(nettest PUBLIC -DNO_SDL -DLOG_HEADER_ONLY)


#add_executable(toolset game/toolset.cpp)
#target_link_libraries(toolset ${SDL2PP_LIBRARIES})

set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/build")


set(BIG_FILES components/script.cpp components/script_impl.cpp components/script_impl_t.cpp)
#set_source_files_properties(${BIG_FILES} PROPERTIES COMPILE_FLAGS -fvisibility=hidden)
set_source_files_properties(${BIG_FILES} PROPERTIES COMPILE_FLAGS -g0)
#set_source_files_properties(${BIG_FILES} PROPERTIES COMPILE_FLAGS -O1)
if(MINGW OR CYGWIN)
    #add_definitions(-Wa,-mbig-obj)
    add_definitions(-O1)
endif()

#include(cotire)
#cotire(zengine)

#set_property(TARGET zengine PROPERTY INTERPROCEDURAL_OPTIMIZATION True)
