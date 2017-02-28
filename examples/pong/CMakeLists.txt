file(GLOB_RECURSE FILES RELATIVE ${PROJECT_SOURCE_DIR}
        *.hpp
        *.cpp
        )

set(GAME_FILES ${FILES} PARENT_SCOPE)
#message(STATUS ${GAME_FILES})