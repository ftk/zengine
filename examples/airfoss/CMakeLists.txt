file(GLOB_RECURSE FILES #RELATIVE ${PROJECT_SOURCE_DIR}
        *.hpp
        main.cpp game.cpp ../../external/raudio.c
        )
add_definitions(-DNO_SCRIPTING -O1)


add_executable(airfoss ${FILES} ${ZENGINE_FILES})
target_link_libraries(airfoss ${ZENGINE_LIBRARIES})
ppp_generator(airfoss ${PROJECT_SOURCE_DIR}/build/inc.pl  ${FILES} ${ZENGINE_FILES})


