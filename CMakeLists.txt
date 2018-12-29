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
set(EXTERNAL_FILES ${EXTERNAL_FILES} ${PROJECT_SOURCE_DIR}/external/boost.cpp)
include_directories(${Boost_INCLUDE_DIR})
#link_libraries(${Boost_LIBRARIES})

# chaiscript
#add_definitions(-DNO_SCRIPTING)
add_definitions(-DCHAISCRIPT_NO_THREADS -DCHAISCRIPT_NO_THREADS_WARNING -DCHAISCRIPT_NO_DYNLOAD)
#include_directories(ChaiScript/include)

#
if (EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    set(EMS_FLAGS "-s USE_GLFW=3 -s SAFE_HEAP=1 -s ALIASING_FUNCTION_POINTERS=0 -s TOTAL_MEMORY=67108864 -s STACK_OVERFLOW_CHECK=2 -s ASSERTIONS=2 -s DISABLE_EXCEPTION_CATCHING=0 -s DEMANGLE_SUPPORT=1")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${EMS_FLAGS} --preload-file config.xml --preload-file ../resources")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${EMS_FLAGS}")
    add_definitions(-DBOOST_ASIO_DISABLE_THREADS)
else()
    # glfw
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL " " FORCE)
    add_subdirectory(external/glfw)
    include_directories(external/glfw/include)
endif()
add_definitions(-DGLFW_INCLUDE_NONE)


add_definitions(-DGLES_EXTENSIONS)

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


macro(srcDir var dir)
    file(GLOB ${var} #RELATIVE ${PROJECT_SOURCE_DIR}
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

set(ZENGINE_FILES ${UTIL_FILES} ${OPENGL_FILES} ${COMPONENTS_FILES} ${ENGINE_FILES} ${GAME_FILES} ${EXTERNAL_FILES})
set(ZENGINE_LIBRARIES glfw ${GLFW_LIBRARIES})


find_package(Perl)

function(relative var prefix)
    set(listVar "")
    foreach(f ${ARGN})
        file(RELATIVE_PATH rel ${prefix} "${f}")
        list(APPEND listVar "${rel}")
    endforeach(f)
    set(${var} "${listVar}" PARENT_SCOPE)
endfunction()

function(ppp_generator target)
    if(PERL_FOUND)
        relative(rel_files ${PROJECT_SOURCE_DIR} ${ARGN})
        add_custom_target(RunGenerator_${target}
                COMMAND ${PERL_EXECUTABLE} ${PROJECT_SOURCE_DIR}/build/pp.pl ${rel_files}
                DEPENDS ${PROJECT_SOURCE_DIR}/build/pp.pl ${files}
                WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                COMMENT "Preprocess files"
                )
        add_dependencies(${target} RunGenerator_${target})
    endif()
endfunction()

#add_executable(nettest  network_test.cpp src/components/network.cpp external/boost.cpp)

#target_compile_definitions(nettest PUBLIC -DBOOST_ASIO_ENABLE_HANDLER_TRACKING)
#target_compile_definitions(nettest PUBLIC -DLOG_HEADER_ONLY)
#target_link_libraries(nettest ${Boost_LIBRARIES})


#add_subdirectory(examples/voxel)
add_subdirectory(examples/snake)
