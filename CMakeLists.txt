cmake_minimum_required(VERSION 3.0)

project(vpl LANGUAGES C)

option(SKBUILD "Should be ON of being build by skbuild, 
and OFF of being build by regular cmake" OFF)

# option(VPLANET_VERSION "description" "0.0.1")

# if (SKBUILD)
# if (NOT SKBUILD)
#   set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
# endif()

add_library(vplanet_core  src/atmesc.c		 src/galhabit.c	         src/spinbody.c
                          src/binary.c		 src/halt.c			         src/stellar.c
                          src/body.c		 	 src/magmoc.c		         src/system.c
                          src/control.c    src/module.c	           src/thermint.c
                          src/distorb.c	   src/options.c		       src/update.c
                          src/distrot.c	   src/output.c		         src/verify.c
                          src/eqtide.c		 src/poise.c			    
                          src/evolve.c		 src/python_interface.c  src/flare.c
                          src/radheat.c)
                                 
add_executable(vplanet src/vplanet.c)
target_link_libraries(vplanet vplanet_core)


# set(module_name "vplanet_core")
# set(generated_module_file ${module_name}${PYTHON_EXTENSION_MODULE_SUFFIX})
# find_package(PythonExtensions REQUIRED)

# target_compile_definitions(${generated_module_file} PRIVATE VPLANET_PYTHON_INTERFACE=1)
# target_link_libraries(${generated_module_file} PUBLIC ${PYTHON_LIBRARIES})
# target_include_directories(${generated_module_file} PUBLIC ${PYTHON_INCLUDE_DIRS})
# endif()


# if (SKBUILD)
#   install(TARGETS ${generated_module_file} DESTINATION vplanet)
# else()
#   install(TARGETS ${generated_module_file} DESTINATION ${CMAKE_SOURCE_DIR}/vplanet)
# endif()