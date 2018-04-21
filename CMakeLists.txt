cmake_minimum_required(VERSION 3.0)

project (tubdown)

add_definitions(-DAIMAKE_BUILDOS_linux)

add_executable(tubdown_c_demo
  demo.c
  src/tubdown.cpp
  src/renderer_uncursed.c

  libuncursed/src/libuncursed.c
  libuncursed/src/plugins.c
  libuncursed/src/plugins/tty.c
  libuncursed/src/plugins/tty.cxx
  )

target_link_libraries(tubdown_c_demo dl)


include_directories(tubdown PUBLIC libuncursed/include)
include_directories(tubdown PUBLIC include)

