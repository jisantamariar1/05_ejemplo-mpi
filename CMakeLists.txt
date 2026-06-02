
cmake_minimum_required(VERSION 3.16)

project(ejemplo-mpi)


find_package(fmt CONFIG REQUIRED)

set(MPT_ROOT "C:/oneAPI/mpi/latest")
set(MPI_INCLUDE "${MPT_ROOT}/include")
set(MPI_LIB "${MPT_ROOT}/lib")

include_directories(
    ${MPI_INCLUDE}
    )

target_link_libraries(ejemplo-mpi PRIVATE
    fmt::fmt 
    ${MPI_LIB}/impi.lib

)    


    
add_executable(ejemplo-mpi
 main.cpp

)

add_executable(matrices-mult
 matrices_mult.cpp)

target_link_libraries(matrices-mult PRIVATE
    fmt::fmt 
    ${MPI_LIB}/impi.lib

)