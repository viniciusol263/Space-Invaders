cmake_minimum_required(VERSION 3.22)
project(SpaceInvader VERSION 1.2.0)

set(CMAKE_CXX_STANDARD 20)

set(${PROJECT_NAME}_SRC 
    components/GameThread/GameThread.cpp
    components/LogicFunctions/LogicFunctions.cpp
    components/Object/Object.cpp
    components/Input/Input.cpp
    utils/GameUtils/GameUtils.cpp
    images/icon.rc)

add_executable(${PROJECT_NAME} main.cpp ${${PROJECT_NAME}_SRC})

target_include_directories(${PROJECT_NAME} PUBLIC 
                            ${CMAKE_CURRENT_SOURCE_DIR}/components
                            ${CMAKE_CURRENT_SOURCE_DIR}/utils
                            ${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML/include)


target_link_directories(${PROJECT_NAME} PUBLIC
                        ${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML/lib)

target_link_libraries(${PROJECT_NAME} sfml-main sfml-system sfml-window sfml-graphics sfml-audio)

file(GLOB DLLS "${CMAKE_CURRENT_SOURCE_DIR}/libs/SFML/bin/*.dll" 
                "${CMAKE_CURRENT_SOURCE_DIR}/libs/Compiler Shared Objects/*.dll")
file(COPY ${DLLS} DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")