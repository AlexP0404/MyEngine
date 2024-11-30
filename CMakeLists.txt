cmake_minimum_required(VERSION 3.17)
project(testGameProj)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS on)
set(ENGINE_USE_VLK ON)
set(SHADER_DIR "${CMAKE_CURRENT_SOURCE_DIR}/engine/vlk/shaders")

add_subdirectory(engine)

set(GAME_DIR "${CMAKE_CURRENT_SOURCE_DIR}/simpleGame")
set(SG_SOURCES # Put cpp sources here
    "${GAME_DIR}/main.cpp")

add_executable(simpleGame ${SG_SOURCES})

target_link_libraries(simpleGame engine)

if(ENGINE_USE_VLK)
  add_custom_command(
    TARGET simpleGame
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E create_symlink ${SHADER_DIR}
            $<TARGET_FILE_DIR:simpleGame>/shaders
    COMMENT "Created symlink to shaders dir"
    VERBATIM)
endif()

execute_process(
  COMMAND
    ${CMAKE_COMMAND} -E create_symlink
    ${CMAKE_BINARY_DIR}/compile_commands.json
    ${CMAKE_SOURCE_DIR}/compile_commands.json)
