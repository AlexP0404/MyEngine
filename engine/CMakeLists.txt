cmake_minimum_required(VERSION 3.16)
project(libengine)

set(CMAKE_CXX_STANDARD 20)

set(GCC_COVERAGE_COMPILE_FLAGS "")

if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
endif()

if(ENGINE_USE_VLK)
  find_package(Vulkan REQUIRED)
  find_package(glfw3 REQUIRED)
  find_package(glm REQUIRED)
  find_package(OpenAL REQUIRED)

  set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui")
  set(IMGUI_DEPS "${IMGUI_DIR}" "${IMGUI_DIR}/backends")

  file(
    GLOB_RECURSE
    ENGINE_SRC
    CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/engine.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/keyCodes.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vlk/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vlk/*.cpp"
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_demo.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_vulkan.cpp")

  add_library(engine STATIC ${ENGINE_SRC})

  target_link_libraries(engine PUBLIC Vulkan::Vulkan)
  target_link_libraries(engine PUBLIC glfw)
  target_link_libraries(engine PUBLIC glm::glm)
  target_link_libraries(engine PUBLIC OpenAL::OpenAL)

  target_include_directories(
    engine
    PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}/vlk"
           ${IMGUI_DEPS} Vulkan::Headers)

  target_compile_definitions(engine PUBLIC ENGINE_USE_VLK)
else()
  find_package(SDL2 REQUIRED CONFIG REQUIRED COMPONENTS SDL2)
  find_package(SDL2 REQUIRED CONFIG COMPONENTS SDL2main)
  find_package(SDL2_mixer REQUIRED)
  find_package(SDL2_ttf REQUIRED)
  find_package(SDL2_image REQUIRED)
  include_directories(${SDL2_INCLUDE_DIRS})

  file(
    GLOB_RECURSE
    ENGINE_SRC
    CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/engine.hpp"
    "${PROJECT_SOURCE_DIR}/keyCodes.hpp"
    "${PROJECT_SOURCE_DIR}/sdl/*.hpp"
    "${PROJECT_SOURCE_DIR}/sdl/*.cpp")

  add_library(engine STATIC ${ENGINE_SRC})

  if(TARGET SDL2::SDL2main)
    target_link_libraries(engine PUBLIC SDL2::SDL2main)
  endif()
  target_link_libraries(engine PUBLIC SDL2::SDL2)
  target_link_libraries(engine PUBLIC SDL2_ttf::SDL2_ttf) # ttf
  target_link_libraries(engine PUBLIC SDL2_mixer::SDL2_mixer)
  target_link_libraries(engine PUBLIC SDL2_image::SDL2_image)

  target_include_directories(engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}"
                                           "${CMAKE_CURRENT_SOURCE_DIR}/sdl")
endif()
