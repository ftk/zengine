file(GLOB_RECURSE FILES #RELATIVE ${PROJECT_SOURCE_DIR}
        *.hpp
        *.cpp
        )
add_definitions(-DNO_SCRIPTING)
add_executable(snake ${FILES} ${ZENGINE_FILES})
target_link_libraries(snake ${ZENGINE_LIBRARIES})
ppp_generator(snake ${PROJECT_SOURCE_DIR}/build/inc.pl ${FILES} ${ZENGINE_FILES})

set(GAME_FILES ${FILES} PARENT_SCOPE)
#message(STATUS ${GAME_FILES})