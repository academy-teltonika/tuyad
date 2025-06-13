cmake_minimum_required(VERSION 3.10)
project(Tuyad)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD 11)

include_directories(
  ${PROJECT_SOURCE_DIR}/src
  /usr/local/include/libtuya
)

file(GLOB SOURCES "src/*.c")
 
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
  -Wall
  # -Werror
  -fsanitize=address
  -g
)

target_link_options(tuyad PRIVATE
  -fsanitize=address
)

