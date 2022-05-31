# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

#FILE(GLOB_RECURSE app_sources ${CMAKE_SOURCE_DIR}/src/*.*)

# idf_component_register(SRCS ${app_sources})
idf_component_register(
SRCS 
  ${CMAKE_SOURCE_DIR}/src/emu.c
  ${CMAKE_SOURCE_DIR}/src/emu_core.c
  ${CMAKE_SOURCE_DIR}/src/dsk.c  
  ${CMAKE_SOURCE_DIR}/src/fd.c
  ${CMAKE_SOURCE_DIR}/src/gblvar.c
  ${CMAKE_SOURCE_DIR}/src/vkey.c
  ${CMAKE_SOURCE_DIR}/src/vz.c
  ${CMAKE_SOURCE_DIR}/src/plat/esp32/plat_esp32.c
  ${CMAKE_SOURCE_DIR}/src/utils/prgdef.c
  ${CMAKE_SOURCE_DIR}/src/z80/z80emu.c
  ${CMAKE_SOURCE_DIR}/src/esp32/main.cpp

)

# spiffs_create_partition_image(storage ../rom FLASH_IN_PROJECT)
