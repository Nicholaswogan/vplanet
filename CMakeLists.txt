cmake_minimum_required(VERSION 3.12)

project(vpl LANGUAGES C)

option(SKBUILD "Should be ON of being build by skbuild, 
and OFF of being build by regular cmake" OFF)

if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release" CACHE STRING "" FORCE)
endif()

file(STRINGS "${CMAKE_SOURCE_DIR}/VERSION" VPLANET_V)
add_compile_definitions(VPLANET_VERSION="${VPLANET_V}")

add_library(_vplanet src/atmesc.c		 src/galhabit.c	         src/spinbody.c
                     src/binary.c		 src/halt.c			         src/stellar.c
                     src/body.c		 	 src/magmoc.c		         src/system.c
                     src/control.c    src/module.c	           src/thermint.c
                     src/distorb.c	   src/options.c		       src/update.c
                     src/distrot.c	   src/output.c		         src/verify.c
                     src/eqtide.c		 src/poise.c			       src/vplanet.c
                     src/evolve.c		 src/flare.c             src/radheat.c)

# Executable
add_executable(vplanet src/main.c)
target_link_libraries(vplanet _vplanet)
target_include_directories(vplanet PUBLIC src/)

# Python Extension
if (NOT SKBUILD)
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
endif()

find_package(PythonExtensions REQUIRED)
add_library(vplanet_core MODULE src/python_interface.c)
target_compile_definitions(vplanet_core PRIVATE VPLANET_PYTHON_INTERFACE=1)
target_link_libraries(vplanet_core _vplanet)
python_extension_module(vplanet_core)

if (SKBUILD)
  install(TARGETS vplanet_core DESTINATION vplanet)
else()
  install(TARGETS vplanet_core DESTINATION ${CMAKE_SOURCE_DIR}/vplanet)
endif()