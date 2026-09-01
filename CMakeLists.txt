cmake_minimum_required(VERSION 3.20)
project(WindTurbineFarm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/.deps" CACHE PATH "Local dependencies" FORCE)

include(FetchContent)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

find_package(glfw3 3.3 QUIET)
if(NOT TARGET glfw)
  FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git
                       GIT_TAG 3.4 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(glfw)
endif()
find_package(glm QUIET)
if(NOT TARGET glm::glm)
  FetchContent_Declare(glm GIT_REPOSITORY https://github.com/g-truc/glm.git
                       GIT_TAG 1.0.3 GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(glm)
endif()

find_package(OpenGL REQUIRED)

FetchContent_Declare(imgui GIT_REPOSITORY https://github.com/ocornut/imgui.git
                     GIT_TAG v1.92.9 GIT_SHALLOW TRUE)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR} ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui PUBLIC glfw OpenGL::GL)
if(APPLE)
  target_compile_definitions(imgui PRIVATE GL_SILENCE_DEPRECATION)
endif()

add_executable(wind_turbine_farm src/main.cpp)
target_link_libraries(wind_turbine_farm PRIVATE OpenGL::GL glfw glm::glm imgui)
if(APPLE)
  target_compile_definitions(wind_turbine_farm PRIVATE GL_SILENCE_DEPRECATION)
endif()
if(MSVC)
  target_compile_options(wind_turbine_farm PRIVATE /W4)
else()
  target_compile_options(wind_turbine_farm PRIVATE -Wall -Wextra -Wpedantic)
endif()
