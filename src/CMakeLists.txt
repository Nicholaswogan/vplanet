
add_subdirectory(liblsoda EXCLUDE_FROM_ALL)

add_library(_vplanet atmesc.c		 galhabit.c	         spinbody.c
                     binary.c		 halt.c			         stellar.c
                     body.c		 	 magmoc.c		         system.c
                     control.c   module.c	           thermint.c
                     distorb.c	 options.c		       update.c
                     distrot.c	 output.c		         verify.c
                     eqtide.c		 poise.c			       vplanet.c
                     evolve.c		 flare.c             radheat.c)
target_link_libraries(_vplanet lsoda)
target_include_directories(_vplanet PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/liblsoda/src/)