
cmake_minimum_required(VERSION 3.31)
project(LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

include_directories(include)

install(DIRECTORY include/ DESTINATION /usr/include/TenSores)

file(GLOB EXAMPLES examples/*.cpp)

foreach(EXAMPLES ${EXAMPLES})
  get_filename_component(EXAMPLE_NAME ${EXAMPLES} NAME_WE)
  add_executable(${EXAMPLE_NAME} ${EXAMPLES})
  target_link_libraries(${EXAMPLE_NAME} pthread)
  list(APPEND EXEXECS ${EXAMPLE_NAME})
endforeach()

add_custom_target(examples DEPENDS ${EXEXECS})
