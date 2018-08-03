CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(gled VERSION 0.1.0)

FILE(GLOB SRC
    *.c
    *.h
    ) 

ADD_EXECUTABLE(${PROJECT_NAME} ${SRC})
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC
    ${CMAKE_INSTALL_PREFIX}/include
    )

FIND_PACKAGE(SDL2 CONFIG REQUIRED)
FIND_PACKAGE(GLEW REQUIRED)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PRIVATE 
    SDL2::SDL2 SDL2::SDL2main
    #GLEW::GLEW
    general
    OPENGL32
    debug 
    ${CMAKE_INSTALL_PREFIX}/debug/lib/glew32d.lib
    ${CMAKE_INSTALL_PREFIX}/debug/lib/SDL2_ttf.lib
    ${CMAKE_INSTALL_PREFIX}/debug/lib/SDL2_ttf.lib
    ${CMAKE_INSTALL_PREFIX}/debug/lib/assimp-vc140-mtd.lib
    optimized 
    ${CMAKE_INSTALL_PREFIX}/lib/glew32.lib
    ${CMAKE_INSTALL_PREFIX}/lib/SDL2_ttf.lib
    ${CMAKE_INSTALL_PREFIX}/lib/assimp-vc140-mt.lib
    )

