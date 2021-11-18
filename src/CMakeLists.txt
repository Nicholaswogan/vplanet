
add_library(lsoda
liblsoda/src/cfode.c 
liblsoda/src/cfode_static.c 
liblsoda/src/common.c 
liblsoda/src/corfailure.c 
liblsoda/src/correction.c 
liblsoda/src/daxpy.c 
liblsoda/src/ddot.c 
liblsoda/src/dgefa.c 
liblsoda/src/dgesl.c 
liblsoda/src/dscal.c 
liblsoda/src/fnorm.c 
liblsoda/src/idamax.c 
liblsoda/src/intdy.c 
liblsoda/src/lsoda.c 
liblsoda/src/methodswitch.c 
liblsoda/src/orderswitch.c 
liblsoda/src/prja.c 
liblsoda/src/scaleh.c 
liblsoda/src/solsy.c 
liblsoda/src/stoda.c 
liblsoda/src/vmnorm.c 
liblsoda/src/strdup_printf.c
)
target_include_directories(lsoda PUBLIC liblsoda/src/)

add_executable(test_lsoda liblsoda/test/test.c)
target_link_libraries(test_lsoda lsoda)

add_library(_vplanet 
atmesc.c
galhabit.c
spinbody.c
binary.c
halt.c
stellar.c
body.c
magmoc.c
system.c
control.c
module.c
thermint.c
distorb.c
options.c
update.c
distrot.c
output.c
verify.c
eqtide.c
poise.c
vplanet.c
evolve.c
flare.c
radheat.c
)
target_link_libraries(_vplanet lsoda)
target_include_directories(_vplanet PUBLIC 
${CMAKE_CURRENT_SOURCE_DIR} 
${CMAKE_CURRENT_SOURCE_DIR}/liblsoda/src/)