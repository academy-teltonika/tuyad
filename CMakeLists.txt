cmake_minimum_required(VERSION 3.10)
project(Tuyad)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

file(GLOB_RECURSE SOURCES "src/*.c")
file(GLOB_RECURSE HEADERS "src/*.h")

set(INCLUDE_DIRS "")
foreach(header ${HEADERS})
    get_filename_component(dir ${header} DIRECTORY)
    list(APPEND INCLUDE_DIRS ${dir})
endforeach()
list(REMOVE_DUPLICATES INCLUDE_DIRS)

include_directories(
  ${INCLUDE_DIRS}
  /usr/local/include/libtuya
)

add_executable(tuyad ${SOURCES})

target_link_libraries(
  tuyad
  PRIVATE
  link_core
  middleware_implementation
  platform_port
  utils_modules
  ubus
  ubox
  blobmsg_json
)

target_compile_options(tuyad PRIVATE
    $<$<CONFIG:DEBUG>: -Wall -fsanitize=address -g >
)

target_link_options(tuyad PRIVATE
    $<$<CONFIG:DEBUG>: -fsanitize=address >
)

install(TARGETS tuyad DESTINATION bin)
